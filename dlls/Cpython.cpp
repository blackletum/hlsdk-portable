//Updated on: March 5th 2005

//.357 Colt Python
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

enum cpython_e {
	CPYTHON_IDLE1 = 0,
	CPYTHON_SHOOT1,
	CPYTHON_SHOOT2,
	CPYTHON_SHOOT_EMPTY,
	CPYTHON_RELOAD,	
	CPYTHON_DRAW,
};

LINK_ENTITY_TO_CLASS( weapon_cpython, CCPython );

void CCPython::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_cpython");
	Precache( );
	m_iId = WEAPON_CPYTHON;
	SET_MODEL(ENT(pev), "models/w_cpython.mdl");

	m_iDefaultAmmo = CPYTHON_MAX_CLIP;

	FallInit();// get ready to fall down.
}

void CCPython::Precache( void )
{
	PRECACHE_MODEL("models/v_cpython.mdl");
	PRECACHE_MODEL("models/w_cpython.mdl");
	PRECACHE_MODEL("models/p_cpython.mdl");

	PRECACHE_SOUND("weapons/cpython-1.wav");
	PRECACHE_SOUND("weapons/cpython_clipout.wav");
	PRECACHE_SOUND("weapons/cpython_clipin.wav");
	PRECACHE_SOUND("weapons/cpython_deploy.wav");

	PRECACHE_SOUND ("weapons/dryfire_pistol.wav");

	m_usCPython = PRECACHE_EVENT( 1, "events/cpython.sc" );
}

int CCPython::AddToPlayer( CBasePlayer *pPlayer )
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

int CCPython::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = ".357";
	p->iMaxAmmo1 = _357_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = CPYTHON_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 7;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_CPYTHON;
	p->iWeight = CPYTHON_WEIGHT;

	return 1;
}

BOOL CCPython::Deploy( )
{
	// pev->body = 1;
m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.935f;
m_pPlayer->m_bResumeZoom = FALSE;
m_bDelayFire = TRUE;
//g_engfuncs.pfnSetClientMaxspeed( ENT( m_pPlayer->pev ), 300 );
return DefaultDeploy( "models/v_cpython.mdl", "models/p_cpython.mdl", CPYTHON_DRAW, "onehanded" );
}

void CCPython::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		CPYTHONFire( (0.13) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		CPYTHONFire( (0.8) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		CPYTHONFire( 0, NULL, TRUE );
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		CPYTHONFire( (0.13) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		CPYTHONFire( (0.13) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		CPYTHONFire( (0.8) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		CPYTHONFire( (0.8) * (m_pPlayer->m_flAccuracy), NULL, TRUE );

	else
		CPYTHONFire( 0, NULL, TRUE );
}

void CCPython::CPYTHONFire( float flSpread , float flCycleTime, BOOL fUseSemi )
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
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;


	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming;
	
	vecAiming = gpGlobals->v_forward;

	Vector vecDir;
	
	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 4096, 3, BULLET_PLAYER_CPYTHON, 0, 0.8, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usCPython, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;

	m_pPlayer->pev->punchangle.x -= 1;
	KickBack (0.6, 0.35, 0.12, 0.015, 4.5, 1.5, 10);
}


void CCPython::Reload( void )
{
	int iResult;

	iResult = DefaultReload(CPYTHON_MAX_CLIP, CPYTHON_RELOAD, 3.5);

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
		m_pPlayer->m_i357--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i357 < 0) 
		{
			m_pPlayer->m_i357 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}



void CCPython::WeaponIdle( void )
{
	ResetEmptySound2( );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;
	
	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		SendWeaponAnim( CPYTHON_IDLE1 );
	}
}

class CCPythonClip : public CBasePlayerAmmo
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
		int bResult = (pOther->GiveAmmo( CPYTHON_MAX_CLIP, ".357", _357_MAXCARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_CPythonclip, CCPythonClip );














