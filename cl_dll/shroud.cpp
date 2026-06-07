/*Haunter
For the Sniper Shourd*/
#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"

DECLARE_MESSAGE(m_Shroud, Shroud )


int CHudShroud::Init(void)
{
	//message  being hooked
	HOOK_MESSAGE( Shroud );

	m_iLos = 0;
	m_iFlags = 0;
	gHUD.AddHudElem(this);

	return 1;
};

int CHudShroud::VidInit(void)
{
	
	Bottom = 0;
	Bot_Right = 0;	
	Bot_Left = 0;
	Top = 0;
	Top_Right = 0;
	Top_Left = 0;
	Right = 0;
	Left = 0;
	
	Bottom2 = 0;
	Bot_Right2 = 0;
	Bot_Left2 = 0;
	Top2 = 0;
	Top_Right2 = 0;
	Top_Left2 = 0;
	Right2 = 0;
	Left2 = 0;
	
	Bottom3 = 0;
	Bot_Right3 = 0;
	Bot_Left3 = 0;
	Top3 = 0;
	Top_Right3 = 0;
	Top_Left3 = 0;
	Right3 = 0;
	Left3 = 0;
	
	Verc = 0;
	Hor = 0;

	//declaration of sprites

	if ( !Bottom )
		Bottom = LoadSprite("sprites/bottom.spr"); //bottom
	if ( !Bot_Right )
		Bot_Right = LoadSprite("sprites/bottom_right.spr"); //bottom right
	if ( !Bot_Left )
		Bot_Left = LoadSprite("sprites/bottom_left.spr"); //bottom left
	if ( !Top )
		Top = LoadSprite("sprites/top.spr"); //top
	if ( !Top_Right )
		Top_Right = LoadSprite("sprites/top_right.spr"); //top right
	if ( !Top_Left )
		Top_Left = LoadSprite("sprites/top_left.spr"); //top left
	if ( !Right )
		Right = LoadSprite("sprites/right.spr"); //right
	if ( !Left )
		Left = LoadSprite("sprites/left.spr"); //left

	if ( !Bottom2 )
		Bottom2 = LoadSprite("sprites/bottom2.spr"); //bottom
	if ( !Bot_Right2 )
		Bot_Right2 = LoadSprite("sprites/bottom_right2.spr"); //bottom right
	if ( !Bot_Left2 )
		Bot_Left2 = LoadSprite("sprites/bottom_left2.spr"); //bottom left
	if ( !Top2 )
		Top2 = LoadSprite("sprites/top2.spr"); //top
	if ( !Top_Right2 )
		Top_Right2 = LoadSprite("sprites/top_right2.spr"); //top right
	if ( !Top_Left2 )
		Top_Left2 = LoadSprite("sprites/top_left2.spr"); //top left
	if ( !Right2 )
		Right2 = LoadSprite("sprites/right2.spr"); //right
	if ( !Left2 )
		Left2 = LoadSprite("sprites/left2.spr"); //left

	if ( !Bottom3 )
		Bottom3 = LoadSprite("sprites/bottom3.spr"); //bottom
	if ( !Bot_Right3 )
		Bot_Right3 = LoadSprite("sprites/bottom_right3.spr"); //bottom right
	if ( !Bot_Left3 )
		Bot_Left3 = LoadSprite("sprites/bottom_left3.spr"); //bottom left
	if ( !Top3 )
		Top3 = LoadSprite("sprites/top3.spr"); //top
	if ( !Top_Right3 )
		Top_Right3 = LoadSprite("sprites/top_right3.spr"); //top right
	if ( !Top_Left3 )
		Top_Left3 = LoadSprite("sprites/top_left3.spr"); //top left
	if ( !Right3 )
		Right3 = LoadSprite("sprites/right3.spr"); //right
	if ( !Left3 )
		Left3 = LoadSprite("sprites/left3.spr"); //left

	if ( !Verc )
		Verc = LoadSprite("sprites/vertical.spr"); //vertical
	if ( !Hor )
		Hor = LoadSprite("sprites/horizontal.spr"); //horizontal

	return 1;
};

