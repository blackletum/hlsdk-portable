/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "usercmd.h"
#include "entity_state.h"
#include "demo_api.h"
#include "pm_defs.h"
#include "event_api.h"
#include "r_efx.h"

#include "../hud_iface.h"
#include "../com_weapons.h"
#include "../demo.h"

extern globalvars_t *gpGlobals;
extern int g_iUser1;

// Pool of client side entities/entvars_t
static entvars_t ev[MAX_WEAPONS];
static int num_ents = 0;

// The entity we'll use to represent the local client
static CBasePlayer player;

// Local version of game .dll global variables ( time, etc. )
static globalvars_t Globals; 

static CBasePlayerWeapon *g_pWpns[MAX_WEAPONS];

float g_flApplyVel = 0.0;
int g_irunninggausspred = 0;

vec3_t previousorigin;

// HLDM Weapon placeholder entities.
//Haunter
CKnife g_Knife;
CHEGrenade g_HE;
CFlashBang g_FB;

CUSP g_USP;
CGlock18 g_Glock18;
CDeagle g_Deagle;
CP228 g_P228;
CFiveseveN g_FiveseveN;
CElite g_Elite;

CM3 g_M3;
CXM1014 g_XM1014;

CMP5N g_MP5N;
CTMP g_TMP;
CP90 g_P90;
CMac10 g_MAC10;
CUMP45 g_UMP45;

CAK47 g_AK47;
CM4A1 g_M4A1;
CSG552 g_SG552;
CAUG g_AUG;
CFAMAS g_FAMAS;
CGalil g_GALIL;

CAWP g_AWP;
CScout g_Scout;
CG3SG1 g_G3SG1;
CSG550 g_SG550;

CM249 g_M249;
CRPGRENADE g_RPGRENADE;
CC4 g_C4;
CULTIMATE g_ULTIMATE;
//Haunter

/*
======================
AlertMessage

Print debug messages to console
======================
*/
void AlertMessage( ALERT_TYPE atype, const char *szFmt, ... )
{
	va_list argptr;
	static char string[1024];

	va_start( argptr, szFmt );
	vsprintf( string, szFmt, argptr );
	va_end( argptr );

	gEngfuncs.Con_Printf( "cl:  " );
	gEngfuncs.Con_Printf( string );
}

//Returns if it's multiplayer.
//Mostly used by the client side weapons.
bool bIsMultiplayer( void )
{
	return gEngfuncs.GetMaxClients() == 1 ? 0 : 1;
}

//Just loads a v_ model.
void LoadVModel( const char *szViewModel, CBasePlayer *m_pPlayer )
{
	gEngfuncs.CL_LoadModel( szViewModel, &m_pPlayer->pev->viewmodel );
}

/*
=====================
HUD_PrepEntity

Links the raw entity to an entvars_s holder.  If a player is passed in as the owner, then
we set up the m_pPlayer field.
=====================
*/
void HUD_PrepEntity( CBaseEntity *pEntity, CBasePlayer *pWeaponOwner )
{
	memset( &ev[num_ents], 0, sizeof(entvars_t) );
	pEntity->pev = &ev[num_ents++];

	pEntity->Precache();
	pEntity->Spawn();

	if( pWeaponOwner )
	{
		ItemInfo info;

		( (CBasePlayerWeapon *)pEntity )->m_pPlayer = pWeaponOwner;

		( (CBasePlayerWeapon *)pEntity )->GetItemInfo( &info );

		g_pWpns[info.iId] = (CBasePlayerWeapon *)pEntity;
	}
}

/*
=====================
CBaseEntity::Killed

If weapons code "kills" an entity, just set its effects to EF_NODRAW
=====================
*/
void CBaseEntity::Killed( entvars_t *pevAttacker, int iGib )
{
	pev->effects |= EF_NODRAW;
}

/*
=====================
CBasePlayerWeapon::DefaultReload
=====================
*/
BOOL CBasePlayerWeapon::DefaultReload( int iClipSize, int iAnim, float fDelay, int body )
{
	if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
		return FALSE;

	int j = Q_min( iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] );

	if( j == 0 )
		return FALSE;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + fDelay;

	//!!UNDONE -- reload sound goes here !!!
	SendWeaponAnim( iAnim, UseDecrement(), body );

	m_fInReload = TRUE;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0f;
	return TRUE;
}

//Haunter XYZ
BOOL CBasePlayerWeapon :: EquipSilencer( int iAnim, float fDelay, int body )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + fDelay;

	//!!UNDONE -- reload sound goes here !!!
	SendWeaponAnim( iAnim );

	if ( (m_pPlayer->m_bIsSilencing == TRUE) && (m_pPlayer->m_iSilencing == 1) )
	{
		if (m_iSilenced == 0)
		{
            m_iSilenced = 1;
		}
		else if (m_iSilenced == 1)
		{
			m_iSilenced = 0;
		}	

		//m_pPlayer->m_bIsSilencing = FALSE;
	}

	m_fInSilencing = TRUE;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + fDelay;

	return TRUE;
}
//Haunter

/*
=====================
CBasePlayerWeapon::CanDeploy
=====================
*/
BOOL CBasePlayerWeapon::CanDeploy( void ) 
{
	/*Haunter
	BOOL bHasAmmo = 0;

	if( !pszAmmo1() )
	{
		// this weapon doesn't use ammo, can always deploy.
		return TRUE;
	}

	if( pszAmmo1() )
	{
		bHasAmmo |= ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0 );
	}
	if( pszAmmo2() )
	{
		bHasAmmo |= ( m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] != 0 );
	}
	if( m_iClip > 0 )
	{
		bHasAmmo |= 1;
	}
	if( !bHasAmmo )
	{
		return FALSE;
	} Haunter */

	return TRUE;
}

