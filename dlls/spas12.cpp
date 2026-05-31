// Updated on: February 3rd 2005

//Franchi Spas12 Shotgun
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

enum spas12_e 
{
	SPAS12_IDLE = 0,
	SPAS12_SHOOT1,
	SPAS12_SHOOT2,
	SPAS12_INSERT,
	SPAS12_AFTER_RELOAD,
	SPAS12_START_RELOAD,
	SPAS12_DRAW,
};

LINK_ENTITY_TO_CLASS( weapon_spas12, CSPAS12 );

void CSPAS12::Spawn( )
{
	Precache( );
	m_iId = WEAPON_SPAS12;
	pev->classname = MAKE_STRING("weapon_spas12");
	SET_MODEL(ENT(pev), "models/w_spas12.mdl");

	m_iDefaultAmmo = DEAGLE_MAX_CLIP;

	FallInit();// get ready to fall
}

void CSPAS12::Precache( void )
{
	PRECACHE_MODEL("models/v_spas12.mdl");
	PRECACHE_MODEL("models/w_spas12.mdl");
	PRECACHE_MODEL("models/p_spas12.mdl");

	m_iShell = PRECACHE_MODEL ("models/shotgunshell.mdl");// m3 shell

	PRECACHE_SOUND ("weapons/spas12-1.wav");//m3

	PRECACHE_SOUND ("weapons/spas12_pump.wav");	// m3 pump action
	PRECACHE_SOUND ("weapons/spas12_insertshell.wav");	// m3 reload

	m_usSPAS12 = PRECACHE_EVENT( 1, "events/spas12.sc" );
}

int CSPAS12::AddToPlayer( CBasePlayer *pPlayer )
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

int CSPAS12::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "slugs";
	p->iMaxAmmo1 = SLUGS_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = DEAGLE_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 7;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_SPAS12;
	p->iWeight = SPAS12_WEIGHT;

	return 1;
}

BOOL CSPAS12::Deploy( )
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.775;
	m_pPlayer->m_bResumeZoom = FALSE;
//	g_engfuncs.pfnSetClientMaxspeed( ENT( m_pPlayer->pev ), 200 );
	return DefaultDeploy( "models/v_spas12.mdl", "models/p_spas12.mdl", SPAS12_DRAW, "shotgun" );
}

void CSPAS12::PrimaryAttack()
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

	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, VECTOR_CONE_2DEGREES, 2048, 3, BULLET_PLAYER_SLUG, 0, 0.8, m_pPlayer->pev, m_pPlayer->random_seed, FALSE );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usSPAS12, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	if (m_iClip != 0)
		m_flPumpTime = gpGlobals->time + 0.5;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.875;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
	
	m_fInReload = 0;

	m_pPlayer->pev->punchangle.x -= 1;
}

void CSPAS12::Reload( void )
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == DEAGLE_MAX_CLIP)
		return;

	// don't reload until recoil is done
	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	// check to see if we're ready to reload
	if (m_fInReload == 0)
	{
		SendWeaponAnim( SPAS12_START_RELOAD );
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

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/spas12_insertshell.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0,0x1f));

		SendWeaponAnim( SPAS12_INSERT );

		m_flNextReload = UTIL_WeaponTimeBase() + 0.475;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
	}
	else
	{
		// Add them to the clip
		m_iClip += 1;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 1;
		m_pPlayer->m_iSlug--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_iSlug < 0) 
		{
			m_pPlayer->m_iSlug = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
		m_fInReload = 1;
	}
}


void CSPAS12::WeaponIdle( void )
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
			if (m_iClip != DEAGLE_MAX_CLIP && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				Reload( );
			}
			else
			{
				// reload debounce has timed out
				SendWeaponAnim( SPAS12_AFTER_RELOAD );
				
				m_fInReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
			}
		}
		else
		{
			SendWeaponAnim( SPAS12_IDLE );
		}
	}
}

class CSPAS12Clip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_shotbox.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_shotbox.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( 7, "slugs", SLUGS_MAXCARRY ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_SPAS12clip, CSPAS12Clip );


