//Updated on: September 7th 2004

// Glock18 9mm handgun
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

enum glock18_e 
{
	GLOCK18_IDLE1 = 0,
	GLOCK18_IDLE2,
	GLOCK18_IDLE3,
	GLOCK18_SHOOT,
	GLOCK18_SHOOT2,
	GLOCK18_SHOOT3,
	GLOCK18_SHOOT_EMPTY,
	GLOCK18_RELOAD,	
	GLOCK18_DRAW,
	GLOCK18_HOLSTER,
	GLOCK18_ADD_SILENCER,
	GLOCK18_DRAW2,
	GLOCK18_RELOAD2,
};

LINK_ENTITY_TO_CLASS( weapon_glock18, CGlock18 );

void CGlock18::Spawn( )
{
	Precache( );
	pev->classname = MAKE_STRING("weapon_glock18");
	m_iId = WEAPON_GLOCK18;
	SET_MODEL(ENT(pev), "models/w_glock18.mdl");

	m_iDefaultAmmo = GLOCK_MAX_CLIP;

	FallInit();// get ready to fall down.
}

void CGlock18::Precache( void )
{
	PRECACHE_MODEL("models/v_glock18.mdl");
	PRECACHE_MODEL("models/w_glock18.mdl");
	PRECACHE_MODEL("models/p_glock18.mdl");

	PRECACHE_SOUND("weapons/glock18-1.wav");
	PRECACHE_SOUND("weapons/glock18-2.wav");
	PRECACHE_SOUND("weapons/clipout1.wav");
	PRECACHE_SOUND("weapons/clipin1.wav");
	PRECACHE_SOUND("weapons/sliderelease1.wav");
	PRECACHE_SOUND("weapons/slideback1.wav");

	m_iShell = PRECACHE_MODEL ("models/9mmshell.mdl");// brass shell
	m_usGlock18 = PRECACHE_EVENT( 1, "events/glock18.sc" );
}

int CGlock18::AddToPlayer( CBasePlayer *pPlayer )
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

int CGlock18::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GLOCK_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_GLOCK18;
	p->iWeight = GLOCK18_WEIGHT;

	return 1;
}

BOOL CGlock18::Deploy( )
{
m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.995;
m_pPlayer->m_flAccuracy = 0.9875;
m_pPlayer->m_bResumeZoom = FALSE;

	if (RANDOM_LONG (0,1) )
		return DefaultDeploy( "models/v_glock18.mdl", "models/p_glock18.mdl", GLOCK18_DRAW, "onehanded" );
	else
		return DefaultDeploy( "models/v_glock18.mdl", "models/p_glock18.mdl", GLOCK18_DRAW2, "onehanded" );
}

