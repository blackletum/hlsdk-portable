//Updated on September 19th 2004
//C4 satchel charge
#if !defined( OEM_BUILD ) && !defined( HLDEMO_BUILD )

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"
#include "maxcarry.h"

enum c4_e {
	C4_IDLE = 0,
	C4_DRAW,
	C4_DROP
};

enum c4_radio_e {
	C4_RADIO_IDLE1 = 0,
	C4_RADIO_FIDGET1,
	C4_RADIO_DRAW,
	C4_RADIO_FIRE,
	C4_RADIO_HOLSTER
};



class CC4Charge : public CGrenade
{
	void Spawn( void );
	void Precache( void );
	void BounceSound( void );

	void EXPORT C4Slide( CBaseEntity *pOther );
	void EXPORT C4Think( void );

public:
	void Deactivate( void );
};
LINK_ENTITY_TO_CLASS( monster_c4, CC4Charge );

//=========================================================
// Deactivate - do whatever it is we do to an orphaned 
// satchel when we don't want it in the world anymore.
//=========================================================
void CC4Charge::Deactivate( void )
{
	pev->solid = SOLID_NOT;
	UTIL_Remove( this );
}


void CC4Charge :: Spawn( void )
{
	Precache( );
	// motor
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/w_c4.mdl");
	//UTIL_SetSize(pev, Vector( -16, -16, -4), Vector(16, 16, 32));	// Old box -- size of headcrab monsters/players get blocked by this
	UTIL_SetSize(pev, Vector( -4, -4, -4), Vector(4, 4, 4));	// Uses point-sized, and can be stepped over
	UTIL_SetOrigin( pev, pev->origin );

	SetTouch( &CC4Charge::C4Slide );
	SetUse( &CC4Charge::DetonateUse );
	SetThink( &CC4Charge::C4Think );
	pev->nextthink = gpGlobals->time + 0.1;

	pev->gravity = 0.5;
	pev->friction = 0.8;

//	pev->dmg = gSkillData.plrDmgSatchel;
	pev->dmg = 200;
	// ResetSequenceInfo( );
}


void CC4Charge::C4Slide( CBaseEntity *pOther )
{
	entvars_t	*pevOther = pOther->pev;

	// don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;

	// pev->avelocity = Vector (300, 300, 300);
	pev->gravity = 1;// normal gravity now

	// HACKHACK - On ground isn't always set, so look for ground underneath
	TraceResult tr;
	UTIL_TraceLine( pev->origin, pev->origin - Vector(0,0,10), ignore_monsters, edict(), &tr );

	if ( tr.flFraction < 1.0 )
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.95;
		pev->avelocity = pev->avelocity * 0.9;
		// play sliding sound, volume based on velocity
	}
	if ( !(pev->flags & FL_ONGROUND) && pev->velocity.Length2D() > 10 )
	{
		BounceSound();
	}
	StudioFrameAdvance( );
}


void CC4Charge :: C4Think( void )
{
	StudioFrameAdvance( );
	pev->nextthink = gpGlobals->time + 0.1;

	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	if (pev->waterlevel == 3)
	{
		pev->movetype = MOVETYPE_FLY;
		pev->velocity = pev->velocity * 0.8;
		pev->avelocity = pev->avelocity * 0.9;
		pev->velocity.z += 8;
	}
	else if (pev->waterlevel == 0)
	{
		pev->movetype = MOVETYPE_BOUNCE;
	}
	else
	{
		pev->velocity.z -= 8;
	}	
}

void CC4Charge :: Precache( void )
{
	PRECACHE_MODEL("models/grenade.mdl");
	PRECACHE_SOUND("weapons/g_bounce1.wav");
	PRECACHE_SOUND("weapons/g_bounce2.wav");
	PRECACHE_SOUND("weapons/g_bounce3.wav");
}

void CC4Charge :: BounceSound( void )
{
	switch ( RANDOM_LONG( 0, 2 ) )
	{
	case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce1.wav", 1, ATTN_NORM);	break;
	case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce2.wav", 1, ATTN_NORM);	break;
	case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce3.wav", 1, ATTN_NORM);	break;
	}
}


LINK_ENTITY_TO_CLASS( weapon_c4, CC4 );


