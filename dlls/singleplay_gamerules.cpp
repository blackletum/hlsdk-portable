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
//
// teamplay_gamerules.cpp
//

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
#include	"skill.h"
#include	"items.h"
#include	"maxcarry.h"

//Atomizer
extern int gmsgShowMenu;
extern int art;

void ShowMenu(CBasePlayer *pPlayer, int bitsValidSlots, int nDisplayTime, BOOL fNeedMore, const char *pszText)
{
	MESSAGE_BEGIN( MSG_ONE, gmsgShowMenu, NULL, pPlayer ->pev);
		WRITE_SHORT( bitsValidSlots);
		WRITE_CHAR( nDisplayTime );
		WRITE_BYTE( fNeedMore );
		WRITE_STRING (pszText);
	MESSAGE_END();
}
//Atom

extern DLL_GLOBAL CGameRules	*g_pGameRules;
extern DLL_GLOBAL BOOL	g_fGameOver;
extern int gmsgDeathMsg;	// client dll messages
extern int gmsgScoreInfo;
extern int gmsgMOTD;

//=========================================================
//=========================================================
CHalfLifeRules::CHalfLifeRules( void )
{
	SERVER_COMMAND( "exec spserver.cfg\n" );
	RefreshSkillData();
}

//=========================================================
//=========================================================
void CHalfLifeRules::Think( void )
{
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules::IsMultiplayer( void )
{
	return FALSE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules::IsDeathmatch( void )
{
	return FALSE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules::IsCoOp( void )
{
	return FALSE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules::FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon )
{
	if( !pPlayer->m_pActiveItem )
	{
		// player doesn't have an active item!
		return TRUE;
	}

	if( !pPlayer->m_iAutoWepSwitch )
	{
		return FALSE;
	}

	if( pPlayer->m_iAutoWepSwitch == 2
	    && pPlayer->m_afButtonLast & ( IN_ATTACK | IN_ATTACK2 ) )
	{
		return FALSE;
	}

	if( !pPlayer->m_pActiveItem->CanHolster() )
	{
		return FALSE;
	}

	return TRUE;
}

//=========================================================
//=========================================================
BOOL HLGetNextBestWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon )
{
	CBasePlayerItem *pCheck;
	CBasePlayerItem *pBest;// this will be used in the event that we don't find a weapon in the same category.
	int iBestWeight;
	int i;

	iBestWeight = -1;// no weapon lower than -1 can be autoswitched to
	pBest = NULL;

	if( !pCurrentWeapon->CanHolster() )
	{
		// can't put this gun away right now, so can't switch.
		return FALSE;
	}

	for( i = 0; i < MAX_ITEM_TYPES; i++ )
	{
		pCheck = pPlayer->m_rgpPlayerItems[i];

		while( pCheck )
		{
			if( !FBitSet( pCheck->iFlags(), ITEM_FLAG_NOAUTOSWITCHTO ))
			{
				if( pCheck->iWeight() > -1 && pCheck->iWeight() == pCurrentWeapon->iWeight() && pCheck != pCurrentWeapon )
				{
					// this weapon is from the same category.
					if ( pCheck->CanDeploy() )
					{
						if ( pPlayer->SwitchWeapon( pCheck ) )
						{
							return TRUE;
						}
					}
				}
				else if( pCheck->iWeight() > iBestWeight && pCheck != pCurrentWeapon )// don't reselect the weapon we're trying to get rid of
				{
					//ALERT ( at_console, "Considering %s\n", STRING( pCheck->pev->classname ) );
					// we keep updating the 'best' weapon just in case we can't find a weapon of the same weight
					// that the player was using. This will end up leaving the player with his heaviest-weighted
					// weapon.
					if( pCheck->CanDeploy() )
					{
						// if this weapon is useable, flag it as the best
						iBestWeight = pCheck->iWeight();
						pBest = pCheck;
					}
				}
			}

			pCheck = pCheck->m_pNext;
		}
	}

	// if we make it here, we've checked all the weapons and found no useable
	// weapon in the same catagory as the current weapon.

	// if pBest is null, we didn't find ANYTHING. Shouldn't be possible- should always
	// at least get the crowbar, but ya never know.
	if( !pBest )
	{
		return FALSE;
	}

	pPlayer->SwitchWeapon( pBest );

	return TRUE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules::GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon )
{
	if( pCurrentWeapon && FBitSet( pCurrentWeapon->iFlags(), ITEM_FLAG_EXHAUSTIBLE ))
		return HLGetNextBestWeapon( pPlayer, pCurrentWeapon );
	return FALSE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules::ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128] )
{
	return TRUE;
}

void CHalfLifeRules::InitHUD( CBasePlayer *pl )
{
}

//=========================================================
//=========================================================
void CHalfLifeRules::ClientDisconnected( edict_t *pClient )
{
}

//Atomizer
//=========================================================
// ClientCommand
// the user has typed a command which is unrecognized by everything else;
// this check to see if the gamerules knows anything about the command
//=========================================================
BOOL CHalfLifeRules :: ClientCommand( CBasePlayer *pPlayer, const char *pcmd )
{
	long int TimeHS;

	pPlayer->m_iTimeSP = (int)CVAR_GET_FLOAT( "cl_buytime" );
	if ( pPlayer->m_iTimeSP <= 0)
	{
		pPlayer->m_iTimeSP = 1;
	}
	extern float g_flWeaponCheat;
	
	if ( FStrEq( pcmd, "zangle" ) )
	{
		if ( CMD_ARGC() < 2 )
			return TRUE;

		int angle = atoi( CMD_ARGV(1) );

		pPlayer->pev->v_angle.z = angle;

		return TRUE;
	}
	else if ( FStrEq( pcmd, "xangle" ) )
	{
		if ( CMD_ARGC() < 2 )
			return TRUE;

		int angle = atoi( CMD_ARGV(1) );

		pPlayer->pev->v_angle.x = angle;

		return TRUE;
	}
	else if ( FStrEq( pcmd, "yangle" ) )
	{
		if ( CMD_ARGC() < 2 )
			return TRUE;

		int angle = atoi( CMD_ARGV(1) );

		pPlayer->pev->v_angle.y = angle;

		return TRUE;
	}
	else if ( FStrEq(pcmd, "buy" ) )
	{
		if ( ( pPlayer->HasNamedPlayerItem("weapon_knife") ) && ( pPlayer->m_fLongJump == TRUE ) )
		{
			TimeHS = (int)CVAR_GET_FLOAT( "cl_buytime" ); //Haunter
			if ( TimeHS <= 0)
				TimeHS = 1;
						
			ShowMenu(pPlayer, 0x203, TimeHS, 0, "#BuyXen"); //default 0x3FF
			pPlayer->m_iMenu = 11;
			pPlayer->m_iBuyable = 1;
			return TRUE;
		}
		else
		{
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You need the Longjump Module\n" ) );
			return TRUE;
		}
	}
	else if ( FStrEq(pcmd, "terra" ) )
	{
			pPlayer->GiveNamedItem( "item_suit" );
			pPlayer->GiveNamedItem( "item_battery" );
			pPlayer->GiveNamedItem( "weapon_knife" );
			pPlayer->m_iMoney = CVAR_GET_FLOAT("cl_maxmoney");
			if (CVAR_GET_FLOAT("cl_maxmoney") > 950000000)
			{
				pPlayer->m_iMoney = 950000000;
			}
			return TRUE;
	}
	else if (FStrEq(pcmd, "money" ))
	{
			pPlayer->m_iMoney = CVAR_GET_FLOAT("cl_maxmoney");
			if (CVAR_GET_FLOAT("cl_maxmoney") > 950000000)
			{
				pPlayer->m_iMoney = 950000000;
			}
			return TRUE;
	}
	/*else if (FStrEq(pcmd, "bank" ))
	{
		ClientPrint(pPlayer->pev, HUD_PRINTTALK, UTIL_VarArgs( "Bank: $%i\n", pPlayer->m_iMoney ) );
		return TRUE;
	}
	/*else if (FStrEq(pcmd, "sil" ))
	{
		ClientPrint(pPlayer->pev, HUD_PRINTTALK, UTIL_VarArgs( "m_iSilencing %i\n", pPlayer->m_iSilencing ) );
		ClientPrint(pPlayer->pev, HUD_PRINTTALK, UTIL_VarArgs( "m_bIsSilencing %i\n", pPlayer->m_bIsSilencing ) );
		return TRUE;
	}*/
	else if ( FStrEq( pcmd, "menuselect" ) )
	{
		if ( CMD_ARGC() < 2 )
			return TRUE;

		int slot = atoi( CMD_ARGV(1) );

	if (pPlayer->m_iBuyable == 1)
	{
		if (pPlayer->m_iMenu == 1)
		{
			switch(slot)
			{
			case 1:
				if ( g_iSkillLevel == SKILL_EASY)
				{
                    ShowMenu(pPlayer, 0x23F, pPlayer->m_iTimeSP, 0, "#BuyPistolEasy"); //default 0x20F
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM )
				{
					ShowMenu(pPlayer, 0x23F, pPlayer->m_iTimeSP, 0, "#BuyPistol"); //default 0x20F
				}
				else if ( g_iSkillLevel == SKILL_HARD )
				{
					ShowMenu(pPlayer, 0x23F, pPlayer->m_iTimeSP, 0, "#BuyPistolHard"); //default 0x20F
				}
				pPlayer->m_iMenu = 2;
				break;
			case 2:
				if ( g_iSkillLevel == SKILL_EASY)
				{
					ShowMenu(pPlayer, 0x203, pPlayer->m_iTimeSP, 0, "#BuyShotgunEasy");
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					ShowMenu(pPlayer, 0x203, pPlayer->m_iTimeSP, 0, "#BuyShotgun");
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					ShowMenu(pPlayer, 0x203, pPlayer->m_iTimeSP, 0, "#BuyShotgunHard");
				}
				pPlayer->m_iMenu = 3;
				break;
			case 3:
				if ( g_iSkillLevel == SKILL_EASY)
				{
					ShowMenu(pPlayer, 0x21F, pPlayer->m_iTimeSP, 0, "#BuySubMachineGunEasy"); //default 0x207
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					ShowMenu(pPlayer, 0x21F, pPlayer->m_iTimeSP, 0, "#BuySubMachineGun"); //default 0x207
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					ShowMenu(pPlayer, 0x21F, pPlayer->m_iTimeSP, 0, "#BuySubMachineGunHard"); //default 0x207
				}
				pPlayer->m_iMenu = 4;
				break;
			case 4:
				ShowMenu(pPlayer, 0x203, pPlayer->m_iTimeSP, 0, "#ChooseType"); //default 0x277
				pPlayer->m_iMenu = 5;
				break;
			case 5:
				if ( g_iSkillLevel == SKILL_EASY)
				{
					ShowMenu(pPlayer, 0x207, pPlayer->m_iTimeSP, 0, "#BuyHeavyEasy");
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					ShowMenu(pPlayer, 0x207, pPlayer->m_iTimeSP, 0, "#BuyHeavy");
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					ShowMenu(pPlayer, 0x207, pPlayer->m_iTimeSP, 0, "#BuyHeavyHard");
				}
				pPlayer->m_iMenu = 6;
				break;
			case 6:
				if ( pPlayer->m_iArmor == 1)
				{
					ShowMenu(pPlayer, 0x21F, pPlayer->m_iTimeSP, 0, "#BuyEquip1");
				}
			
				if ( pPlayer->m_iArmor == 0 )
				{
					ShowMenu(pPlayer, 0x21F, pPlayer->m_iTimeSP, 0, "#BuyEquip2");
				}
				pPlayer->m_iMenu = 7;
				break;
			case 7:
				ShowMenu(pPlayer, 0x23F, pPlayer->m_iTimeSP, 0, "#BuyAmmo");
				pPlayer->m_iMenu = 8;
			    break;
			}
		}
		else if (pPlayer->m_iMenu == 11)
		{
			switch(slot)
			{
				case 1:
					ShowMenu(pPlayer, 0x20F, pPlayer->m_iTimeSP, 0, "#BuyEquipXen");
					pPlayer->m_iMenu = 12;
					break;
				case 2:
					ShowMenu(pPlayer, 0x23F, pPlayer->m_iTimeSP, 0, "#BuyAmmo");
					pPlayer->m_iMenu = 8;
					break;
			}
		}
		else if (pPlayer->m_iMenu == 2) //Pistols
		{
			switch(slot)
			{
			case 1: //Glock18
				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 200)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 400)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 600)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				
				if ( pPlayer->HasNamedPlayerItem("weapon_usp") | pPlayer->HasNamedPlayerItem("weapon_glock18") | pPlayer->HasNamedPlayerItem("weapon_deagle") | pPlayer->HasNamedPlayerItem("weapon_p228") 
					| pPlayer->HasNamedPlayerItem("weapon_fiveseven") | pPlayer->HasNamedPlayerItem("weapon_elite"))
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a handgun\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_glock18" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 200;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 400;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 600;	
				}
				pPlayer->m_iBuyable = 0;
				break;
			case 2: //USP .45
				/*if(pPlayer->m_iMoney < 500)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 250)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 500)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 750)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_usp") | pPlayer->HasNamedPlayerItem("weapon_glock18") | pPlayer->HasNamedPlayerItem("weapon_deagle") | pPlayer->HasNamedPlayerItem("weapon_p228") 
					| pPlayer->HasNamedPlayerItem("weapon_fiveseven") | pPlayer->HasNamedPlayerItem("weapon_elite"))
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a handgun\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_usp" );
				
				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 250;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 500;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 750;	
				}
				
				pPlayer->m_iBuyable = 0;
				break;
			case 3: //P228
				/*if(pPlayer->m_iMoney < 600)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 300)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 600)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 900)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_usp") | pPlayer->HasNamedPlayerItem("weapon_glock18") | pPlayer->HasNamedPlayerItem("weapon_deagle") | pPlayer->HasNamedPlayerItem("weapon_p228") 
					| pPlayer->HasNamedPlayerItem("weapon_fiveseven") | pPlayer->HasNamedPlayerItem("weapon_elite")) 
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a handgun\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_p228" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 300;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 600;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 900;	
				}

				pPlayer->m_iBuyable = 0;
				break;
			case 4: //Deagle
				/*if(pPlayer->m_iMoney < 650)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 350)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 650)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 1000)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_usp") | pPlayer->HasNamedPlayerItem("weapon_glock18") | pPlayer->HasNamedPlayerItem("weapon_deagle") | pPlayer->HasNamedPlayerItem("weapon_p228") 
					| pPlayer->HasNamedPlayerItem("weapon_fiveseven") | pPlayer->HasNamedPlayerItem("weapon_elite"))
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a handgun\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_deagle" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 350;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 650;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 1000;	
				}
				pPlayer->m_iBuyable = 0;
				break;
			case 5:
			/*	if(pPlayer->m_iMoney < 750) //FiveseveN
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/
			
				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 350)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 750)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 1100)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_usp") | pPlayer->HasNamedPlayerItem("weapon_glock18") | pPlayer->HasNamedPlayerItem("weapon_deagle") | pPlayer->HasNamedPlayerItem("weapon_p228") 
					| pPlayer->HasNamedPlayerItem("weapon_fiveseven") | pPlayer->HasNamedPlayerItem("weapon_elite"))
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a handgun\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_fiveseven" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 350;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 750;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 1100;	
				}
				pPlayer->m_iBuyable = 0;
				break;
			case 6:
				/*if(pPlayer->m_iMoney < 800) //Elites
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 400)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 800)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 1200)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_usp") | pPlayer->HasNamedPlayerItem("weapon_glock18") | pPlayer->HasNamedPlayerItem("weapon_deagle") | pPlayer->HasNamedPlayerItem("weapon_p228") 
					| pPlayer->HasNamedPlayerItem("weapon_fiveseven") | pPlayer->HasNamedPlayerItem("weapon_elite"))
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a handgun\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_elite" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 400;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 800;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 1200;	
				}
				pPlayer->m_iBuyable = 0;
				break;
			}
			pPlayer->m_iMenu = 0;
		}
		else if (pPlayer->m_iMenu == 3) //Shotguns
		{
			switch(slot)
			{
			case 1://M3 Super 90
				/*if(pPlayer->m_iMoney < 1700)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 850)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 1700)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 2550)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_m3") | pPlayer->HasNamedPlayerItem("weapon_xm1014"))
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a shotgun\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_m3" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 850;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 1700;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 2550;	
				}
				pPlayer->m_iBuyable = 0;
				break;
			case 2: //M1014

				/*if(pPlayer->m_iMoney < 3000)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 1500)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 3000)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 4500)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_m3") | pPlayer->HasNamedPlayerItem("weapon_xm1014"))
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a shotgun\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_xm1014" );
				
				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 1500;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 3000;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 4500;	
				}
				pPlayer->m_iBuyable = 0;
				break;
			}
			pPlayer->m_iMenu = 0;
		}
		else if (pPlayer->m_iMenu == 4) //Sub-Machine guns
		{
			switch(slot)
			{
			case 1: //TMP
				/*if(pPlayer->m_iMoney < 1250)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 650)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 1250)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 1900)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_mp5navy") | pPlayer->HasNamedPlayerItem("weapon_tmp") | pPlayer->HasNamedPlayerItem("weapon_p90") | pPlayer->HasNamedPlayerItem("weapon_mac10") | pPlayer->HasNamedPlayerItem("weapon_ump45"))
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a Sub-Machine Gun\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_tmp" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 650;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 1250;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 1900;	
				}
				pPlayer->m_iBuyable = 0;
				break;
			case 2://MP5
				/*
				if(pPlayer->m_iMoney < 1500) //MP5
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 750)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 1500)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 2250)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_mp5navy") | pPlayer->HasNamedPlayerItem("weapon_tmp") | pPlayer->HasNamedPlayerItem("weapon_p90") | pPlayer->HasNamedPlayerItem("weapon_mac10") | pPlayer->HasNamedPlayerItem("weapon_ump45"))
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a Sub-Machine Gun\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_mp5navy" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 750;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 1500;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 2250;	
				}
				pPlayer->m_iBuyable = 0;
				break;
			case 3: //UMP.45
				/*if(pPlayer->m_iMoney < 1700) //UMP .45
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 850)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 1700)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 2550)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_mp5navy") | pPlayer->HasNamedPlayerItem("weapon_tmp") | pPlayer->HasNamedPlayerItem("weapon_p90") | pPlayer->HasNamedPlayerItem("weapon_mac10") | pPlayer->HasNamedPlayerItem("weapon_ump45"))
				{
					//UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "You already have a Sub-Machine Gun\n" ) );
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a Sub-Machine Gun\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_ump45" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 850;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 1700;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 2550;	
				}
				pPlayer->m_iBuyable = 0;
				break;
			case 4://P90
				/*if(pPlayer->m_iMoney < 2350) //P90
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 1170)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 2350)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 3520)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_mp5navy") | pPlayer->HasNamedPlayerItem("weapon_tmp") | pPlayer->HasNamedPlayerItem("weapon_p90") | pPlayer->HasNamedPlayerItem("weapon_mac10") | pPlayer->HasNamedPlayerItem("weapon_ump45"))
				{
					//UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "You already have a Sub-Machine Gun\n" ) );
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a Sub-Machine Gun\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_p90" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 1170;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 2350;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 3520;	
				}
				pPlayer->m_iBuyable = 0;
				break;
			case 5://Mac10
				/*if(pPlayer->m_iMoney < 1400) //Mac10
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 700)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 1400)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 2100)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_mp5navy") | pPlayer->HasNamedPlayerItem("weapon_tmp") | pPlayer->HasNamedPlayerItem("weapon_p90") | pPlayer->HasNamedPlayerItem("weapon_mac10") | pPlayer->HasNamedPlayerItem("weapon_ump45"))
				{
					//UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "You already have a Sub-Machine Gun\n" ) );
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a Sub-Machine Gun\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_mac10" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 700;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 1400;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 2100;	
				}
				pPlayer->m_iBuyable = 0;
				break;
			}
			pPlayer->m_iMenu = 0;
		}
		else if (pPlayer->m_iMenu == 5) //Rifles
		{
			switch(slot)
			{
			case 1:
				if ( g_iSkillLevel == SKILL_EASY)
				{
                    ShowMenu(pPlayer, 0x23F, pPlayer->m_iTimeSP, 0, "#BuyAssaultRifleEasy");
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM )
				{
					ShowMenu(pPlayer, 0x23F, pPlayer->m_iTimeSP, 0, "#BuyAssaultRifle");
				}
				else if ( g_iSkillLevel == SKILL_HARD )
				{
					ShowMenu(pPlayer, 0x23F, pPlayer->m_iTimeSP, 0, "#BuyAssaultRifleHard");
				}
				pPlayer->m_iMenu = 9;
				break;
			case 2:
				if ( g_iSkillLevel == SKILL_EASY)
				{
                    ShowMenu(pPlayer, 0x20F, pPlayer->m_iTimeSP, 0, "#BuySniperRifleEasy");
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM )
				{
					ShowMenu(pPlayer, 0x20F, pPlayer->m_iTimeSP, 0, "#BuySniperRifle");
				}
				else if ( g_iSkillLevel == SKILL_HARD )
				{
					ShowMenu(pPlayer, 0x20F, pPlayer->m_iTimeSP, 0, "#BuySniperRifleHard");
				}
				pPlayer->m_iMenu = 10;
				break;
			}
		}
	
		else if ( pPlayer->m_iMenu == 9)
		{
			switch(slot)
			{
			case 1://Galil
				/*if(pPlayer->m_iMoney < 2000 ) //Galil
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/
				
				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 1000)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 2000)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 3000)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_famas") | pPlayer->HasNamedPlayerItem("weapon_galil") | 
					pPlayer->HasNamedPlayerItem("weapon_ak47") | pPlayer->HasNamedPlayerItem("weapon_aug") | 
					pPlayer->HasNamedPlayerItem("weapon_m4a1") | pPlayer->HasNamedPlayerItem("weapon_sg552") )
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have an assault rifle\n" ) );
					return TRUE;
				}
					
				pPlayer->GiveNamedItem( "weapon_galil" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 1000;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 2000;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 3000;	
				}
				
				pPlayer->m_iBuyable = 0;
				break;			
			case 2://FAMAS
				/*if(pPlayer->m_iMoney < 2250) //FAMAS
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 1150)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 2250)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 3400)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_famas") | pPlayer->HasNamedPlayerItem("weapon_galil") | 
					pPlayer->HasNamedPlayerItem("weapon_ak47") | pPlayer->HasNamedPlayerItem("weapon_aug") | 
					pPlayer->HasNamedPlayerItem("weapon_m4a1") | pPlayer->HasNamedPlayerItem("weapon_sg552") )
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have an assault rifle\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_famas" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 1150;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 2250;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 3400;	
				}
				
				pPlayer->m_iBuyable = 0;
				break;			
			case 3://AK47
				/*if(pPlayer->m_iMoney < 2500) //AK47
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 1750)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 2500)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 4250)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_famas") | pPlayer->HasNamedPlayerItem("weapon_galil") | 
					pPlayer->HasNamedPlayerItem("weapon_ak47") | pPlayer->HasNamedPlayerItem("weapon_aug") | 
					pPlayer->HasNamedPlayerItem("weapon_m4a1") | pPlayer->HasNamedPlayerItem("weapon_sg552") )
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have an assault rifle\n" ) );
					return TRUE;
				}
					
				pPlayer->GiveNamedItem( "weapon_ak47" );
			
				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 1750;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 2500;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 4250;	
				}
				
				pPlayer->m_iBuyable = 0;
				break;
			case 4://M4A1
				/*if(pPlayer->m_iMoney < 3500) //M4A1 w/M203
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 1550)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 3100)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 4650)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_famas") | pPlayer->HasNamedPlayerItem("weapon_galil") | 
					pPlayer->HasNamedPlayerItem("weapon_ak47") | pPlayer->HasNamedPlayerItem("weapon_aug") | 
					pPlayer->HasNamedPlayerItem("weapon_m4a1") | pPlayer->HasNamedPlayerItem("weapon_sg552") )
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have an assault rifle\n" ) );
					return TRUE;
				}
			
				pPlayer->GiveNamedItem( "weapon_m4a1" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 1550;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 3100;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 4650;	
				}
				
				pPlayer->m_iBuyable = 0;
				break;

			case 5://Aug
				/*if(pPlayer->m_iMoney < 3500) //AUG
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 1750)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 3500)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 5250)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_famas") | pPlayer->HasNamedPlayerItem("weapon_galil") | 
					pPlayer->HasNamedPlayerItem("weapon_ak47") | pPlayer->HasNamedPlayerItem("weapon_aug") | 
					pPlayer->HasNamedPlayerItem("weapon_m4a1") | pPlayer->HasNamedPlayerItem("weapon_sg552") )
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have an assault rifle\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_aug" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 1750;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 3500;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 5250;	
				}
				
				pPlayer->m_iBuyable = 0;
				break;
			
			case 6://SG552
				/*if(pPlayer->m_iMoney < 3500) //SG552
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 1750)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 3500)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 5250)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_famas") | pPlayer->HasNamedPlayerItem("weapon_galil") | 
					pPlayer->HasNamedPlayerItem("weapon_ak47") | pPlayer->HasNamedPlayerItem("weapon_aug") | 
					pPlayer->HasNamedPlayerItem("weapon_m4a1") | pPlayer->HasNamedPlayerItem("weapon_sg552") )
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have an assault rifle\n" ) );
					return TRUE;
				}
					
				pPlayer->GiveNamedItem( "weapon_sg552" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 1750;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 3500;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 5250;	
				}
				
				pPlayer->m_iBuyable = 0;
				break;
			}
			pPlayer->m_iMenu = 0;
		}
		
		else if ( pPlayer->m_iMenu == 10)
		{
			switch(slot)
			{
			case 1://Scout
				/*if(pPlayer->m_iMoney < 2750) //Scout
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 1380)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 2750)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 4130)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_scout") | pPlayer->HasNamedPlayerItem("weapon_awp") |
					pPlayer->HasNamedPlayerItem("weapon_g3sg1") | pPlayer->HasNamedPlayerItem("weapon_sg550"))
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a sniper rifle\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_scout" );
				
				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 1380;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 2750;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 4130;	
				}
				
				pPlayer->m_iBuyable = 0;
				break;
			case 2://AW/M
				/*if(pPlayer->m_iMoney < 4750) //AW/M
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 2380)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 4750)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 7130)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				
				if ( pPlayer->HasNamedPlayerItem("weapon_scout") | pPlayer->HasNamedPlayerItem("weapon_awp") |
					pPlayer->HasNamedPlayerItem("weapon_g3sg1") | pPlayer->HasNamedPlayerItem("weapon_sg550") )
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a sniper rifle\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_awp" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 2380;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 4750;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 7130;	
				}
				
				pPlayer->m_iBuyable = 0;
				break;
			
			case 3://SG550
				/*if(pPlayer->m_iMoney < 4200) //SG550
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 2100)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 4200)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 6300)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_scout") | pPlayer->HasNamedPlayerItem("weapon_awp") |
					pPlayer->HasNamedPlayerItem("weapon_g3sg1") | pPlayer->HasNamedPlayerItem("weapon_sg550") )
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a sniper rifle\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_sg550" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 2100;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 4200;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 6300;	
				}
				
				pPlayer->m_iBuyable = 0;
				break;
			
			case 4://G3SG1
				/*if(pPlayer->m_iMoney < 5000) //G3SG1
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 2500)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 5000)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 7500)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_scout") | pPlayer->HasNamedPlayerItem("weapon_awp") |
					pPlayer->HasNamedPlayerItem("weapon_g3sg1") | pPlayer->HasNamedPlayerItem("weapon_sg550"))
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a sniper rifle\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_g3sg1" );

				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 2500;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 5000;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 7500;	
				}
				
				pPlayer->m_iBuyable = 0;
				break;	
			}
			pPlayer->m_iMenu = 0;
		}
		else if (pPlayer->m_iMenu == 6) //Machine guns
		{
			switch(slot)
			{
			case 1://M249
				/*if(pPlayer->m_iMoney < 5750) //M249
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 2880)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 5750)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 8630)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_m249") )
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a Machine Gun\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_m249" );
				
				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 2880;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 5750;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 8630;	
				}
				
				pPlayer->m_iBuyable = 0;
				break;
			
			case 2://RPG
				/*if(pPlayer->m_iMoney < 4500)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}*/

				if ( g_iSkillLevel == SKILL_EASY)
				{
					if(pPlayer->m_iMoney < 2250)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					if(pPlayer->m_iMoney < 4500)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					if(pPlayer->m_iMoney < 6750)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_rpgrenade") )
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have a LAW\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_rpgrenade" );
				
				if ( g_iSkillLevel == SKILL_EASY)
				{
					pPlayer->m_iMoney -= 2250;	
				}
				else if ( g_iSkillLevel == SKILL_MEDIUM)
				{
					pPlayer->m_iMoney -= 4500;	
				}
				else if ( g_iSkillLevel == SKILL_HARD)
				{
					pPlayer->m_iMoney -= 6750;	
				}
				
				pPlayer->m_iBuyable = 0;
				break;

			case 3:
				int woot;
				woot = CVAR_GET_FLOAT("cl_maxmoney");
				if (woot > 950000000)
				{
					woot = 950000000;
				}
				if( pPlayer->m_iMoney < woot )
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "Try getting $%i\n", woot ) );
					return TRUE;
				}

				if ( pPlayer->HasNamedPlayerItem("weapon_ultimate") )
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "ERROR 404!*@#LOLZZ!!\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "weapon_ultimate" );
				pPlayer->m_iMoney -= woot;
				pPlayer->m_iBuyable = 0;
				break;
			}
			pPlayer->m_iMenu = 0;
		}
		else if (pPlayer->m_iMenu == 7) //Equipment
		{
			switch(slot)
			{
			case 1:
				if ( pPlayer->m_iArmor == 1) //Sells Armor
				{
					if( pPlayer->m_iMoney < 650 )
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if( pPlayer->pev->armorvalue >= 100)
					{
						return TRUE;
					}
						
						pPlayer->m_iMoney -= (((pPlayer->GiveArmor( 100 )) * 0) + 650);
						//ClientPrint(pPlayer->pev, HUD_PRINTTALK, UTIL_VarArgs( "Bank: $%i\n", pPlayer->m_iMoney ) );
				}	
			
				if ( pPlayer->m_iArmor == 0) //Sells Health
				{
					if( pPlayer->m_iMoney <= 0 )
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					pPlayer->m_iMoney -= ((pPlayer->TakeHealth( 100, DMG_GENERIC )) * 0) + 100;
				}
				pPlayer->m_iBuyable = 0;
				break;
		/*	case 2:
				if(pPlayer->m_iMoney < 200)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}

				if(pPlayer->m_iNumFlashbangs >= 5)
				{
					pPlayer->m_iNumFlashbangs = 5; //Just incase
					//UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "You cannot carry anymore Flashbangs\n" ) );
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have 5 Flashbangs!\n" ) );
					return TRUE;
				}
				
				pPlayer->m_iNumFlashbangs++;
				pPlayer->m_iFB++;
				pPlayer->GiveNamedItem( "weapon_flashbang" );
				pPlayer->m_iMoney -= 200;
				break;*/
			case 2:
				if(pPlayer->m_iMoney < 300)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}
				
				if(pPlayer->m_iNumHEGrenades >= HELOOP)
				{
					pPlayer->m_iNumHEGrenades = HELOOP; //Just incase
					//UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "You cannot carry anymore HE Grenades\n" ) );
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have 10 HEGrenades\n" ) );
					return TRUE;
				}

				pPlayer->m_iNumHEGrenades++;
				pPlayer->m_iHE++;
				pPlayer->GiveNamedItem( "weapon_hegrenade" );
				pPlayer->m_iMoney -= 300;
				pPlayer->m_iBuyable = 0;
				break;
		/*	case 4:
				if(pPlayer->m_iMoney < 200)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}
				if( pPlayer->m_iNumFlashbangs >= 5)
				{
					pPlayer->m_iNumFlashbangs = 5; //Just incase
					//UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "You cannot carry anymore Flashbangs\n" ) );
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have 5 Flashbangs!\n" ) );
					return TRUE;
				}
				
				for( pPlayer->m_iNumFlashbangs = pPlayer->m_iFB; pPlayer->m_iNumFlashbangs <= 5; pPlayer->m_iNumFlashbangs++)
				{
					if ( (pPlayer->m_iMoney < 200) || (pPlayer->m_iNumFlashbangs == 5 ))
					{
						pPlayer->m_iFB = pPlayer->m_iNumFlashbangs; 
						break;
					}
					pPlayer->GiveNamedItem( "weapon_flashbang" );
					pPlayer->m_iMoney -= 200;
				}
				break;*/
			case 3:
				if(pPlayer->m_iMoney < 300)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}

				if(	pPlayer->m_iNumHEGrenades >= HELOOP)
				{
					pPlayer->m_iNumHEGrenades = HELOOP; //Just incase
					//UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "You cannot carry anymore HE Grenades\n" ) );
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You cannot carry anymore\n" ) );
					return TRUE;
				}
				
				for ( pPlayer->m_iNumHEGrenades = pPlayer->m_iHE; pPlayer->m_iNumHEGrenades <= HELOOP; pPlayer->m_iNumHEGrenades++)
				{
					if ( (pPlayer->m_iMoney < 300) || (pPlayer->m_iNumHEGrenades == HELOOP ))
					{
						pPlayer->m_iHE = pPlayer->m_iNumHEGrenades;
						break;
					}
					else
					{
						pPlayer->GiveNamedItem( "weapon_hegrenade" );
					}
					pPlayer->m_iMoney -= 300;
				}
				pPlayer->m_iBuyable = 0;
				break;
			case 4:
				if(pPlayer->m_iMoney < 1500)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}
				
				if(pPlayer->m_iNumC4 >= C4LOOP)
				{
					pPlayer->m_iNumC4 = C4LOOP; //Just incase
					//UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "You cannot carry anymore HE Grenades\n" ) );
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have 3 C4s\n" ) );
					return TRUE;
				}

				pPlayer->m_iNumC4++;
				pPlayer->m_iC4++;
				pPlayer->GiveNamedItem( "weapon_c4" );
				pPlayer->m_iMoney -= 1500;
				pPlayer->m_iBuyable = 0;
				break;
			case 5:
				if(pPlayer->m_iMoney < 1500)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}

				if(	pPlayer->m_iNumC4 >= C4LOOP)
				{
					pPlayer->m_iNumC4 = C4LOOP; //Just incase
					//UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "You cannot carry anymore HE Grenades\n" ) );
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You cannot carry anymore\n" ) );
					return TRUE;
				}
				
				for ( pPlayer->m_iNumC4 = pPlayer->m_iC4; pPlayer->m_iNumC4 <= C4LOOP; pPlayer->m_iNumC4++)
				{
					if ( (pPlayer->m_iMoney < 1500) || (pPlayer->m_iNumC4 == C4LOOP ))
					{
						pPlayer->m_iC4 = pPlayer->m_iNumC4;
						break;
					}
					else
					{
						pPlayer->GiveNamedItem( "weapon_c4" );
					}
					pPlayer->m_iMoney -= 1500;
				}
				
				pPlayer->m_iBuyable = 0;
				break;
			/*case 4:
				if(pPlayer->m_iMoney < 1250)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}
				if ( pPlayer->m_fNVG == TRUE )
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have one\n" ) );
					return TRUE;
				}

				pPlayer->GiveNamedItem( "item_nightvision" );
				pPlayer->m_iMoney -= 1250;
				pPlayer->m_iBuyable = 0;
				break;*/
			}
			pPlayer->m_iMenu = 0;// select the item from the current menu
			}
		else if (pPlayer->m_iMenu == 12) //Equipment for Xen
		{
			switch(slot)
			{
			case 1:
				if(pPlayer->m_iMoney < 300)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}
				
				if(pPlayer->m_iNumHEGrenades >= HELOOP)
				{
					pPlayer->m_iNumHEGrenades = HELOOP; //Just incase
					//UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "You cannot carry anymore HE Grenades\n" ) );
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have 10 HEGrenades\n" ) );
					return TRUE;
				}

				pPlayer->m_iNumHEGrenades++;
				pPlayer->m_iHE++;
				pPlayer->GiveNamedItem( "weapon_hegrenade" );
				pPlayer->m_iMoney -= 300;
				pPlayer->m_iBuyable = 0;
				break;
		
			case 2:
				if(pPlayer->m_iMoney < 300)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}

				if(	pPlayer->m_iNumHEGrenades >= HELOOP)
				{
					pPlayer->m_iNumHEGrenades = HELOOP; //Just incase
					//UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "You cannot carry anymore HE Grenades\n" ) );
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You cannot carry anymore\n" ) );
					return TRUE;
				}
				
				for ( pPlayer->m_iNumHEGrenades = pPlayer->m_iHE; pPlayer->m_iNumHEGrenades <= HELOOP; pPlayer->m_iNumHEGrenades++)
				{
					if ( (pPlayer->m_iMoney < 300) || (pPlayer->m_iNumHEGrenades == HELOOP ))
					{
						pPlayer->m_iHE = pPlayer->m_iNumHEGrenades;
						break;
					}
					else
					{
						pPlayer->GiveNamedItem( "weapon_hegrenade" );
					}
					pPlayer->m_iMoney -= 300;
				}
				pPlayer->m_iBuyable = 0;
				break;
			
			case 3:
				if(pPlayer->m_iMoney < 1500)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}
				
				if(pPlayer->m_iNumC4 >= C4LOOP)
				{
					pPlayer->m_iNumC4 = C4LOOP; //Just incase
					//UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "You cannot carry anymore HE Grenades\n" ) );
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You already have 3 C4s\n" ) );
					return TRUE;
				}

				pPlayer->m_iNumC4++;
				pPlayer->m_iC4++;
				pPlayer->GiveNamedItem( "weapon_c4" );
				pPlayer->m_iMoney -= 1500;
				pPlayer->m_iBuyable = 0;
				break;
			
			case 4:
				if(pPlayer->m_iMoney < 1500)
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
					return TRUE;
				}

				if(	pPlayer->m_iNumC4 >= C4LOOP)
				{
					pPlayer->m_iNumC4 = C4LOOP; //Just incase
					//UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "You cannot carry anymore HE Grenades\n" ) );
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You cannot carry anymore\n" ) );
					return TRUE;
				}
				
				for ( pPlayer->m_iNumC4 = pPlayer->m_iC4; pPlayer->m_iNumC4 <= C4LOOP; pPlayer->m_iNumC4++)
				{
					if ( (pPlayer->m_iMoney < 1500) || (pPlayer->m_iNumC4 == C4LOOP ))
					{
						pPlayer->m_iC4 = pPlayer->m_iNumC4;
						break;
					}
					else
					{
						pPlayer->GiveNamedItem( "weapon_c4" );
					}
					pPlayer->m_iMoney -= 1500;
				}
				
				pPlayer->m_iBuyable = 0;
				break;
			}
			pPlayer->m_iMenu = 0;// select the item from the current menu
			}
		else if (pPlayer->m_iMenu == 8) //ammunition
		{
			switch(slot)
			{
			case 1:
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_KNIFE || pPlayer->m_pActiveItem->m_iId == WEAPON_FLASHBANG || pPlayer->m_pActiveItem->m_iId == WEAPON_HEGRENADE )
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You do not need ammunition for this\n" ) );
					return TRUE;
				}

				//guns using 5.7mm rounds
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_P90 || pPlayer->m_pActiveItem->m_iId == WEAPON_FIVESEVEN)
				{
					if (pPlayer->m_iMoney < 50)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_i57 == _57LOOP )
					{
						return TRUE;
					}

					if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
					{
						pPlayer->GiveNamedItem("ammo_P90clip");
						pPlayer->m_iMoney -= 50;
					}
				}
				
				//guns using DOT .50AE rounds (seems like I only know this guys full name :) ) 
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_DEAGLE )
				{
					if (pPlayer->m_iMoney < 40)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_i50 == _50LOOP )
					{
						return TRUE;
					}

					if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
					{
						pPlayer->GiveNamedItem("ammo_DEclip");
						pPlayer->m_iMoney -= 40;
					}
				}

				//guns using .45ACP round (same for this guy too. heh heh. :) )
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_USP || pPlayer->m_pActiveItem->m_iId == WEAPON_MAC10 || pPlayer->m_pActiveItem->m_iId == WEAPON_UMP45 )
				{
					if (pPlayer->m_iMoney < 25)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_i45 == _45LOOP )
					{
						return TRUE;
					}

					if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
					{
						pPlayer->GiveNamedItem("ammo_USPclip");
						pPlayer->m_iMoney -= 25;
					}
				}

				//guns using 9mm rounds
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_GLOCK18 || pPlayer->m_pActiveItem->m_iId == WEAPON_MP5N || pPlayer->m_pActiveItem->m_iId == WEAPON_TMP || pPlayer->m_pActiveItem->m_iId == WEAPON_ELITE)
				{
					if (pPlayer->m_iMoney < 20)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
					
					if ( pPlayer->m_i9mm == _9MMLOOP )
					{
						return TRUE;
					}
					
					if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
					{
						pPlayer->GiveNamedItem("ammo_MP5clip");
						pPlayer->m_iMoney -= 20;
					}
				}

				//guns using .357 SIG rounds
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_P228)
				{
					if (pPlayer->m_iMoney < 50)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
					
					if ( pPlayer->m_i357 == _357LOOP )
					{
						return TRUE;
					}
					
					if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
					{
						pPlayer->GiveNamedItem("ammo_P228clip");
						pPlayer->m_iMoney -= 50;
					}
				}

				//guns using .338 Magnum rounds
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_AWP )
				{
					if (pPlayer->m_iMoney < 125)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
					
					if ( pPlayer->m_i338 == _338LOOP )
					{
						return TRUE;
					}
					
					if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
					{
						pPlayer->GiveNamedItem("ammo_AWPclip");
						pPlayer->m_iMoney -= 125;
					}
				}
				
				//guns using 7.62mm rounds
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_AK47 || pPlayer->m_pActiveItem->m_iId == WEAPON_SCOUT || pPlayer->m_pActiveItem->m_iId == WEAPON_G3SG1 )
				{
					if (pPlayer->m_iMoney < 80)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
					
					if ( pPlayer->m_i762 == _762LOOP )
					{
						return TRUE;
					}
					
					if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
					{
						pPlayer->GiveNamedItem("ammo_AK47clip");
					    pPlayer->m_iMoney -= 80;
					}
				}

				//guns using 5.56mm round. Note: I split the M249 rounds with the rest even though
				//it also uses 5.56mm. This will not let players buy a clip of 100 5.56mm rounds.
				//Ha ha ha!
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_M4A1 || pPlayer->m_pActiveItem->m_iId == WEAPON_SG552 || pPlayer->m_pActiveItem->m_iId == WEAPON_AUG || pPlayer->m_pActiveItem->m_iId == WEAPON_SG550 || pPlayer->m_pActiveItem->m_iId == WEAPON_GALIL || pPlayer->m_pActiveItem->m_iId == WEAPON_FAMAS )
				{
					if (pPlayer->m_iMoney < 60)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
					
					if ( pPlayer->m_i556 == _556LOOP )
					{
						return TRUE;
					}
					
					if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
					{
						pPlayer->GiveNamedItem("ammo_M4A1clip");
						pPlayer->m_iMoney -= 60;
					}
				}
				
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_M249 )
				{
					if (pPlayer->m_iMoney < 60)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}
					
					if ( pPlayer->m_i5562 == _5562LOOP )
					{
						return TRUE;
					}
					
					if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
					{
						pPlayer->GiveNamedItem("ammo_M249clip");
					    pPlayer->m_iMoney -= 60;
					}
				}

				//guns using 12 Gauge rounds
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_XM1014 || pPlayer->m_pActiveItem->m_iId == WEAPON_M3)
				{
					if (pPlayer->m_iMoney < 65)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_i12G == _12GLOOP )
					{
						return TRUE;
					}
					
					if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
					{
						pPlayer->GiveNamedItem("ammo_XM4clip");
						pPlayer->m_iMoney -= 65;
					}
				}
				//For RPG
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_RPGRENADE)
				{
					if (pPlayer->m_iMoney < 250)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_iRPG == RPGLOOP )
					{
						return TRUE;
					}
					
					if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
					{
						pPlayer->GiveNamedItem("ammo_RPclip");
						pPlayer->m_iMoney -= 250;
					}
				}

				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_ULTIMATE)
				{
					if (pPlayer->m_iMoney < 300)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_iJud == JudgmentLOOP )
					{
						return TRUE;
					}
					
					if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
					{
						pPlayer->GiveNamedItem("ammo_judclip");
						EMIT_SOUND(ENT(pPlayer->pev), CHAN_VOICE, "weapons/ultimate_judgment.wav", 1, ATTN_NORM);
						pPlayer->m_iMoney -= 300;
					}
				}

				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_NONE)
				{
					return TRUE;
				}
				pPlayer->m_iBuyable = 0;
				break;
			case 2:
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_KNIFE || pPlayer->m_pActiveItem->m_iId == WEAPON_FLASHBANG || pPlayer->m_pActiveItem->m_iId == WEAPON_HEGRENADE )
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You do not need ammunition for this!\n" ) );
					return TRUE;
				}

				//For guns using 9mm rounds
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_GLOCK18 || pPlayer->m_pActiveItem->m_iId == WEAPON_MP5N || pPlayer->m_pActiveItem->m_iId == WEAPON_TMP || pPlayer->m_pActiveItem->m_iId == WEAPON_ELITE )
				{
					if (pPlayer->m_iMoney < 20)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_i9mm == _9MMLOOP )
					{
						return TRUE;
					}
				
					for ( pPlayer->m_i9mmAmmo = pPlayer->m_i9mm; pPlayer->m_i9mmAmmo <= _9MMLOOP; pPlayer->m_i9mmAmmo ++)
					{
						if ( (pPlayer->m_iMoney < 20) || (pPlayer->m_i9mmAmmo == _9MMLOOP ) )
						{
							pPlayer->m_i9mm = pPlayer->m_i9mmAmmo;
							break;
						}
						else
						{
							if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
							{
								pPlayer->GiveNamedItem("ammo_MP5clip");
								pPlayer->m_iMoney -= 20;
							}
						}
					}
				}
				//For guns using .357 rounds
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_P228 )
				{
					if (pPlayer->m_iMoney < 50)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_i357 == _357LOOP )
					{
						return TRUE;
					}
				
					for ( pPlayer->m_i357Ammo = pPlayer->m_i357; pPlayer->m_i357Ammo <= _357LOOP; pPlayer->m_i357Ammo ++)
					{
						if ( (pPlayer->m_iMoney < 50) || (pPlayer->m_i357Ammo == _357LOOP ) )
						{
							pPlayer->m_i357 = pPlayer->m_i357Ammo;
							break;
						}
						else
						{
							if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
							{
								pPlayer->GiveNamedItem("ammo_P228clip");
								pPlayer->m_iMoney -= 50;
							}
						}
					}
				}
				//For guns using 12 Gauge ammo
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_XM1014 || pPlayer->m_pActiveItem->m_iId == WEAPON_M3)
				{
					if (pPlayer->m_iMoney < 65)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_i12G == _12GLOOP )
					{
						return TRUE;
					}
				
					for ( pPlayer->m_i12GAmmo = pPlayer->m_i12G; pPlayer->m_i12GAmmo <= _12GLOOP; pPlayer->m_i12GAmmo ++)
					{
						if ( (pPlayer->m_iMoney < 65) || (pPlayer->m_i12GAmmo == _12GLOOP ) )
						{
							pPlayer->m_i12G = pPlayer->m_i12GAmmo;
							break;
						}
						else
						{
							if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
							{
								pPlayer->GiveNamedItem("ammo_XM4clip");
								pPlayer->m_iMoney -= 65;
							}
						}
					}
				}
				//For guns using 7.62mm rounds
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_AK47 || pPlayer->m_pActiveItem->m_iId == WEAPON_SCOUT || pPlayer->m_pActiveItem->m_iId == WEAPON_G3SG1 )
				{
					if (pPlayer->m_iMoney < 80)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_i762 == _762LOOP )
					{
						return TRUE;
					}
				
					for ( pPlayer->m_i762Ammo = pPlayer->m_i762; pPlayer->m_i762Ammo <= _762LOOP; pPlayer->m_i762Ammo ++)
					{
						if ( (pPlayer->m_iMoney < 80) || (pPlayer->m_i762Ammo == _762LOOP ) )
						{
							pPlayer->m_i762 = pPlayer->m_i762Ammo;
							break;
						}
						else
						{
							if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
							{
								pPlayer->GiveNamedItem("ammo_AK47clip");
								pPlayer->m_iMoney -= 80;
							}
						}
					}
				}
				//For guns using .50AE rounds
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_DEAGLE )
				{
					if (pPlayer->m_iMoney < 40)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_i50 == _50LOOP )
					{
						return TRUE;
					}
				
					for ( pPlayer->m_i50Ammo = pPlayer->m_i50; pPlayer->m_i50Ammo <= _50LOOP; pPlayer->m_i50Ammo ++)
					{
						if ( (pPlayer->m_iMoney < 40) || (pPlayer->m_i50Ammo == _50LOOP ) )
						{
							pPlayer->m_i50 = pPlayer->m_i50Ammo;
							break;
						}
						else
						{
							if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
							{
								pPlayer->GiveNamedItem("ammo_DEclip");
								pPlayer->m_iMoney -= 40;
							}
						}
					}
				}
				
				//For guns using 5.56mm rounds
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_M4A1 || pPlayer->m_pActiveItem->m_iId == WEAPON_SG552 || pPlayer->m_pActiveItem->m_iId == WEAPON_AUG || pPlayer->m_pActiveItem->m_iId == WEAPON_SG550 || pPlayer->m_pActiveItem->m_iId == WEAPON_GALIL || pPlayer->m_pActiveItem->m_iId == WEAPON_FAMAS )
				{
					if (pPlayer->m_iMoney < 60)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_i556 == _556LOOP	 )
					{
						return TRUE;
					}

					for ( pPlayer->m_i556Ammo = pPlayer->m_i556; pPlayer->m_i556Ammo <= _556LOOP	; pPlayer->m_i556Ammo ++)
					{
						if ( (pPlayer->m_iMoney < 60) || (pPlayer->m_i556Ammo == _556LOOP ) )
						{
							pPlayer->m_i556 = pPlayer->m_i556Ammo;
							break;
						}
						else
						{
							if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
							{
								pPlayer->GiveNamedItem("ammo_M4A1clip");
								pPlayer->m_iMoney -= 60;
							}
						}
					}
				}
				//For other guns using 5.56mm rounds
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_M249)
				{
					if (pPlayer->m_iMoney < 60)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_i5562 == _5562LOOP )
					{
						return TRUE;
					}

					for ( pPlayer->m_i5562Ammo = pPlayer->m_i5562; pPlayer->m_i5562Ammo <= _5562LOOP; pPlayer->m_i5562Ammo++)
					{
						if ( (pPlayer->m_iMoney < 60) || (pPlayer->m_i5562Ammo == _5562LOOP ) )
						{
							pPlayer->m_i5562 = pPlayer->m_i5562Ammo;
							break;
						}
						else
						{
							if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
							{
								pPlayer->GiveNamedItem("ammo_M249clip");
								pPlayer->m_iMoney -= 60;
							}
						}
					}
				}
				//For guns using .338Magnum rounds
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_AWP)
				{
					if (pPlayer->m_iMoney < 125)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_i338 == _338LOOP )
					{
						return TRUE;
					}
				
					for ( pPlayer->m_i338Ammo = pPlayer->m_i338; pPlayer->m_i338Ammo <= _338LOOP; pPlayer->m_i338Ammo ++)
					{
						if ( (pPlayer->m_iMoney < 125) || (pPlayer->m_i338Ammo == _338LOOP ) )
						{
							pPlayer->m_i338 = pPlayer->m_i338Ammo;
							break;
						}
						else
						{
							if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
							{
								pPlayer->GiveNamedItem("ammo_AWPclip");
								pPlayer->m_iMoney -= 125;
							}
						}
					}
				}
				//For guns using .45ACP
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_USP || pPlayer->m_pActiveItem->m_iId == WEAPON_MAC10 || pPlayer->m_pActiveItem->m_iId == WEAPON_UMP45 )
				{
					if (pPlayer->m_iMoney < 25)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_i45 == _45LOOP )
					{
						return TRUE;
					}
				
					for ( pPlayer->m_i45Ammo = pPlayer->m_i45; pPlayer->m_i45Ammo <= _45LOOP; pPlayer->m_i45Ammo ++)
					{
						if ( (pPlayer->m_iMoney < 25) || (pPlayer->m_i45Ammo == _45LOOP ) )
						{
							pPlayer->m_i45 = pPlayer->m_i45Ammo;
							break;
						}
						else
						{
							if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
							{
								pPlayer->GiveNamedItem("ammo_USPclip");
								pPlayer->m_iMoney -= 25;
							}
						}
					}
				}
				//For guns using 5.7mm rounds
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_P90 || pPlayer->m_pActiveItem->m_iId == WEAPON_FIVESEVEN)
				{
					if (pPlayer->m_iMoney < 50)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_i57 == _57LOOP )
					{
						return TRUE;
					}
				
					for ( pPlayer->m_i57Ammo = pPlayer->m_i57; pPlayer->m_i57Ammo <= _57LOOP; pPlayer->m_i57Ammo ++)
					{
						if ( (pPlayer->m_iMoney < 50) || (pPlayer->m_i57Ammo == _57LOOP ) )
						{
							pPlayer->m_i57 = pPlayer->m_i57Ammo;
							break;
						}
						else
						{
							if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
							{
								pPlayer->GiveNamedItem("ammo_P90clip");
								pPlayer->m_iMoney -= 50;
							}
						}
					}
				}
				//For RPG
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_RPGRENADE)
				{
					if (pPlayer->m_iMoney < 250)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_iRPG == RPGLOOP )
					{
						return TRUE;
					}
				
					for ( pPlayer->m_iRPGAmmo = pPlayer->m_iRPG; pPlayer->m_iRPGAmmo <= RPGLOOP; pPlayer->m_iRPGAmmo ++)
					{
						if ( (pPlayer->m_iMoney < 250) || (pPlayer->m_iRPGAmmo == RPGLOOP ) )
						{
							pPlayer->m_iRPG = pPlayer->m_iRPGAmmo;
							break;
						}
						else
						{
							if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
							{
								pPlayer->GiveNamedItem("ammo_RPclip");
								pPlayer->m_iMoney -= 250;
							}
						}
					}
				}

				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_ULTIMATE)
				{
					if (pPlayer->m_iMoney < 300)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_iJud == JudgmentLOOP )
					{
						return TRUE;
					}
				
					for ( pPlayer->m_iJudAmmo = pPlayer->m_iJud; pPlayer->m_iJudAmmo <= JudgmentLOOP; pPlayer->m_iJudAmmo ++)
					{
						if ( (pPlayer->m_iMoney < 300) || (pPlayer->m_iJudAmmo == JudgmentLOOP) )
						{
							pPlayer->m_iJud = pPlayer->m_iJudAmmo;
							break;
						}
						else
						{
							if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo1(), pPlayer->m_pActiveItem->iMaxAmmo1() ) > 0)
							{
								pPlayer->GiveNamedItem("ammo_judclip");
								pPlayer->m_iMoney -= 300;
								EMIT_SOUND(ENT(pPlayer->pev), CHAN_VOICE, "weapons/ultimate_judgment.wav", 1, ATTN_NORM);
							}
						}
					}
				}
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_NONE)
				{
					return TRUE;
				}
				pPlayer->m_iBuyable = 0;
				break;
			
			case 3:
				//For guns using M203 rounds
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_M4A1)
				{
					if (pPlayer->m_iMoney < 30)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_iM203 == M203LOOP )
					{
						return TRUE;
					}
					
					if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo2(), pPlayer->m_pActiveItem->iMaxAmmo2() ) > 0)
					{
						pPlayer->GiveNamedItem("ammo_M203Gren");
						pPlayer->m_iMoney -= 30;
					}
				}
				else if (!(pPlayer->m_pActiveItem->m_iId == WEAPON_M4A1))
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "It's for the M4A1!\n" ) );
					return TRUE;
				}
				
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_NONE )
				{
					return TRUE;
				}

				pPlayer->m_iBuyable = 0;
				break;
			
			case 4:
				//For guns using M203 rounds
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_M4A1 )
				{
					if (pPlayer->m_iMoney < 30)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_iM203 == M203LOOP )
					{
						return TRUE;
					}

					for ( pPlayer->m_iM203Ammo = pPlayer->m_iM203; pPlayer->m_iM203Ammo <= M203LOOP; pPlayer->m_iM203Ammo ++)
					{
						if ( (pPlayer->m_iMoney < 30) || (pPlayer->m_iM203Ammo == M203LOOP ) )
						{
							pPlayer->m_iM203 = pPlayer->m_iM203Ammo;
							break;
						}
						else
						{
							if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo2(), pPlayer->m_pActiveItem->iMaxAmmo2() ) > 0)
							{
								pPlayer->GiveNamedItem("ammo_M203Gren");
								pPlayer->m_iMoney -= 30;
							}
						}
					}
				}
				else if (!(pPlayer->m_pActiveItem->m_iId == WEAPON_M4A1))
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "It's for the M4A1!\n" ) );
					return TRUE;
				}
				
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_NONE )
				{
					return TRUE;
				}

				pPlayer->m_iBuyable = 0;
				break;
				
			case 5:
				//Ultimate
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_ULTIMATE)
				{
					if (pPlayer->m_iMoney < 400)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_iJus == JusticeLOOP )
					{
						return TRUE;
					}
					
					if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo2(), pPlayer->m_pActiveItem->iMaxAmmo2() ) > 0)
					{
						pPlayer->GiveNamedItem("ammo_jusrock");
						pPlayer->m_iMoney -= 400;
						EMIT_SOUND(ENT(pPlayer->pev), CHAN_VOICE, "weapons/ultimate_justice.wav", 1, ATTN_NORM);
					}
				}
				else if (!(pPlayer->m_pActiveItem->m_iId == WEAPON_ULTIMATE))
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "ERROR 404!*@#LOLZZ!!\n" ) );
					return TRUE;
				}
				
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_NONE )
				{
					return TRUE;
				}

				pPlayer->m_iBuyable = 0;
				break;
			
			case 6:
				//Ultimate
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_ULTIMATE )
				{
					if (pPlayer->m_iMoney < 400)
					{
						ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "You have insufficient funds!\n" ) );
						return TRUE;
					}

					if ( pPlayer->m_iJus == JusticeLOOP )
					{
						return TRUE;
					}

					for ( pPlayer->m_iJusAmmo = pPlayer->m_iJus; pPlayer->m_iJusAmmo <= JusticeLOOP; pPlayer->m_iJusAmmo ++)
					{
						if ( (pPlayer->m_iMoney < 400) || (pPlayer->m_iJusAmmo == JusticeLOOP ) )
						{
							pPlayer->m_iJus = pPlayer->m_iJusAmmo;
							break;
						}
						else
						{
							if (pPlayer->GiveAmmo( 0, (char *)pPlayer->m_pActiveItem->pszAmmo2(), pPlayer->m_pActiveItem->iMaxAmmo2() ) > 0)
							{
								pPlayer->GiveNamedItem("ammo_jusrock");
								pPlayer->m_iMoney -= 400;
								EMIT_SOUND(ENT(pPlayer->pev), CHAN_VOICE, "weapons/ultimate_justice.wav", 1, ATTN_NORM);
							}
						}
					}
				}
				else if (!(pPlayer->m_pActiveItem->m_iId == WEAPON_ULTIMATE))
				{
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs( "ERROR 404!*@#LOLZZ!!\n" ) );
					return TRUE;
				}
				
				if ( pPlayer->m_pActiveItem->m_iId == WEAPON_NONE )
				{
					return TRUE;
				}

				pPlayer->m_iBuyable = 0;
				break;
			}
			pPlayer->m_iMenu = 0;
		}
		return TRUE;
	}
	}
	else if (pPlayer->m_iBuyable == 0)
	{
		if ( FStrEq( pcmd, "menuselect" ) )
		{
			if ( CMD_ARGC() < 2 )
			return TRUE;

		int slot = atoi( CMD_ARGV(1) );
		if (pPlayer->m_iMenu == 0)
		{
			switch(slot)
			{
			case 1:
				pPlayer->m_iMenu = 0;
				break;
			case 2:
				pPlayer->m_iMenu = 0;
				break;
			case 3:
				pPlayer->m_iMenu = 0;
				break;
			case 4:
				pPlayer->m_iMenu = 0;
				break;
			case 5:
				pPlayer->m_iMenu = 0;
				break;
			case 6:
				pPlayer->m_iMenu = 0;
				break;
			case 7:
				pPlayer->m_iMenu = 0;
				break;
			case 8:
				pPlayer->m_iMenu = 0;
				break;
			}
		}
		}
		return TRUE;
	}
	return FALSE;
}
//Atom
//=========================================================
//=========================================================
float CHalfLifeRules::FlPlayerFallDamage( CBasePlayer *pPlayer )
{
	// subtract off the speed at which a player is allowed to fall without being hurt,
	// so damage will be based on speed beyond that, not the entire fall
	pPlayer->m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
	return pPlayer->m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
}

