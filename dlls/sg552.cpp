//Updated on: February 17th 2004

// Sig SG-552 Assault Rifle
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

enum sg552_e {
	SG552_IDLE1 = 0,
	SG552_RELOAD,	
	SG552_DRAW,
	SG552_SHOOT1,
	SG552_SHOOT2,
	SG552_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_sg552, CSG552 );

void CSG552::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_sg552");
	Precache( );
	m_iId = WEAPON_SG552;
	SET_MODEL(ENT(pev), "models/w_sg552.mdl");

	m_iDefaultAmmo = AK47_MAX_CLIP;

	FallInit();// get ready to fall down.
}

void CSG552::Precache( void )
{
	PRECACHE_MODEL("models/v_sg552.mdl");
	PRECACHE_MODEL("models/w_sg552.mdl");
	PRECACHE_MODEL("models/p_sg552.mdl");

	PRECACHE_SOUND("weapons/sg552-1.wav");
	PRECACHE_SOUND("weapons/sg552-2.wav");
	PRECACHE_SOUND("weapons/sg552_clipout.wav");
	PRECACHE_SOUND("weapons/sg552_clipin.wav");
	PRECACHE_SOUND("weapons/sg552_boltpull.wav");

	m_iShell = PRECACHE_MODEL ("models/556mmshell.mdl");// brass shell

	m_usSG552 = PRECACHE_EVENT( 1, "events/sg552.sc" );
}

int CSG552::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = _556MM_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AK47_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_SG552;
	p->iWeight = SG552_WEIGHT;

	return 1;
}

int CSG552::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CSG552::Deploy( )
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.775;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.775;
	
	iShellOn = 1;
	m_pPlayer->m_bResumeZoom = FALSE;

	return DefaultDeploy( "models/v_sg552.mdl", "models/p_sg552.mdl", SG552_DRAW, "mp5" );
}

void CSG552::SecondaryAttack( void )
{
	if (m_pPlayer->pev->fov == 0)
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 45;
	else if (m_pPlayer->pev->fov == 45)
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
	else 
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
	
	/*if ( m_pPlayer->pev->fov == 0 )
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 45;
	}
	else if ( m_pPlayer->pev->fov == 45 )
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV =  10;
	}
	else 
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;  // 0 means reset to default fov
		m_pPlayer->m_bResumeZoom = FALSE;
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_sg552.mdl");
	}*/

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.275;
}

void CSG552::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		SG552Fire( (0.155) * (m_pPlayer->m_flAccuracy), 0.08571);
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		SG552Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.08571);
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		SG552Fire( (0.0355) * (m_pPlayer->m_flAccuracy), 0.08571);
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		SG552Fire( (0.155) * (m_pPlayer->m_flAccuracy), 0.08571);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		SG552Fire( (0.155) * (m_pPlayer->m_flAccuracy), 0.08571);

	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		SG552Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.08571);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		SG552Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.08571);
	
	else
		SG552Fire( (0.020) * (m_pPlayer->m_flAccuracy), 0.08571);
}

void CSG552::SG552Fire( float flSpread , float flCycleTime)
{
	m_bDelayFire = TRUE;

	m_pPlayer->m_iShotsFired++;

	m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 200) + 0.35;
	if (m_pPlayer->m_flAccuracy > 0.75)
		m_pPlayer->m_flAccuracy = 0.75;

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

	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, 3, BULLET_PLAYER_SG552, 0, 0.98, m_pPlayer->pev, m_pPlayer->random_seed, FALSE );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usSG552, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	KickBack (1.0, 0.4, 0.15, 0.05, 7.75, 3.5, 3);
	//KickBack (1.15, 0.375, 0.25, 0.0125, 10.5, 4.75, 3);
}

void CSG552::Reload( void )
{
	int iResult;

	iResult = DefaultReload( AK47_MAX_CLIP, SG552_RELOAD, 2.95);

	if (m_pPlayer->pev->fov != 0)
	{
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_sg552.mdl");

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

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.5;
		m_pPlayer->m_i556--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i556 < 0) 
		{
			m_pPlayer->m_i556 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}

void CSG552::WeaponIdle( void )
{
	ResetEmptySound( );

	//m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;
	// only idle if the slid isn't back
	SendWeaponAnim( SG552_IDLE1 );
}

