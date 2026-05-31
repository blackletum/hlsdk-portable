//Updated on: February 11th 2004

// Ingram Mac10
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

enum mac10_e {
	MAC10_IDLE1 = 0,
	MAC10_RELOAD,	
	MAC10_DRAW,
	MAC10_SHOOT1,
	MAC10_SHOOT2,
	MAC10_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_mac10, CMac10 );

void CMac10::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_mac10");
	Precache( );
	m_iId = WEAPON_MAC10;
	SET_MODEL(ENT(pev), "models/w_mac10.mdl");

	m_iDefaultAmmo = MAC10_MAX_CLIP;

	FallInit();// get ready to fall down.
}


void CMac10::Precache( void )
{
	PRECACHE_MODEL("models/v_mac10.mdl");
	PRECACHE_MODEL("models/w_mac10.mdl");
	PRECACHE_MODEL("models/p_mac10.mdl");

	PRECACHE_SOUND("weapons/mac10-1.wav");
	PRECACHE_SOUND("weapons/mac10_boltpull.wav");
	PRECACHE_SOUND("weapons/mac10_clipout.wav");
	PRECACHE_SOUND("weapons/mac10_clipin.wav");
	

	m_iShell = PRECACHE_MODEL ("models/45calshell.mdl");// brass shell

	m_usMac10 = PRECACHE_EVENT( 1, "events/mac10.sc" );
}

int CMac10::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "45ACP";
	p->iMaxAmmo1 = _45ACP_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AK47_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 3;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_MAC10;
	p->iWeight = MAC10_WEIGHT;

	return 1;
}

int CMac10::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CMac10::Deploy( )
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.85;
	m_pPlayer->m_bResumeZoom = FALSE;
	iShellOn = 1;
	return DefaultDeploy( "models/v_mac10.mdl", "models/p_mac10.mdl", MAC10_DRAW, "onehanded" );
}

void CMac10::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		MAC10Fire( (0.1) * (m_pPlayer->m_flAccuracy), 0.075);
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		MAC10Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.075);
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		MAC10Fire( (0.0255) * (m_pPlayer->m_flAccuracy), 0.075);
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		MAC10Fire( (0.1) * (m_pPlayer->m_flAccuracy), 0.075);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		MAC10Fire( (0.1) * (m_pPlayer->m_flAccuracy), 0.075);
	
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		MAC10Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.075);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		MAC10Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.075);
		
	else
		MAC10Fire( (0.035) * (m_pPlayer->m_flAccuracy), 0.075);
}

void CMac10::MAC10Fire( float flSpread , float flCycleTime)
{
	m_bDelayFire = TRUE;

	m_pPlayer->m_iShotsFired++;

	m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 250) + 0.4;
	if (m_pPlayer->m_flAccuracy > 0.98)
		m_pPlayer->m_flAccuracy = 0.98;

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

	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, 1, BULLET_PLAYER_USP, 0, 0.8, m_pPlayer->pev, m_pPlayer->random_seed, FALSE );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usMac10, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	KickBack (0.625, 0.275, 0.15, 0.035, 3.75, 1.5, 5);
}

void CMac10::Reload( void )
{
	int iResult;
	m_pPlayer->m_iShotsFired = 0;

	iResult = DefaultReload( MAC10_MAX_CLIP, MAC10_RELOAD, 3.155);

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

void CMac10::WeaponIdle( void )
{
	ResetEmptySound( );

	//m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;

	// only idle if the slid isn't back
	SendWeaponAnim( MAC10_IDLE1 );
}