/*
=====================
CBasePlayerWeapon::DefaultDeploy

=====================
*/
BOOL CBasePlayerWeapon::DefaultDeploy( const char *szViewModel, const char *szWeaponModel, int iAnim, const char *szAnimExt, int skiplocal, int body )
{
	if( !CanDeploy() )
		return FALSE;

	gEngfuncs.CL_LoadModel( szViewModel, &m_pPlayer->pev->viewmodel );

	SendWeaponAnim( iAnim, skiplocal, body );

	g_irunninggausspred = false;
	m_pPlayer->m_flNextAttack = 0.5f;
	m_flTimeWeaponIdle = 1.55f; //Haunter default:1.0;
	return TRUE;
}

/*
=====================
CBasePlayerWeapon::PlayEmptySound

=====================
*/
BOOL CBasePlayerWeapon::PlayEmptySound( void )
{
	if( m_iPlayEmptySound )
	{
		HUD_PlaySound( "weapons/dryfire_rifle.wav", 0.8f );
		m_iPlayEmptySound = 1;//Haunter
		//m_iPlayEmptySound = 0;
		return 0;
	}
	return 0;
}

/*
=====================
CBasePlayerWeapon::ResetEmptySound

=====================
*/
void CBasePlayerWeapon::ResetEmptySound( void )
{
	m_iPlayEmptySound = 1;
}

/*
=====================
CBasePlayerWeapon :: PlayEmptySound2

For handguns
=====================
*/
//Haunter
BOOL CBasePlayerWeapon :: PlayEmptySound2( void )
{
	if (m_iPlayEmptySound2)
	{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dryfire_pistol.wav", 0.8, ATTN_NORM);
		m_iPlayEmptySound2 = 1;// default: 0
		return 0;
	}
	return 0;
}

/*
=====================
CBasePlayerWeapon :: ResetEmptySound2

For handguns
=====================
*/
void CBasePlayerWeapon :: ResetEmptySound2( void )
{
	m_iPlayEmptySound2 = 1;
}
//Haunter

/*
=====================
CBasePlayerWeapon::Holster

Put away weapon
=====================
*/
void CBasePlayerWeapon::Holster( int skiplocal /* = 0 */ )
{ 
	m_fInReload = FALSE; // cancel any reload in progress.
	//Haunter XYZ
	//m_pPlayer->m_iSilencing = 0;
	//m_pPlayer->m_bIsSilencing = FALSE;
	//Haunter
	m_pPlayer->pev->viewmodel = 0; 
	m_pPlayer->pev->weaponmodel = 0;

	//Atomizer
	if (m_pPlayer->pev->fov != 0)
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
	//Atom
}

/*
=====================
CBasePlayerWeapon::SendWeaponAnim

Animate weapon model
=====================
*/
void CBasePlayerWeapon::SendWeaponAnim( int iAnim, int skiplocal, int body )
{
	m_pPlayer->pev->weaponanim = iAnim;

	HUD_SendWeaponAnim( iAnim, body, 0 );
}

/*
=====================
CBaseEntity::FireBulletsPlayer

Only produces random numbers to match the server ones.
=====================
*/
Vector CBaseEntity::FireBulletsPlayer ( ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker, int shared_rand )
{
	float x = 0.0f, y = 0.0f, z;

	for( ULONG iShot = 1; iShot <= cShots; iShot++ )
	{
		if( pevAttacker == NULL )
		{
			// get circular gaussian spread
			do {
					x = RANDOM_FLOAT( -0.5f, 0.5f ) + RANDOM_FLOAT( -0.5f, 0.5f );
					y = RANDOM_FLOAT( -0.5f, 0.5f ) + RANDOM_FLOAT( -0.5f, 0.5f );
					z = x * x + y * y;
			} while( z > 1 );
		}
		else
		{
			//Use player's random seed.
			// get circular gaussian spread
			x = UTIL_SharedRandomFloat( shared_rand + iShot, -0.5f, 0.5f ) + UTIL_SharedRandomFloat( shared_rand + ( 1 + iShot ) , -0.5f, 0.5f );
			y = UTIL_SharedRandomFloat( shared_rand + ( 2 + iShot ), -0.5f, 0.5f ) + UTIL_SharedRandomFloat( shared_rand + ( 3 + iShot ), -0.5f, 0.5f );
			// z = x * x + y * y;
		}			
	}

	return Vector( x * vecSpread.x, y * vecSpread.y, 0.0f );
}

/* Haunter
=====================
CBaseEntity::FireBulletsPlayer2

Only produces random numbers to match the server ones.
=====================
*/
Vector CBaseEntity::FireBulletsPlayer2( ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t *pevAttacker, int shared_rand, bool bPistol )
{
	float x = 0.0f, y = 0.0f, z;

	for( ULONG iShot = 1; iShot <= cShots; iShot++ )
	{
		if( pevAttacker == NULL )
		{
			// get circular gaussian spread
			do {
					x = RANDOM_FLOAT( -0.5f, 0.5f ) + RANDOM_FLOAT( -0.5f, 0.5f );
					y = RANDOM_FLOAT( -0.5f, 0.5f ) + RANDOM_FLOAT( -0.5f, 0.5f );
					z = x * x + y * y;
			} while( z > 1 );
		}
		else
		{
			//Use player's random seed.
			// get circular gaussian spread
			x = UTIL_SharedRandomFloat( shared_rand + iShot, -0.5f, 0.5f ) + UTIL_SharedRandomFloat( shared_rand + ( 1 + iShot ) , -0.5f, 0.5f );
			y = UTIL_SharedRandomFloat( shared_rand + ( 2 + iShot ), -0.5f, 0.5f ) + UTIL_SharedRandomFloat( shared_rand + ( 3 + iShot ), -0.5f, 0.5f );
			z = x * x + y * y;
		}
	}

	return Vector( x * vecSpread.x, y * vecSpread.y, 0.0f );
}

