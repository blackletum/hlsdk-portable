//Updated on: December 10th 2003

//Avtomat Kalashnikov AK-47
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

enum ak47_e 
{
	AK47_IDLE1 = 0,
	AK47_RELOAD,	
	AK47_DRAW,
	AK47_SHOOT1,
	AK47_SHOOT2,
	AK47_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_ak47, CAK47 );

void CAK47::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_ak47");
	Precache( );
	m_iId = WEAPON_AK47;
	SET_MODEL(ENT(pev), "models/w_ak47.mdl");

	m_iDefaultAmmo = AK47_MAX_CLIP;

	FallInit();// get ready to fall down.
}

void CAK47::Precache( void )
{
	PRECACHE_MODEL("models/v_ak47.mdl");
	PRECACHE_MODEL("models/w_ak47.mdl");
	PRECACHE_MODEL("models/p_ak47.mdl");

	PRECACHE_SOUND("weapons/ak47-1.wav");
	PRECACHE_SOUND("weapons/ak47-2.wav");
	PRECACHE_SOUND("weapons/ak47_clipout.wav");
	PRECACHE_SOUND("weapons/ak47_clipin.wav");
	PRECACHE_SOUND("weapons/ak47_boltpull.wav");

	PRECACHE_SOUND ("weapons/dryfire_rifle.wav");

	m_iShell = PRECACHE_MODEL ("models/762mmshell.mdl");// brass shell

	m_usAK47 = PRECACHE_EVENT( 1, "events/ak47.sc" );
}

int CAK47::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "762Nato";
	p->iMaxAmmo1 = _762MM_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AK47_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_AK47;
	p->iWeight = AK47_WEIGHT;

	return 1;
}

int CAK47::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CAK47::Deploy( )
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.95;
	m_pPlayer->m_bResumeZoom = FALSE;
	iShellOn = 1;
	return DefaultDeploy( "models/v_ak47.mdl", "models/p_ak47.mdl", AK47_DRAW, "mp5" );
}

void CAK47::PrimaryAttack( void )
{
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		AK47Fire( (0.15) * (m_pPlayer->m_flAccuracy), 0.07742);
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		AK47Fire( (0.515) * (m_pPlayer->m_flAccuracy), 0.07742);
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		AK47Fire( (0.02) * (m_pPlayer->m_flAccuracy), 0.07742);

	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		AK47Fire( (0.15) * (m_pPlayer->m_flAccuracy), 0.07742);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		AK47Fire( (0.15) * (m_pPlayer->m_flAccuracy), 0.07742);
	
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		AK47Fire( (0.515) * (m_pPlayer->m_flAccuracy), 0.07742);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		AK47Fire( (0.515) * (m_pPlayer->m_flAccuracy), 0.07742);
	
	else
		AK47Fire( (0.0250) * (m_pPlayer->m_flAccuracy), 0.07742);
}

void CAK47::AK47Fire( float flSpread , float flCycleTime)
{
	m_bDelayFire = TRUE;

	m_pPlayer->m_iShotsFired++;

	m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 200) + 0.35;
	if (m_pPlayer->m_flAccuracy > 0.75)
		m_pPlayer->m_flAccuracy = 0.75;

	//ALERT (at_console, "Accuracy= %f\n", m_pPlayer->m_flAccuracy);
	
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

	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, 3, BULLET_PLAYER_AK47, 0, 0.98, m_pPlayer->pev, m_pPlayer->random_seed, FALSE );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usAK47, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	KickBack (1.15, 0.375, 0.25, 0.05, 10.5, 4.75, 2);
}

void CAK47::Reload( void )
{
	int iResult;

	iResult = DefaultReload( AK47_MAX_CLIP, AK47_RELOAD, 2.275);

	if (iResult)
	{
		if( (m_pPlayer->m_afButtonPressed & IN_ATTACK) || ( (!(m_pPlayer->m_afButtonPressed & IN_ATTACK)) ) )
		{
			m_pPlayer->m_iShotsFired = 0;
		}
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.15;
		m_pPlayer->m_i762--; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i762 < 0) 
		{
			m_pPlayer->m_i762 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}


void CAK47::WeaponIdle( void )
{	
	ResetEmptySound( );
	
	//Haunter
	//m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	//Haunter

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;

	// only idle if the slid isn't back
	SendWeaponAnim( AK47_IDLE1 );
}

class CAK47Clip : public CBasePlayerAmmo
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
		int bResult = (pOther->GiveAmmo( AK47_MAX_CLIP, "762Nato", _762MM_MAXCARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_AK47clip, CAK47Clip );