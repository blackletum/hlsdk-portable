//Updated on: November 2nd 2004

//Fabrique Nationale FiveSeven
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

enum fiveseven_e {
	FIVESEVEN_IDLE1 = 0,
	FIVESEVEN_SHOOT1,
	FIVESEVEN_SHOOT2,
	FIVESEVEN_SHOOT_EMPTY,
	FIVESEVEN_RELOAD,	
	FIVESEVEN_DRAW,
};

LINK_ENTITY_TO_CLASS( weapon_fiveseven, CFiveseveN );

void CFiveseveN::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_fiveseven");
	Precache( );
	m_iId = WEAPON_FIVESEVEN;
	SET_MODEL(ENT(pev), "models/w_fiveseven.mdl");

	m_iDefaultAmmo = G3SG1_MAX_CLIP;

	FallInit();// get ready to fall down.
}


void CFiveseveN::Precache( void )
{
	PRECACHE_MODEL("models/v_fiveseven.mdl");
	PRECACHE_MODEL("models/w_fiveseven.mdl");
	PRECACHE_MODEL("models/p_fiveseven.mdl");

	PRECACHE_SOUND("weapons/fiveseven-1.wav");
	PRECACHE_SOUND("weapons/fiveseven_clipout.wav");
	PRECACHE_SOUND("weapons/fiveseven_clipin.wav");
	PRECACHE_SOUND("weapons/fiveseven_sliderelease.wav");
	PRECACHE_SOUND("weapons/fiveseven_slidepull.wav");

	m_iShell = PRECACHE_MODEL ("models/57mmshell.mdl");// brass shell
	
	m_usFS = PRECACHE_EVENT( 1, "events/fiveseven.sc" );
}

int CFiveseveN::AddToPlayer( CBasePlayer *pPlayer )
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

int CFiveseveN::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "57mm";
	p->iMaxAmmo1 = _57MM_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = G3SG1_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 4;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_FIVESEVEN;
	p->iWeight = FIVESEVEN_WEIGHT;

	return 1;
}

BOOL CFiveseveN::Deploy( )
{
	// pev->body = 1;
m_pPlayer->m_flAccuracy = 0;
m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.85;
m_pPlayer->m_bResumeZoom = FALSE;
return DefaultDeploy( "models/v_fiveseven.mdl", "models/p_fiveseven.mdl", FIVESEVEN_DRAW, "onehanded" );
}

void CFiveseveN::PrimaryAttack( void )
{
	if ( m_pPlayer->m_iShotsFired > 0 )
	{
		if(!( m_pPlayer->m_afButtonPressed & IN_ATTACK ) )
			return;
	}

	if ( m_pPlayer->m_afButtonPressed & IN_ATTACK )
	{
		m_pPlayer->m_iShotsFired = 0;
	}

	if (m_pPlayer->pev->velocity.Length2D() > 10)
		FIVESEVENFire( (0.16) * (m_pPlayer->m_flAccuracy), TRUE );
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		FIVESEVENFire( (0.35) * (m_pPlayer->m_flAccuracy), TRUE );
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		FIVESEVENFire( (0.055) * (m_pPlayer->m_flAccuracy), TRUE );
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 10) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		FIVESEVENFire( (0.16) * (m_pPlayer->m_flAccuracy), TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 10) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		FIVESEVENFire( (0.16) * (m_pPlayer->m_flAccuracy), TRUE );
	
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		FIVESEVENFire( (0.35) * (m_pPlayer->m_flAccuracy), TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 10) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		FIVESEVENFire( (0.35) * (m_pPlayer->m_flAccuracy), TRUE );
	
	else
		FIVESEVENFire( (0.055) * (m_pPlayer->m_flAccuracy), TRUE );// 0.03
	
	m_pPlayer->m_iShotsFired++;

}

void CFiveseveN::FIVESEVENFire( float flSpread , BOOL fUseSemi )
{
	/*if ( m_pPlayer->m_iShotsFired > 1)
		m_pPlayer->m_iShotsFired = 0;*/

	//m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired + 1) * 0.055;
	
	/*if ( fUseSemi == TRUE && m_pPlayer->m_iShotsFired > 0 )
	{
		if(!( m_pPlayer->m_afButtonPressed & IN_ATTACK ) )
			return;
	}*/

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
	//m_pPlayer->m_iShotsFired++;
	//ALERT (at_console, "shots fired = %d\n", m_pPlayer->m_iShotsFired);
	m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 250) + 0.3;
	if (m_pPlayer->m_flAccuracy > 0.98)
		m_pPlayer->m_flAccuracy = 0.98;
	//semi-auto code here 

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	int flags;

#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
	
	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming;
	
	vecAiming = gpGlobals->v_forward;

	Vector vecDir;
	
	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 4096, 3, BULLET_PLAYER_P90, 0, 0.9, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );
	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFS, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );
	
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;

	KickBack (1.55, NULL, 1.25, NULL, 0.85, NULL, 1);
}


void CFiveseveN::Reload( void )
{
	int iResult;

	iResult = DefaultReload( G3SG1_MAX_CLIP, FIVESEVEN_RELOAD, 2.65);

	//m_pPlayer->m_iShotsFired = 0;	

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.415;
		//m_pPlayer->m_iShotsFired = 0;
		m_pPlayer->m_i57--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i57 < 0) 
		{
			m_pPlayer->m_i57 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}



void CFiveseveN::WeaponIdle( void )
{
	ResetEmptySound2( );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	//m_pPlayer->m_iShotsFired = 0;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{	
		m_flTimeWeaponIdle = gpGlobals->time + 0.0;
		SendWeaponAnim( FIVESEVEN_IDLE1 );
	}
}

class CFSClip : public CBasePlayerAmmo
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
LINK_ENTITY_TO_CLASS( ammo_FSclip, CFSClip );














