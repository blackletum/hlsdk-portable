//Updated on: February 20th 2004

//Benelli M4 Super 90
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"
#include "maxcarry.h"

enum xm1014_e {
	XM1014_IDLE = 0,
	XM1014_FIRE1,
	XM1014_FIRE2,
	XM1014_INSERT,
	XM1014_AFTER_RELOAD,
	XM1014_START_RELOAD,
	XM1014_DRAW,
};

LINK_ENTITY_TO_CLASS( weapon_xm1014, CXM1014 );

void CXM1014::Spawn( )
{
	Precache( );
	m_iId = WEAPON_XM1014;
	pev->classname = MAKE_STRING("weapon_xm1014");
	SET_MODEL(ENT(pev), "models/w_xm1014.mdl");

	m_iDefaultAmmo = DEAGLE_MAX_CLIP;

	FallInit();// get ready to fall
}

void CXM1014::Precache( void )
{
	PRECACHE_MODEL("models/v_xm1014.mdl");
	PRECACHE_MODEL("models/w_xm1014.mdl");
	PRECACHE_MODEL("models/p_xm1014.mdl");

	m_iShell = PRECACHE_MODEL ("models/shotgunshell.mdl");// xm1014 shell

	PRECACHE_SOUND ("weapons/xm1014-1.wav");//xm1014
	
	PRECACHE_SOUND ("weapons/sshell1.wav");	// shotgun reload - played on client
	PRECACHE_SOUND ("weapons/sshell2.wav");
	PRECACHE_SOUND ("weapons/sshell3.wav");
	
	PRECACHE_SOUND ("weapons/reload1.wav");	// xm1014 reload

	m_usXM1014 = PRECACHE_EVENT( 1, "events/xm1014.sc" );
}

int CXM1014::AddToPlayer( CBasePlayer *pPlayer )
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

int CXM1014::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "buckshot";
	p->iMaxAmmo1 = BUCKSHOT_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = DEAGLE_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 6;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_XM1014;
	p->iWeight = XM1014_WEIGHT;

	return 1;
}

BOOL CXM1014::Deploy( )
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.775;
	m_pPlayer->m_bResumeZoom = FALSE;
	return DefaultDeploy( "models/v_xm1014.mdl", "models/p_xm1014.mdl", XM1014_DRAW, "shotgun" );
}

void CXM1014::PrimaryAttack()
{
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

	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	m_iClip--;
	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = gpGlobals->v_forward;

	Vector vecDir;

	vecDir = m_pPlayer->FireBulletsPlayer( 8, vecSrc, vecAiming, VECTOR_CONE_7DEGREES, 2048, BULLET_PLAYER_XM1014, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usXM1014, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.25;

	if (m_iClip != 0)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
	else
		m_flTimeWeaponIdle = 0.75;
	
	m_fInReload = 0;

	m_pPlayer->pev->punchangle.x -= 1.75;
}

void CXM1014::Reload( void )
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == DEAGLE_MAX_CLIP)
		return;

	// don't reload until recoil is done
	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	// check to see if we're ready to reload
	if (m_fInReload == 0)
	{
		SendWeaponAnim( XM1014_START_RELOAD );
		m_fInReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.4;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.4;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4;
		return;
	}
	else if (m_fInReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;
		// was waiting for gun to move to side
		m_fInReload = 2;

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/reload1.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0,0x1f));

		SendWeaponAnim( XM1014_INSERT );

		m_flNextReload = UTIL_WeaponTimeBase() + 0.01;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.265;
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


void CXM1014::WeaponIdle( void )
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
				SendWeaponAnim( XM1014_AFTER_RELOAD );
				m_fInReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
			}
		}
		else
		{
			SendWeaponAnim( XM1014_IDLE );
		}
	}
}

class CXM4Clip : public CBasePlayerAmmo
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
		if (pOther->GiveAmmo( 8, "buckshot", BUCKSHOT_MAXCARRY ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_XM4clip, CXM4Clip );