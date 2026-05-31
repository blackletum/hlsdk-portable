//Updated on: December 10th 2003

// Heckler & Koch MP-5Navy
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

enum mp5_e {
	MP5N_IDLE1 = 0,
	MP5N_RELOAD,	
	MP5N_DRAW,
	MP5N_SHOOT1,
	MP5N_SHOOT2,
	MP5N_SHOOT3,
};

LINK_ENTITY_TO_CLASS( weapon_mp5navy, CMP5N );

void CMP5N::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_mp5navy"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/w_mp5.mdl");
	m_iId = WEAPON_MP5N;

	m_iDefaultAmmo = AK47_MAX_CLIP;

	FallInit();// get ready to fall down.
}


void CMP5N::Precache( void )
{
	PRECACHE_MODEL("models/v_mp5.mdl");
	PRECACHE_MODEL("models/w_mp5.mdl");
	PRECACHE_MODEL("models/p_mp5.mdl");

	PRECACHE_SOUND("weapons/mp5-1.wav");
	PRECACHE_SOUND("weapons/mp5-2.wav");
	PRECACHE_SOUND("weapons/mp5_clipout.wav");
	PRECACHE_SOUND("weapons/mp5_clipin.wav");
	PRECACHE_SOUND("weapons/mp5_slideback.wav");

	m_iShell = PRECACHE_MODEL ("models/9mmshell.mdl");// brass shell

	m_usMP5N = PRECACHE_EVENT( 1, "events/mp5n.sc" );
}

int CMP5N::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AK47_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_MP5N;
	p->iWeight = MP5N_WEIGHT;

	return 1;
}

int CMP5N::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CMP5N::Deploy( )
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.775;
	m_pPlayer->m_bResumeZoom = FALSE;
	m_pPlayer->m_flAccuracy = 0.0;
	iShellOn = 1;
	return DefaultDeploy( "models/v_mp5.mdl", "models/p_mp5.mdl", MP5N_DRAW, "mp5" );
}

void CMP5N::PrimaryAttack( void )
{	
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		MP5NFire( (0.08) * (m_pPlayer->m_flAccuracy), 0.075);
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		MP5NFire( (0.4) * (m_pPlayer->m_flAccuracy), 0.075);
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		MP5NFire( (0.0335) * (m_pPlayer->m_flAccuracy), 0.075);
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		MP5NFire( (0.08) * (m_pPlayer->m_flAccuracy), 0.075);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		MP5NFire( (0.08) * (m_pPlayer->m_flAccuracy), 0.075);
	
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		MP5NFire( (0.4) * (m_pPlayer->m_flAccuracy), 0.075);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		MP5NFire( (0.4) * (m_pPlayer->m_flAccuracy), 0.075);
	
	else
		MP5NFire( (0.04) * (m_pPlayer->m_flAccuracy), 0.075);
}

void CMP5N::MP5NFire( float flSpread , float flCycleTime)
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

	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle ); //This is why the spread doesn't work!!!
																			  //Because I deleted this line!!! ARGH!!!
	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = gpGlobals->v_forward;
	Vector vecDir;

	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, 1, BULLET_PLAYER_MP5N, 0, 0.8, m_pPlayer->pev, m_pPlayer->random_seed, FALSE );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usMP5N, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	KickBack (0.425, 0.225, 0.125, 0.02, 3.05, 1.25, 5);
}

void CMP5N::Reload( void )
{
	int iResult;

	iResult = DefaultReload( AK47_MAX_CLIP, MP5N_RELOAD, 2.55);

	if (iResult)
	{
		if( (m_pPlayer->m_afButtonPressed & IN_ATTACK) || ( (!(m_pPlayer->m_afButtonPressed & IN_ATTACK)) ) )
		{
			m_pPlayer->m_iShotsFired = 0;
		}
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
		m_pPlayer->m_i9mm--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i9mm < 0) 
		{
			m_pPlayer->m_i9mm = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}


void CMP5N::WeaponIdle( void )
{
	ResetEmptySound( );

	//m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	// only idle if the slid isn't back
	SendWeaponAnim( MP5N_IDLE1 );
}

class CMP5Clip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_9mmARclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_9mmARclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( 30, "9mm", _9MM_MAXCARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_MP5clip, CMP5Clip );