/*
=====================
CBasePlayerWeapon::Kickback

This is for the recoil
=====================
*/
//Kick the view..
void CBasePlayerWeapon::KickBack( float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max, float lateral_max, int direction_change )
{
	float flKickUp;
	float flKickLateral;

	if( m_pPlayer->m_iShotsFired == 1 ) // This is the first round fired
	{
		flKickUp = up_base;
		flKickLateral = lateral_base;
	}
	else
	{
		flKickUp = up_base + m_pPlayer->m_iShotsFired * up_modifier;
		flKickLateral = lateral_base + m_pPlayer->m_iShotsFired * lateral_modifier;
	}

//	ALERT (at_console, "Max UP KICK : %fl\n", up_max);
//	ALERT (at_console, "Current UP KICK : %fl\n\n", flKickUp);

	m_pPlayer->pev->punchangle.x -= flKickUp;
	if( m_pPlayer->pev->punchangle.x < -1 * up_max )
		m_pPlayer->pev->punchangle.x = -1 * up_max;

	if( !RANDOM_LONG( 0, direction_change ) )
	{
		flKickLateral /= 2;
		m_iDirection = 1 - m_iDirection;
	}

	if( m_iDirection == 1 )
	{
		m_pPlayer->pev->punchangle.y += flKickLateral;
		if( m_pPlayer->pev->punchangle.y > lateral_max )
			m_pPlayer->pev->punchangle.y = lateral_max;
	}
	else
	{
		m_pPlayer->pev->punchangle.y -= flKickLateral;
		if( m_pPlayer->pev->punchangle.y < -1 * lateral_max )
			m_pPlayer->pev->punchangle.y = -1 * lateral_max;
	}
}
//Haunter

/*
=====================
CBasePlayerWeapon::ItemPostFrame

Handles weapon firing, reloading, etc.
Note: By putting some things from the server to the client, it works!
=====================
*/
void CBasePlayerWeapon::ItemPostFrame( void )
{
	//Haunter

	//Return zoom level back to previous zoom level before we fired a shot. This is used only for the AWP and the Scout.
	if( ( m_flNextPrimaryAttack <= UTIL_WeaponTimeBase() ) && ( m_pPlayer->m_bResumeZoom == TRUE ) )
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = m_pPlayer->m_iLastZoom;

		if( m_pPlayer->pev->fov == m_pPlayer->m_iLastZoom )
		{
			// get rid of the model and return the fade level in zoom.

//			if( this->m_iId == 24 )
//				UTIL_ScreenFade( m_pPlayer, Vector(10,180,10), 1.5, 9999, 80, FFADE_OUT );
		//	m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_awp.mdl");
			m_pPlayer->m_bResumeZoom = FALSE;
		}
	}

	//Haunter XYZ
	/*if( ( m_pPlayer->m_bIsSilencing == TRUE ) && ( m_pPlayer->m_iSilencing == 1 ) )
	{
		//m_iSilenced = m_pPlayer->m_iSilencing; // Is this correct?

		if( m_iSilenced == 1 )
		{
			m_iSilenced = 1;
		}
		else if( m_iSilenced == 0 )
		{
			m_iSilenced = 0;
		}

		//m_pPlayer->m_iSilencing = 0;
		//m_fInSilencing = FALSE;
	}

	if( ( m_pPlayer->m_bIsSilencing == FALSE ) && ( m_pPlayer->m_iSilencing == 1 ) )
	{
		if( m_iSilenced == 1 )
		{
			m_iSilenced = 0;
		}
		else if( m_iSilenced == 0 )
		{
			m_iSilenced = 1;
		}

		m_pPlayer->m_bIsSilencing = TRUE;
		m_pPlayer->m_iSilencing = 0;
	}

	if( m_pPlayer->m_pActiveItem->m_iId == WEAPON_USP )
	{
		if( ( m_pPlayer->m_bIsSilencing == TRUE ) && ( m_flTimeWeaponIdle <= 0.0 ) )
		{
			m_pPlayer->m_bIsSilencing = FALSE;
		}
	}*/
	//Haunter

	if( ( m_fInReload ) && ( m_pPlayer->m_flNextAttack <= 0.0f ) )
	{
#if 1
		// complete the reload. 
		ItemInfo itemInfo;
		memset( &itemInfo, 0, sizeof( itemInfo ) );
		GetItemInfo( &itemInfo );

		int j = Q_min( itemInfo.iMaxClip - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] );

		// Add them to the clip
		m_iClip += j;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;
#else
		m_iClip += 10;
