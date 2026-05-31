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
/*

===== h_battery.cpp ========================================================

  battery-related code

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "skill.h"
#include "gamerules.h"
#include "weapons.h"
#include "game.h"

//Atomizer
#include "player.h"
#include "weapons.h"
//Atom

class CRecharge : public CBaseToggle
{
public:
	void Spawn();
	void Precache( void );
	void EXPORT Off(void);
	void EXPORT Recharge(void);
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int ObjectCaps( void ) { return ( CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE ) & ~FCAP_ACROSS_TRANSITION; }
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	static TYPEDESCRIPTION m_SaveData[];

	float m_flNextCharge;
	int m_iReactivate; // DeathMatch Delay until reactvated
	int m_iJuice;
	int m_iOn;			// 0 = off, 1 = startup, 2 = going
	float m_flSoundTime;
	int TimeES;
};

TYPEDESCRIPTION CRecharge::m_SaveData[] =
{
	DEFINE_FIELD( CRecharge, m_flNextCharge, FIELD_TIME ),
	DEFINE_FIELD( CRecharge, m_iReactivate, FIELD_INTEGER ),
	DEFINE_FIELD( CRecharge, m_iJuice, FIELD_INTEGER ),
	DEFINE_FIELD( CRecharge, m_iOn, FIELD_INTEGER ),
	DEFINE_FIELD( CRecharge, m_flSoundTime, FIELD_TIME ),
	DEFINE_FIELD( CRecharge, TimeES, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CRecharge, CBaseToggle )

LINK_ENTITY_TO_CLASS( func_recharge, CRecharge )

void CRecharge::KeyValue( KeyValueData *pkvd )
{
	if( FStrEq( pkvd->szKeyName, "style" ) ||
		FStrEq( pkvd->szKeyName, "height" ) ||
		FStrEq( pkvd->szKeyName, "value1" ) ||
		FStrEq( pkvd->szKeyName, "value2" ) ||
		FStrEq( pkvd->szKeyName, "value3" ) )
	{
		pkvd->fHandled = TRUE;
	}
	else if( FStrEq( pkvd->szKeyName, "dmdelay" ) )
	{
		m_iReactivate = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue( pkvd );
}

void CRecharge::Spawn()
{
	Precache();

	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin( pev, pev->origin );		// set size and link into world
	UTIL_SetSize( pev, pev->mins, pev->maxs );
	SET_MODEL( ENT( pev ), STRING( pev->model ) );
	m_iJuice = (int)gSkillData.suitchargerCapacity;
	pev->frame = 0;			
}

void CRecharge::Precache()
{
	PRECACHE_SOUND( "items/suitcharge1.wav" );
	PRECACHE_SOUND( "items/suitchargeno1.wav" );
	PRECACHE_SOUND( "items/suitchargeok1.wav" );
}

void CRecharge::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if( g_pGameRules->IsMultiplayer() )
	{
		TimeES = (int)CVAR_GET_FLOAT( "mp_buytime" ); //Haunter
		if( TimeES <= 0 )
			TimeES = 1;
	}
	else
	{
		TimeES = (int)CVAR_GET_FLOAT( "cl_buytime" ); //Haunter
		if( TimeES <= 0 )
			TimeES = 1;
	}

	// Make sure that we have a caller
	if( !pActivator )
		return;

	// if it's not a player, ignore
	if( !pActivator->IsPlayer() )
		return;

	//Atomizer - Unlimited usage
	// if there is no juice left, turn it off
	/*if( m_iJuice <= 0 )
	{
		pev->frame = 1;
		Off();
	}*/

	// if the player doesn't have the suit, or there is no juice left, make the deny noise
	if( !( pActivator->pev->weapons & ( 1 << WEAPON_SUIT ) ) || ( !( pActivator->pev->weapons & ( 1 << WEAPON_KNIFE ) ) ) )
	{
		ClientPrint( pActivator->pev, HUD_PRINTCENTER, UTIL_VarArgs( "Acquire the HEV suit & the knife first...\n" ) );
		if( m_flSoundTime <= gpGlobals->time )
		{
			m_flSoundTime = gpGlobals->time + 0.62f;
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/suitchargeno1.wav", 0.85, ATTN_NORM );
		}
		return;
	}

	pev->nextthink = pev->ltime + 0.25f;
	SetThink( &CRecharge::Off );

	// Time to recharge yet?
	if( m_flNextCharge >= gpGlobals->time )
		return;

	m_hActivator = pActivator;

	// Play the on sound or the looping charging sound
	//Atomizer
	CBasePlayer *pPlayer = (CBasePlayer *)pActivator;

	ShowMenu( pPlayer, 0x27F, TimeES, 0, "#Buy" );
	pPlayer->m_iArmor = 1; //Haunter
	pPlayer->m_iMenu = 1;
	pPlayer->m_iBuyable = 1;


//	EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/suitchargeok1.wav", 0.85, ATTN_NORM );
	m_flSoundTime = 0.56f + gpGlobals->time;

	/*if( pPlayer->GiveAmmo( 5, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) )
	{
		//Original Code
		if( !m_iOn )
		{
			m_iOn++;
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/suitchargeok1.wav", 0.85, ATTN_NORM );
			m_flSoundTime = 0.56f + gpGlobals->time;
		}
		if( ( m_iOn == 1 ) && ( m_flSoundTime <= gpGlobals->time ) )
		{
			m_iOn++;
			EMIT_SOUND( ENT( pev ), CHAN_STATIC, "items/suitcharge1.wav", 0.85, ATTN_NORM );
		}
		//Original Code
	}
	else
	{
		if( m_flSoundTime <= gpGlobals->time )
		{
			m_flSoundTime = gpGlobals->time + 0.62;
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/suitchargeno1.wav", 0.85, ATTN_NORM );
		}
		return;
	}*/


	//Give Ammo instead of Armour
	// charge the player
	/*if( m_hActivator->pev->armorvalue < 100 )
	{
		m_iJuice--;
		m_hActivator->pev->armorvalue += 1;

		if( m_hActivator->pev->armorvalue > 100 )
			m_hActivator->pev->armorvalue = 100;
	}*/
	//Atom

	// govern the rate of charge
	m_flNextCharge = gpGlobals->time + 0.1f;
}

void CRecharge::Recharge( void )
{
	m_iJuice = (int)gSkillData.suitchargerCapacity;
	pev->frame = 0;	
	SetThink( &CBaseEntity::SUB_DoNothing );
}

void CRecharge::Off( void )
{
	// Stop looping sound.
	if( m_iOn > 1 )
		STOP_SOUND( ENT( pev ), CHAN_STATIC, "items/suitcharge1.wav" );

	m_iOn = 0;

	if( ( !m_iJuice ) &&  ( ( m_iReactivate = (int)g_pGameRules->FlHEVChargerRechargeTime() ) > 0 ) )
	{
		pev->nextthink = pev->ltime + m_iReactivate;
		SetThink( &CRecharge::Recharge );
	}
	else
		SetThink( &CBaseEntity::SUB_DoNothing );
}
