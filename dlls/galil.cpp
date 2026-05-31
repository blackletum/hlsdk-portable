//Updated on: February 25th 2004

//Galil Assault Rifle
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

enum Galil_e {
	GALIL_IDLE1 = 0,
	GALIL_RELOAD,	
	GALIL_DRAW,
	GALIL_SHOOT1,
	GALIL_SHOOT2,
	GALIL_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_galil, CGalil );

void CGalil::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_galil");
	Precache( );
	m_iId = WEAPON_GALIL;
	SET_MODEL(ENT(pev), "models/w_galil.mdl");

	m_iDefaultAmmo = GALIL_MAX_CLIP;

	FallInit();// get ready to fall down.
}

void CGalil::Precache( void )
{
	PRECACHE_MODEL("models/v_galil.mdl");
	PRECACHE_MODEL("models/w_galil.mdl");
	PRECACHE_MODEL("models/p_galil.mdl");

	PRECACHE_SOUND("weapons/galil-1.wav");
	PRECACHE_SOUND("weapons/galil-2.wav");
	PRECACHE_SOUND("weapons/galil_boltpull.wav");
	PRECACHE_SOUND("weapons/galil_clipin.wav");
	PRECACHE_SOUND("weapons/galil_clipout.wav");

	PRECACHE_SOUND ("weapons/dryfire_rifle.wav");

	m_iShell = PRECACHE_MODEL ("models/556mmshell.mdl");// brass shell

	m_usGalil = PRECACHE_EVENT( 1, "events/galil.sc" );
}

int CGalil::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = _556MM_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GALIL_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 5;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_GALIL;
	p->iWeight = GALIL_WEIGHT;

	return 1;
}

int CGalil::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CGalil::Deploy( )
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.775;
	iShellOn = 1;
	m_pPlayer->m_bResumeZoom = FALSE;
	return DefaultDeploy( "models/v_galil.mdl", "models/p_galil.mdl", GALIL_DRAW, "mp5" );
}

void CGalil::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		GALILFire( (0.1) * (m_pPlayer->m_flAccuracy), 0.07742);
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		GALILFire( (0.5) * (m_pPlayer->m_flAccuracy), 0.07742);
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		GALILFire( (0.0175) * (m_pPlayer->m_flAccuracy), 0.07742);
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		GALILFire( (0.1) * (m_pPlayer->m_flAccuracy), 0.07742);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		GALILFire( (0.1) * (m_pPlayer->m_flAccuracy), 0.07742);

	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		GALILFire( (0.5) * (m_pPlayer->m_flAccuracy), 0.07742);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		GALILFire( (0.5) * (m_pPlayer->m_flAccuracy), 0.07742);
	
	else
		GALILFire( (0.02) * (m_pPlayer->m_flAccuracy), 0.07742);
}

void CGalil::GALILFire( float flSpread , float flCycleTime)
{
	m_bDelayFire = TRUE;

	m_pPlayer->m_iShotsFired++;

	m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 250) + 0.4;
	if (m_pPlayer->m_flAccuracy > 0.85)
		m_pPlayer->m_flAccuracy = 0.85;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = gpGlobals->v_forward;
	Vector vecDir;

	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, 3, BULLET_PLAYER_AUG, 0, 0.97, m_pPlayer->pev, m_pPlayer->random_seed, FALSE );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usGalil, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	KickBack (1.05, 0.375, 0.15, 0.05, 6.25, 1.5, 3);
}

void CGalil::Reload( void )
{
	int iResult;

	iResult = DefaultReload( GALIL_MAX_CLIP, GALIL_RELOAD, 2.475);

	if (iResult)
	{
		if( (m_pPlayer->m_afButtonPressed & IN_ATTACK) || ( (!(m_pPlayer->m_afButtonPressed & IN_ATTACK)) ) )
		{
			m_pPlayer->m_iShotsFired = 0;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.275;
		
		m_pPlayer->m_i556 = m_pPlayer->m_i556 - 2; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i556 < 0) 
		{
			m_pPlayer->m_i556 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}

void CGalil::WeaponIdle( void )
{
	ResetEmptySound( );

	//m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
	// only idle if the slid isn't back
	SendWeaponAnim( GALIL_IDLE1 );
}

