/*Haunter
For the Sniper Shourd*/
#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"

DECLARE_MESSAGE(m_Bank, Bank )


int CHudBank::Init(void)
{
	//message  being hooked
	HOOK_MESSAGE( Bank );

	m_iBank = CVAR_GET_FLOAT( "cl_startmoney" );
	gHUD.AddHudElem(this);

	return 1;
};

int CHudBank::VidInit(void)
{
	return 1;
};

int CHudBank::Draw(float fTime)
{
	if (gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT)))
	{
		//Haunter
		// gHUD. is added to DrawHudString Function because of its class in gHUD
		gHUD.DrawHudNumberString(ScreenWidth / 12, ScreenHeight / 1.1 , 0, m_iBank, 0, 150, 0);
		//Haunter
	}

	return 1;
}

int CHudBank::MsgFunc_Bank(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );	

	// update Shroud data
	m_iBank = READ_LONG();

	//gEngfuncs.Con_Printf("Data sent: %d\n", m_iLos);

	if (m_iBank >= 0)
		m_iFlags |= HUD_ACTIVE;
	else
		m_iFlags &= ~HUD_ACTIVE;

	return 1;
}