//Updated on: January 3rd 2004

//Steyr Aug Assault Rifle
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

enum aug_e {
	AUG_IDLE1 = 0,
	AUG_RELOAD,	
	AUG_DRAW,
	AUG_SHOOT1,
	AUG_SHOOT2,
	AUG_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_aug, CAUG );

void CAUG::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_aug");
	Precache( );
	m_iId = WEAPON_AUG;
	SET_MODEL(ENT(pev), "models/w_aug.mdl");

	m_iDefaultAmmo = AK47_MAX_CLIP;

	FallInit();// get ready to fall down.
}

void CAUG::Precache( void )
{
PRECACHE_MODEL("models/v_aug.mdl");
	PRECACHE_MODEL("models/w_aug.mdl");
	PRECACHE_MODEL("models/p_aug.mdl");

	PRECACHE_SOUND("weapons/aug-1.wav");
	PRECACHE_SOUND("weapons/aug_boltpull.wav");
	PRECACHE_SOUND("weapons/aug_boltslap.wav");
	PRECACHE_SOUND("weapons/aug_clipin.wav");
	PRECACHE_SOUND("weapons/aug_clipout.wav");
	PRECACHE_SOUND("weapons/aug_forearm.wav");

	PRECACHE_SOUND ("weapons/dryfire_rifle.wav");

	m_iShell = PRECACHE_MODEL ("models/556mmshell.mdl");// brass shell

	m_usAug = PRECACHE_EVENT( 1, "events/aug.sc" );
}

int CAUG::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = _556MM_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AK47_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 3;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_AUG;
	p->iWeight = AUG_WEIGHT;

	return 1;
}

int CAUG::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CAUG::Deploy( )
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.775;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.775;
	iShellOn = 1;
	m_pPlayer->m_bResumeZoom = FALSE;
	return DefaultDeploy( "models/v_aug.mdl", "models/p_aug.mdl", AUG_DRAW, "p90" );
}

void CAUG::SecondaryAttack( void )
{
	if (m_pPlayer->pev->fov == 0)
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 45;
	else if (m_pPlayer->pev->fov == 45)
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
	else 
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.275;
}

void CAUG::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		AUGFire( (0.1) * (m_pPlayer->m_flAccuracy), 0.0923);
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		AUGFire( (0.5) * (m_pPlayer->m_flAccuracy), 0.0923);
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		AUGFire( (0.0175) * (m_pPlayer->m_flAccuracy), 0.0923);
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		AUGFire( (0.1) * (m_pPlayer->m_flAccuracy), 0.0923);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		AUGFire( (0.1) * (m_pPlayer->m_flAccuracy), 0.0923);
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		AUGFire( (0.5) * (m_pPlayer->m_flAccuracy), 0.0923);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		AUGFire( (0.5) * (m_pPlayer->m_flAccuracy), 0.0923);

	else
		AUGFire( (0.020) * (m_pPlayer->m_flAccuracy), 0.0923);
}

void CAUG::AUGFire( float flSpread , float flCycleTime)
{
	m_bDelayFire = TRUE;

	m_pPlayer->m_iShotsFired++;

	m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 225) + 0.35;
	if (m_pPlayer->m_flAccuracy > 0.575)
		m_pPlayer->m_flAccuracy = 0.575;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
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

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usAug, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	KickBack (1.0, 0.375, 0.12, 0.05, 6.5, 3.45, 3);
	//KickBack (1.0, 0.375, 0.15, 0.05, 6.5, 2.5, 3);
}

void CAUG::Reload( void )
{
	int iResult;

	iResult = DefaultReload( AK47_MAX_CLIP, AUG_RELOAD, 3.185);

	if (m_pPlayer->pev->fov != 0)
	{
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_aug.mdl");

		m_pPlayer->m_iLastZoom = m_pPlayer->pev->fov;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
		m_pPlayer->m_bResumeZoom = FALSE;
		
	}

	if (iResult)
	{
		if( (m_pPlayer->m_afButtonPressed & IN_ATTACK) || ( (!(m_pPlayer->m_afButtonPressed & IN_ATTACK)) ) )
		{
			m_pPlayer->m_iShotsFired = 0;
		}
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.45;
		m_pPlayer->m_i556--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i556 < 0) 
		{
			m_pPlayer->m_i556 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}

void CAUG::WeaponIdle( void )
{
	ResetEmptySound( );
	
	//m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
	// only idle if the slid isn't back
	SendWeaponAnim( AUG_IDLE1 );
}