#endif
		m_fInReload = FALSE;
	}

	if( ( m_pPlayer->pev->button & IN_ATTACK2 ) && ( m_flNextSecondaryAttack <= 0.0f ) )
	{
		if( pszAmmo2() && !m_pPlayer->m_rgAmmo[SecondaryAmmoIndex()] )
		{
			m_fFireOnEmpty = TRUE;
		}

		SecondaryAttack();
		m_pPlayer->pev->button &= ~IN_ATTACK2;
	}
	else if( ( m_pPlayer->pev->button & IN_ATTACK ) && ( m_flNextPrimaryAttack <= 0.0f ) )
	{
		if( ( m_iClip == 0 && pszAmmo1() ) || ( iMaxClip() == -1 && !m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] ) )
		{
			m_fFireOnEmpty = TRUE;
		}

		PrimaryAttack();
	}
	else if( m_pPlayer->pev->button & IN_RELOAD && iMaxClip() != WEAPON_NOCLIP && !m_fInReload )
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
	}
	else if( !( m_pPlayer->pev->button & ( IN_ATTACK | IN_ATTACK2 ) ) )
	{
		// no fire buttons down

		//Atomizer
		// The following code prevents the player from tapping the firebutton repeatedly
		// to simulate full auto and retaining the single shot accuracy of single fire
		if( m_bDelayFire == TRUE )
		{
			m_bDelayFire = FALSE;
			m_pPlayer->m_iShotsFired = 20;
		}

		if( m_pPlayer->m_iShotsFired > 0 )
		{
			m_pPlayer->m_iShotsFired--;
		}
		//Atom

		m_fFireOnEmpty = FALSE;

		// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
		if( m_iClip == 0 && !( iFlags() & ITEM_FLAG_NOAUTORELOAD ) && m_flNextPrimaryAttack <= 0.0f )
		{
			Reload();
			return;
		}

		WeaponIdle( );
		return;
	}

	// catch all
	if( ShouldWeaponIdle() )
	{
		WeaponIdle();
	}
}

/*
=====================
CBasePlayer::SelectItem

  Switch weapons
=====================
*/
void CBasePlayer::SelectItem( const char *pstr )
{
	if( !pstr )
		return;

	CBasePlayerItem *pItem = NULL;

	if( !pItem )
		return;

	if( pItem == m_pActiveItem )
		return;

	if( m_pActiveItem )
		m_pActiveItem->Holster();

	m_pLastItem = m_pActiveItem;
	m_pActiveItem = pItem;

	if( m_pActiveItem )
	{
		m_pActiveItem->Deploy();
	}
}
//Atom
/*

/*
=====================
CBasePlayer::SelectLastItem

=====================
*/
void CBasePlayer::SelectLastItem( void )
{
	if( !m_pLastItem )
	{
		return;
	}

	if( m_pActiveItem && !m_pActiveItem->CanHolster() )
	{
		return;
	}

	if( m_pActiveItem )
		m_pActiveItem->Holster();

	CBasePlayerItem *pTemp = m_pActiveItem;
	m_pActiveItem = m_pLastItem;
	m_pLastItem = pTemp;
	m_pActiveItem->Deploy( );
}

/*
=====================
CBasePlayer::Killed

=====================
*/
void CBasePlayer::Killed( entvars_t *pevAttacker, int iGib )
{
	// Holster weapon immediately, to allow it to cleanup
	if( m_pActiveItem )
		 m_pActiveItem->Holster();

	g_irunninggausspred = false;
}

//Haunter
/*
=====================
CBasePlayer :: GiveArmor

The hell I know what this does
=====================
*/
int CBasePlayer :: GiveArmor( float flArmor )
{
	int ArmorGiven;

	if (!pev->takedamage)
		return 0;

	if ( pev->armorvalue >= 100 )
		return 0;

	ArmorGiven = 101 - pev->armorvalue;

	if (ArmorGiven >= flArmor)
	{
		ArmorGiven = flArmor;
	}

	pev->armorvalue += ArmorGiven;

	if (pev->armorvalue > 100 )
	{
		pev->armorvalue = 100;
	}
	
	return ArmorGiven;
}
//Haunter
/*
=====================
CBasePlayer::Spawn

=====================
*/
void CBasePlayer::Spawn( void )
{
	if( m_pActiveItem )
		m_pActiveItem->Deploy( );

	g_irunninggausspred = false;
}

/*
=====================
UTIL_TraceLine

Don't actually trace, but act like the trace didn't hit anything.
=====================
*/
void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr )
{
	memset( ptr, 0, sizeof(*ptr) );
	ptr->flFraction = 1.0f;
}

/*
=====================
UTIL_ParticleBox

For debugging, draw a box around a player made out of particles
=====================
*/
void UTIL_ParticleBox( CBasePlayer *player, float *mins, float *maxs, float life, unsigned char r, unsigned char g, unsigned char b )
{
	int i;
	vec3_t mmin, mmax;

	for( i = 0; i < 3; i++ )
	{
		mmin[i] = player->pev->origin[i] + mins[i];
		mmax[i] = player->pev->origin[i] + maxs[i];
	}

	gEngfuncs.pEfxAPI->R_ParticleBox( (float *)&mmin, (float *)&mmax, 5.0, 0, 255, 0 );
}

/*
=====================
UTIL_ParticleBoxes

For debugging, draw boxes for other collidable players
=====================
*/
void UTIL_ParticleBoxes( void )
{
	int idx;
	physent_t *pe;
	cl_entity_t *player;
	vec3_t mins, maxs;

	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	player = gEngfuncs.GetLocalPlayer();
	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers ( player->index - 1 );	

	for( idx = 1; idx < 100; idx++ )
	{
		pe = gEngfuncs.pEventAPI->EV_GetPhysent( idx );
		if( !pe )
			break;

		if( pe->info >= 1 && pe->info <= gEngfuncs.GetMaxClients() )
		{
			mins = pe->origin + pe->mins;
			maxs = pe->origin + pe->maxs;

			gEngfuncs.pEfxAPI->R_ParticleBox( (float *)&mins, (float *)&maxs, 0, 0, 255, 2.0 );
		}
	}

	gEngfuncs.pEventAPI->EV_PopPMStates();
}