//=========================================================
//=========================================================
void CHalfLifeRules::PlayerSpawn( CBasePlayer *pPlayer )
{
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules::AllowAutoTargetCrosshair( void )
{
	return ( g_iSkillLevel == SKILL_EASY );
}

//=========================================================
//=========================================================
void CHalfLifeRules::PlayerThink( CBasePlayer *pPlayer )
{
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules::FPlayerCanRespawn( CBasePlayer *pPlayer )
{
	return TRUE;
}

//=========================================================
//=========================================================
float CHalfLifeRules::FlPlayerSpawnTime( CBasePlayer *pPlayer )
{
	return gpGlobals->time;//now!
}

//=========================================================
// IPointsForKill - how many points awarded to anyone
// that kills this player?
//=========================================================
int CHalfLifeRules::IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled )
{
	return 1;
}

//=========================================================
// PlayerKilled - someone/something killed this player
//=========================================================
void CHalfLifeRules::PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor )
{
}

//=========================================================
// Deathnotice
//=========================================================
void CHalfLifeRules::DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor )
{
}

//=========================================================
// PlayerGotWeapon - player has grabbed a weapon that was
// sitting in the world
//=========================================================
void CHalfLifeRules::PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon )
{
}

//=========================================================
// FlWeaponRespawnTime - what is the time in the future
// at which this weapon may spawn?
//=========================================================
float CHalfLifeRules::FlWeaponRespawnTime( CBasePlayerItem *pWeapon )
{
	return -1;
}

