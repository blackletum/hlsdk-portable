//Updated on: March 10th 2004

//Ebony & Ivory
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

enum EI_e {
	EI_IDLE1 = 0,
	EI_IDLE2,
	EI_IDLE3,
	EI_SHOOT_R,
	EI_SHOOT_L,
	EI_SHOOT_EMPTY_R,
	EI_SHOOT_EMPTY_L,
	EI_RELOAD,
	EI_RELOAD_NOSHOT,
	EI_DRAW,
	EI_HOLSTER,
	EI_IDLE_EMPTY,
	EI_IDLE_RIGHT_EMPTY,
};

LINK_ENTITY_TO_CLASS( weapon_EI, CEI );

void CEI::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_EI");
	Precache( );
	m_iId = WEAPON_EI;
	SET_MODEL(ENT(pev), "models/w_dante.mdl");

	m_iDefaultAmmo = 30;

	FallInit();// get ready to fall down.
}


void CEI::Precache( void )
{
	PRECACHE_MODEL("models/v_dante.mdl");
	PRECACHE_MODEL("models/w_dante.mdl");
	PRECACHE_MODEL("models/p_dante.mdl");

	PRECACHE_SOUND("anaconda/close.wav");
	PRECACHE_SOUND("colt1911/fire.wav");
	PRECACHE_SOUND("colt1911/slide.wav");
	PRECACHE_SOUND("colt1911/clip_in.wav");
	PRECACHE_SOUND("colt1911/clip_out.wav");
	PRECACHE_SOUND("colt1911/switch.wav");

	m_iShell = PRECACHE_MODEL ("models/pshell.mdl");// brass shell
	m_usEILEFT = PRECACHE_EVENT( 1, "events/ei_left.sc" );
	m_usEIRIGHT = PRECACHE_EVENT( 1, "events/ei_right.sc" );
}

int CEI::AddToPlayer( CBasePlayer *pPlayer )
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

int CEI::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "13mm";
	p->iMaxAmmo1 = EI_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AK47_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 6;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_EI;
	p->iWeight = EI_WEIGHT;

	return 1;
}

BOOL CEI::Deploy( )
{
	// pev->body = 1;
m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.745;
m_pPlayer->m_bResumeZoom = FALSE;
g_engfuncs.pfnSetClientMaxspeed( ENT( m_pPlayer->pev ), 930 );
return DefaultDeploy( "models/v_dante.mdl", "models/p_dante.mdl", EI_DRAW, "dualpistols" );
}

void CEI::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		EIFire( (0.1) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		EIFire( (0.4) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		EIFire( (0.075) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		EIFire( (0.1) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		EIFire( (0.1) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		EIFire( (0.4) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		EIFire( (0.4) * (m_pPlayer->m_flAccuracy), NULL, TRUE );

	else
		EIFire( (0.085) * (m_pPlayer->m_flAccuracy), NULL, TRUE );
}

void CEI::EIFire( float flSpread , float flCycleTime, BOOL fUseSemi )
{
	int right;
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
			right = 1;
		}
		
		if (m_iClip == 1)
		{
			right = 1;
		}
	}

		// player "shoot" animation
	if (right == 1)
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
	
	if ( right == 1)
	{
		vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecRight, vecAiming, Vector( flSpread, flSpread, flSpread ), 2048, 3, BULLET_PLAYER_13MM, 0, 0.83, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );
		PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usEIRIGHT, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );
	}
	else
	{
		vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecLeft, vecAiming, Vector( flSpread, flSpread, flSpread ), 2048, 3, BULLET_PLAYER_13MM, 0, 0.83, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );
		PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usEILEFT, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;

	KickBack (0.6, 0.35, 0.12, 0.015, 4.5, 1.5, 10);
}


void CEI::Reload( void )
{
	int iResult;

	iResult = DefaultReload( 30, EI_RELOAD, 2.365);

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
		m_pPlayer->m_i13mm--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i13mm < 0) 
		{
			m_pPlayer->m_i13mm = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
	m_bDelayFire = TRUE;
}



void CEI::WeaponIdle( void )
{
	ResetEmptySound2( );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;
	
	// only idle if the slid isn't back
	if (m_iClip == 1)
	{	
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
		SendWeaponAnim( EI_IDLE_RIGHT_EMPTY );
	}
}

class CEIClip : public CBasePlayerAmmo
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
		int bResult = (pOther->GiveAmmo( 30, "13MM", EI_MAXCARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_EIclip, CEIClip );