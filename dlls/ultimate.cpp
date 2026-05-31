//Updated on: July 16th 2008

// The ultimate weapon
#include "extdll.h" 
#include "decals.h" 
#include "util.h" 
#include "cbase.h" 
#include "monsters.h" 
#include "weapons.h" 
#include "nodes.h" 
#include "player.h" 
#include "soundent.h" 
#include "shake.h" 
#include "gamerules.h"
#include "maxcarry.h"

enum ultimate_e {
	ULTI_JUSIDLE = 0,
	ULTI_JUSSHOOT1,	
	ULTI_JUSSHOOT2,
	ULTI_JUSSHOOT3,
	ULTI_JUSRELOAD,
	ULTI_JUSDRAW,
	ULTI_JUSTICE,
	ULTI_JUDIDLE,
	ULTI_JUDSHOOT1,	
	ULTI_JUDSHOOT2,
	ULTI_JUDSHOOT3,
	ULTI_JUDRELOAD,
	ULTI_JUDDRAW,
	ULTI_JUDGMENT,
};

LINK_ENTITY_TO_CLASS( weapon_ultimate, CULTIMATE );

#ifndef CLIENT_DLL

LINK_ENTITY_TO_CLASS( justice_rocket, CJusticeRocket );

//=========================================================
//=========================================================
CJusticeRocket *CJusticeRocket::CreateJusticeRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CULTIMATE *pLauncher )
{
	CJusticeRocket *pRocket = GetClassPtr( (CJusticeRocket *)NULL );

	UTIL_SetOrigin( pRocket->pev, vecOrigin );
	pRocket->pev->angles = vecAngles;
	pRocket->Spawn();
	pRocket->SetTouch( &CJusticeRocket::RocketTouch );
	pRocket->m_pLauncher = pLauncher;// remember what RPG fired me. 
	pRocket->m_pLauncher->m_cActiveRockets++;// register this missile as active for the launcher
	pRocket->pev->owner = pOwner->edict();

	return pRocket;
}

//=========================================================
//=========================================================
void CJusticeRocket :: Spawn( void )
{
	Precache( );
	// motor
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/rpgrocket.mdl");
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin( pev, pev->origin );

	pev->classname = MAKE_STRING("justice_rocket");

	SetThink( &CJusticeRocket::IgniteThink );
	SetTouch( &CJusticeRocket::ExplodeTouchRPG );

	pev->angles.x -= 0;
	UTIL_MakeVectors( pev->angles );
	pev->angles.x = -(pev->angles.x + 0);

	pev->velocity = gpGlobals->v_forward * 450;
	pev->gravity = 0.5;

	pev->nextthink = gpGlobals->time + 0.0;

	pev->dmg = JUSTICE_HIT;
}

//=========================================================
//=========================================================
void CJusticeRocket :: RocketTouch ( CBaseEntity *pOther )
{
	if ( m_pLauncher )
	{
		// my launcher is still around, tell it I'm dead.
		m_pLauncher->m_cActiveRockets--;
	}

	STOP_SOUND( edict(), CHAN_VOICE, "weapons/rocket1.wav" );
	ExplodeTouchRPG( pOther );
}

//=========================================================
//=========================================================
void CJusticeRocket :: Precache( void )
{
	PRECACHE_MODEL("models/rpgrocket.mdl");
	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	PRECACHE_SOUND ("weapons/rocket1.wav");
}


void CJusticeRocket :: IgniteThink( void  )
{
	// pev->movetype = MOVETYPE_TOSS;

	pev->movetype = MOVETYPE_FLY;
	pev->effects |= EF_LIGHT;

	// make rocket sound
	EMIT_SOUND( ENT(pev), CHAN_VOICE, "weapons/rocket1.wav", 1, 0.5 );

	// rocket trail
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );

		WRITE_BYTE( TE_BEAMFOLLOW );
		WRITE_SHORT(entindex());	// entity
		WRITE_SHORT(m_iTrail );	// model
		WRITE_BYTE( 40 ); // life
		WRITE_BYTE( 5 );  // width
		WRITE_BYTE( 224 );   // r, g, b
		WRITE_BYTE( 224 );   // r, g, b
		WRITE_BYTE( 255 );   // r, g, b
		WRITE_BYTE( 255 );	// brightness

	MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	m_flIgniteTime = gpGlobals->time;

	// set to follow laser spot
	SetThink( &CJusticeRocket::FollowThink );
	pev->nextthink = gpGlobals->time + 0.1;
}


