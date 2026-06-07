//Updated on: November 26th 2003

// Heckler & Koch Universal Self-loading Pistol(USP)
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

enum usp_e {
	USP_SIDLE = 0,
	USP_SSHOOT,
	USP_SSHOOT2,
	USP_SSHOOT3,
	USP_SSHOOTLAST,
	USP_SRELOAD,	
	USP_SDRAW,
	USP_ADD_SILENCER,
	USP_IDLE,
	USP_SHOOT,
	USP_SHOOT2,
	USP_SHOOT3,
	USP_SHOOTLAST,
	USP_RELOAD,	
	USP_DRAW,
	USP_REMOVE_SILENCER,
};

LINK_ENTITY_TO_CLASS( weapon_usp, CUSP );

void CUSP::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_usp");
	Precache( );
	m_iId = WEAPON_USP;
	SET_MODEL(ENT(pev), "models/w_usp.mdl");

	m_bIsSilencing = 0;
	m_iSilenced = 0; //to define on startup if the silencer added or not

	m_iDefaultAmmo = USP_MAX_CLIP;

	FallInit();// get ready to fall down.
}

void CUSP::Precache( void )
{
	PRECACHE_MODEL("models/v_usp.mdl");
	PRECACHE_MODEL("models/w_usp.mdl");
	PRECACHE_MODEL("models/p_usp.mdl");

	PRECACHE_SOUND("weapons/usp1.wav");//Silenced
	PRECACHE_SOUND("weapons/usp2.wav");//Silenced
	PRECACHE_SOUND("weapons/usp_unsil-1.wav");//Unsilenced

	PRECACHE_SOUND("weapons/usp_clipout.wav");
	PRECACHE_SOUND("weapons/usp_clipin.wav");
	PRECACHE_SOUND("weapons/usp_sliderelease.wav");
	PRECACHE_SOUND("weapons/usp_slideback.wav");

	PRECACHE_SOUND("weapons/usp_silencer_on.wav");
	PRECACHE_SOUND("weapons/usp_silencer_off.wav");

	PRECACHE_SOUND ("weapons/dryfire_pistol.wav");

	m_iShell = PRECACHE_MODEL ("models/45calshell.mdl");// brass shell
	m_usUSP = PRECACHE_EVENT( 1, "events/usp.sc" );
}

int CUSP::AddToPlayer( CBasePlayer *pPlayer )
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

int CUSP::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "45ACP";
	p->iMaxAmmo1 = _45ACP_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = USP_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_USP;
	p->iWeight = USP_WEIGHT;

	return 1;
}

BOOL CUSP::Deploy( )
{
	/*if (m_bIsSilencing == 0)
	{*/
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.955;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.955;
	/*}
	else if (m_bIsSilencing == 1 )
	{
		if (m_flNextPrimaryAttack <= UTIL_WeaponTimeBase() && m_flNextSecondaryAttack <= UTIL_WeaponTimeBase())
		{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.975;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.975;
		}

		m_bIsSilencing = 0;
	}*/
			
	m_pPlayer->m_bResumeZoom = FALSE;
	//m_pPlayer->m_flAccuracy = 0.98;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.0;
	
	if (m_iSilenced == 1)
	{
		return DefaultDeploy( "models/v_usp.mdl", "models/p_usp.mdl", USP_SDRAW, "onehanded" );
	}
	else
	{
        return DefaultDeploy( "models/v_usp.mdl", "models/p_usp.mdl", USP_DRAW, "onehanded" );
	}
}

void CUSP::SecondaryAttack( void )
{
//	int iResult;

//	m_pPlayer->m_iSilencing = 1;
	m_bIsSilencing = 1;

	if (m_iSilenced == 1)
	{
		m_iSilenced = 0;
		SendWeaponAnim( USP_REMOVE_SILENCER );
	}
	else
	{
		m_iSilenced = 1;
		SendWeaponAnim( USP_ADD_SILENCER );
	}

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.975;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.15;

	/*if (m_flTimeWeaponIdle <= 0.0)
	{
		m_pPlayer->m_bIsSilencing = FALSE;
	}*/

	/*if ( m_pPlayer->m_bIsSilencing == TRUE)
	{
        if (m_iSilenced == 0)
		{
			iResult = EquipSilencer( USP_ADD_SILENCER, 0.0);
		}
		else if (m_iSilenced == 1 )
		{
			iResult = EquipSilencer( USP_REMOVE_SILENCER, 0.0);
		}

		if (iResult)
		{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.975;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.975;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.075;
		}
	}*/
}