//=========================================================
// FlWeaponRespawnTime - Returns 0 if the weapon can respawn 
// now,  otherwise it returns the time at which it can try
// to spawn again.
//=========================================================
float CHalfLifeRules::FlWeaponTryRespawn( CBasePlayerItem *pWeapon )
{
	return 0;
}

//=========================================================
// VecWeaponRespawnSpot - where should this weapon spawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHalfLifeRules::VecWeaponRespawnSpot( CBasePlayerItem *pWeapon )
{
	return pWeapon->pev->origin;
}

//=========================================================
// WeaponShouldRespawn - any conditions inhibiting the
// respawning of this weapon?
//=========================================================
int CHalfLifeRules::WeaponShouldRespawn( CBasePlayerItem *pWeapon )
{
	return GR_WEAPON_RESPAWN_NO;
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules::CanHaveItem( CBasePlayer *pPlayer, CItem *pItem )
{
	return TRUE;
}

//=========================================================
//=========================================================
void CHalfLifeRules::PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem )
{
}

//=========================================================
//=========================================================
int CHalfLifeRules::ItemShouldRespawn( CItem *pItem )
{
	return GR_ITEM_RESPAWN_NO;
}

//=========================================================
// At what time in the future may this Item respawn?
//=========================================================
float CHalfLifeRules::FlItemRespawnTime( CItem *pItem )
{
	return -1;
}

