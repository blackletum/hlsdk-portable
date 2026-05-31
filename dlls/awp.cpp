//Updated on: December 11th 2003

//Arctic WarFare Magnum
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "monsters.h"
#include "player.h"
#include "gamerules.h"
#include "maxcarry.h"

enum awp_e 
{
	AWP_IDLE = 0,
	AWP_SHOOT1,
	AWP_SHOOT2,
	AWP_SHOOT3,
	AWP_RELOAD,	
	AWP_DRAW,
};

LINK_ENTITY_TO_CLASS( weapon_awp, CAWP );

int CAWP::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "338Magnum";
	p->iMaxAmmo1 = _338MAG_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AWP_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 7;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_AWP;
	p->iWeight = AWP_WEIGHT;

	return 1;
}

int CAWP::AddToPlayer( CBasePlayer *pPlayer )
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

void CAWP::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_awp"); // hack to allow for old names
	Precache( );
	m_iId = WEAPON_AWP;
	SET_MODEL(ENT(pev), "models/w_awp.mdl");

	m_iDefaultAmmo = AWP_MAX_CLIP;

	FallInit();// get ready to fall down.
}


void CAWP::Precache( void )
{
	PRECACHE_MODEL("models/v_awp.mdl");
	PRECACHE_MODEL("models/w_awp.mdl");
	PRECACHE_MODEL("models/p_awp.mdl");

	PRECACHE_SOUND("weapons/awp1.wav");
	PRECACHE_SOUND("weapons/boltpull1.wav");
	PRECACHE_SOUND("weapons/boltup.wav");
	PRECACHE_SOUND("weapons/boltdown.wav");
	PRECACHE_SOUND("weapons/zoom.wav");

	PRECACHE_SOUND("weapons/awp_deploy.wav");
	PRECACHE_SOUND("weapons/awp_clipin.wav");
	PRECACHE_SOUND("weapons/awp_clipout.wav");
	
	m_iShell = PRECACHE_MODEL ("models/762mmshell.mdl");// brass shell
	m_usAWP = PRECACHE_EVENT( 1, "events/awp.sc" );
}

BOOL CAWP::Deploy( )
{
    /*if ( m_pPlayer->m_iFire == 0)
	{*/
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.15;
	/*}
	else if ( m_pPlayer->m_iFire == 1)
	{
		m_pPlayer->m_iFire = 0;
	}*/
    
	m_pPlayer->m_bResumeZoom = FALSE;
    return DefaultDeploy( "models/v_awp.mdl", "models/p_awp.mdl", AWP_DRAW, "sniper" );
}

void CAWP::SecondaryAttack( void )
{
	if ( m_pPlayer->pev->fov == 0 )
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 45;
	}
	else if ( m_pPlayer->pev->fov == 45 )
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV =  10;
	}
	else 
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;  // 0 means reset to default fov
		m_pPlayer->m_bResumeZoom = FALSE;
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_awp.mdl");
	}

	/*if (m_pPlayer->pev->fov == 0)
	{
		m_pPlayer->pev->fov = 45;
	}
	else if (m_pPlayer->pev->fov == 45)
	{
		m_pPlayer->pev->fov = 10;
	}
	else 
	{
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_awp.mdl");
		m_pPlayer->pev->fov = 0;
	}*/

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/zoom.wav", 1, ATTN_NORM);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CAWP::PrimaryAttack( void )
{
	int iCycletime = 1.25;

		if (m_pPlayer->pev->velocity.Length2D() > 0)
			AWPFire( 0.195, iCycletime);
		else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
			AWPFire( 0.35, iCycletime);
		else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
			AWPFire( 0.0, iCycletime);
		
		else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
			AWPFire( 0.195, iCycletime);
		else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
			AWPFire( 0.195, iCycletime);
	
		else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
			AWPFire( 0.35, iCycletime);
		else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
			AWPFire( 0.35, iCycletime);
			
		else
			AWPFire( 0.0, iCycletime);
}

void CAWP::AWPFire( float flSpread , float flCycleTime)
{
	m_pPlayer->m_iFire = 1;

	if ( m_pPlayer->pev->fov != 0)
	{	
		// resume the light level to normal.. and return the view model.
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_awp.mdl");

		m_pPlayer->m_bResumeZoom = TRUE;
		m_pPlayer->m_iLastZoom = m_pPlayer->pev->fov;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
	}
	/*else if ( m_pPlayer->pev->fov == 0 )
	{
		flSpread += 0.155;
	}*/

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );


	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = gpGlobals->v_forward;

	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, 3, BULLET_PLAYER_AWP, 0, 0.99, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );

    int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usAWP, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );


	m_flNextPrimaryAttack =  m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.25;
	
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	m_pPlayer->pev->punchangle.x -= 2;
}

void CAWP::Reload( void )
{
	int iResult;

	iResult = DefaultReload( AWP_MAX_CLIP, AWP_RELOAD, 2.75);

	if (m_pPlayer->pev->fov != 0)
	{
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_awp.mdl");

		m_pPlayer->m_iLastZoom = m_pPlayer->pev->fov;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
		m_pPlayer->m_bResumeZoom = FALSE;
		
	}


	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.875;
		m_pPlayer->m_i338--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i338 < 0) 
		{
			m_pPlayer->m_i338 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}

void CAWP::WeaponIdle( void )
{
	ResetEmptySound( );

	//m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	SendWeaponAnim( AWP_IDLE );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.

}

class CAWPClip : public CBasePlayerAmmo
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
		int bResult = (pOther->GiveAmmo( 10, "338Magnum", _338MAG_MAXCARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_AWPclip, CAWPClip );
