//Updated on: February 11th 2004

//Fabrique Nationale M249 Para
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

enum m249_e 
{
	M249_IDLE1 = 0,
	M249_SHOOT1,
	M249_SHOOT2,
	M249_RELOAD,	
	M249_DRAW,
};

LINK_ENTITY_TO_CLASS( weapon_m249, CM249 );

void CM249::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_m249");
	Precache( );
	SET_MODEL(ENT(pev), "models/w_m249.mdl");
	m_iId = WEAPON_M249;
	
	m_iDefaultAmmo = M249_MAX_CLIP;

	FallInit();// get ready to fall down.
}

void CM249::Precache( void )
{
	PRECACHE_MODEL("models/v_m249.mdl");
	PRECACHE_MODEL("models/w_m249.mdl");
	PRECACHE_MODEL("models/p_m249.mdl");

	PRECACHE_SOUND("weapons/m249-1.wav");
	PRECACHE_SOUND("weapons/m249-2.wav");
	PRECACHE_SOUND("weapons/m249_boxout.wav");
	PRECACHE_SOUND("weapons/m249_boxin.wav");
	PRECACHE_SOUND("weapons/m249_chain.wav");
	PRECACHE_SOUND("weapons/m249_coverup.wav");
	PRECACHE_SOUND("weapons/m249_coverdown.wav");

	m_iShell = PRECACHE_MODEL ("models/556mmshell.mdl");// brass shell

	m_usM249 = PRECACHE_EVENT( 1, "events/m249.sc" );
}

int CM249::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato2"; //Haunter So that it will have different rounds
	p->iMaxAmmo1 = _556MM2_MAXCARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = M249_MAX_CLIP;
	p->iSlot = 4;
	p->iPosition = 4;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_M249;
	p->iWeight = M249_WEIGHT;

	return 1;
}

int CM249::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CM249::Deploy( )
{
	m_pPlayer->m_flAccuracy = 0.0;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.775;
	m_pPlayer->m_bResumeZoom = FALSE;
	iShellOn = 1;
	return DefaultDeploy( "models/v_m249.mdl", "models/p_m249.mdl", M249_DRAW, "mp5" );
}

void CM249::PrimaryAttack( void )
{	
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		M249Fire( (0.1) * (m_pPlayer->m_flAccuracy), 0.07059);
	else if ( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
		M249Fire( (0.35) * (m_pPlayer->m_flAccuracy), 0.07059);
	else if ( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
		M249Fire( (0.02) * (m_pPlayer->m_flAccuracy), 0.07059);
	
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		M249Fire( (0.1) * (m_pPlayer->m_flAccuracy), 0.07059);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) )
		M249Fire( (0.1) * (m_pPlayer->m_flAccuracy), 0.07059);
	
	else if ( (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		M249Fire( (0.35) * (m_pPlayer->m_flAccuracy), 0.07059);
	else if ( (m_pPlayer->pev->velocity.Length2D() > 0) && (!FBitSet( m_pPlayer->pev->flags, FL_ONGROUND )) && (FBitSet( m_pPlayer->pev->flags, FL_DUCKING )) )
		M249Fire( (0.35) * (m_pPlayer->m_flAccuracy), 0.07059);

	else
		M249Fire( (0.02) * (m_pPlayer->m_flAccuracy), 0.07059);
}

void CM249::M249Fire( float flSpread , float flCycleTime)
{
	m_bDelayFire = TRUE;
	
	m_pPlayer->m_iShotsFired++;
	
	m_pPlayer->m_flAccuracy = (m_pPlayer->m_iShotsFired * m_pPlayer->m_iShotsFired / 250) + 0.4;
	if (m_pPlayer->m_flAccuracy > 0.48)
		m_pPlayer->m_flAccuracy = 0.48;

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

	vecDir = m_pPlayer->FireBulletsPlayer2( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, 3, BULLET_PLAYER_SG552, 0, 0.8, m_pPlayer->pev, m_pPlayer->random_seed, FALSE );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usM249, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	KickBack (0.425, 0.225, 0.125, 0.02, 3.05, 1.25, 5);
}

void CM249::Reload( void )
{
	int iResult;

	iResult = DefaultReload( M249_MAX_CLIP, M249_RELOAD, 4.415);

	if (iResult)
	{
		if( (m_pPlayer->m_afButtonPressed & IN_ATTACK) || ( (!(m_pPlayer->m_afButtonPressed & IN_ATTACK)) ) )
		{
			m_pPlayer->m_iShotsFired = 0;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
		m_pPlayer->m_i5562 = m_pPlayer->m_i5562 - 4; //when reloading, the value goes down by one
		if ( m_pPlayer->m_i5562 < 0) 
		{
			m_pPlayer->m_i5562 = 0; //if the value goes less than 0 e.g. -1,
								// the value would reset back to 0.
		}
	}
}


void CM249::WeaponIdle( void )
{	
	ResetEmptySound( );

	//m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	m_pPlayer->m_iShotsFired = 0;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.15;

	// only idle if the slid isn't back
	SendWeaponAnim( M249_IDLE1 );
}

class CM249Clip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_chainammo.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_chainammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( 30, "556Nato2", _556MM2_MAXCARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_M249clip, CM249Clip );