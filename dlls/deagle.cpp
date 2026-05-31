//Updated on: November 14th 2003

//Desert Eagle DOT .50AE
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

enum deagle_e {
	DEAGLE_IDLE1 = 0,
	DEAGLE_SHOOT1,
	DEAGLE_SHOOT2,
	DEAGLE_SHOOT_EMPTY,
	DEAGLE_RELOAD,	
	DEAGLE_DRAW,
};

LINK_ENTITY_TO_CLASS( weapon_deagle, CDeagle );

void CDeagle::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_deagle");
	Precache( );
	m_iId = WEAPON_DEAGLE;
	SET_MODEL(ENT(pev), "models/w_deagle.mdl");

	m_iDefaultAmmo = DEAGLE_MAX_CLIP;

	FallInit();// get ready to fall down.
}

void CDeagle::Precache( void )
{
	PRECACHE_MODEL("models/v_deagle.mdl");
	PRECACHE_MODEL("models/w_deagle.mdl");
	PRECACHE_MODEL("models/p_deagle.mdl");

	PRECACHE_SOUND("weapons/deagle-1.wav");
	PRECACHE_SOUND("weapons/deagle-2.wav");
	PRECACHE_SOUND("weapons/de_clipout.wav");
	PRECACHE_SOUND("weapons/de_clipin.wav");
	PRECACHE_SOUND("weapons/de_deploy.wav");

	PRECACHE_SOUND ("weapons/dryfire_pistol.wav");

	m_iShell = PRECACHE_MODEL ("models/50calshell.mdl");// brass shell
	m_usDeagle = PRECACHE_EVENT( 1, "events/deagle.sc" );
}

int CDeagle::AddToPlayer( CBasePlayer *pPlayer )
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

int CDeagle::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "50AE";
	p->iMaxAmmo1 = _50AE_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = DEAGLE_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_DEAGLE;
	p->iWeight = DEAGLE_WEIGHT;

	return 1;
}

BOOL CDeagle::Deploy( )
{
	// pev->body = 1;
m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.775f;
m_pPlayer->m_bResumeZoom = FALSE;
m_bDelayFire = TRUE;
return DefaultDeploy( "models/v_deagle.mdl", "models/p_deagle.mdl", DEAGLE_DRAW, "onehanded" );
}

void CDeagle::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		DEAGLEFire( (0.13) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		DEAGLEFire( (0.4) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		DEAGLEFire( (0.055) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		DEAGLEFire( (0.13) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		DEAGLEFire( (0.13) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		DEAGLEFire( (0.4) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		DEAGLEFire( (0.4) * (m_pPlayer->m_flAccuracy), NULL, TRUE );

	else
		DEAGLEFire( (0.075) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
}

void CDeagle::DEAGLEFire( float flSpread , float flCycleTime, BOOL fUseSemi )
{
	if ( m_pPlayer->m_iShotsFired > 1)
			m_pPlayer->m_iShotsFired = 0;
	//semi-auto code here
	if ( fUseSemi == TRUE  && m_pPlayer->m_iShotsFired > 0 )
	{
	//	ALERT (at_console, "shots fired = %i\n", m_pPlayer->m_iShotsFired);
				
		if(!( m_pPlayer->m_afButtonPressed & IN_ATTACK ) )
		return;
	}
	
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.205f;

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

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;


	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming;
	
	vecAiming = gpGlobals->v_forward;

	Vector vecDir;
	
	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 4096, 3, BULLET_PLAYER_DE, 0, 0.8, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usDeagle, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;

	m_pPlayer->pev->punchangle.x -= 1;
	KickBack (0.6, 0.35, 0.12, 0.015, 4.5, 1.5, 10);
}


void CDeagle::Reload( void )
{
	int iResult;

	iResult = DefaultReload( DEAGLE_MAX_CLIP, DEAGLE_RELOAD, 1.875);

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.65;
		m_pPlayer->m_i50--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i50 < 0) 
		{
			m_pPlayer->m_i50 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}



void CDeagle::WeaponIdle( void )
{
	ResetEmptySound2( );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		SendWeaponAnim( DEAGLE_IDLE1 );
	}
}

class CDEClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_357ammobox.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_357ammobox.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( DEAGLE_MAX_CLIP, "50AE", _50AE_MAXCARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_DEclip, CDEClip );