int CHudShroud::Draw(float fTime)
{
	int A, B, C, D;
	int E, F, G, H;
	int I;
		
	//Draw the sprites
	if ( gHUD.m_iFOV < 90 && ScreenWidth <= 640 )
	{
		//Sprites measurements
		A = (ScreenWidth/2) - 128;
		B = (ScreenHeight/2) + 128;
		C = (ScreenWidth/2) + 128;
		D = (ScreenWidth/2) - 319;
		E = (ScreenHeight/2) - 240;
		F = (ScreenWidth/2) - 320;
	
		G = (ScreenHeight/2) - 128;
		I = (ScreenHeight/2) - 270;

		//sprites being set drawn
		SPR_Set(Bottom, 255, 255, 255 );
		SPR_DrawHoles( 0,  A, B, NULL);//Bottom

		SPR_Set( Bot_Right, 255, 255, 255 );
		SPR_DrawHoles( 0,  C, B, NULL ); //Bottom right

		SPR_Set(Bot_Left, 255, 255, 255 );
		SPR_DrawHoles( 0,  D, B, NULL); //Bottom left

		SPR_Set(Top, 255, 255, 255 );
		SPR_DrawHoles( 0,  A, E, NULL);//Top

		SPR_Set(Top_Right, 255, 255, 255 );
		SPR_DrawHoles( 0,  C, E, NULL);//Top right

		SPR_Set(Top_Left, 255, 255, 255 );
		SPR_DrawHoles( 0,  D, E, NULL); //Top left

		SPR_Set(Right, 255, 255, 255 );
		SPR_DrawHoles( 0,  C, G, NULL); //right

		SPR_Set(Left, 255, 255, 255 );
		SPR_DrawHoles( 0,  D, G, NULL); //left

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  F, B, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  F, G, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  F, E, NULL); //vertical

		SPR_Set(Hor, 255, 255, 255 );
		SPR_DrawHoles( 0,  A, I, NULL); //horizontal

		SPR_Set(Hor, 255, 255, 255 );
		SPR_DrawHoles( 0,  C, I, NULL); //horizontal

		SPR_Set(Hor, 255, 255, 255 );
		SPR_DrawHoles( 0,  D, I, NULL); //horizontal
	}
	else if ( gHUD.m_iFOV < 90 && ScreenWidth == 800)
	{
		//Sprites measurements
		A = (ScreenWidth/2) - 128;
		B = (ScreenHeight/2) + 126;
		C = (ScreenWidth/2) + 127;
		D = (ScreenWidth/2) - 383;
		E = (ScreenHeight/2) - 302;
		F = (ScreenWidth/2) - 400;
		G = (ScreenHeight/2) - 128;

		H = (ScreenWidth/2) + 370;

		//sprites being set drawn
		SPR_Set(Bottom2, 255, 255, 255 );
		SPR_DrawHoles( 0,  A, B, NULL);//Bottom

		SPR_Set(Bot_Right2, 255, 255, 255 );
		SPR_DrawHoles( 0,  C, B, NULL); //Bottom right
		
		SPR_Set(Bot_Left2, 255, 255, 255 );
		SPR_DrawHoles( 0,  D, B, NULL); //Bottom left

		SPR_Set(Top2, 255, 255, 255 );
		SPR_DrawHoles( 0,  A, E, NULL);//Top

		SPR_Set(Top_Right2, 255, 255, 255 );
		SPR_DrawHoles( 0,  C, E, NULL);//Top right

		SPR_Set(Top_Left2, 255, 255, 255 );
		SPR_DrawHoles( 0,  D, E, NULL); //Top left

		SPR_Set(Right2, 255, 255, 255 );
		SPR_DrawHoles( 0,  C, G, NULL); //right

		SPR_Set(Left2, 255, 255, 255 );
		SPR_DrawHoles( 0,  D, G, NULL); //left

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  F, B, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  F, G, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  F, E, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  H, B, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  H, G, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  H, E, NULL); //vertical
	}
	else if ( gHUD.m_iFOV < 90 && ScreenWidth > 800 )
	{
		int I, J, K;
		int L, M, N;
		//Sprites measurements
		A = (ScreenWidth/2) - 128;
		B = (ScreenHeight/2) + 128;
		C = (ScreenWidth/2) + 128;
		D = (ScreenWidth/2) - 384;
		E = (ScreenHeight/2) - 384;
		F = (ScreenWidth/2) - 423;
		G = (ScreenHeight/2) - 128;

		H = (ScreenWidth/2) + 370;

		I = (ScreenWidth/2) - 460;
		J = (ScreenWidth/2) - 490;
		K = (ScreenWidth/2) - 520;

		L = (ScreenWidth/2) + 410;
		M = (ScreenWidth/2) + 440;
		N = (ScreenWidth/2) + 480;

		//sprites being set drawn
		SPR_Set(Bottom3, 255, 255, 255 );
		SPR_DrawHoles( 0,  A, B, NULL);//Bottom

		SPR_Set(Bot_Right3, 255, 255, 255 );
		SPR_DrawHoles( 0,  C, B, NULL); //Bottom right
		
		SPR_Set(Bot_Left3, 255, 255, 255 );
		SPR_DrawHoles( 0,  D, B, NULL); //Bottom left

		SPR_Set(Top3, 255, 255, 255 );
		SPR_DrawHoles( 0,  A, E, NULL);//Top

		SPR_Set(Top_Right3, 255, 255, 255 );
		SPR_DrawHoles( 0,  C, E, NULL);//Top right

		SPR_Set(Top_Left3, 255, 255, 255 );
		SPR_DrawHoles( 0,  D, E, NULL); //Top left

		SPR_Set(Right3, 255, 255, 255 );
		SPR_DrawHoles( 0,  C, G, NULL); //right

		SPR_Set(Left3, 255, 255, 255 );
		SPR_DrawHoles( 0,  D, G, NULL); //left

		//To block the left side
		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  F, B, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  F, G, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  F, E, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  I, B, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  I, G, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  I, E, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  J, B, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  J, G, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  J, E, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  K, B, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  K, G, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  K, E, NULL); //vertical

		//To block the right side
		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  H, B, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  H, G, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  H, E, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  L, B, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  L, G, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  L, E, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  M, B, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  M, G, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  M, E, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  N, B, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  N, G, NULL); //vertical

		SPR_Set(Verc, 255, 255, 255 );
		SPR_DrawHoles( 0,  N, E, NULL); //vertical
	}
	return 1;

	/*if (gHUD.m_iFOV >= 90)
	{
		return 1;
	}*/
}

int CHudShroud::MsgFunc_Shroud(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );	

	// update Shroud data
	m_iLos = READ_BYTE();

	//gEngfuncs.Con_Printf("Data sent: %d\n", m_iLos);

	if (m_iLos)
		m_iFlags |= HUD_ACTIVE;
	else
		m_iFlags &= ~HUD_ACTIVE;

	return 1;
}
