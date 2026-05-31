//Updated on: February 20th 2004

// Heckler & Koch Universal Machine Pistol 45(UMP45)
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

enum ump45_e {
	UMP45_IDLE1 = 0,
	UMP45_RELOAD,	
	UMP45_DRAW,
	UMP45_SHOOT1,
	UMP45_SHOOT2,
	UMP45_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_ump45, CUMP45 );

void CUMP45::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_ump45");
	Precache( );
	m_iId = WEAPON_UMP45;
	SET_MODEL(ENT(pev), "models/w_ump45.mdl");

	m_iDefaultAmmo = UMP45_MAX_CLIP;

	FallInit();// get ready to fall down.
}

void CUMP45::Precache( void )
{
	PRECACHE_MODEL("models/v_ump45.mdl");
	PRECACHE_MODEL("models/w_ump45.mdl");
	PRECACHE_MODEL("models/p_ump45.mdl");

	PRECACHE_SOUND("weapons/ump45-1.wav");
	PRECACHE_SOUND("weapons/ump45_boltslap.wav");
	PRECACHE_SOUND("weapons/ump45_clipout.wav");
	PRECACHE_SOUND("weapons/ump45_clipin.wav");

	m_iShell = PRECACHE_MODEL ("models/45calshell.mdl");// brass shell

	m_usUMP45= PRECACHE_EVENT( 1, "events/ump45.sc" );
}

int CUMP45::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "45ACP";
	p->iMaxAmmo1 = _45ACP_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = UMP45_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 4;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_UMP45;
	p->iWeight = UMP45_WEIGHT;

	return 1;
}

int CUMP45::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CUMP45::Deploy( )
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.715;
	m_pPlayer->m_bResumeZoom = FALSE;
	m_pPlayer->m_flAccuracy = 0.0;
	iShellOn = 1;
	return DefaultDeploy( "models/v_ump45.mdl", "models/p_ump45.mdl", UMP45_DRAW, "mp5" );
}

void CUMP45::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		UMP45Fire( (0.1) * (m_pPlayer->m_flAccuracy), 0.08571);
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		UMP45Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.08571);
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		UMP45Fire( (0.04) * (m_pPlayer->m_flAccuracy), 0.08571);
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		UMP45Fire( (0.1) * (m_pPlayer->m_flAccuracy), 0.08571);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		UMP45Fire( (0.1) * (m_pPlayer->m_flAccuracy), 0.08571);
	
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		UMP45Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.08571);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		UMP45Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.08571);
		
	else
		UMP45Fire( (0.05) * (m_pPlayer->m_flAccuracy), 0.08571);
}

void CUMP45::UMP45Fire( float flSpread , float flCycleTime)
{
	m_bDelayFire = TRUE;

	m_pPlayer->m_iShotsFired++;
	
	m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 250) + 0.4;
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

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

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

	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, 1, BULLET_PLAYER_USP, 0, 0.9, m_pPlayer->pev, m_pPlayer->random_seed, FALSE );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usUMP45, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	KickBack (0.6, 0.35, 0.15, 0.025, 2.75, 2.25, 9);
}

void CUMP45::Reload( void )
{
	int iResult;

	iResult = DefaultReload( UMP45_MAX_CLIP, UMP45_RELOAD, 3.25);

	if (iResult)
	{
		if( (m_pPlayer->m_afButtonPressed & IN_ATTACK) || ( (!(m_pPlayer->m_afButtonPressed & IN_ATTACK)) ) )
		{
			m_pPlayer->m_iShotsFired = 0;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
		m_pPlayer->m_i45 = m_pPlayer->m_i45 - 3; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i45 < 0) 
		{
			m_pPlayer->m_i45 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}


void CUMP45::WeaponIdle( void )
{
	ResetEmptySound( );

	//m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	// only idle if the slid isn't back
	SendWeaponAnim( UMP45_IDLE1 );
}
