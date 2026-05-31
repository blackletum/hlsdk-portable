//Flashbang grenade
//Updated on: September 7th 2003
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "maxcarry.h"


#define	FLASHBANG_PRIMARY_VOLUME		450

enum flashbang_e {
	FLASHBANG_IDLE = 0,
	FLASHBANG_PINPULL,
	FLASHBANG_THROW,	// toss
	FLASHBANG_DRAW
};


LINK_ENTITY_TO_CLASS( weapon_flashbang, CFlashBang );

void CFlashBang::Spawn( )
{
	Precache( );
	m_iId = WEAPON_FLASHBANG;
	SET_MODEL(ENT(pev), "models/w_flashbang.mdl");

	m_iDefaultAmmo = 1;

	FallInit();// get ready to fall down.
}


void CFlashBang::Precache( void )
{
	PRECACHE_MODEL("models/w_flashbang.mdl");
	PRECACHE_MODEL("models/v_flashbang.mdl");
	PRECACHE_MODEL("models/p_flashbang.mdl");

	PRECACHE_SOUND("weapons/flashbang-1.wav");
	PRECACHE_SOUND("weapons/flashbang-2.wav");
	PRECACHE_SOUND("weapons/pinpull.wav");
	PRECACHE_SOUND("radio/ct_fireinhole.wav");
}

int CFlashBang::AddToPlayer( CBasePlayer *pPlayer )
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

int CFlashBang::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Flashbang";
	p->iMaxAmmo1 = FLASHBANG_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 1;
	p->iPosition = 8;
	p->iId = m_iId = WEAPON_FLASHBANG;
	p->iWeight = FLASHBANG_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}


BOOL CFlashBang::Deploy( )
{
m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.775;
m_pPlayer->m_bResumeZoom = FALSE;
m_flStartThrow = 0;
m_flReleaseThrow = -1;
return DefaultDeploy( "models/v_flashbang.mdl", "models/p_flashbang.mdl", FLASHBANG_DRAW, "crowbar" );
}

BOOL CFlashBang::CanHolster( void )
{
	// can only holster hand grenades when not primed!
	return ( m_flStartThrow == 0 );
}

void CFlashBang::Holster( )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 5)
		{
			m_pPlayer->m_iNumFlashbangs = 5;
			m_pPlayer->m_iFB = 5;
		}
	}
	else
	{
		// no more grenades!
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_FLASHBANG);
		SetThink( DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;

		m_pPlayer->m_iNumFlashbangs = 0;
		m_pPlayer->m_iFB = 0;
	}

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
}

void CFlashBang::PrimaryAttack()
{
	if (!m_flStartThrow && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = 0;

		SendWeaponAnim( FLASHBANG_PINPULL );
		m_flTimeWeaponIdle = gpGlobals->time + 0.5;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		// no more grenades!
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_FLASHBANG);
		SetThink( DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;

		m_pPlayer->m_iNumFlashbangs = 0;
		m_pPlayer->m_iFB = 0;
		Holster();
	}
}


void CFlashBang::WeaponIdle( void )
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		// no more grenades!
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_FLASHBANG);
		SetThink( DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;

		m_pPlayer->m_iNumFlashbangs = 0;
		m_pPlayer->m_iFB = 0;
		Holster();
	}

	if (m_flReleaseThrow == 0)
		m_flReleaseThrow = gpGlobals->time;

	if (m_flTimeWeaponIdle > gpGlobals->time)
		return;

	if (m_flStartThrow)
	{
		//m_pPlayer->Radio("FIREINHOLE");

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

		// alway explode 3 seconds after the pin was pulled
		float time = m_flStartThrow - gpGlobals->time + 3.0;
		if (time < 0)
			time = 0;

		CGrenade::ShootTimed( m_pPlayer->pev, vecSrc, vecThrow, 2 );

		SendWeaponAnim( FLASHBANG_THROW );

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "radio/ct_fireinhole.wav", 1, ATTN_NORM);
		
		m_flStartThrow = 0;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.65;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		m_pPlayer->m_iNumFlashbangs--;
		m_pPlayer->m_iFB--;

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
			SendWeaponAnim( FLASHBANG_DRAW );
		}
		else
		{
			RetireWeapon();
			return;
		}

		m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT ( 10, 15 );
		m_flReleaseThrow = -1;
		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		SendWeaponAnim( FLASHBANG_IDLE );
	}
}