/*
=====================
UTIL_ParticleLine

For debugging, draw a line made out of particles
=====================
*/
void UTIL_ParticleLine( CBasePlayer *player, float *start, float *end, float life, unsigned char r, unsigned char g, unsigned char b )
{
	gEngfuncs.pEfxAPI->R_ParticleLine( start, end, r, g, b, life );
}

/*
=====================
HUD_InitClientWeapons

Set up weapons, player and functions needed to run weapons code client-side.
=====================
*/
void HUD_InitClientWeapons( void )
{
	static int initialized = 0;
	if( initialized )
		return;

	initialized = 1;

	// Set up pointer ( dummy object )
	gpGlobals = &Globals;

	// Fill in current time ( probably not needed )
	gpGlobals->time = gEngfuncs.GetClientTime();

	// Fake functions
	g_engfuncs.pfnPrecacheModel = stub_PrecacheModel;
	g_engfuncs.pfnPrecacheSound = stub_PrecacheSound;
	g_engfuncs.pfnPrecacheEvent = stub_PrecacheEvent;
	g_engfuncs.pfnNameForFunction = stub_NameForFunction;
	g_engfuncs.pfnSetModel = stub_SetModel;
	g_engfuncs.pfnSetClientMaxspeed = HUD_SetMaxSpeed;

	// Handled locally
	g_engfuncs.pfnPlaybackEvent = HUD_PlaybackEvent;
	g_engfuncs.pfnAlertMessage = AlertMessage;

	// Pass through to engine
	g_engfuncs.pfnPrecacheEvent = gEngfuncs.pfnPrecacheEvent;
	g_engfuncs.pfnRandomFloat = gEngfuncs.pfnRandomFloat;
	g_engfuncs.pfnRandomLong = gEngfuncs.pfnRandomLong;

	// Allocate a slot for the local player
	HUD_PrepEntity( &player, NULL );

	// Allocate slot(s) for each weapon that we are going to be predicting
	//Haunter
	HUD_PrepEntity( &g_Knife, &player );
	HUD_PrepEntity( &g_HE, &player );
	HUD_PrepEntity( &g_FB, &player );

	HUD_PrepEntity( &g_USP, &player );
	HUD_PrepEntity( &g_Glock18, &player );
	HUD_PrepEntity( &g_Deagle, &player );
	HUD_PrepEntity( &g_P228, &player );
	HUD_PrepEntity( &g_FiveseveN, &player );
	HUD_PrepEntity( &g_Elite, &player );

	HUD_PrepEntity( &g_M3, &player );
	HUD_PrepEntity( &g_XM1014, &player );

	HUD_PrepEntity( &g_MP5N, &player );
	HUD_PrepEntity( &g_TMP, &player );
	HUD_PrepEntity( &g_P90, &player );
	HUD_PrepEntity( &g_MAC10, &player );
	HUD_PrepEntity( &g_UMP45, &player );

	HUD_PrepEntity( &g_AK47, &player );
	HUD_PrepEntity( &g_M4A1, &player );
	HUD_PrepEntity( &g_SG552, &player );
	HUD_PrepEntity( &g_AUG, &player );
	HUD_PrepEntity( &g_FAMAS, &player );
	HUD_PrepEntity( &g_GALIL, &player );

	HUD_PrepEntity( &g_AWP, &player );
	HUD_PrepEntity( &g_Scout, &player );
	HUD_PrepEntity( &g_G3SG1, &player );
	HUD_PrepEntity( &g_SG550, &player );

	HUD_PrepEntity( &g_M249, &player );
	HUD_PrepEntity( &g_RPGRENADE, &player );
	HUD_PrepEntity( &g_C4, &player );
	HUD_PrepEntity( &g_ULTIMATE, &player );
	//Haunter
}

/*
=====================
HUD_GetLastOrg

Retruns the last position that we stored for egon beam endpoint.
=====================
*/
void HUD_GetLastOrg( float *org )
{
	int i;

	// Return last origin
	for( i = 0; i < 3; i++ )
	{
		org[i] = previousorigin[i];
	}
}

/*
=====================
HUD_SetLastOrg

Remember our exact predicted origin so we can draw the egon to the right position.
=====================
*/
void HUD_SetLastOrg( void )
{
	int i;

	// Offset final origin by view_offset
	for( i = 0; i < 3; i++ )
	{
		previousorigin[i] = g_finalstate->playerstate.origin[i] + g_finalstate->client.view_ofs[i];
	}
}

