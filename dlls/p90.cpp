//Updated on: February 17th 2004

//Fabrique Nationale P90
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

enum p90_e {
	P90_IDLE1 = 0,
	P90_RELOAD,	
	P90_DRAW,
	P90_SHOOT1,
	P90_SHOOT2,
	P90_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_p90, CP90 );

void CP90::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_p90");
	Precache( );
	m_iId = WEAPON_P90;
	SET_MODEL(ENT(pev), "models/w_p90.mdl");

	m_iDefaultAmmo = P90_MAX_CLIP;

	FallInit();// get ready to fall down.
}

void CP90::Precache( void )
{
	PRECACHE_MODEL("models/v_p90.mdl");
	PRECACHE_MODEL("models/w_p90.mdl");
	PRECACHE_MODEL("models/p_p90.mdl");

	PRECACHE_SOUND("weapons/p90-1.wav");
	PRECACHE_SOUND("weapons/p90_clipout.wav");
	PRECACHE_SOUND("weapons/p90_clipin.wav");
	PRECACHE_SOUND("weapons/p90_boltpull.wav");
	PRECACHE_SOUND("weapons/p90_cliprelease.wav");

	m_iShell = PRECACHE_MODEL ("models/57mmshell.mdl");// brass shell

	m_usP90 = PRECACHE_EVENT( 1, "events/p90.sc" );
}

int CP90::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "57mm";
	p->iMaxAmmo1 = _57MM_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = P90_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_P90;
	p->iWeight = P90_WEIGHT;

	return 1;
}

int CP90::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CP90::Deploy( )
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.95;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.95;
	m_pPlayer->m_bResumeZoom = FALSE;
	iShellOn = 1;
	return DefaultDeploy( "models/v_p90.mdl", "models/p_p90.mdl", P90_DRAW, "p90" );
}

void CP90::SecondaryAttack( void )
{
	if (m_pPlayer->pev->fov == 0)
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 45;
	else if (m_pPlayer->pev->fov == 45)
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
	else 
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.275;
}

void CP90::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		P90Fire( (0.07) * (m_pPlayer->m_flAccuracy), 0.06667);
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		P90Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.06667);
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		P90Fire( (0.0255) * (m_pPlayer->m_flAccuracy), 0.06667);
		
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		P90Fire( (0.07) * (m_pPlayer->m_flAccuracy), 0.06667);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		P90Fire( (0.07) * (m_pPlayer->m_flAccuracy), 0.06667);
		
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		P90Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.06667);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		P90Fire( (0.4) * (m_pPlayer->m_flAccuracy), 0.06667);
		
	else
		P90Fire( (0.035) * (m_pPlayer->m_flAccuracy), 0.06667);
}

void CP90::P90Fire( float flSpread , float flCycleTime)
{
	m_bDelayFire = TRUE;
	//ALERT (at_console, "shots fired = %i\n", m_pPlayer->m_iShotsFired);

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

	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, 3, BULLET_PLAYER_P90, 0, 0.8, m_pPlayer->pev, m_pPlayer->random_seed, FALSE );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usP90, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	KickBack (0.625, 0.275, 0.13, 0.035, 3.5, 1.5, 4);
}

void CP90::Reload( void )
{
	int iResult;

	iResult = DefaultReload( P90_MAX_CLIP, P90_RELOAD, 3.215);

	if (m_pPlayer->pev->fov != 0)
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
	}

	if (iResult)
	{
		if( (m_pPlayer->m_afButtonPressed & IN_ATTACK) || ( (!(m_pPlayer->m_afButtonPressed & IN_ATTACK)) ) )
		{
			m_pPlayer->m_iShotsFired = 0;
		}
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
		m_pPlayer->m_i57--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i57 < 0) 
		{
			m_pPlayer->m_i57 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}


void CP90::WeaponIdle( void )
{
	ResetEmptySound( );

	//m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	// only idle if the slid isn't back
	SendWeaponAnim( P90_IDLE1 );
}

class CP90Clip : public CBasePlayerAmmo
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
		int bResult = (pOther->GiveAmmo( 50, "57mm", _57MM_MAXCARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_P90clip, CP90Clip );