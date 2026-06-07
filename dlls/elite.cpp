//Updated on: January 5th 2004

//Dual Berettas 96G
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

enum elite_e {
	ELITE_IDLE = 0,
	ELITE_IDLE_LEFTEMPTY,
	ELITE_SHOOT_LEFT1,
	ELITE_SHOOT_LEFT2,
	ELITE_SHOOT_LEFT3,
	ELITE_SHOOT_LEFT4,
	ELITE_SHOOT_LEFT5,
	ELITE_SHOOT_LEFTLAST,
	ELITE_SHOOT_RIGHT1,
	ELITE_SHOOT_RIGHT2,
	ELITE_SHOOT_RIGHT3,
	ELITE_SHOOT_RIGHT4,
	ELITE_SHOOT_RIGHT5,
	ELITE_SHOOT_RIGHTLAST,
	ELITE_RELOAD,	
	ELITE_DRAW,
};

LINK_ENTITY_TO_CLASS( weapon_elite, CElite );

void CElite::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_elite");
	Precache( );
	m_iId = WEAPON_ELITE;
	SET_MODEL(ENT(pev), "models/w_elite.mdl");

	m_iDefaultAmmo = ELITE_MAX_CLIP;

	FallInit();// get ready to fall down.
}


void CElite::Precache( void )
{
	PRECACHE_MODEL("models/v_elite.mdl");
	PRECACHE_MODEL("models/w_elite.mdl");
	PRECACHE_MODEL("models/p_elite.mdl");

	PRECACHE_SOUND("weapons/elite_clipout.wav");
	PRECACHE_SOUND("weapons/elite_deploy.wav");
	PRECACHE_SOUND("weapons/elite_fire.wav");
	PRECACHE_SOUND("weapons/elite_leftclipin.wav");
	PRECACHE_SOUND("weapons/elite_reloadstart.wav");
	PRECACHE_SOUND("weapons/elite_rightclipin.wav");
	PRECACHE_SOUND("weapons/elite_sliderelease.wav");
	PRECACHE_SOUND("weapons/elite_twirl.wav");

	m_iShell = PRECACHE_MODEL ("models/9mmshell.mdl");// brass shell
	m_usELEFT = PRECACHE_EVENT( 1, "events/elite_left.sc" );
	m_usERIGHT = PRECACHE_EVENT( 1, "events/elite_right.sc" );
}

int CElite::AddToPlayer( CBasePlayer *pPlayer )
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

int CElite::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = ELITE_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 5;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_ELITE;
	p->iWeight = ELITE_WEIGHT;

	return 1;
}

BOOL CElite::Deploy( )
{
	// pev->body = 1;
m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.745;
m_pPlayer->m_bResumeZoom = FALSE;

return DefaultDeploy( "models/v_elite.mdl", "models/p_elite.mdl", ELITE_DRAW, "dualpistols" );
}

void CElite::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		ELITEFire( (0.1) * (m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		ELITEFire( (0.4) * (m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		ELITEFire( (0.075) * (m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		ELITEFire( (0.1) * (m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		ELITEFire( (0.1) * (m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		ELITEFire( (0.4) * (m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		ELITEFire( (0.4) * (m_pPlayer->m_flAccuracy), 0, TRUE );
	else
		ELITEFire( (0.085) * (m_pPlayer->m_flAccuracy), 0, TRUE );
}

void CElite::ELITEFire( float flSpread , float flCycleTime, BOOL fUseSemi )
{
	int left;
	if ( m_pPlayer->m_iShotsFired > 1)
		m_pPlayer->m_iShotsFired = 0;

	if ( fUseSemi == TRUE && m_pPlayer->m_iShotsFired > 0)
	{
		if(!( m_pPlayer->m_afButtonPressed & IN_ATTACK ) )
		return;
	}
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 250) + 0.3;
	if (m_pPlayer->m_flAccuracy > 0.55)
		m_pPlayer->m_flAccuracy = 0.55;

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
		vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecLeft, vecAiming, Vector( flSpread, flSpread, flSpread ), 2048, 1, BULLET_PLAYER_MP5N, 0, 0.83, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );
		PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usELEFT, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );
	}
	else
	{
		vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecRight, vecAiming, Vector( flSpread, flSpread, flSpread ), 2048, 1, BULLET_PLAYER_MP5N, 0, 0.83, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );
		PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usERIGHT, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;

	KickBack (0.6, 0.35, 0.12, 0.015, 4.5, 1.5, 10);
}


void CElite::Reload( void )
{
	int iResult;

	iResult = DefaultReload( ELITE_MAX_CLIP, ELITE_RELOAD, 4.375);

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.25;
		m_pPlayer->m_i9mm--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i9mm < 0) 
		{
			m_pPlayer->m_i9mm = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}



void CElite::WeaponIdle( void )
{
	ResetEmptySound2( );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;


	// only idle if the slid isn't back
	if (m_iClip == 1)
	{	
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;
		SendWeaponAnim( ELITE_IDLE_LEFTEMPTY );
	}
}