/*
=====================
HUD_WeaponsPostThink

Run Weapon firing code on client
=====================
*/
void HUD_WeaponsPostThink( local_state_s *from, local_state_s *to, usercmd_t *cmd, double time, unsigned int random_seed )
{
	int i;
	int buttonsChanged;
	CBasePlayerWeapon *pWeapon = NULL;
	CBasePlayerWeapon *pCurrent;
	weapon_data_t nulldata = {0}, *pfrom, *pto;
	static int lasthealth;

	HUD_InitClientWeapons();

	// Get current clock
	gpGlobals->time = time;

	// Fill in data based on selected weapon
	// FIXME, make this a method in each weapon?  where you pass in an entity_state_t *?
	switch( from->client.m_iId )
	{
		//Haunter
		case WEAPON_KNIFE:
			pWeapon = &g_Knife;
			break;
		case WEAPON_HEGRENADE:
			pWeapon = &g_HE;
			break;
		case WEAPON_FLASHBANG:
			pWeapon = &g_FB;
			break;

		case WEAPON_USP:
			pWeapon = &g_USP;
			break;
		case WEAPON_GLOCK18:
			pWeapon = &g_Glock18;
			break;
		case WEAPON_DEAGLE:
			pWeapon = &g_Deagle;
			break;
		case WEAPON_P228:
			pWeapon = &g_P228;
			break;
		case WEAPON_FIVESEVEN:
			pWeapon = &g_FiveseveN;
			break;
		case WEAPON_ELITE:
			pWeapon = &g_Elite;
			break;

		case WEAPON_M3:
			pWeapon = &g_M3;
			break;
		case WEAPON_XM1014:
			pWeapon = &g_XM1014;
			break;

		case WEAPON_MP5N:
			pWeapon = &g_MP5N;
			break;
		case WEAPON_TMP:
			pWeapon = &g_TMP;
			break;
		case WEAPON_P90:
			pWeapon = &g_P90;
			break;
		case WEAPON_MAC10:
			pWeapon = &g_MAC10;
			break;
		case WEAPON_UMP45:
			pWeapon = &g_UMP45;
			break;

		case WEAPON_AK47:
			pWeapon = &g_AK47;
			break;
		case WEAPON_M4A1:
			pWeapon = &g_M4A1;
			break;
		case WEAPON_SG552:
			pWeapon = &g_SG552;
			break;
		case WEAPON_AUG:
			pWeapon = &g_AUG;
			break;
		case WEAPON_FAMAS:
			pWeapon = &g_FAMAS;
			break;
		case WEAPON_GALIL:
			pWeapon = &g_GALIL;
			break;

		case WEAPON_AWP:
			pWeapon = &g_AWP;
			break;
		case WEAPON_SCOUT:
			pWeapon = &g_Scout;
			break;
		case WEAPON_G3SG1:
			pWeapon = &g_G3SG1;
			break;
		case WEAPON_SG550:
			pWeapon = &g_SG550;
			break;

		case WEAPON_M249:
			pWeapon = &g_M249;
			break;
		case WEAPON_RPGRENADE:
			pWeapon = &g_RPGRENADE;
			break;
		case WEAPON_C4:
			pWeapon = &g_C4;
			break;
		case WEAPON_ULTIMATE:
			pWeapon = &g_ULTIMATE;
			break;
		//Haunter
	}

	// Store pointer to our destination entity_state_t so we can get our origin, etc. from it
	//  for setting up events on the client
	g_finalstate = to;

	// If we are running events/etc. go ahead and see if we
	//  managed to die between last frame and this one
	// If so, run the appropriate player killed or spawn function
	if( g_runfuncs )
	{
		if( to->client.health <= 0 && lasthealth > 0 )
		{
			player.Killed( NULL, 0 );
		}
		else if( to->client.health > 0 && lasthealth <= 0 )
		{
			player.Spawn();
		}

		lasthealth = to->client.health;
	}

	// We are not predicting the current weapon, just bow out here.
	if( !pWeapon )
		return;

	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		pCurrent = g_pWpns[i];
		if( !pCurrent )
		{
			continue;
		}

		//Haunter
/*		if( from->weapondata[i].m_iId == 0.000000 || from->weapondata[i].m_flNextPrimaryAttack == 0.000000 )
		{
//			if( i == WEAPON_PYTHON ) //omega; debug printing for testing
//			ALERT(at_console, "skipped weapondata..\n");
			continue;
		}
		//Haunter*/

		pfrom = &from->weapondata[i];

		pCurrent->m_fInReload = pfrom->m_fInReload;
		pCurrent->m_fInSpecialReload = pfrom->m_fInSpecialReload;
		//pCurrent->m_flPumpTime = pfrom->m_flPumpTime;
		pCurrent->m_iClip = pfrom->m_iClip;
		pCurrent->m_flNextPrimaryAttack	= pfrom->m_flNextPrimaryAttack;
		pCurrent->m_flNextSecondaryAttack = pfrom->m_flNextSecondaryAttack;
		pCurrent->m_flTimeWeaponIdle = pfrom->m_flTimeWeaponIdle;
		pCurrent->pev->fuser1 = pfrom->fuser1;
		pCurrent->m_flStartThrow = pfrom->fuser2;
	//	pCurrent->m_flReleaseThrow = pfrom->fuser3;
		//Haunter
		pCurrent->m_iBurstFire = pfrom->fuser3;

		pCurrent->m_iSilenced = pfrom->iuser1;
		pCurrent->m_fInAttack = pfrom->iuser2;
		//Haunter

		pCurrent->m_iSecondaryAmmoType = (int)from->client.vuser3[2];
		pCurrent->m_iPrimaryAmmoType = (int)from->client.vuser4[0];
		player.m_rgAmmo[pCurrent->m_iPrimaryAmmoType] = (int)from->client.vuser4[1];
		player.m_rgAmmo[pCurrent->m_iSecondaryAmmoType] = (int)from->client.vuser4[2];
	}

	// For random weapon events, use this seed to seed random # generator
	player.random_seed = random_seed;

	// Get old buttons from previous state.
	player.m_afButtonLast = from->playerstate.oldbuttons;

	// Which buttsons chave changed
	buttonsChanged = ( player.m_afButtonLast ^ cmd->buttons );	// These buttons have changed this frame

	// Debounced button codes for pressed/released
	// The changed ones still down are "pressed"
	player.m_afButtonPressed =  buttonsChanged & cmd->buttons;	
	// The ones not down are "released"
	player.m_afButtonReleased = buttonsChanged & ( ~cmd->buttons );

	// Set player variables that weapons code might check/alter
	player.pev->button = cmd->buttons;

	player.pev->velocity = from->client.velocity;
	player.pev->flags = from->client.flags;

	player.pev->deadflag = from->client.deadflag;
	player.pev->waterlevel = from->client.waterlevel;
	player.pev->maxspeed = from->client.maxspeed;
	player.pev->fov = from->client.fov;
	player.pev->weaponanim = from->client.weaponanim;
	player.pev->viewmodel = from->client.viewmodel;
	player.m_flNextAttack = from->client.m_flNextAttack;
	player.m_flNextAmmoBurn = from->client.fuser2;
	player.m_flAmmoStartCharge = from->client.fuser3;

	//Stores all our ammo info, so the client side weapons can use them.
	player.ammo_9mm = (int)from->client.vuser1[0];
	player.ammo_357 = (int)from->client.vuser1[1];
	player.ammo_argrens = (int)from->client.vuser1[2];
	player.ammo_bolts = (int)from->client.ammo_nails; //is an int anyways...
	player.ammo_buckshot = (int)from->client.ammo_shells; 
	player.ammo_uranium = (int)from->client.ammo_cells;
	player.ammo_hornets = (int)from->client.vuser2[0];
	player.ammo_rockets = (int)from->client.ammo_rockets;

	// Point to current weapon object
	if( from->client.m_iId )
	{
		player.m_pActiveItem = g_pWpns[from->client.m_iId];
	}

	//Haunter
	if( player.m_pActiveItem->m_iId == WEAPON_RPGRENADE )
	{
		// ( (CRpg *)player.m_pActiveItem )->m_fSpotActive = (int)from->client.vuser2[1];
		( (CRPGRENADE *)player.m_pActiveItem )->m_cActiveRockets = (int)from->client.vuser2[2];
	}
	//Haunter
