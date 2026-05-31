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

===== items.cpp ========================================================

  functions governing the selection/use of weapons for players

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "skill.h"
#include "items.h"
#include "gamerules.h"

//Haunter
#include "maxcarry.h"
//Haunter

extern int gmsgItemPickup;

class CWorldItem : public CBaseEntity
{
public:
	void KeyValue( KeyValueData *pkvd ); 
	void Spawn( void );
	int m_iType;
};

LINK_ENTITY_TO_CLASS( world_items, CWorldItem )

void CWorldItem::KeyValue( KeyValueData *pkvd )
{
	if( FStrEq( pkvd->szKeyName, "type" ) )
	{
		m_iType = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

void CWorldItem::Spawn( void )
{
	CBaseEntity *pEntity = NULL;

	switch( m_iType ) 
	{
	case 44: // ITEM_BATTERY:
		pEntity = CBaseEntity::Create( "item_battery", pev->origin, pev->angles );
		break;
	case 42: // ITEM_ANTIDOTE:
		pEntity = CBaseEntity::Create( "item_antidote", pev->origin, pev->angles );
		break;
	case 43: // ITEM_SECURITY:
		pEntity = CBaseEntity::Create( "item_security", pev->origin, pev->angles );
		break;
	case 45: // ITEM_SUIT:
		pEntity = CBaseEntity::Create( "item_suit", pev->origin, pev->angles );
		break;
	}

	if( !pEntity )
	{
		ALERT( at_console, "unable to create world_item %d\n", m_iType );
	}
	else
	{
		pEntity->pev->target = pev->target;
		pEntity->pev->targetname = pev->targetname;
		pEntity->pev->spawnflags = pev->spawnflags;
	}

	REMOVE_ENTITY( edict() );
}

void CItem::Spawn( void )
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize( pev, Vector( -16, -16, 0 ), Vector( 16, 16, 16 ) );
	SetTouch( &CItem::ItemTouch );

	if( DROP_TO_FLOOR(ENT( pev ) ) == 0 )
	{
		ALERT(at_error, "Item %s fell out of level at %f,%f,%f\n", STRING( pev->classname ), (double)pev->origin.x, (double)pev->origin.y, (double)pev->origin.z);
		UTIL_Remove( this );
		return;
	}
}

extern int gEvilImpulse101;

void CItem::ItemTouch( CBaseEntity *pOther )
{
	// if it's not a player, ignore
	if( !pOther->IsPlayer() )
	{
		return;
	}

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	// ok, a player is touching this item, but can he have it?
	if( !g_pGameRules->CanHaveItem( pPlayer, this ) )
	{
		// no? Ignore the touch.
		return;
	}

	if( MyTouch( pPlayer ) )
	{
		SUB_UseTargets( pOther, USE_TOGGLE, 0 );
		SetTouch( NULL );
		
		// player grabbed the item. 
		g_pGameRules->PlayerGotItem( pPlayer, this );
		if( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_YES )
		{
			Respawn(); 
		}
		else
		{
			UTIL_Remove( this );
		}
	}
	else if( gEvilImpulse101 )
	{
		UTIL_Remove( this );
	}
}

CBaseEntity* CItem::Respawn( void )
{
	SetTouch( NULL );
	pev->effects |= EF_NODRAW;

	UTIL_SetOrigin( pev, g_pGameRules->VecItemRespawnSpot( this ) );// blip to whereever you should respawn.

	SetThink( &CItem::Materialize );
	pev->nextthink = g_pGameRules->FlItemRespawnTime( this ); 
	return this;
}

void CItem::Materialize( void )
{
	if( pev->effects & EF_NODRAW )
	{
		// changing from invisible state to visible.
		EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150 );
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	SetTouch( &CItem::ItemTouch );
	SetThink( NULL );
}

#define SF_SUIT_SHORTLOGON		0x0001

class CItemSuit : public CItem
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_suit.mdl" );
		CItem::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_suit.mdl" );
		PRECACHE_SOUND( "radio/letsgo.wav" ); //Haunter
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if( pPlayer->pev->weapons & ( 1<<WEAPON_SUIT ) )
			return FALSE;

		EMIT_SOUND( pPlayer->edict(), CHAN_ITEM, "radio/letsgo.wav", 1, ATTN_NORM ); //Haunter
		pPlayer->pev->weapons |= ( 1 << WEAPON_SUIT );
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS( item_suit, CItemSuit )

class CItemBattery : public CItem
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_battery.mdl" );
		CItem::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_battery.mdl" );
		PRECACHE_SOUND( "items/kevlar.wav" );
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if( pPlayer->pev->deadflag != DEAD_NO )
		{
			return FALSE;
		}

		if( ( pPlayer->pev->armorvalue < MAX_NORMAL_BATTERY ) &&
			( pPlayer->pev->weapons & ( 1 << WEAPON_SUIT ) ) )
		{
			int pct;

			pPlayer->pev->armorvalue += gSkillData.batteryCapacity;
			pPlayer->pev->armorvalue = Q_min( pPlayer->pev->armorvalue, MAX_NORMAL_BATTERY );

			EMIT_SOUND( pPlayer->edict(), CHAN_ITEM, "items/kevlar.wav", 1, ATTN_NORM );

			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev );
				WRITE_STRING( STRING( pev->classname ) );
			MESSAGE_END();

			// Suit reports new power level
			// For some reason this wasn't working in release build -- round it.
			pct = (int)( (float)( pPlayer->pev->armorvalue * 100.0f ) * ( 1.0f / MAX_NORMAL_BATTERY ) + 0.5f );
			pct = ( pct / 5 );
			if( pct > 0 )
				pct--;

			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( item_battery, CItemBattery )

class CItemAntidote : public CItem
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_antidote.mdl" );
		CItem::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_antidote.mdl" );
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		pPlayer->m_rgItems[ITEM_ANTIDOTE] += 1;
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS( item_antidote, CItemAntidote )

class CItemSecurity : public CItem
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_security.mdl" );
		CItem::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_security.mdl" );
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		pPlayer->m_rgItems[ITEM_SECURITY] += 1;
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS( item_security, CItemSecurity )

class CItemLongJump : public CItem
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_longjump.mdl" );
		CItem::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_longjump.mdl" );
		PRECACHE_SOUND( "radio/letsgo.wav" ); //Haunter
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if( pPlayer->m_fLongJump )
		{
			return FALSE;
		}

		if( ( pPlayer->pev->weapons & ( 1 << WEAPON_SUIT ) ) )
		{
			pPlayer->m_fLongJump = TRUE;// player now has longjump module

			g_engfuncs.pfnSetPhysicsKeyValue( pPlayer->edict(), "slj", "1" );
			EMIT_SOUND( pPlayer->edict(), CHAN_ITEM, "radio/letsgo.wav", 1, ATTN_NORM ); //Haunter

			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev );
				WRITE_STRING( STRING( pev->classname ) );
			MESSAGE_END();

			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( item_longjump, CItemLongJump )

class CBigMoney : public CBasePlayerItem
{
	void Spawn( void )
	{
		Precache( );
		// motor
		pev->movetype = MOVETYPE_TOSS;
		pev->solid = SOLID_TRIGGER;
		UTIL_SetOrigin( pev, pev->origin );
		UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));

		SET_MODEL(ENT(pev), "models/w_backpack.mdl");

		SetTouch( &CBigMoney::GiveTouch );

		pev->takedamage		= DAMAGE_NO;

		//pev->dmg = 100;//Base money value
		if( CVAR_GET_FLOAT("cl_getcash") == 1 )
		{
			m_iAmount = 250;
		}
		else if( CVAR_GET_FLOAT("cl_getcash") != 1 )
		{
			m_iAmount = 1500;
		}
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_backpack.mdl");
		PRECACHE_SOUND("items/flip.wav");
	}
	void EXPORT GiveTouch( CBaseEntity *pOther )
	{
		if( pOther->IsPlayer() )
		{
			CBasePlayer *pPlayer = (CBasePlayer *)pOther;

			if( g_pGameRules->IsMultiplayer() )
			{
				if( pPlayer->m_iMoney < MP_MAXMONEY )
				{
					pev->model = iStringNull;// make invisible
					SetThink( &CBaseEntity::SUB_Remove );
					SetTouch( NULL );
					pev->nextthink = gpGlobals->time + 0.1;

					pPlayer->m_iMoney += 1500;
					if( pPlayer->m_iMoney > MP_MAXMONEY )
						pPlayer->m_iMoney = MP_MAXMONEY;
					EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/flip.wav", 1, ATTN_NORM);
				}
			}
			else
			{
				if( pPlayer->m_iMoney < CVAR_GET_FLOAT("cl_maxmoney") )
				{
					if( CVAR_GET_FLOAT("cl_maxmoney") > 950000000 )
					{
						return;
					}
					else
					{
						pev->model = iStringNull;// make invisible
						SetThink( &CBaseEntity::SUB_Remove );
						SetTouch( NULL );
						pev->nextthink = gpGlobals->time + 0.1;
						if( CVAR_GET_FLOAT("cl_getcash") == 1 )
						{
							pPlayer->m_iMoney += 250;
						}
						else if( CVAR_GET_FLOAT("cl_getcash") != 1 )
						{
							pPlayer->m_iMoney += 1500;
						}
						EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/flip.wav", 1, ATTN_NORM);
					}
				}
			}
		}
	}
	short int m_iAmount;
};

