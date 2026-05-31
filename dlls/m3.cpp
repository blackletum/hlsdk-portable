// Updated on: January 3rd 2004

//Benelli M3 Super 90
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"
#include "maxcarry.h"

// special deathmatch shotgun spreads
#define VECTOR_CONE_DM_SHOTGUN	Vector( 0.08716, 0.04362, 0.00  )// 10 degrees by 5 degrees
#define VECTOR_CONE_DM_DOUBLESHOTGUN Vector( 0.17365, 0.04362, 0.00 ) // 20 degrees by 5 degrees

enum m3_e 
{
	M3_IDLE = 0,
	M3_SHOOT1,
	M3_SHOOT2,
	M3_INSERT,
	M3_AFTER_RELOAD,
	M3_START_RELOAD,
	M3_DRAW,
};

LINK_ENTITY_TO_CLASS( weapon_m3, CM3 );

void CM3::Spawn( )
{
	Precache( );
	m_iId = WEAPON_M3;
	pev->classname = MAKE_STRING("weapon_m3");
	SET_MODEL(ENT(pev), "models/w_m3.mdl");

	m_iDefaultAmmo = M3_MAX_CLIP;

	FallInit();// get ready to fall
}

void CM3::Precache( void )
{
	PRECACHE_MODEL("models/v_m3.mdl");
	PRECACHE_MODEL("models/w_m3.mdl");
	PRECACHE_MODEL("models/p_m3.mdl");

	m_iShell = PRECACHE_MODEL ("models/shotgunshell.mdl");// m3 shell

	PRECACHE_SOUND ("weapons/m3-1.wav");//m3

	PRECACHE_SOUND ("weapons/m3_pump.wav");	// m3 pump action
	PRECACHE_SOUND ("weapons/m3_insertshell.wav");	// m3 reload

	m_usM3 = PRECACHE_EVENT( 1, "events/m3.sc" );
}

int CM3::AddToPlayer( CBasePlayer *pPlayer )
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

int CM3::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "buckshot";
	p->iMaxAmmo1 = BUCKSHOT_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = M3_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 5;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_M3;
	p->iWeight = M3_WEIGHT;

	return 1;
}

BOOL CM3::Deploy( )
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.855;
	m_pPlayer->m_bResumeZoom = FALSE;
	return DefaultDeploy( "models/v_m3.mdl", "models/p_m3.mdl", M3_DRAW, "shotgun" );
}

void CM3::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		return;
	}

	if (m_iClip <= 0)
	{
		Reload( );
		if (m_iClip == 0)
		{
			PlayEmptySound( );
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = gpGlobals->v_forward;

	Vector vecDir;

	vecDir = m_pPlayer->FireBulletsPlayer( 8, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 2048, BULLET_PLAYER_XM1014, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usM3, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	if (m_iClip != 0)
		m_flPumpTime = gpGlobals->time + 0.5;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.875;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
	
	m_fInReload = 0;

	m_pPlayer->pev->punchangle.x -= 1;
}

void CM3::Reload( void )
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == M3_MAX_CLIP)
		return;

	// don't reload until recoil is done
	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	// check to see if we're ready to reload
	if (m_fInReload == 0)
	{
		SendWeaponAnim( M3_START_RELOAD );
		m_fInReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.6;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6;
	//	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
		return;
	}
	else if (m_fInReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;
		// was waiting for gun to move to side
		m_fInReload = 2;

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/m3_insertshell.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0,0x1f));

		SendWeaponAnim( M3_INSERT );

		m_flNextReload = UTIL_WeaponTimeBase() + 0.475;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
	}
	else
	{
		// Add them to the clip
		m_iClip += 1;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 1;
		m_pPlayer->m_i12G--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i12G < 0) 
		{
			m_pPlayer->m_i12G = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
		m_fInReload = 1;
	}
}


void CM3::WeaponIdle( void )
{
	ResetEmptySound( );

	if ( m_flPumpTime && m_flPumpTime < gpGlobals->time )
	{
		m_flPumpTime = 0;
	}

	if (m_flTimeWeaponIdle <  UTIL_WeaponTimeBase() )
	{
		if (m_iClip == 0 && m_fInReload == 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			Reload( );
		}
		else if (m_fInReload != 0)
		{
			if (m_iClip != M3_MAX_CLIP && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				Reload( );
			}
			else
			{
				// reload debounce has timed out
				SendWeaponAnim( M3_AFTER_RELOAD );
				
				m_fInReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
			}
		}
		else
		{
			SendWeaponAnim( M3_IDLE );
		}
	}
}