/*	if( player.m_pActiveItem->m_iId == WEAPON_RPG )
	{
		( (CRpg *)player.m_pActiveItem )->m_fSpotActive = (int)from->client.vuser2[1];
		( (CRpg *)player.m_pActiveItem )->m_cActiveRockets = (int)from->client.vuser2[2];
	}*/

	// Don't go firing anything if we have died.
	// Or if we don't have a weapon model deployed
	if( ( player.pev->deadflag != ( DEAD_DISCARDBODY + 1 ) ) && 
		 !CL_IsDead() && player.pev->viewmodel && !g_iUser1 )
	{
		if( player.m_flNextAttack <= 0 )
		{
			pWeapon->ItemPostFrame();
		}
	}

	// Assume that we are not going to switch weapons
	to->client.m_iId = from->client.m_iId;

	// Now see if we issued a changeweapon command ( and we're not dead )
	if( cmd->weaponselect && ( player.pev->deadflag != ( DEAD_DISCARDBODY + 1 ) ) )
	{
		// Switched to a different weapon?
		if( from->weapondata[cmd->weaponselect].m_iId == cmd->weaponselect )
		{
			CBasePlayerWeapon *pNew = g_pWpns[cmd->weaponselect];
			if( pNew && ( pNew != pWeapon ) )
			{
				// Put away old weapon
				if( player.m_pActiveItem )
					player.m_pActiveItem->Holster();

				player.m_pLastItem = player.m_pActiveItem;
				player.m_pActiveItem = pNew;

				// Deploy new weapon
				if( player.m_pActiveItem )
				{
					player.m_pActiveItem->Deploy();
				}

				// Update weapon id so we can predict things correctly.
				to->client.m_iId = cmd->weaponselect;
			}
		}
	}

	// Copy in results of prediction code
	to->client.viewmodel = player.pev->viewmodel;
	to->client.fov = player.pev->fov;
	to->client.weaponanim = player.pev->weaponanim;
	to->client.m_flNextAttack = player.m_flNextAttack;
	to->client.fuser2 = player.m_flNextAmmoBurn;
	to->client.fuser3 = player.m_flAmmoStartCharge;
	to->client.maxspeed = player.pev->maxspeed;

	//HL Weapons
	to->client.vuser1[0] = player.ammo_9mm;
	to->client.vuser1[1] = player.ammo_357;
	to->client.vuser1[2] = player.ammo_argrens;

	to->client.ammo_nails = player.ammo_bolts;
	to->client.ammo_shells = player.ammo_buckshot;
	to->client.ammo_cells = player.ammo_uranium;
	to->client.vuser2[0] = player.ammo_hornets;
	to->client.ammo_rockets = player.ammo_rockets;

	if( player.m_pActiveItem->m_iId == WEAPON_RPGRENADE )
	{
		// to->client.vuser2[1] = ( (CRpg *)player.m_pActiveItem)->m_fSpotActive;
		to->client.vuser2[2] = ( (CRPGRENADE *)player.m_pActiveItem)->m_cActiveRockets;
	}

