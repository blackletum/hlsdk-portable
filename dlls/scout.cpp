//Updated on: February 17th 2004

//Steyr Scout Sniper Rifle
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

enum scout_e {
	SCOUT_IDLE = 0,
	SCOUT_SHOOT,
	SCOUT_SHOOT2,
	SCOUT_RELOAD,	
	SCOUT_DRAW,
};

LINK_ENTITY_TO_CLASS( weapon_scout, CScout );

int CScout::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "762Nato";
	p->iMaxAmmo1 = _762MM_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SCOUT_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 6;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_SCOUT;
	p->iWeight = SCOUT_WEIGHT;

	return 1;
}

int CScout::AddToPlayer( CBasePlayer *pPlayer )
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

void CScout::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_scout");
	Precache( );
	m_iId = WEAPON_SCOUT;
	SET_MODEL(ENT(pev), "models/w_scout.mdl");

	m_iDefaultAmmo = SCOUT_MAX_CLIP;

	FallInit();// get ready to fall down.
}


void CScout::Precache( void )
{
	PRECACHE_MODEL("models/v_scout.mdl");
	PRECACHE_MODEL("models/w_scout.mdl");
	PRECACHE_MODEL("models/p_scout.mdl");

	PRECACHE_SOUND("weapons/scout_fire-1.wav");
	PRECACHE_SOUND("weapons/scout_bolt.wav");
	PRECACHE_SOUND("weapons/scout_clipin.wav");
	PRECACHE_SOUND("weapons/scout_clipout.wav");
	PRECACHE_SOUND("weapons/zoom.wav");

	m_iShell = PRECACHE_MODEL ("models/762mmshell.mdl");// brass shell
	m_usScout = PRECACHE_EVENT( 1, "events/scout.sc" );
}

BOOL CScout::Deploy( )
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
	m_pPlayer->m_bResumeZoom = FALSE;
	return DefaultDeploy( "models/v_scout.mdl", "models/p_scout.mdl", SCOUT_DRAW, "silentsniper" );
}

void CScout::SecondaryAttack( void )
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
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_scout.mdl");
	}

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/zoom.wav", 1, ATTN_NORM);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CScout::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		SCOUTFire( 0.195, 1.15);
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		SCOUTFire( 0.35, 1.15);
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		SCOUTFire( 0.0, 1.15);
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		SCOUTFire( 0.195, 1.15);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		SCOUTFire( 0.195, 1.15);
	
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		SCOUTFire( 0.35, 1.15);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		SCOUTFire( 0.35, 1.15);
	
	else
		SCOUTFire( 0.0, 1.15);
}

void CScout::SCOUTFire( float flSpread , float flCycleTime)
{
	if ( m_pPlayer->pev->fov != 0)
	{	
		// resume the light level to normal.. and return the view model.
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_scout.mdl");

		m_pPlayer->m_bResumeZoom = TRUE;
		m_pPlayer->m_iLastZoom = m_pPlayer->pev->fov;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
	}
/*	else if ( m_pPlayer->pev->fov == 0 )
	{
		flSpread += 0.155;
	}*/

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
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );


	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = gpGlobals->v_forward;

	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, 3, BULLET_PLAYER_SCOUT, 0, 0.98, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );

    int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usScout, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );

	m_flNextPrimaryAttack =  m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
	
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	m_pPlayer->pev->punchangle.x -= 2;
}


void CScout::Reload( void )
{
	int iResult;

	iResult = DefaultReload( SCOUT_MAX_CLIP, SCOUT_RELOAD, 2.015);

	if (m_pPlayer->pev->fov != 0)
	{
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_scout.mdl");

		m_pPlayer->m_iLastZoom = m_pPlayer->pev->fov;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
		m_pPlayer->m_bResumeZoom = FALSE;
	}


	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
		m_pPlayer->m_i762--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i762 < 0) 
		{
			m_pPlayer->m_i762 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}

void CScout::WeaponIdle( void )
{
	ResetEmptySound( );

//	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	SendWeaponAnim( SCOUT_IDLE );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.

}