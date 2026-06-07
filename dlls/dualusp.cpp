//Updated on: November 7th 2004

//Dual USP .40 Tactical
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

enum dualusp_e {
	DUALUSP_IDLE = 0,
	DUALUSP_IDLE_LEFTEMPTY,
	DUALUSP_SHOOT_LEFT1,
	DUALUSP_SHOOT_LEFT2,
	DUALUSP_SHOOT_LEFT3,
	DUALUSP_SHOOT_LEFT4,
	DUALUSP_SHOOT_LEFT5,
	DUALUSP_SHOOT_LEFTLAST,
	DUALUSP_SHOOT_RIGHT1,
	DUALUSP_SHOOT_RIGHT2,
	DUALUSP_SHOOT_RIGHT3,
	DUALUSP_SHOOT_RIGHT4,
	DUALUSP_SHOOT_RIGHT5,
	DUALUSP_SHOOT_RIGHTLAST,
	DUALUSP_RELOAD,	
	DUALUSP_DRAW,
};

LINK_ENTITY_TO_CLASS( weapon_dualusp, CDUSP );

void CDUSP::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_dualusp");
	Precache( );
	m_iId = WEAPON_DUALUSP;
	SET_MODEL(ENT(pev), "models/w_dualusp.mdl");

	m_iDefaultAmmo = USP_MAX_CLIP * 2;

	FallInit();// get ready to fall down.
}


void CDUSP::Precache( void )
{
	PRECACHE_MODEL("models/v_dualusp.mdl");
	PRECACHE_MODEL("models/w_dualusp.mdl");
	PRECACHE_MODEL("models/p_dualusp.mdl");

	PRECACHE_SOUND("weapons/dualusp_clipout.wav");
	PRECACHE_SOUND("weapons/dualusp_deploy.wav");
	PRECACHE_SOUND("weapons/dualusp_fire.wav");
	PRECACHE_SOUND("weapons/dualusp_leftclipin.wav");
	PRECACHE_SOUND("weapons/dualusp_reloadstart.wav");
	PRECACHE_SOUND("weapons/dualusp_rightclipin.wav");
	PRECACHE_SOUND("weapons/dualusp_sliderelease.wav");
	PRECACHE_SOUND("weapons/dualusp_twirl.wav");

	m_iShell = PRECACHE_MODEL ("models/45calshell.mdl");// brass shell
	m_usDUSPLEFT = PRECACHE_EVENT( 1, "events/dualusp_left.sc" );
	m_usDUSPRIGHT = PRECACHE_EVENT( 1, "events/dualusp_right.sc" );
}

int CDUSP::AddToPlayer( CBasePlayer *pPlayer )
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

int CDUSP::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "45ACP";
	p->iMaxAmmo1 = _45ACP_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = USP_MAX_CLIP * 2;
	p->iSlot = 1;
	p->iPosition = 6;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_DUALUSP;
	p->iWeight = DUALUSP_WEIGHT;

	return 1;
}

BOOL CDUSP::Deploy( )
{
	// pev->body = 1;
m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.745;
m_pPlayer->m_bResumeZoom = FALSE;
g_engfuncs.pfnSetClientMaxspeed( ENT( m_pPlayer->pev ), 300 );
return DefaultDeploy( "models/v_dualusp.mdl", "models/p_dualusp.mdl", DUALUSP_DRAW, "dualpistols" );
}

void CDUSP::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		DUSPFire( (0.1) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		DUSPFire( (0.4) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		DUSPFire( (0.075) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		DUSPFire( (0.1) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		DUSPFire( (0.1) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		DUSPFire( (0.4) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		DUSPFire( (0.4) * (m_pPlayer->m_flAccuracy), NULL, TRUE );

	else
		DUSPFire( (0.085) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
}

void CDUSP::DUSPFire( float flSpread , float flCycleTime, BOOL fUseSemi )
{
	int left = 0;
	if ( m_pPlayer->m_iShotsFired > 1)
		m_pPlayer->m_iShotsFired = 0;

	if ( fUseSemi == TRUE && m_pPlayer->m_iShotsFired > 0)
	{
		if(!( m_pPlayer->m_afButtonPressed & IN_ATTACK ) )
		return;
	}
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 250) + 0.3;
	if (m_pPlayer->m_flAccuracy > 0.5)
		m_pPlayer->m_flAccuracy = 0.5;

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

	if ( m_iClip != 0)
	{
		if ( (float)(m_iClip % 2) == 0 ) 
		{
		}
		else
		{	
			left = 1;
		}
		
		if (m_iClip == 1)
		{
			left = 1;
		}
	}

		// player "shoot" animation
	if (left == 1)
	{
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
	}
	else
	{
		m_pPlayer->SetAnimation( PLAYER_ATTACK2 );
	}

	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	// Special f/x
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecLeft = m_pPlayer->GetGunPosition( ) - gpGlobals->v_right * 5;
	Vector vecRight = m_pPlayer->GetGunPosition( ) + gpGlobals->v_right * 5;
	
	Vector vecAiming;
	vecAiming = gpGlobals->v_forward;

	Vector vecDir;
	
	if ( left == 1)
	{
		vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecLeft, vecAiming, Vector( flSpread, flSpread, flSpread ), 2048, 1, BULLET_PLAYER_USP, 0, 0.83, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );
		PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usDUSPLEFT, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );
	}
	else
	{
		vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecRight, vecAiming, Vector( flSpread, flSpread, flSpread ), 2048, 1, BULLET_PLAYER_USP, 0, 0.83, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );
		PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usDUSPRIGHT, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;

	KickBack (0.6, 0.35, 0.12, 0.015, 4.5, 1.5, 10);
}


void CDUSP::Reload( void )
{
	int iResult;

	iResult = DefaultReload( USP_MAX_CLIP * 2, DUALUSP_RELOAD, 4.465);

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
		m_pPlayer->m_i45 = m_pPlayer->m_i45 - 2; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i45 < 0) 
		{
			m_pPlayer->m_i45 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
	m_bDelayFire = TRUE;
}



void CDUSP::WeaponIdle( void )
{
	ResetEmptySound2( );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;
	
	// only idle if the slid isn't back
	if (m_iClip == 1)
	{	
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
		SendWeaponAnim( DUALUSP_IDLE_LEFTEMPTY );
	}
}
