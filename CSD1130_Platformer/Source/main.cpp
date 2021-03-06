/******************************************************************************/
/*!
\file		main.cpp
\author 	DigiPen
\par    	email: digipen\@digipen.edu
\date   	February 01, 20xx
\brief

Copyright (C) 20xx DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "main.h"

#include <cstdlib>
#include <crtdbg.h>

// ---------------------------------------------------------------------------
// Globals
float	 g_dt;
double	 g_appTime;
bool		is_level2 = false;
char		fontId;


/******************************************************************************/
/*!
	Starting point of the application
*/
/******************************************************************************/
int WINAPI WinMain( HINSTANCE instanceH, HINSTANCE prevInstanceH, LPSTR command_line, int show )
{
	UNREFERENCED_PARAMETER( prevInstanceH );
	UNREFERENCED_PARAMETER( command_line );

	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
#endif



	// Initialize the system
	AESysInit( instanceH, show, 800, 600, 1, 60, false, NULL );

	// Changing the window title
	AESysSetWindowTitle( "Miore" );

	//set background color
	AEGfxSetBackgroundColor( 0.53f, 0.81f, 0.98f );

	fontId = AEGfxCreateFont( "../Resources/Fonts/Strawberry_Muffins_Demo.ttf", 20 );

	GameStateMgrInit( GS_MAINMENU );

	while ( gGameStateCurr != GS_QUIT )
	{
		// reset the system modules
		AESysReset();

		// If not restarting, load the gamestate
		if ( gGameStateCurr != GS_RESTART )
		{
			GameStateMgrUpdate();
			GameStateLoad();
		}
		else
			gGameStateNext = gGameStateCurr = gGameStatePrev;

		// Initialize the gamestate
		GameStateInit();

		while ( gGameStateCurr == gGameStateNext )
		{
			AESysFrameStart();

			AEInputUpdate();

			GameStateUpdate();

			GameStateDraw();

			AESysFrameEnd();

			// check if forcing the application to quit
			if ( ( AESysDoesWindowExist() == false ) || AEInputCheckTriggered( AEVK_ESCAPE ) )
				gGameStateNext = GS_QUIT;

			g_dt = ( f32 )AEFrameRateControllerGetFrameTime();
			g_appTime += g_dt;
		}

		GameStateFree();

		if ( gGameStateNext != GS_RESTART )
			GameStateUnload();

		gGameStatePrev = gGameStateCurr;
		gGameStateCurr = gGameStateNext;
	}

	// free the system
	AESysExit();
	
}

