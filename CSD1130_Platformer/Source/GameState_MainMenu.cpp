#include <iostream>
#include "main.h"
#include "Sprite.h"



AEVec2 Window{ 800.0f, 600.0f };

AEGfxTexture* Background_Png;
AEGfxTexture* Button_Level1_Png;
AEGfxTexture* Button_Level2_Png;
AEGfxTexture* Button_Exit_Png;
AEGfxTexture* Hero_Png;

AEVec2 Background_Size{ 800.0f , 600.0f };
AEVec2 Button_Size{ 250.0f, 60.0f };
AEVec2 Hero_Size{ 100.0f , 150.0f };

AEVec2 Background_Pos{ 0 };
AEVec2 Button_Level1_Pos{ 0,600.0f / 6.0f };
AEVec2 Button_Level2_Pos{ 0 };
AEVec2 Button_Exit_Pos{ 0 ,-600.0f / 6.0f };
AEVec2 Hero_Pos{ Button_Size.x/2.0f + Hero_Size.x/2.0f, 0 };
AEVec2 HeroMoveTo{ Hero_Pos };
float HeroMoveThere = 0;


Sprite Background( Background_Size, Background_Pos );
Sprite Button_Level1( Button_Size, Button_Level1_Pos );
Sprite Button_Level2( Button_Size, Button_Level2_Pos );
Sprite Button_Exit( Button_Size, Button_Exit_Pos );
Sprite Hero( Hero_Size, Hero_Pos );

void GameStateMainMenuLoad( void )
{
	Background_Png = AEGfxTextureLoad( "../Resources/Textures/MainMenu.png" );
	Background.Load( Background_Png );

	Button_Level1_Png = AEGfxTextureLoad( "../Resources/Textures/L1_1.png" );
	Button_Level1.Load( Button_Level1_Png );


	Button_Level2_Png = AEGfxTextureLoad( "../Resources/Textures/L2_1.png" );
	Button_Level2.Load( Button_Level2_Png );

	Button_Exit_Png = AEGfxTextureLoad( "../Resources/Textures/EXIT_1.png" );
	Button_Exit.Load( Button_Exit_Png );

	Hero_Png = AEGfxTextureLoad( "../Resources/Textures/Hero.png" );
	Hero.Load( Hero_Png );
}

void GameStateMainMenuInit( void )
{}

void GameStateMainMenuUpdate( void )
{
	int CursorPosition_x = 0;
	int CursorPosition_y = 0;
	AEInputGetCursorPosition( &CursorPosition_x, &CursorPosition_y );

	HeroMoveThere = HeroMoveTo.y - Hero_Pos.y;
	Hero_Pos.y += HeroMoveThere *0.1f;

	if ( CursorPosition_x > Window.x / 2.0f - Button_Size.x / 2.0f &&
		 CursorPosition_x < Window.x / 2.0f + Button_Size.x / 2.0f &&
		 CursorPosition_y > Window.y / 6.0f * 2.0f - Button_Size.y / 2.0f &&
		 CursorPosition_y < Window.y / 6.0f * 2.0f + Button_Size.y / 2.0f )
	{
		HeroMoveTo.y = Button_Level1_Pos.y;
	
		if ( AEInputCheckTriggered( AEVK_LBUTTON ) )
		{
			is_level2 = false;
			gGameStateNext = GS_PLATFORM;
		}
	}

	if ( CursorPosition_x > Window.x / 2.0f - Button_Size.x / 2.0f &&
		 CursorPosition_x < Window.x / 2.0f + Button_Size.x / 2.0f &&
		 CursorPosition_y > Window.y / 6.0f * 3.0f - Button_Size.y / 2.0f &&
		 CursorPosition_y < Window.y / 6.0f * 3.0f + Button_Size.y / 2.0f )
	{
		HeroMoveTo.y = Button_Level2_Pos.y;
		
		if ( AEInputCheckTriggered( AEVK_LBUTTON ) )
		{
			is_level2 = true;
			gGameStateNext = GS_PLATFORM;
		}
	}

	if ( CursorPosition_x > Window.x / 2.0f - Button_Size.x / 2.0f &&
		 CursorPosition_x < Window.x / 2.0f + Button_Size.x / 2.0f &&
		 CursorPosition_y > Window.y / 6.0f * 4.0f - Button_Size.y / 2.0f &&
		 CursorPosition_y < Window.y / 6.0f * 4.0f + Button_Size.y / 2.0f )
	{
		HeroMoveTo.y = Button_Exit_Pos.y;

		if ( AEInputCheckTriggered( AEVK_LBUTTON ) )
		{
			gGameStateNext = GS_QUIT;
		}
	}
		Hero.SetPosition( Hero_Pos );

}

void GameStateMainMenuDraw( void )
{
	Background.Draw();
	Button_Level1.Draw();
	Button_Level2.Draw();
	Button_Exit.Draw();
	Hero.Draw();
}

void GameStateMainMenuFree( void )
{}

void GameStateMainMenuUnload( void )
{
	Background.Unload();
	Button_Level1.Unload();
	Button_Level2.Unload();
	Button_Exit.Unload();
	Hero.Unload();
}