void CUSP::Holster( void )
{
	m_fInSilencing = FALSE; //if holstered stop equipping silencer
	//m_pPlayer->m_iSilencing = 0;
	//m_pPlayer->m_bIsSilencing = FALSE;
}

void CUSP::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		USPFire( (0.13) * ( m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		USPFire( (0.4) * ( m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		USPFire( (0.055) * ( m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		USPFire( (0.13) * ( m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		USPFire( (0.13) * ( m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		USPFire( (0.4) * ( m_pPlayer->m_flAccuracy), 0, TRUE );
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		USPFire( (0.4) * ( m_pPlayer->m_flAccuracy), 0, TRUE );
	else
		USPFire( (0.075) * ( m_pPlayer->m_flAccuracy), 0, TRUE );
}

void CUSP::USPFire( float flSpread , float flCycleTime, BOOL fUseSemi )
{
	if ( m_pPlayer->m_iShotsFired > 1)
		m_pPlayer->m_iShotsFired = 0;

	if ( fUseSemi == TRUE && m_pPlayer->m_iShotsFired > 0)
	{
		if(!( m_pPlayer->m_afButtonPressed & IN_ATTACK ) )
		return;
	}
	
	/*if (m_pPlayer->m_flLastFire == 0)
		m_pPlayer->m_flLastFire = gpGlobals->time;
	else 
	{
		m_pPlayer->m_flAccuracy = 0.7 + (0.3)*(gpGlobals->time - m_pPlayer->m_flLastFire);

		if (m_pPlayer->m_flAccuracy > 0.98)
			m_pPlayer->m_flAccuracy = 0.98;

		m_pPlayer->m_flLastFire = gpGlobals->time;
	}*/

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
	
	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = gpGlobals->v_forward;
	Vector vecDir;

#ifdef CLIENT_DLL
    if ( m_iSilenced == 0 )
#else
    if ( m_iSilenced == 0 )
#endif
	{
		m_iSilenced = 0; //No silencer

		vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 4096, 1, BULLET_PLAYER_USP, 0, 0.83, m_pPlayer->pev, m_pPlayer->random_seed, TRUE);
		PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usUSP, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );
	
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	}
	
	if ( m_iSilenced != 0 )//Silencer added
    {
		m_iSilenced = 1;
		
		vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 4096, 1, BULLET_PLAYER_USP, 0, 0.83, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );
		PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usUSP, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 1, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );
		
		//silenced
		m_pPlayer->m_iWeaponVolume = 80;
		m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	m_pPlayer->pev->punchangle.x -= 1;
//	KickBack (1.975, NULL, 1.25, NULL, 3.25, NULL, 1);
}


void CUSP::Reload( void )
{
	int iResult;

	if (m_iSilenced == 0)
		iResult = DefaultReload( USP_MAX_CLIP, USP_RELOAD, 2.5);
	else
		iResult = DefaultReload( USP_MAX_CLIP, USP_SRELOAD, 2.5);

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.075;
		m_pPlayer->m_i45--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i45 < 0) 
		{
			m_pPlayer->m_i45 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}

void CUSP::WeaponIdle( void )
{
	ResetEmptySound2( );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	//m_pPlayer->m_iSilencing = 0;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		int iAnim;
		if (m_iSilenced == 0)
		{
			iAnim = USP_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.0;
		}
		else
		{
			iAnim = USP_SIDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.0;
		}
		SendWeaponAnim( iAnim );
	}
}

class CUSPClip : public CBasePlayerAmmo
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
		int bResult = (pOther->GiveAmmo( 12, "45ACP", _45ACP_MAXCARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_USPclip, CUSPClip );














