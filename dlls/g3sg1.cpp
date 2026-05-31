//Updated on: January 9th 2004

// Heckler & Koch G3SG1 Sniper Rifle
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

enum g3sg1_e 
{
	G3SG1_IDLE = 0,
	G3SG1_SHOOT,
	G3SG1_SHOOT2,
	G3SG1_RELOAD,	
	G3SG1_DRAW,
};

LINK_ENTITY_TO_CLASS( weapon_g3sg1, CG3SG1 );

void CG3SG1::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_g3sg1");
	Precache( );
	m_iId = WEAPON_G3SG1;
	SET_MODEL(ENT(pev), "models/w_g3sg1.mdl");

	m_iDefaultAmmo = G3SG1_MAX_CLIP;

	FallInit();// get ready to fall down.
}

void CG3SG1::Precache( void )
{
	PRECACHE_MODEL("models/v_g3sg1.mdl");
	PRECACHE_MODEL("models/w_g3sg1.mdl");
	PRECACHE_MODEL("models/p_g3sg1.mdl");

	PRECACHE_SOUND("weapons/g3sg1-1.wav");
	PRECACHE_SOUND("weapons/g3sg1_slide.wav");
	PRECACHE_SOUND("weapons/g3sg1_clipin.wav");
	PRECACHE_SOUND("weapons/g3sg1_clipout.wav");
	PRECACHE_SOUND("weapons/zoom.wav");

	m_iShell = PRECACHE_MODEL ("models/762mmshell.mdl");// brass shell

	m_usG3SG1 = PRECACHE_EVENT( 1, "events/g3sg1.sc" );
}

int CG3SG1::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "762Nato";
	p->iMaxAmmo1 = _762MM_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = G3SG1_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 8;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_G3SG1;
	p->iWeight = G3SG1_WEIGHT;

	return 1;
}

int CG3SG1::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CG3SG1::Deploy( )
{
	// pev->body = 1;
m_flNextPrimaryAttack = m_flNextSecondaryAttack = 0.755;
m_pPlayer->m_bResumeZoom = FALSE;
return DefaultDeploy( "models/v_g3sg1.mdl", "models/p_g3sg1.mdl", G3SG1_DRAW, "mp5" );
}

void CG3SG1::SecondaryAttack( void )
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
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_g3sg1.mdl");
	}

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/zoom.wav", 1, ATTN_NORM);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CG3SG1::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		G3SG1Fire( (0.1) * (m_pPlayer->m_flAccuracy), 0.25);
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		G3SG1Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.25);
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		G3SG1Fire( (0.02) * (m_pPlayer->m_flAccuracy), 0.25);
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		G3SG1Fire( (0.07) * (m_pPlayer->m_flAccuracy), 0.25);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		G3SG1Fire( (0.07) * (m_pPlayer->m_flAccuracy), 0.25);
	
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		G3SG1Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.25);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		G3SG1Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.25);

	else
		G3SG1Fire( (0.02) * (m_pPlayer->m_flAccuracy), 0.25);
}

void CG3SG1::G3SG1Fire( float flSpread , float flCycleTime)
{
	m_bDelayFire = TRUE;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
	
	m_pPlayer->m_flAccuracy = ( m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 250) + 0.4;
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
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

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

	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, 3, BULLET_PLAYER_G3SG1, 0, 0.98, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usG3SG1, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;

	KickBack (0.6, 0.35, 0.12, 0.015, 4.5, 1.5, 10);
}

void CG3SG1::Reload( void )
{
	int iResult;

	iResult = DefaultReload( G3SG1_MAX_CLIP, G3SG1_RELOAD, 3.405);

	if (m_pPlayer->pev->fov != 0)
	{
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_g3sg1.mdl");

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

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.25;
		m_pPlayer->m_i762--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i762 < 0) 
		{
			m_pPlayer->m_i762 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}


void CG3SG1::WeaponIdle( void )
{
	ResetEmptySound( );

	//m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	// only idle if the slid isn't back
	SendWeaponAnim( G3SG1_IDLE );
}