//LINK_ENTITY_TO_CLASS( weaponbox, CBigMoney );

class CTempMoney : public CBasePlayerItem
{
	void Spawn( void )
	{
		Precache( );
		// motor
		pev->movetype = MOVETYPE_TOSS;
		pev->solid = SOLID_TRIGGER;
		UTIL_SetOrigin( pev, pev->origin );
		UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));


		SET_MODEL(ENT(pev), "models/money.mdl");

		SetTouch( &CTempMoney::GiveTouch );

		pev->takedamage		= DAMAGE_NO;

		//pev->dmg = 100;//Base money value
		if( CVAR_GET_FLOAT("cl_getcash") == 1 )
		{
			m_iAmount = 50;
		}
		else if( CVAR_GET_FLOAT("cl_getcash") != 1 )
		{
			m_iAmount = 300;
		}
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/money.mdl");
		PRECACHE_SOUND("items/flip.wav");
	}
	void EXPORT GiveTouch( CBaseEntity *pOther )
	{
		if( pOther->IsPlayer() )
		{
			CBasePlayer *pPlayer = (CBasePlayer *)pOther;

			if( g_pGameRules->IsMultiplayer() )
			{
				if( pPlayer->m_iMoney < MP_MAXMONEY )
				{
					pev->model = iStringNull;// make invisible
					SetThink( &CBaseEntity::SUB_Remove );
					SetTouch( NULL );
					pev->nextthink = gpGlobals->time + 0.1;

					pPlayer->m_iMoney += 300;
					if( pPlayer->m_iMoney > MP_MAXMONEY )
						pPlayer->m_iMoney = MP_MAXMONEY;
					EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/flip.wav", 1, ATTN_NORM);
				}
			}
			else
			{
				if( pPlayer->m_iMoney < CVAR_GET_FLOAT("cl_maxmoney") )
				{
					if( CVAR_GET_FLOAT("cl_maxmoney") > 950000000 )
					{
						return;
					}
					else
					{
						pev->model = iStringNull;// make invisible
						SetThink( &CBaseEntity::SUB_Remove );
						SetTouch( NULL );
						pev->nextthink = gpGlobals->time + 0.1;
						if( CVAR_GET_FLOAT("cl_getcash") == 1 )
						{
							pPlayer->m_iMoney += 50;
						}
						else if( CVAR_GET_FLOAT("cl_getcash") != 1 )
						{
							pPlayer->m_iMoney += 300;
						}
						EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/flip.wav", 1, ATTN_NORM);
					}
				}
			}
		}
	}
	short int m_iAmount;
};

