//Updated on: December 10th 2003

// Sig SG-550 Sniper Rifle
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

enum sg550_e {
	SG550_IDLE1 = 0,
	SG550_SHOOT,
	SG550_SHOOT2,
	SG550_RELOAD,	
	SG550_DRAW,
};

LINK_ENTITY_TO_CLASS( weapon_sg550, CSG550 );

void CSG550::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_sg550");
	Precache( );
	m_iId = WEAPON_SG550;
	SET_MODEL(ENT(pev), "models/w_sg550.mdl");

	m_iDefaultAmmo = AK47_MAX_CLIP;

	FallInit();// get ready to fall down.
}

void CSG550::Precache( void )
{
	PRECACHE_MODEL("models/v_sg550.mdl");
	PRECACHE_MODEL("models/w_sg550.mdl");
	PRECACHE_MODEL("models/p_sg550.mdl");

	PRECACHE_SOUND("weapons/sg550-1.wav");
	PRECACHE_SOUND("weapons/sg550_boltpull.wav");
	PRECACHE_SOUND("weapons/sg550_clipin.wav");
	PRECACHE_SOUND("weapons/sg550_clipout.wav");
	PRECACHE_SOUND("weapons/zoom.wav");

	m_usSG550 = PRECACHE_EVENT( 1, "events/sg550.sc" );
}

int CSG550::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = _556MM_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AK47_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 9;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_SG550;
	p->iWeight = SG550_WEIGHT;

	return 1;
}

int CSG550::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CSG550::Deploy( )
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.775;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.755;
	m_pPlayer->m_bResumeZoom = FALSE;
	return DefaultDeploy( "models/v_sg550.mdl", "models/p_sg550.mdl", SG550_DRAW, "mp5" );
}

void CSG550::SecondaryAttack( void )
{	
	if ( m_pPlayer->pev->fov == 0 )
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
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_sg550.mdl");
	}

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/zoom.wav", 1, ATTN_NORM);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CSG550::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		SG550Fire( (0.1) * (m_pPlayer->m_flAccuracy), 0.25);
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		SG550Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.25);
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		SG550Fire( (0.02) * (m_pPlayer->m_flAccuracy), 0.25);
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		SG550Fire( (0.07) * (m_pPlayer->m_flAccuracy), 0.25);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		SG550Fire( (0.07) * (m_pPlayer->m_flAccuracy), 0.25);
	
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		SG550Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.25);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		SG550Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.25);
	
	else
		SG550Fire( (0.02) * (m_pPlayer->m_flAccuracy), 0.25);
}

void CSG550::SG550Fire( float flSpread , float flCycleTime)
{
	m_bDelayFire = TRUE;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
	
	m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 250) + 0.4;
	if (m_pPlayer->m_flAccuracy > 0.48)
		m_pPlayer->m_flAccuracy = 0.48;


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

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

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

	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, 3, BULLET_PLAYER_SG550, 0, 0.98, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usSG550, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	KickBack (0.6, 0.35, 0.12, 0.015, 4.5, 1.5, 10);
}

void CSG550::Reload( void )
{
	int iResult;

	iResult = DefaultReload( AK47_MAX_CLIP, SG550_RELOAD, 3.45);

	if (m_pPlayer->pev->fov != 0)
	{
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_sg550.mdl");

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

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
		m_pPlayer->m_i556--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i556 < 0) 
		{
			m_pPlayer->m_i556 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}


void CSG550::WeaponIdle( void )
{
	ResetEmptySound( );

	//m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	// only idle if the slid isn't back
	SendWeaponAnim( SG550_IDLE1 );
}
