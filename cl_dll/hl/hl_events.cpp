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

#include "../hud.h"
#include "../cl_util.h"
#include "event_api.h"

extern "C"
{
// HLDM
//Haunter
void EV_Knife( struct event_args_s *args );

void EV_USP( struct event_args_s *args );
void EV_Glock18( struct event_args_s *args );
void EV_Deagle( struct event_args_s *args );
void EV_P228( struct event_args_s *args );
void EV_ELEFT( struct event_args_s *args );
void EV_ERIGHT( struct event_args_s *args );
void EV_FiveseveN( struct event_args_s *args );

void EV_M3( struct event_args_s *args );
void EV_XM1014( struct event_args_s *args );

void EV_MP5N( struct event_args_s *args );
void EV_TMP( struct event_args_s *args );
void EV_P90( struct event_args_s *args );
void EV_MAC10( struct event_args_s *args );
void EV_UMP45( struct event_args_s *args );

void EV_AK47( struct event_args_s *args );
void EV_M4A1( struct event_args_s *args );
void EV_M4A12( struct event_args_s *args );
void EV_SG552( struct event_args_s *args );
void EV_AUG( struct event_args_s *args );
void EV_FAMAS( struct event_args_s *args );
void EV_GALIL( struct event_args_s *args );

void EV_AWP( struct event_args_s *args );
void EV_SCOUT( struct event_args_s *args );
void EV_G3SG1( struct event_args_s *args );
void EV_SG550( struct event_args_s *args );

void EV_M249( struct event_args_s *args );
void EV_RPGRENADE( struct event_args_s *args );
void EV_ULTIMATE( struct event_args_s *args );
void EV_ULTIMATE2( struct event_args_s *args );
//Haunter

void EV_TrainPitchAdjust( struct event_args_s *args );
void EV_VehiclePitchAdjust( event_args_t *args );
}

/*
======================
Game_HookEvents

Associate script file name with callback functions.  Callback's must be extern "C" so
 the engine doesn't get confused about name mangling stuff.  Note that the format is
 always the same.  Of course, a clever mod team could actually embed parameters, behavior
 into the actual .sc files and create a .sc file parser and hook their functionality through
 that.. i.e., a scripting system.

That was what we were going to do, but we ran out of time...oh well.
======================
*/
void Game_HookEvents( void )
{
	//Haunter
	gEngfuncs.pfnHookEvent( "events/knife.sc", EV_Knife );

	gEngfuncs.pfnHookEvent( "events/usp.sc", EV_USP );
	gEngfuncs.pfnHookEvent( "events/glock18.sc", EV_Glock18 );
	gEngfuncs.pfnHookEvent( "events/deagle.sc", EV_Deagle );
	gEngfuncs.pfnHookEvent( "events/p228.sc", EV_P228 );
	gEngfuncs.pfnHookEvent( "events/elite_left.sc", EV_ELEFT );
	gEngfuncs.pfnHookEvent( "events/elite_right.sc", EV_ERIGHT );
	gEngfuncs.pfnHookEvent( "events/fiveseven.sc", EV_FiveseveN );

	gEngfuncs.pfnHookEvent( "events/m3.sc", EV_M3 );
	gEngfuncs.pfnHookEvent( "events/xm1014.sc", EV_XM1014 );

	gEngfuncs.pfnHookEvent( "events/mp5n.sc", EV_MP5N );
	gEngfuncs.pfnHookEvent( "events/tmp.sc", EV_TMP );
	gEngfuncs.pfnHookEvent( "events/p90.sc", EV_P90 );
	gEngfuncs.pfnHookEvent( "events/mac10.sc", EV_MAC10 );
	gEngfuncs.pfnHookEvent( "events/ump45.sc", EV_UMP45 );

	gEngfuncs.pfnHookEvent( "events/ak47.sc", EV_AK47 );
	gEngfuncs.pfnHookEvent( "events/m4a1.sc", EV_M4A1 );
	gEngfuncs.pfnHookEvent( "events/m4a12.sc", EV_M4A12 );
	gEngfuncs.pfnHookEvent( "events/sg552.sc", EV_SG552 );
	gEngfuncs.pfnHookEvent( "events/aug.sc", EV_AUG );
	gEngfuncs.pfnHookEvent( "events/famas.sc", EV_FAMAS );
	gEngfuncs.pfnHookEvent( "events/galil.sc", EV_GALIL );

	gEngfuncs.pfnHookEvent( "events/awp.sc", EV_AWP );
	gEngfuncs.pfnHookEvent( "events/scout.sc", EV_SCOUT );
	gEngfuncs.pfnHookEvent( "events/g3sg1.sc", EV_G3SG1 );
	gEngfuncs.pfnHookEvent( "events/sg550.sc", EV_SG550 );

	gEngfuncs.pfnHookEvent( "events/m249.sc", EV_M249 );
	gEngfuncs.pfnHookEvent( "events/rpgrenade.sc", EV_RPGRENADE );
	gEngfuncs.pfnHookEvent( "events/ultimate.sc", EV_ULTIMATE );
	gEngfuncs.pfnHookEvent( "events/ultimate2.sc", EV_ULTIMATE2 );
	//Haunter

	gEngfuncs.pfnHookEvent( "events/train.sc", EV_TrainPitchAdjust );
	gEngfuncs.pfnHookEvent( "events/vehicle.sc", EV_VehiclePitchAdjust );
}