LINK_ENTITY_TO_CLASS( weapon_crossbow, CTempMoney )
LINK_ENTITY_TO_CLASS( ammo_crossbow, CTempMoney )
//Knife - LINK_ENTITY_TO_CLASS( weapon_crowbar, CTempMoney )
LINK_ENTITY_TO_CLASS( weapon_hornetgun, CTempMoney )
LINK_ENTITY_TO_CLASS( weapon_egon, CTempMoney )
LINK_ENTITY_TO_CLASS( ammo_egonclip, CTempMoney )
LINK_ENTITY_TO_CLASS( weapon_gauss, CTempMoney )
LINK_ENTITY_TO_CLASS( ammo_gaussclip, CTempMoney )
LINK_ENTITY_TO_CLASS( weapon_glock, CTempMoney )
LINK_ENTITY_TO_CLASS( weapon_9mmhandgun, CTempMoney )
LINK_ENTITY_TO_CLASS( ammo_glockclip, CTempMoney )
LINK_ENTITY_TO_CLASS( ammo_9mmclip, CTempMoney )
LINK_ENTITY_TO_CLASS( weapon_handgrenade, CTempMoney )
LINK_ENTITY_TO_CLASS( weapon_mp5, CTempMoney )
LINK_ENTITY_TO_CLASS( weapon_9mmAR, CTempMoney )
LINK_ENTITY_TO_CLASS( ammo_mp5clip, CTempMoney )
LINK_ENTITY_TO_CLASS( ammo_9mmAR, CTempMoney )
LINK_ENTITY_TO_CLASS( ammo_9mmbox, CTempMoney )
LINK_ENTITY_TO_CLASS( ammo_mp5grenades, CTempMoney )
LINK_ENTITY_TO_CLASS( ammo_ARgrenades, CTempMoney )
LINK_ENTITY_TO_CLASS( weapon_python, CTempMoney )
LINK_ENTITY_TO_CLASS( weapon_357, CTempMoney )
LINK_ENTITY_TO_CLASS( ammo_357, CTempMoney )
LINK_ENTITY_TO_CLASS( weapon_rpg, CTempMoney )
LINK_ENTITY_TO_CLASS( ammo_rpgclip, CTempMoney )
LINK_ENTITY_TO_CLASS( weapon_satchel, CTempMoney )
LINK_ENTITY_TO_CLASS( weapon_shotgun, CTempMoney )
LINK_ENTITY_TO_CLASS( ammo_buckshot, CTempMoney )
LINK_ENTITY_TO_CLASS( weapon_snark, CTempMoney )
LINK_ENTITY_TO_CLASS( weapon_tripmine, CTempMoney )
