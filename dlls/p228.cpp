//Updated on: February 15th 2004

//Sig P228
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

enum p228_e {
	P228_IDLE = 0,
	P228_SHOOT1,
	P228_SHOOT2,
	P228_SHOOT3,
	P228_SHOOT_EMPTY,
	P228_RELOAD,	
	P228_DRAW,
};

LINK_ENTITY_TO_CLASS( weapon_p228, CP228 );

void CP228::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_p228");
	Precache( );
	m_iId = WEAPON_P228;
	SET_MODEL(ENT(pev), "models/w_p228.mdl");

	m_iDefaultAmmo = P228_MAX_CLIP;

	FallInit();// get ready to fall down.
}

void CP228::Precache( void )
{
	PRECACHE_MODEL("models/v_p228.mdl");
	PRECACHE_MODEL("models/w_p228.mdl");
	PRECACHE_MODEL("models/p_p228.mdl");

	PRECACHE_SOUND("weapons/p228-1.wav");
	PRECACHE_SOUND("weapons/p228_clipout.wav");
	PRECACHE_SOUND("weapons/p228_clipin.wav");
	PRECACHE_SOUND("weapons/p228_sliderelease.wav");
	PRECACHE_SOUND("weapons/p228_slidepull.wav");

	m_iShell = PRECACHE_MODEL ("models/9mmshell.mdl");// brass shell
	m_usP228 = PRECACHE_EVENT( 1, "events/p228.sc" );
}

int CP228::AddToPlayer( CBasePlayer *pPlayer )
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

int CP228::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "357SIG";
	p->iMaxAmmo1 = _357SIG_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = P228_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 3;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_P228;
	p->iWeight = P228_WEIGHT;

	return 1;
}

BOOL CP228::Deploy( )
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.85;
	m_pPlayer->m_bResumeZoom = FALSE;
	m_pPlayer->m_flAccuracy = 0.9875;
	return DefaultDeploy( "models/v_p228.mdl", "models/p_p228.mdl", P228_DRAW, "onehanded" );
}

void CP228::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 170)
		P228Fire( (0.2) * (1 - m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		P228Fire( (0.5) * (1 - m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		P228Fire( (0.075) * (1 - m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		P228Fire( (0.2) * (1 - m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		P228Fire( (0.2) * (1 - m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		P228Fire( (0.5) * (1 - m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		P228Fire( (0.5) * (1 - m_pPlayer->m_flAccuracy), 0, TRUE );
	else
		P228Fire( (0.1) * (1 - m_pPlayer->m_flAccuracy), 0, TRUE );
}

void CP228::P228Fire( float flSpread , float flCycleTime, BOOL fUseSemi )
{
	if ( m_pPlayer->m_iShotsFired > 1)
			m_pPlayer->m_iShotsFired = 0;
	//semi-auto code here
	if ( fUseSemi == TRUE  && m_pPlayer->m_iShotsFired > 0 )
	{
		if(!( m_pPlayer->m_afButtonPressed & IN_ATTACK ) )
		return;
	}
	
	// Mark the time of this shot and determine the accuracy modifier based on the last shot fired...
	if (m_pPlayer->m_flLastFire == 0)
		m_pPlayer->m_flLastFire = gpGlobals->time;
	else 
	{
		m_pPlayer->m_flAccuracy = 0.65 + (0.2)*(gpGlobals->time - m_pPlayer->m_flLastFire);

		if (m_pPlayer->m_flAccuracy > 0.98)
			m_pPlayer->m_flAccuracy = 0.98;

		m_pPlayer->m_flLastFire = gpGlobals->time;
	}

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

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;


	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming;
	
	vecAiming = gpGlobals->v_forward;

	Vector vecDir;
	
	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 4096, 1, BULLET_PLAYER_P228, 0, 0.9, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usP228, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;

	m_pPlayer->pev->punchangle.x -= 1;
}


void CP228::Reload( void )
{
	int iResult;

	iResult = DefaultReload( P228_MAX_CLIP, P228_RELOAD, 2.415); //iResult is assigned to being reload.

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.5;
		m_pPlayer->m_i357--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i357 <= 0 || m_pPlayer->m_i357Ammo <= 0) 
		{
			m_pPlayer->m_i357 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
			m_pPlayer->m_i357Ammo = 0;
		}
	}
}

void CP228::WeaponIdle( void )
{
	ResetEmptySound2( );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;
	
	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		SendWeaponAnim( P228_IDLE );
	}
}

class CP228Clip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_9mmclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( 13, "357SIG", _357SIG_MAXCARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_P228clip, CP228Clip );