void CJusticeRocket :: FollowThink( void  )
{
	CBaseEntity *pOther = NULL;
	Vector vecTarget;
	Vector vecDir;
//	float flDist, flMax, flDot;
	TraceResult tr;

	UTIL_MakeAimVectors( pev->angles );

	vecTarget = gpGlobals->v_forward;
//	flMax = 4096;

	// Examine all entities within a reasonable radius
	/*while ((pOther = UTIL_FindEntityInSphere( pOther, pev->origin, 350 )) != NULL)
	{
		if ( pOther->pev->takedamage != DAMAGE_NO && pOther->pev->deadflag != DEAD_DEAD)
		{	//finds entitys that take damage and are not dead.
			UTIL_TraceLine ( pev->origin, pOther->pev->origin, dont_ignore_monsters, ENT(pev), &tr );
			// ALERT( at_console, "%f\n", tr.flFraction );
			if (tr.flFraction >= 0.90)
			{
				vecDir = pOther->pev->origin - pev->origin;
				flDist = vecDir.Length( );
				vecDir = vecDir.Normalize( );
				flDot = DotProduct( gpGlobals->v_forward, vecDir );
				if ((flDot > 0) && (flDist * (1 - flDot) < flMax))
				{
					flMax = flDist * (1 - flDot);
					vecTarget = vecDir;
				}
			}
		}
	}*/


	pev->angles = UTIL_VecToAngles( vecTarget );

	// this acceleration and turning math is totally wrong, but it seems to respond well so don''t change it.
	float flSpeed = pev->velocity.Length();
	if (gpGlobals->time - m_flIgniteTime < 1.0)
	{
		pev->velocity = pev->velocity * 0.2 + vecTarget * (flSpeed * 0.8 + 400);
		if (pev->waterlevel == 3)
		{
			// go slow underwater
			if (pev->velocity.Length() > 300)
			{
				pev->velocity = pev->velocity.Normalize() * 300;
			}
				UTIL_BubbleTrail( pev->origin - pev->velocity * 0.1, pev->origin, 4 );
		} 
		else 
		{
			if (pev->velocity.Length() > 2000)
			{
				pev->velocity = pev->velocity.Normalize() * 2000;
			}
		}
	}
	else
	{
		if (pev->effects & EF_LIGHT)
		{
			pev->effects = 0;
			STOP_SOUND( ENT(pev), CHAN_VOICE, "weapons/rocket1.wav" );
		}
		pev->velocity = pev->velocity * 0.2 + vecTarget * flSpeed * 0.798;
		if (pev->waterlevel == 0 && pev->velocity.Length() < 1500)
		{
			Detonate( );
		}
	}
	// ALERT( at_console, "%.0f\n", flSpeed );

	pev->nextthink = gpGlobals->time + 0.1;
}

#endif

int CULTIMATE::SecondaryAmmoIndex( void )
{
	return m_iSecondaryAmmoType;
}

void CULTIMATE::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_ultimate");
	Precache( );
	m_iId = WEAPON_ULTIMATE;
	SET_MODEL(ENT(pev), "models/w_ultimate.mdl");

    m_iDefaultAmmo = P90_MAX_CLIP;

	FallInit();// get ready to fall down.
}

void CULTIMATE::Precache( void )
{
	//Left Handed Models
	PRECACHE_MODEL("models/v_ultimate.mdl");
	PRECACHE_MODEL("models/w_ultimate.mdl");
	PRECACHE_MODEL("models/p_ultimate.mdl");

	PRECACHE_SOUND("weapons/ultimate-1.wav");
	PRECACHE_SOUND("weapons/ultimate-2.wav");

	PRECACHE_SOUND("weapons/ultimate_deploy.wav");

	PRECACHE_SOUND("weapons/ultimate_clipin.wav");
	PRECACHE_SOUND("weapons/ultimate_clipout.wav");

	PRECACHE_SOUND("weapons/ultimate_justice.wav");
	PRECACHE_SOUND("weapons/ultimate_judgment.wav");

	UTIL_PrecacheOther( "justice_rocket" );

	m_usULTIMATE = PRECACHE_EVENT( 1, "events/ultimate.sc" );
	m_usULTIMATE2 = PRECACHE_EVENT( 1, "events/ultimate2.sc" );
}