void CGlock18::SecondaryAttack( void )
{

	if (m_iBurstFire == 0)
	{
		m_iBurstFire = 1;
		ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "Switched to full-auto" );
	}
	else
	{
		m_iBurstFire = 0;
		ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "Switched to semi-automatic" );
	}

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CGlock18::PrimaryAttack( void )
{
	if (m_iBurstFire == 1)
	{
		if (m_pPlayer->pev->velocity.Length2D() > 10)
			GLOCK18Fire( (0.08) * (m_pPlayer->m_flAccuracy), 0.04615, FALSE );
		else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
			GLOCK18Fire( (0.5) * (m_pPlayer->m_flAccuracy), 0.04615, FALSE );
		else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
			GLOCK18Fire( (0.075) * ( m_pPlayer->m_flAccuracy), 0.04615, FALSE );
	
		else if ( (m_pPlayer->pev->velocity.Length2D() > 10) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
			GLOCK18Fire( (0.2) * (m_pPlayer->m_flAccuracy), 0.04615, FALSE );
		else if ( (m_pPlayer->pev->velocity.Length2D() > 10) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
			GLOCK18Fire( (0.2) * (m_pPlayer->m_flAccuracy), 0.04615, FALSE );
	
		else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
			GLOCK18Fire( (0.5) * (m_pPlayer->m_flAccuracy), 0.04615, FALSE );
		else if ( (m_pPlayer->pev->velocity.Length2D() > 10) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
			GLOCK18Fire( (0.5) * (m_pPlayer->m_flAccuracy), 0.04615, FALSE );
	
		else
			GLOCK18Fire( (0.04) * (m_pPlayer->m_flAccuracy), 0.04615, FALSE );
		}
	
	if (m_iBurstFire == 0)
	{
		if (m_pPlayer->pev->velocity.Length2D() > 10)
			GLOCK18Fire( (0.1) * ( m_pPlayer->m_flAccuracy), 0, TRUE );
		else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
			GLOCK18Fire( (0.5) * ( m_pPlayer->m_flAccuracy), 0, TRUE );
		else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
			GLOCK18Fire( (0.075) * ( m_pPlayer->m_flAccuracy), 0, TRUE );
		else if ( (m_pPlayer->pev->velocity.Length2D() > 10) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
			GLOCK18Fire( (0.2) * ( m_pPlayer->m_flAccuracy), 0, TRUE );
		else if ( (m_pPlayer->pev->velocity.Length2D() > 10) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
			GLOCK18Fire( (0.2) * ( m_pPlayer->m_flAccuracy), 0, TRUE );
		else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
			GLOCK18Fire( (0.5) * ( m_pPlayer->m_flAccuracy), 0, TRUE );
		else if ( (m_pPlayer->pev->velocity.Length2D() > 10) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
			GLOCK18Fire( (0.5) * ( m_pPlayer->m_flAccuracy), 0, TRUE );
		else
		    GLOCK18Fire( (0.05) * ( m_pPlayer->m_flAccuracy), 0, TRUE );
	}
}

void CGlock18::GLOCK18Fire( float flSpread , float flCycleTime, BOOL fUseSemi )
{
	m_bDelayFire = TRUE;

	if ( m_pPlayer->m_iShotsFired > 1)
		m_pPlayer->m_iShotsFired = 0;

	if ( fUseSemi == TRUE && m_pPlayer->m_iShotsFired > 0)
	{
		if(!( m_pPlayer->m_afButtonPressed & IN_ATTACK ) )
		return;
	}

	if (m_pPlayer->m_flLastFire == 0)
		m_pPlayer->m_flLastFire = gpGlobals->time;
	else 
	{
		m_pPlayer->m_flAccuracy = 0.65 + (0.2)*(gpGlobals->time - m_pPlayer->m_flLastFire);

		if (m_pPlayer->m_flAccuracy > 0.48)
			m_pPlayer->m_flAccuracy = 0.48;

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

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = gpGlobals->v_forward;

	Vector vecDir;

	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 4096, 1, BULLET_PLAYER_GLOCK, 0, 0.87, m_pPlayer->pev, m_pPlayer->random_seed, TRUE );
	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usGlock18, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );

	#ifdef CLIENT_DLL
    if ( m_iBurstFire != 0 )
#else
    if ( m_iBurstFire != 0 )
#endif
	{
		m_iBurstFire = 1;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
	}
	
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.15;

	m_pPlayer->pev->punchangle.x -= 0;
}


void CGlock18::Reload( void )
{
	int iResult;

	if (RANDOM_LONG (0,1))
		iResult = DefaultReload( GLOCK_MAX_CLIP, GLOCK18_RELOAD, 2.0);
	else
		iResult = DefaultReload( GLOCK_MAX_CLIP, GLOCK18_RELOAD2, 2.0);

	if (iResult)
	{
		if( (m_pPlayer->m_afButtonPressed & IN_ATTACK) || ( (!(m_pPlayer->m_afButtonPressed & IN_ATTACK)) ) )
		{
			m_pPlayer->m_iShotsFired = 0;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.615;
		m_pPlayer->m_i9mm--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i9mm < 0) 
		{
			m_pPlayer->m_i9mm = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}



void CGlock18::WeaponIdle( void )
{
	ResetEmptySound2( );

	//m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;
	
	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		int iAnim;
		float flRand = RANDOM_FLOAT(0, 1);
		if (flRand <= 0.3 + 0 * 0.75)
		{
			iAnim = GLOCK18_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
		}
		else if (flRand <= 0.6 + 0 * 0.875)
		{
			iAnim = GLOCK18_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
		}
		else
		{
			iAnim = GLOCK18_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
		}
		SendWeaponAnim( iAnim );
	}
}