/*	if( player.m_pActiveItem->m_iId == WEAPON_RPG )
	{
		to->client.vuser2[1] = ( (CRpg *)player.m_pActiveItem)->m_fSpotActive;
		to->client.vuser2[2] = ( (CRpg *)player.m_pActiveItem)->m_cActiveRockets;
	}*/

	// Make sure that weapon animation matches what the game .dll is telling us
	//  over the wire ( fixes some animation glitches )
	if( g_runfuncs && ( HUD_GetWeaponAnim() != to->client.weaponanim ) )
	{
		int body = 2;

		/*Haunter //Pop the model to body 0.
		if( pWeapon == &g_Tripmine )
			 body = 0;*/

		// Force a fixed anim down to viewmodel
		HUD_SendWeaponAnim( to->client.weaponanim, body, 1 );
	}

	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		pCurrent = g_pWpns[i];

		pto = &to->weapondata[i];

		if( !pCurrent )
		{
			memset( pto, 0, sizeof(weapon_data_t) );
			continue;
		}

		pto->m_fInReload = pCurrent->m_fInReload;
		pto->m_fInSpecialReload = pCurrent->m_fInSpecialReload;
		//pto->m_flPumpTime = pCurrent->m_flPumpTime;
		pto->m_iClip = pCurrent->m_iClip; 
		pto->m_flNextPrimaryAttack = pCurrent->m_flNextPrimaryAttack;
		pto->m_flNextSecondaryAttack = pCurrent->m_flNextSecondaryAttack;
		pto->m_flTimeWeaponIdle = pCurrent->m_flTimeWeaponIdle;
		pto->fuser1 = pCurrent->pev->fuser1;
		pto->fuser2 = pCurrent->m_flStartThrow;
	//	pto->fuser3 = pCurrent->m_flReleaseThrow;
		pto->fuser3 = pCurrent->m_iBurstFire;
		//Haunter
		pto->iuser1 = pCurrent->m_iSilenced;
		pto->iuser2 = pCurrent->m_fInAttack;
		//Haunter


		// Decrement weapon counters, server does this at same time ( during post think, after doing everything else )
		pto->m_flNextReload -= cmd->msec / 1000.0f;
		pto->m_fNextAimBonus -= cmd->msec / 1000.0f;
		pto->m_flNextPrimaryAttack -= cmd->msec / 1000.0f;
		pto->m_flNextSecondaryAttack -= cmd->msec / 1000.0f;
		pto->m_flTimeWeaponIdle -= cmd->msec / 1000.0f;
		pto->fuser1 -= cmd->msec / 1000.0f;

		to->client.vuser3[2] = pCurrent->m_iSecondaryAmmoType;
		to->client.vuser4[0] = pCurrent->m_iPrimaryAmmoType;
		to->client.vuser4[1] = player.m_rgAmmo[pCurrent->m_iPrimaryAmmoType];
		to->client.vuser4[2] = player.m_rgAmmo[pCurrent->m_iSecondaryAmmoType];

/*		if( pto->m_flPumpTime != -9999.0f )
		{
			pto->m_flPumpTime -= cmd->msec / 1000.0f;
			if( pto->m_flPumpTime < -0.001f )
				pto->m_flPumpTime = -0.001f;
		}*/

		if( pto->m_fNextAimBonus < -1.0f )
		{
			pto->m_fNextAimBonus = -1.0f;
		}

		if( pto->m_flNextPrimaryAttack < -1.0f )
		{
			pto->m_flNextPrimaryAttack = -1.0f;
		}

		if( pto->m_flNextSecondaryAttack < -0.001f )
		{
			pto->m_flNextSecondaryAttack = -0.001f;
		}

		if( pto->m_flTimeWeaponIdle < -0.001f )
		{
			pto->m_flTimeWeaponIdle = -0.001f;
		}

		if( pto->m_flNextReload < -0.001f )
		{
			pto->m_flNextReload = -0.001f;
		}

		if( pto->fuser1 < -0.001f )
		{
			pto->fuser1 = -0.001f;
		}
	}

	// m_flNextAttack is now part of the weapons, but is part of the player instead
	to->client.m_flNextAttack -= cmd->msec / 1000.0f;
	if( to->client.m_flNextAttack < -0.001f )
	{
		to->client.m_flNextAttack = -0.001f;
	}

	to->client.fuser2 -= cmd->msec / 1000.0f;
	if( to->client.fuser2 < -0.001f )
	{
		to->client.fuser2 = -0.001f;
	}

	to->client.fuser3 -= cmd->msec / 1000.0f;
	if( to->client.fuser3 < -0.001f )
	{
		to->client.fuser3 = -0.001f;
	}

	// Store off the last position from the predicted state.
	HUD_SetLastOrg();

	// Wipe it so we can't use it after this frame
	g_finalstate = NULL;
}

/*
=====================
HUD_PostRunCmd

Client calls this during prediction, after it has moved the player and updated any info changed into to->
time is the current client clock based on prediction
cmd is the command that caused the movement, etc
runfuncs is 1 if this is the first time we've predicted this command.  If so, sounds and effects should play, otherwise, they should
be ignored
=====================
*/
void _DLLEXPORT HUD_PostRunCmd( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed )
{
	g_runfuncs = runfuncs;

#if CLIENT_WEAPONS
	if( cl_lw && cl_lw->value )
	{
		HUD_WeaponsPostThink( from, to, cmd, time, random_seed );
	}
	else
#endif
	{
		to->client.fov = g_lastFOV;
	}

	if( g_irunninggausspred == 1 )
	{
		Vector forward;
		gEngfuncs.pfnAngleVectors( v_angles, forward, NULL, NULL );
		to->client.velocity = to->client.velocity - forward * g_flApplyVel * 5; 
		g_irunninggausspred = false;
	}

	// All games can use FOV state
	g_lastFOV = to->client.fov;
}