//=========================================================
// CALLED THROUGH the newly-touched weapon's instance. The existing player weapon is pOriginal
//=========================================================
int CC4::AddDuplicate( CBasePlayerItem *pOriginal )
{
	CC4 *pC4;

#ifdef CLIENT_DLL
	if ( bIsMultiplayer() )
#else
	if ( g_pGameRules->IsMultiplayer() )
#endif
	{
		pC4 = (CC4 *)pOriginal;

		if ( pC4->m_chargeReady != 0 )
		{
			// player has some C4s deployed. Refuse to add more.
			return FALSE;
		}
	}

	return CBasePlayerWeapon::AddDuplicate ( pOriginal );
}

//=========================================================
//=========================================================
int CC4::AddToPlayer( CBasePlayer *pPlayer )
{
	int bResult = CBasePlayerItem::AddToPlayer( pPlayer );

	pPlayer->pev->weapons |= (1<<m_iId);
	m_chargeReady = 0;// this C4 charge weapon now forgets that any C4s are deployed by it.

	if ( bResult )
	{
		return AddWeapon( );
	}
	return FALSE;
}

void CC4::Spawn( )
{
	Precache( );
	m_iId = WEAPON_C4;
	SET_MODEL(ENT(pev), "models/w_c4.mdl");

	m_iDefaultAmmo = C4_MAX_CLIP;
		
	FallInit();// get ready to fall down.
}


void CC4::Precache( void )
{
	PRECACHE_MODEL("models/v_c4.mdl");
	PRECACHE_MODEL("models/v_c4_radio.mdl");
	PRECACHE_MODEL("models/w_c4.mdl");
	PRECACHE_MODEL("models/p_c4.mdl");
	PRECACHE_MODEL("models/p_c4_radio.mdl");
	PRECACHE_SOUND("weapons/c4_plant.wav");

	UTIL_PrecacheOther( "monster_c4" );
}


int CC4::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "C4 Charge";
	p->iMaxAmmo1 = C4_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 1;
	p->iPosition = 10;
	p->iFlags = ITEM_FLAG_SELECTONEMPTY | ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
	p->iId = m_iId = WEAPON_C4;
	p->iWeight = C4_WEIGHT;

	return 1;
}

//=========================================================
//=========================================================
BOOL CC4::IsUseable( void )
{
	if ( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] > 0 ) 
	{
		// player is carrying some C4s
		return TRUE;
	}

	if ( m_chargeReady != 0 )
	{
		// player isn't carrying any C4s, but has some out
		return TRUE;
	}

	return FALSE;
}

BOOL CC4::CanDeploy( void )
{
	if ( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] > 0 ) 
	{
		// player is carrying some C4s
		return TRUE;
	}

	if ( m_chargeReady != 0 )
	{
		// player isn't carrying any C4s, but has some out
		return TRUE;
	}

	return FALSE;
}

BOOL CC4::Deploy( )
{
	m_pPlayer->m_bResumeZoom = FALSE;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	if ( m_chargeReady )
		return DefaultDeploy( "models/v_c4_radio.mdl", "models/p_c4_radio.mdl", C4_RADIO_DRAW, "hive" );
	else
		return DefaultDeploy( "models/v_c4.mdl", "models/p_c4.mdl", C4_DRAW, "trip" );

	
	return TRUE;
}


void CC4::Holster( int skiplocal /* = 0 */ )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	
	if ( m_chargeReady )
	{
		SendWeaponAnim( C4_RADIO_HOLSTER );
	}
	else
	{
		SendWeaponAnim( C4_DROP );
	}
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 2)
		{
			m_pPlayer->m_iNumC4 = 2;
			m_pPlayer->m_iC4 = 2;
		}
	}
	else if ( !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] && !m_chargeReady )
	{
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_C4);
		SetThink( &CC4::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;

		m_pPlayer->m_iNumC4 = 0;
		m_pPlayer->m_iC4 = 0;
	}
}



void CC4::PrimaryAttack()
{
	switch (m_chargeReady)
	{
	case 0:
		{
		Throw( );
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/c4_plant.wav", 1, ATTN_NORM);
		}
		break;
	case 1:
		{
		SendWeaponAnim( C4_RADIO_FIRE );

		edict_t *pPlayer = m_pPlayer->edict( );

		CBaseEntity *pC4 = NULL;

		while ((pC4 = UTIL_FindEntityInSphere( pC4, m_pPlayer->pev->origin, 4096 )) != NULL)
		{
			if (FClassnameIs( pC4->pev, "monster_c4"))
			{
				if (pC4->pev->owner == pPlayer)
				{
					pC4->Use( m_pPlayer, m_pPlayer, USE_ON, 0 );
					m_chargeReady = 2;
				}
			}
		}

		m_chargeReady = 2;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
		break;
		}

	case 2:
		// we're reloading, don't allow fire
		{
		}
		break;
	}
}


