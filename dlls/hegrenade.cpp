//High Explosive grenade

//Updated on: September 7th 2003
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "items.h"
#include "player.h"
#include "maxcarry.h"

#define	HEGRENADE_PRIMARY_VOLUME		450

enum hegrenade_e {
	HEGRENADE_IDLE = 0,
	HEGRENADE_PINPULL,
	HEGRENADE_THROW,	// toss
	HEGRENADE_DRAW
};

LINK_ENTITY_TO_CLASS( weapon_hegrenade, CHEGrenade );


void CHEGrenade::Spawn( )
{
	Precache( );
	m_iId = WEAPON_HEGRENADE;
	SET_MODEL(ENT(pev), "models/w_hegrenade.mdl");

	m_iDefaultAmmo = 1;

	FallInit();// get ready to fall down.
}


void CHEGrenade::Precache( void )
{
	PRECACHE_MODEL("models/w_hegrenade.mdl");
	PRECACHE_MODEL("models/v_hegrenade.mdl");
	PRECACHE_MODEL("models/p_hegrenade.mdl");

	PRECACHE_SOUND("weapons/hegrenade-1.wav");
	PRECACHE_SOUND("weapons/hegrenade-2.wav");
	PRECACHE_SOUND("weapons/he_bounce-1.wav");
	PRECACHE_SOUND("weapons/pinpull.wav");
	PRECACHE_SOUND("radio/ct_fireinhole.wav");
}

int CHEGrenade::AddToPlayer( CBasePlayer *pPlayer )
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

int CHEGrenade::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "HEGrenade";
	p->iMaxAmmo1 = HEGRENADE_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 1;
	p->iPosition = 9;
	p->iId = m_iId = WEAPON_HEGRENADE;
	p->iWeight = HEGRENADE_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}


BOOL CHEGrenade::Deploy( )
{
m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.775;
m_pPlayer->m_bResumeZoom = FALSE;
m_flReleaseThrow = -1;
m_flStartThrow = 0;
return DefaultDeploy( "models/v_hegrenade.mdl", "models/p_hegrenade.mdl", HEGRENADE_DRAW, "crowbar" );
}

BOOL CHEGrenade::CanHolster( void )
{
	// can only holster hand grenades when not primed!
	return ( m_flStartThrow == 0 );
}

void CHEGrenade::Holster( )
{
	//m_flStartThrow = 0;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 10)
		{
			m_pPlayer->m_iNumHEGrenades = 10;
			m_pPlayer->m_iHE = 10;
		}
	}
	else
	{
		// no more grenades!
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_HEGRENADE);
		SetThink( DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;

		m_pPlayer->m_iNumHEGrenades = 0;
		m_pPlayer->m_iHE = 0;
	}

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
}

void CHEGrenade::PrimaryAttack()
{
	if (!m_flStartThrow && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = 0;

		SendWeaponAnim( HEGRENADE_PINPULL );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		// no more grenades!
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_HEGRENADE);
		SetThink( DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;

		m_pPlayer->m_iNumHEGrenades = 0;
		m_pPlayer->m_iHE = 0;
		Holster();
	}
}

void CHEGrenade::Grenade(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	CBasePlayer *pPlayer = (CBasePlayer *)pActivator;

	if (!(pPlayer->pev->weapons &= ~(1<<WEAPON_HEGRENADE)))
	{
		// no more grenades!
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_HEGRENADE);
		SetThink( DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;

		m_pPlayer->m_iNumHEGrenades = 0;
		m_pPlayer->m_iHE = 0;
		Holster();
	}
}

void CHEGrenade::WeaponIdle( void )
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		// no more grenades!
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_HEGRENADE);
		SetThink( DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;

		m_pPlayer->m_iNumHEGrenades = 0;
		m_pPlayer->m_iHE = 0;
		Holster();
	}
	
	if (m_flReleaseThrow == 0)
		m_flReleaseThrow = gpGlobals->time;

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_flStartThrow)
	{
		Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if (angThrow.x < 0)
			angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
		else
			angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);

		float flVel = (90 - angThrow.x) * 6;
		if (flVel > 750)
			flVel = 750;

		UTIL_MakeVectors( angThrow );

		Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;

		Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

		// alway explode 2 seconds after the pin was pulled
		float time = m_flStartThrow - gpGlobals->time + 3.0;
		if (time < 0)
			time = 0;

		// ignore the time variable.. always explode 2.5 seconds after the grenade has been thrown
		CGrenade::ShootTimed2( m_pPlayer->pev, vecSrc, vecThrow, 1.75 ); 

		SendWeaponAnim( HEGRENADE_THROW );

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "radio/ct_fireinhole.wav", 1, ATTN_NORM);
		//ClientPrint(m_pPlayer->pev, HUD_PRINTTALK, UTIL_VarArgs( "FIRE IN THE HOLE!\n" ) );
		
		m_flStartThrow = 0;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.65;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		m_pPlayer->m_iNumHEGrenades--;
		m_pPlayer->m_iHE--;
		
		if (m_pPlayer->m_iNumHEGrenades <= 0)
		{
			m_pPlayer->m_iNumHEGrenades = 0;
			m_pPlayer->m_iHE = 0;
		}
		
		if ( !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
		{
			// just threw last grenade
			// set attack times in the future, and weapon idle in the future so we can see the whole throw
			// animation, weapon idle will automatically retire the weapon for us.
			m_flTimeWeaponIdle = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;// ensure that the animation can finish playing
		}
		return;
	}
	else if (m_flReleaseThrow > 0)
	{
		// we've finished the throw, restart.
		m_flStartThrow = 0;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			SendWeaponAnim( HEGRENADE_DRAW );
		}
		else
		{
			RetireWeapon();
			return;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;
		m_flReleaseThrow = -1;
		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		int iAnim;
		float flRand = RANDOM_FLOAT(0, 1);
		if (flRand <= 0.75)
		{
			iAnim = HEGRENADE_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT ( 10, 15 );// how long till we do this again.
		}
		else 
		{
			iAnim = HEGRENADE_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 75.0 / 30.0;
		}

		SendWeaponAnim( iAnim );
	}
}




