//Updated on: December 11th 2003

// Colt M4A1 Carbine Assault Rifle w/ m203
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

enum m4a1_e {
	M4A1_SIDLE = 0,
	M4A1_SSHOOT1,	
	M4A1_SSHOOT2,
	M4A1_SSHOOT3,
	M4A1_SRELOAD,
	M4A1_SDRAW,
	M4A1_ADD_SILENCER,
	M4A1_IDLE,
	M4A1_SHOOT1,	
	M4A1_SHOOT2,
	M4A1_SHOOT3,
	M4A1_RELOAD,
	M4A1_DRAW,
	M4A1_REMOVE_SILENCER,
};

LINK_ENTITY_TO_CLASS( weapon_m4a1, CM4A1 );

int CM4A1::SecondaryAmmoIndex( void )
{
	return m_iSecondaryAmmoType;
}

void CM4A1::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_m4a1");
	Precache( );
	m_iId = WEAPON_M4A1;
	SET_MODEL(ENT(pev), "models/w_m4a1.mdl");

	m_iDefaultAmmo = AK47_MAX_CLIP;

	FallInit();// get ready to fall down.
}

void CM4A1::Precache( void )
{
	//Left Handed Models
	PRECACHE_MODEL("models/v_m4a1.mdl");
	PRECACHE_MODEL("models/w_m4a1.mdl");
	PRECACHE_MODEL("models/p_m4a1.mdl");

	PRECACHE_SOUND("weapons/m4a1-1.wav");
	PRECACHE_SOUND("weapons/m4a1_unsil-1.wav");
	PRECACHE_SOUND("weapons/m4a1_unsil-2.wav");

	PRECACHE_SOUND("weapons/m4a1_deploy.wav");

	PRECACHE_SOUND("weapons/m4a1_boltpull.wav");
	PRECACHE_SOUND("weapons/m4a1_clipin.wav");
	PRECACHE_SOUND("weapons/m4a1_clipout.wav");

	PRECACHE_SOUND( "weapons/glauncher.wav" );
	PRECACHE_SOUND( "weapons/glauncher2.wav" );

	//PRECACHE_SOUND("weapons/m4a1_silencer_on.wav");
	//PRECACHE_SOUND("weapons/m4a1_silencer_off.wav");

	m_iShell = PRECACHE_MODEL ("models/556mmshell.mdl");// brass shell

	m_usM4A1 = PRECACHE_EVENT( 1, "events/m4a1.sc" );
	m_usM4A12 = PRECACHE_EVENT( 1, "events/m4a12.sc" );
}

int CM4A1::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = _556MM_MAXCARRY;
	p->pszAmmo2 = "M203Gren";
	p->iMaxAmmo2 = _M203_MAXCARRY;
	p->iMaxClip = AK47_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 1;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_M4A1;
	p->iWeight = M4A1_WEIGHT;

	return 1;
}

int CM4A1::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CM4A1::Deploy( )
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.675;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.675;
	
	m_pPlayer->m_bResumeZoom = FALSE;
	m_pPlayer->m_iShotsFired = 0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.0;
		
	return DefaultDeploy( "models/v_m4a1.mdl", "models/p_m4a1.mdl", M4A1_DRAW, "mp5" );
}

void CM4A1::SecondaryAttack( void )
{	
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	//m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;
	m_pPlayer->m_flStopExtraSoundTime = UTIL_WeaponTimeBase() + 0.2;
			
	int M203Shot;

    M203Shot =	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

 	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	// we don't add in player velocity anymore.
	CGrenade::ShootM203( m_pPlayer->pev, m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16, 
							gpGlobals->v_forward * 800 );

	if (M203Shot)
	{
		m_pPlayer->m_iM203--; //when shooting, the value goes down by one
		if ( m_pPlayer->m_iM203 < 0) 
		{
			m_pPlayer->m_iM203 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usM4A12 );
	
	//Valve
/*	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5;// idle pretty soon after shooting.*/

	//Haunter
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.05;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1;
}

void CM4A1::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		M4A1Fire( (0.15) * (m_pPlayer->m_flAccuracy), 0.07273, 1);
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		M4A1Fire( (0.35) * (m_pPlayer->m_flAccuracy), 0.07273, 1);
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		M4A1Fire( (0.02) * (m_pPlayer->m_flAccuracy), 0.07273, 1);
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		M4A1Fire( (0.15) * (m_pPlayer->m_flAccuracy), 0.07273, 1);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		M4A1Fire( (0.15) * (m_pPlayer->m_flAccuracy), 0.07273, 1);
	
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		M4A1Fire( (0.35) * (m_pPlayer->m_flAccuracy), 0.07273, 1);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		M4A1Fire( (0.35) * (m_pPlayer->m_flAccuracy), 0.07273, 1);
	
	else
		M4A1Fire( (0.02) * (m_pPlayer->m_flAccuracy), 0.07273, 1);
}

void CM4A1::M4A1Fire( float flSpread , float flCycleTime, int iShotsFired)
{
	m_bDelayFire = TRUE;

	//ALERT (at_console, "shots fired = %i\n", m_pPlayer->m_iShotsFired);

	m_pPlayer->m_iShotsFired++;

	m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 250) + 0.4;
	if (m_pPlayer->m_flAccuracy > 0.975)
		m_pPlayer->m_flAccuracy = 0.975;
	
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

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
	
	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, 3, BULLET_PLAYER_AUG, 0, 0.97, m_pPlayer->pev, m_pPlayer->random_seed, FALSE );
	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usM4A1, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );
		
	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	
	
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.975;

	KickBack (1.0, 0.375, 0.15, 0.05, 6.25, 2.5, 3);
}

void CM4A1::Reload( void )
{
	int iResult;
	
	iResult = DefaultReload( AK47_MAX_CLIP, M4A1_RELOAD, 2.975);
	
	if (iResult)
	{
		if( (m_pPlayer->m_afButtonPressed & IN_ATTACK) || ( (!(m_pPlayer->m_afButtonPressed & IN_ATTACK)) ) )
		{
			m_pPlayer->m_iShotsFired = 0;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.75;
		m_pPlayer->m_i556--;
		if ( m_pPlayer->m_i556 < 0) 
		{
			m_pPlayer->m_i556 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}


void CM4A1::WeaponIdle( void )
{	
	ResetEmptySound( );

	//m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	/*this is to keep in check the idle timing taking place for
	some animations*/
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() ) 
		return;

	m_pPlayer->m_iShotsFired = 0;

	SendWeaponAnim( M4A1_IDLE );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.0;
}

class CM4A1Clip : public CBasePlayerAmmo
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
		int bResult = (pOther->GiveAmmo( 30, "556Nato", _556MM_MAXCARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_M4A1clip, CM4A1Clip );

class CM203Gren : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_ARgrenade.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_ARgrenade.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( 2, "M203Gren", _M203_MAXCARRY ) != -1);

		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_M203Gren, CM203Gren );