void CC4::SecondaryAttack( void )
{
	if ( m_chargeReady == 0 )
	{
		Slide( );
	}
}


void CC4::Throw( void )
{
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
	{
		Vector vecSrc = m_pPlayer->pev->origin;

//		Vector vecThrow = gpGlobals->v_forward * 274 + m_pPlayer->pev->velocity;

#ifndef CLIENT_DLL
		CBaseEntity *pC4 = Create( "monster_c4", vecSrc, Vector( 0, 0, 0), m_pPlayer->edict() );
//		pC4->pev->velocity = vecThrow;
	//	pC4->pev->avelocity.y = 400;

		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_c4_radio.mdl");
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_c4_radio.mdl");
#else
		LoadVModel ( "models/v_c4_radio.mdl", m_pPlayer );
#endif

		SendWeaponAnim( C4_RADIO_DRAW );

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		m_chargeReady = 1;
		
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		m_pPlayer->m_iNumC4--;
		m_pPlayer->m_iC4--;
		
		if (m_pPlayer->m_iNumC4 <= 0)
		{
			m_pPlayer->m_iNumC4 = 0;
			m_pPlayer->m_iC4 = 0;
		}

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
	}
}

void CC4::Slide( void )
{
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
	{
		Vector vecSrc = m_pPlayer->pev->origin;

		Vector vecThrow = gpGlobals->v_forward * 274 + m_pPlayer->pev->velocity;

#ifndef CLIENT_DLL
		CBaseEntity *pC4 = Create( "monster_c4", vecSrc, Vector( 0, 0, 0), m_pPlayer->edict() );
		pC4->pev->velocity = vecThrow;
		pC4->pev->avelocity.y = 400;

		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_c4_radio.mdl");
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_c4_radio.mdl");
#else
		LoadVModel ( "models/v_c4_radio.mdl", m_pPlayer );
#endif

		SendWeaponAnim( C4_RADIO_DRAW );

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		m_chargeReady = 1;
		
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		m_pPlayer->m_iNumC4--;
		m_pPlayer->m_iC4--;
		
		if (m_pPlayer->m_iNumC4 <= 0)
		{
			m_pPlayer->m_iNumC4 = 0;
			m_pPlayer->m_iC4 = 0;
		}

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
	}
}

void CC4::WeaponIdle( void )
{
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	switch( m_chargeReady )
	{
	case 0:
		SendWeaponAnim( C4_IDLE );
		// use tripmine animations
		strcpy( m_pPlayer->m_szAnimExtention, "trip" );
		break;
	case 1:
		SendWeaponAnim( C4_RADIO_FIDGET1 );
		// use hivehand animations
		strcpy( m_pPlayer->m_szAnimExtention, "hive" );
		break;
	case 2:
		if ( !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
		{
			m_chargeReady = 0;
			RetireWeapon();
			return;
		}

#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_c4.mdl");
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_c4.mdl");
#else
		LoadVModel ( "models/v_c4.mdl", m_pPlayer );
#endif

		SendWeaponAnim( C4_DRAW );

		// use tripmine animations
		strcpy( m_pPlayer->m_szAnimExtention, "trip" );

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_chargeReady = 0;
		break;
	}
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );// how long till we do this again.
}

//=========================================================
// DeactivateC4 - removes all C4s owned by
// the provided player. Should only be used upon death.
//
// Made this global on purpose.
//=========================================================
void DeactivateC4( CBasePlayer *pOwner )
{
	edict_t *pFind; 

	pFind = FIND_ENTITY_BY_CLASSNAME( NULL, "monster_c4" );

	while ( !FNullEnt( pFind ) )
	{
		CBaseEntity *pEnt = CBaseEntity::Instance( pFind );
		CC4Charge *pC4 = (CC4Charge *)pEnt;

		if ( pC4 )
		{
			if ( pC4->pev->owner == pOwner->edict() )
			{
				pC4->Deactivate();
			}
		}

		pFind = FIND_ENTITY_BY_CLASSNAME( pFind, "monster_c4" );
	}
}

#endif