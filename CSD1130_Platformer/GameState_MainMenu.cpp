#include <iostream>
#include "GameState_MainMenu.h"
#include "../Sprite.h"
#include "AEEngine.h"



AEVec2 Window{ 800.0f, 600.0f };

AEGfxTexture* Background_Png;
AEGfxTexture* Button_Level1_Png;
AEGfxTexture* Button_Level2_Png;
AEGfxTexture* Button_Exit_Png;

AEVec2 Background_Pos{ 0 };
AEVec2 Button_Level1_Pos{ 0,600.0f/6.0f };
AEVec2 Button_Level2_Pos{ 0 };
AEVec2 Button_Exit_Pos{ 0 ,-600.0f / 6.0f };

AEVec2 Background_Size{ 800.0f , 600.0f };
AEVec2 Button_Size{ 250.0f, 60.0f };

Sprite Background( Background_Size, Background_Pos );
Sprite Button_Level1( Button_Size, Button_Level1_Pos );
Sprite Button_Level2( Button_Size, Button_Level2_Pos );
Sprite Button_Exit( Button_Size, Button_Exit_Pos );

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
}

void GameStateMainMenuInit( void )
{}

void GameStateMainMenuUpdate( void )
{
	int CursorPosition_x = 0;
	int CursorPosition_y = 0;
	AEInputGetCursorPosition( &CursorPosition_x, &CursorPosition_y );


	if ( CursorPosition_x  > Window.x /2.0f - )



	if ( AEInputCheckTriggered( AEVK_LBUTTON ) )
	{
		std::cout << "herereee" << WindowHeight << " y: " << WindowWidth << std::endl;
	}

}

void GameStateMainMenuDraw( void )
{
	Background.Draw();
	Button_Level1.Draw();
	Button_Level2.Draw();
	Button_Exit.Draw();
}

void GameStateMainMenuFree( void )
{}

void GameStateMainMenuUnload( void )
{
	Background.Unload();
	Button_Level1.Unload();
	Button_Level2.Unload();
	Button_Exit.Unload();
}