int CULTIMATE::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Judgment";
	p->iMaxAmmo1 = ULTIMATE_JUDGMENT;
	p->pszAmmo2 = "Justice";
	p->iMaxAmmo2 = ULTIMATE_JUSTICE;
	p->iMaxClip = P90_MAX_CLIP;
	p->iSlot = 4;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_ULTIMATE;
	p->iWeight = ULTIMATE_WEIGHT;

	return 1;
}

int CULTIMATE::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}

	return FALSE;
}

BOOL CULTIMATE::Deploy( )
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.675;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.675;
	
	m_pPlayer->m_bResumeZoom = FALSE;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.0;
		
	return DefaultDeploy( "models/v_ultimate.mdl", "models/p_ultimate.mdl", ULTI_JUDDRAW, "mp5" );
}

void CULTIMATE::SecondaryAttack( void )
{
	ULTIMATEJustice();
}

void CULTIMATE::PrimaryAttack( void )
{
	ULTIMATEJudgment(0.07059);
}

void CULTIMATE::ULTIMATEJudgment(float flCycleTime)
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	int flags;

#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	
	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = gpGlobals->v_forward;
	Vector vecDir;
	
	//vecDir = m_pPlayer->FireBulletsPlayer2( 10, vecSrc, vecAiming, Vector( VECTOR_CONE_1DEGREES), 2048, 3, BULLET_PLAYER_ULTIMATE, 0, 0.97, m_pPlayer->pev, m_pPlayer->random_seed, FALSE );
	vecDir = m_pPlayer->FireBulletsPlayer( 10, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 2048, BULLET_PLAYER_XM1014, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usULTIMATE, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );
		
	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.975;

	m_pPlayer->pev->punchangle.x -= 1;
}

void CULTIMATE::ULTIMATEJustice()
{
	if ( m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0 )
	{
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
	
		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		Vector vecSrc = m_pPlayer->GetGunPosition( ) + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -13;//-8;
		
		CJusticeRocket *pRocket = CJusticeRocket::CreateJusticeRocket( vecSrc, m_pPlayer->pev->v_angle, m_pPlayer, this );

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );// RpgRocket::Create stomps on globals, so remake.
		pRocket->pev->velocity = pRocket->pev->velocity + gpGlobals->v_forward * DotProduct( m_pPlayer->pev->velocity, gpGlobals->v_forward );
#endif

		// firing RPG no longer turns on the designator. ALT fire is a toggle switch for the LTD.
		// Ken signed up for this as a global change (sjb)

		int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

		PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usULTIMATE2 );

		int JusticeShot;

		JusticeShot = m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
				
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.975;

		if (JusticeShot)
		{	
			m_pPlayer->m_iJus--; //when shooting, the value goes down by one
			if ( m_pPlayer->m_iJus < 0) 
			{
				m_pPlayer->m_iJus = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
			}
		}
	}
	else if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{	
		PlayEmptySound( );
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
	}
}

void CULTIMATE::Reload( void )
{
	int iResult;
	
	iResult = DefaultReload( P90_MAX_CLIP, ULTI_JUDRELOAD, 2.975);
	
	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.75;
		m_pPlayer->m_iJud--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_iJud < 0) 
		{
			m_pPlayer->m_iJud = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}		
	}
}


void CULTIMATE::WeaponIdle( void )
{	
	ResetEmptySound( );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() ) 
		return;

	SendWeaponAnim( ULTI_JUDIDLE );
	
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.0;
}

class CJudClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_chainammo.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_chainammo.mdl");
		PRECACHE_SOUND("weapons/ultimate_judgment.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( 50, "Judgment", ULTIMATE_JUDGMENT) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_judclip, CJudClip );

class CJusRock : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_rpgammo.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_rpgammo.mdl");
		PRECACHE_SOUND("weapons/ultimate_justice.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( 2, "Justice", ULTIMATE_JUSTICE) != -1);
		
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_jusrock, CJusRock );