//=========================================================
// Where should this item respawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHalfLifeRules::VecItemRespawnSpot( CItem *pItem )
{
	return pItem->pev->origin;
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules::IsAllowedToSpawn( CBaseEntity *pEntity )
{
	return TRUE;
}

//=========================================================
//=========================================================
void CHalfLifeRules::PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount )
{
}

//=========================================================
//=========================================================
int CHalfLifeRules::AmmoShouldRespawn( CBasePlayerAmmo *pAmmo )
{
	return GR_AMMO_RESPAWN_NO;
}

//=========================================================
//=========================================================
float CHalfLifeRules::FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo )
{
	return -1;
}

//=========================================================
//=========================================================
Vector CHalfLifeRules::VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo )
{
	return pAmmo->pev->origin;
}

//=========================================================
//=========================================================
float CHalfLifeRules::FlHealthChargerRechargeTime( void )
{
	return 0;// don't recharge
}

//=========================================================
//=========================================================
int CHalfLifeRules::DeadPlayerWeapons( CBasePlayer *pPlayer )
{
	return GR_PLR_DROP_GUN_NO;
}

//=========================================================
//=========================================================
int CHalfLifeRules::DeadPlayerAmmo( CBasePlayer *pPlayer )
{
	return GR_PLR_DROP_AMMO_NO;
}

//=========================================================
//=========================================================
int CHalfLifeRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
	// why would a single player in half life need this? 
	return GR_NOTTEAMMATE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeRules::FAllowMonsters( void )
{
	return TRUE;
}
