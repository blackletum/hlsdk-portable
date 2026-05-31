//Updated on: September 6th 2004

//Colt 1911
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
#include "effects.h"
#include "maxcarry.h"

enum colt_e {
	COLT_IDLE1 = 0,
	COLT_SHOOT1,
	COLT_SHOOT2,
	COLT_SHOOT_EMPTY,
	COLT_RELOAD,	
	COLT_DRAW,
};

LINK_ENTITY_TO_CLASS( weapon_colt, CColt );

void CColt::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_colt");
	Precache( );
	m_iId = WEAPON_COLT;
	SET_MODEL(ENT(pev), "models/w_colt.mdl");

	m_iDefaultAmmo = DEAGLE_MAX_CLIP;

	FallInit();// get ready to fall down.
}


void CColt::Precache( void )
{
	PRECACHE_MODEL("models/v_colt.mdl");
	PRECACHE_MODEL("models/w_colt.mdl");
	PRECACHE_MODEL("models/p_colt.mdl");

	PRECACHE_SOUND("weapons/colt_fire-1.wav");
	PRECACHE_SOUND("weapons/colt_fire-2.wav");
	PRECACHE_SOUND("weapons/colt_clipout.wav");
	PRECACHE_SOUND("weapons/colt_clipin.wav");
	PRECACHE_SOUND("weapons/colt_deploy.wav");

	m_iShell = PRECACHE_MODEL ("models/pshell.mdl");// brass shell
	
	m_usColt = PRECACHE_EVENT( 1, "events/colt.sc" );
}

int CColt::AddToPlayer( CBasePlayer *pPlayer )
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

int CColt::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "45ACP";
	p->iMaxAmmo1 = _45ACP_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = DEAGLE_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 6;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_COLT;
	p->iWeight = COLT_WEIGHT;

	return 1;
}

BOOL CColt::Deploy( )
{
	// pev->body = 1;
	//m_pPlayer->m_flAccuracy = 0.98;
m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.85;
m_pPlayer->m_bResumeZoom = FALSE;
g_engfuncs.pfnSetClientMaxspeed( ENT( m_pPlayer->pev ), 300 );
return DefaultDeploy( "models/v_colt.mdl", "models/p_colt.mdl", COLT_DRAW, "onehanded" );
}

void CColt::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		COLTFire( (0.16) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		COLTFire( (0.65) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		COLTFire( (0.055) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		COLTFire( (0.16) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		COLTFire( (0.16) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		COLTFire( (0.65) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		COLTFire( (0.65) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	
	else
		COLTFire( (0.065) * (m_pPlayer->m_flAccuracy), NULL, TRUE );// 0.03
}

void CColt::COLTFire( float flSpread , float flCycleTime, BOOL fUseSemi )
{
	if ( m_pPlayer->m_iShotsFired > 1)
		m_pPlayer->m_iShotsFired = 0;

	if ( fUseSemi == TRUE && m_pPlayer->m_iShotsFired > 0)
	{
		if(!( m_pPlayer->m_afButtonPressed & IN_ATTACK ) )
		return;
	}

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.200f;

	m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 250) + 0.3;
	if (m_pPlayer->m_flAccuracy > 0.98)
		m_pPlayer->m_flAccuracy = 0.98;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound2();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

	m_iClip--;
	m_pPlayer->m_iShotsFired++;
	//semi-auto code here 

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	int flags;

#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
	
	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming;
	
	vecAiming = gpGlobals->v_forward;

	Vector vecDir;
	
	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 4096, 3, BULLET_PLAYER_COLT, 0, 0.9, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );
	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usColt, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );
	
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;

	KickBack (1.55, NULL, 1.25, NULL, 0.85, NULL, 1);

	m_pPlayer->pev->punchangle.x -= 1;
}


void CColt::Reload( void )
{
	int iResult;

	iResult = DefaultReload( DEAGLE_MAX_CLIP, COLT_RELOAD, 2.365);

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.075;
		m_pPlayer->m_i45--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i45 < 0) 
		{
			m_pPlayer->m_i45 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
	m_bDelayFire = TRUE;
}



void CColt::WeaponIdle( void )
{
	ResetEmptySound2( );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{	
		m_flTimeWeaponIdle = gpGlobals->time + 0.0;
		SendWeaponAnim( COLT_IDLE1 );
	}
}