/******************************************************************************/
/*!
\file		GameState_Asteroids.cpp
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
#include "ParticleSystem.h"


/******************************************************************************/
/*!
	Defines
*/
/******************************************************************************/
const unsigned int	GAME_OBJ_NUM_MAX = 32;	//The total number of different objects (Shapes)
const unsigned int	GAME_OBJ_INST_NUM_MAX = 2048;	//The total number of different game object instances

//Gameplay related variables and values
const float			GRAVITY = -3.0f;
const float			JUMP_VELOCITY = 11.0f;
const float			MOVE_VELOCITY_HERO = 4.0f;
const float			MOVE_VELOCITY_ENEMY = 7.5f;
const double		ENEMY_IDLE_TIME = 2.0;
const int			HERO_LIVES = 3;

//Flags
const unsigned int	FLAG_ACTIVE = 0x00000001;
const unsigned int	FLAG_VISIBLE = 0x00000002;
const unsigned int	FLAG_NON_COLLIDABLE = 0x00000004;

//Collision flags
const unsigned int	COLLISION_LEFT = 0x00000001;	//0001
const unsigned int	COLLISION_RIGHT = 0x00000002;	//0010
const unsigned int	COLLISION_TOP = 0x00000004;	//0100
const unsigned int	COLLISION_BOTTOM = 0x00000008;	//1000


enum TYPE_OBJECT
{
	TYPE_OBJECT_EMPTY,			//0
	TYPE_OBJECT_COLLISION,		//1
	TYPE_OBJECT_HERO,			//2
	TYPE_OBJECT_ENEMY1,			//3
	TYPE_OBJECT_COIN			//4
};
//State machine states
enum STATE
{

	STATE_NONE,
	STATE_GOING_LEFT,
	STATE_GOING_RIGHT
};

//State machine inner states
enum INNER_STATE
{
	INNER_STATE_ON_ENTER,
	INNER_STATE_ON_UPDATE,
	INNER_STATE_ON_EXIT
};

/******************************************************************************/
/*!
	Struct/Class Definitions
*/
/******************************************************************************/
struct GameObj
{
	unsigned int		type;		// object type
	AEGfxVertexList* pMesh;		// pbject
	Sprite DrawSprite;
};


struct GameObjInst
{
	GameObj* pObject;	// pointer to the 'original'
	unsigned int	flag;		// bit flag or-ed together
	float			scale;
	AEVec2			posCurr;	// object current position
	AEVec2			velCurr;	// object current velocity
	float			dirCurr;	// object current direction

	AEMtx33			transform;	// object drawing matrix

	AABB			boundingBox;// object bouding box that encapsulates the object

	//Used to hold the current 
	int				gridCollisionFlag;

	// pointer to custom data specific for each object type
	void* pUserData;

	//State of the object instance
	enum			STATE state;
	enum			INNER_STATE innerState;

	//General purpose counter (This variable will be used for the enemy state machine)
	double			counter;
};


/******************************************************************************/
/*!
	File globals
*/
/******************************************************************************/
static int				HeroLives;
static int				Hero_Initial_X;
static int				Hero_Initial_Y;
static int				TotalCoins;
static int				TotalCoinsCollected;

// list of original objects
static GameObj*			sGameObjList;
static unsigned int		sGameObjNum;


// list of object instances
static GameObjInst*		sGameObjInstList;
static unsigned int		sGameObjInstNum;

//Binary map data
static int**			MapData;
static int**			BinaryCollisionArray;
static int				BINARY_MAP_WIDTH;
static int				BINARY_MAP_HEIGHT;
static GameObjInst*		pBlackInstance;
static GameObjInst*		pWhiteInstance;

static AEMtx33			MapTransform;

int						GetCellValue( int X, int Y );
int						CheckInstanceBinaryMapCollision( float PosX, float PosY,
														 float scaleX, float scaleY );
void					SnapToCell( float* Coordinate );
int						ImportMapDataFromFile( char* FileName );
void					FreeMapData( void );

// function to create/destroy a game object instance
static GameObjInst*		gameObjInstCreate( unsigned int type, float scale,
									   AEVec2* pPos, AEVec2* pVel,
									   float dir, enum STATE startState );
static void				gameObjInstDestroy( GameObjInst* pInst );

//We need a pointer to the hero's instance for input purposes
static GameObjInst*		pHero;

//State machine functions
void					EnemyStateMachine( GameObjInst* pInst );



f32 TextWidth, TextHeight;
float GridWidth = 40.0f;
float GridHeight = 40.0f;

// Extra Credit !
ParticleSystem ps;
/******************************************************************************/
/*!
	"Load" function of this state
*/
/******************************************************************************/
void GameStatePlatformLoad( void )
{
	sGameObjList = ( GameObj* )calloc( GAME_OBJ_NUM_MAX, sizeof( GameObj ) );
	sGameObjInstList = ( GameObjInst* )calloc( GAME_OBJ_INST_NUM_MAX, sizeof( GameObjInst ) );
	sGameObjNum = 0;


	GameObj* pObj;

	//Creating the black object
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_OBJECT_EMPTY;

	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFF6495ED, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF6495ED, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFF6495ED, 0.0f, 0.0f );

	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFF6495ED, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF6495ED, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFF6495ED, 0.0f, 0.0f );

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG( pObj->pMesh, "fail to create object!!" );


	//Creating the white object
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_OBJECT_COLLISION;

	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFF4169E1, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF4169E1, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFF4169E1, 0.0f, 0.0f );

	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFF4169E1, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF4169E1, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFF4169E1, 0.0f, 0.0f );

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG( pObj->pMesh, "fail to create object!!" );


	//Creating the hero object
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_OBJECT_HERO;
	pObj->DrawSprite.Load  (AEGfxTextureLoad( "../Resources/Textures/Hero.png" ));
	pObj->DrawSprite.SetScale( { 1.0f, 1.0f } );
	

	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFF0000FF, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF0000FF, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFF0000FF, 0.0f, 0.0f );

	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFF0000FF, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF0000FF, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFF0000FF, 0.0f, 0.0f );

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG( pObj->pMesh, "fail to create object!!" );


	//Creating the enemey1 object
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_OBJECT_ENEMY1;


	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFF0000, 0.0f, 0.0f );

	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFFFF0000, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFFFF0000, 0.0f, 0.0f );

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG( pObj->pMesh, "fail to create object!!" );


	//Creating the Coin object
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_OBJECT_COIN;


	AEGfxMeshStart();
	//Creating the circle shape
	int Parts = 12;
	for ( float i = 0; i < Parts; ++i )
	{
		AEGfxTriAdd(
			0.0f, 0.0f, 0xFFFFFF00, 0.0f, 0.0f,
			cosf( i * 2 * PI / Parts ) * 0.5f, sinf( i * 2 * PI / Parts ) * 0.5f, 0xFFFFFF00, 0.0f, 0.0f,
			cosf( ( i + 1 ) * 2 * PI / Parts ) * 0.5f, sinf( ( i + 1 ) * 2 * PI / Parts ) * 0.5f, 0xFFFFFF00, 0.0f, 0.0f );
	}

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG( pObj->pMesh, "fail to create object!!" );

	//Setting intital binary map values
	MapData = 0;
	BinaryCollisionArray = 0;
	BINARY_MAP_WIDTH = 0;
	BINARY_MAP_HEIGHT = 0;

	//Importing Data
	if ( !is_level2 )
	{
		if ( !ImportMapDataFromFile( "../Resources/Levels/Exported.txt" ) )
		{
			gGameStateNext = GS_QUIT;
		}
	}
	if ( is_level2 )
	{
		if ( !ImportMapDataFromFile( "../Resources/Levels/Exported2.txt" ) )
		{
			gGameStateNext = GS_QUIT;
		}
	}

	//Computing the matrix which take a point out of the normalized coordinates system
	//of the binary map
	/***********
	Compute a transformation matrix and save it in "MapTransform".
	This transformation transforms any point from the normalized coordinates system of the binary map.
	Later on, when rendering each object instance, we should concatenate "MapTransform" with the
	object instance's own transformation matrix

	Compute a translation matrix (-Grid width/2, -Grid height/2) and save it in "trans"
	Compute a scaling matrix and save it in "scale")
	Concatenate scale and translate and save the result in "MapTransform"
	***********/
	AEMtx33 scale, trans;


	AEMtx33Trans( &trans, -( ( float )BINARY_MAP_WIDTH ) / 2.0f, -( ( float )BINARY_MAP_HEIGHT ) / 2.0f );
	AEMtx33Scale( &scale, (float)(AEGetWindowWidth() / BINARY_MAP_WIDTH),(float)( AEGetWindowHeight() / BINARY_MAP_HEIGHT ));
	AEMtx33Concat( &MapTransform, &scale, &trans );


	UNREFERENCED_PARAMETER( scale );
	UNREFERENCED_PARAMETER( trans );

}

/******************************************************************************/
/*!
	"Initialize" function of this state
*/
/******************************************************************************/
void GameStatePlatformInit( void )
{
	int i, j;
	//UNREFERENCED_PARAMETER( j );

	pHero = 0;
	pBlackInstance = 0;
	pWhiteInstance = 0;
	TotalCoins = 0;
	TotalCoinsCollected = 0;

	//Create an object instance representing the black cell.
	//This object instance should not be visible. When rendering the grid cells, each time we have
	//a non collision cell, we position this instance in the correct location and then we render it
	pBlackInstance = gameObjInstCreate( TYPE_OBJECT_EMPTY, 1.0f, 0, 0, 0.0f, STATE_NONE );
	pBlackInstance->flag ^= FLAG_VISIBLE;
	pBlackInstance->flag |= FLAG_NON_COLLIDABLE;

	//Create an object instance representing the white cell.
	//This object instance should not be visible. When rendering the grid cells, each time we have
	//a collision cell, we position this instance in the correct location and then we render it
	pWhiteInstance = gameObjInstCreate( TYPE_OBJECT_COLLISION, 1.0f, 0, 0, 0.0f, STATE_NONE );
	pWhiteInstance->flag ^= FLAG_VISIBLE;
	pWhiteInstance->flag |= FLAG_NON_COLLIDABLE;

	//Setting the inital number of hero lives
	HeroLives = HERO_LIVES;

	/*GameObjInst* pInst;
	AEVec2 Pos;

	UNREFERENCED_PARAMETER( pInst );
	UNREFERENCED_PARAMETER( Pos );*/

	// creating the main character, the enemies and the coins according 
	// to their initial positions in MapData

	/***********
	Loop through all the array elements of MapData
	(which was initialized in the "GameStatePlatformLoad" function
	from the .txt file
		if the element represents a collidable or non collidable area
			don't do anything

		if the element represents the hero
			Create a hero instance
			Set its position depending on its array indices in MapData
			Save its array indices in Hero_Initial_X and Hero_Initial_Y
			(Used when the hero dies and its position needs to be reset)

		if the element represents an enemy
			Create an enemy instance
			Set its position depending on its array indices in MapData

		if the element represents a coin
			Create a coin instance
			Set its position depending on its array indices in MapData

	***********/

	for ( i = 0; i < BINARY_MAP_WIDTH; ++i )
	{
		for ( j = 0; j < BINARY_MAP_HEIGHT; ++j )
		{
			AEVec2 Position{ ( float )i + 0.5f , ( float )j + 0.5f };
			if ( MapData[i][j] == TYPE_OBJECT_HERO )
			{
				pHero = gameObjInstCreate( TYPE_OBJECT_HERO, 1.0f, &Position, 0, 0.0f, STATE_NONE );
				Hero_Initial_X = static_cast< int >( Position.x );
				Hero_Initial_Y = static_cast< int >( Position.y );
			}
			if ( MapData[i][j] == TYPE_OBJECT_ENEMY1 )
			{
				gameObjInstCreate( TYPE_OBJECT_ENEMY1, 1.0f, &Position, 0, 0.0f, STATE_GOING_LEFT );
			}
			if ( MapData[i][j] == TYPE_OBJECT_COIN )
			{
				gameObjInstCreate( TYPE_OBJECT_COIN, 1.0f, &Position, 0, 0.0f, STATE_NONE );
				++TotalCoins;
				TotalCoinsCollected = 0;
			}
		}

	}
}
/******************************************************************************/
/*!
	"Update" function of this state
*/
/******************************************************************************/
void GameStatePlatformUpdate( void )
{
	int i, j;
	GameObjInst* pInst;
	ps._position = pHero->posCurr;
	ps.Update( g_dt );

	UNREFERENCED_PARAMETER( j );
	UNREFERENCED_PARAMETER( pInst );

	//Handle Input
	/***********
	if right is pressed
		Set hero velocity X to MOVE_VELOCITY_HERO
	else
	if left is pressed
		Set hero velocity X to -MOVE_VELOCITY_HERO
	else
		Set hero velocity X to 0

	if space is pressed AND Hero is colliding from the bottom
		Set hero velocity Y to JUMP_VELOCITY

	if Escape is pressed
		Exit to menu
	***********/


	if ( AEInputCheckCurr( AEVK_RIGHT ) )
	{
		pHero->velCurr.x = MOVE_VELOCITY_HERO;
	}
	else if ( AEInputCheckCurr( AEVK_LEFT ) )
	{
		pHero->velCurr.x = -MOVE_VELOCITY_HERO;
	}
	else
	{
		pHero->velCurr.x = 0;
	}

	if ( AEInputCheckCurr( AEVK_SPACE ) && ( pHero->gridCollisionFlag & COLLISION_BOTTOM ) == COLLISION_BOTTOM )
	{
		pHero->velCurr.y = JUMP_VELOCITY;
	}


	if ( AEInputCheckTriggered( AEVK_ESCAPE ) )
	{
		gGameStateNext = GS_QUIT;
	}


	//Update object instances physics and behavior
	for ( i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i )
	{
		pInst = sGameObjInstList + i;

		// skip non-active object
		if ( 0 == ( pInst->flag & FLAG_ACTIVE ) )
			continue;


		/****************
		Apply gravity
			Velocity Y = Gravity * Frame Time + Velocity Y

		If object instance is an enemy
			Apply enemy state machine
		****************/
		if ( pInst->pObject->type != TYPE_OBJECT_COIN )
		{
			pInst->velCurr.y = GRAVITY * g_dt + pInst->velCurr.y;

			if ( pInst->pObject->type == TYPE_OBJECT_ENEMY1 )
			{
				EnemyStateMachine( pInst );
			}
		}
	}

	//Update object instances positions
	for ( i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i )
	{
		pInst = sGameObjInstList + i;

		// skip non-active object
		if ( 0 == ( pInst->flag & FLAG_ACTIVE ) )
			continue;

		/**********
		update the position using: P1 = V1*dt + P0
		Get the bouding rectangle of every active instance:
			boundingRect_min = -BOUNDING_RECT_SIZE * instance->scale + instance->pos
			boundingRect_max = BOUNDING_RECT_SIZE * instance->scale + instance->pos
		**********/
		pInst->posCurr.x = pInst->velCurr.x * g_dt + pInst->posCurr.x;
		pInst->posCurr.y = pInst->velCurr.y * g_dt + pInst->posCurr.y;

		pInst->boundingBox.min.x = -0.5f * pInst->scale + pInst->posCurr.x;
		pInst->boundingBox.min.y = -0.5f * pInst->scale + pInst->posCurr.y;

		pInst->boundingBox.max.x = 0.5f * pInst->scale + pInst->posCurr.x;
		pInst->boundingBox.max.y = 0.5f * pInst->scale + pInst->posCurr.y;

	}

	//Check for grid collision
	for ( i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i )
	{
		pInst = sGameObjInstList + i;

		// skip non-active object instances
		if ( 0 == ( pInst->flag & FLAG_ACTIVE ) )
			continue;

		/*************
		Update grid collision flag

		if collision from bottom
			Snap to cell on Y axis
			Velocity Y = 0

		if collision from top
			Snap to cell on Y axis
			Velocity Y = 0

		if collision from left
			Snap to cell on X axis
			Velocity X = 0

		if collision from right
			Snap to cell on X axis
			Velocity X = 0
		*************/



		pInst->gridCollisionFlag = CheckInstanceBinaryMapCollision( pInst->posCurr.x, pInst->posCurr.y, pInst->scale, pInst->scale );

		if ( ( COLLISION_BOTTOM & pInst->gridCollisionFlag ) == COLLISION_BOTTOM )
		{
			SnapToCell( &pInst->posCurr.y );
			pInst->velCurr.y = 0;

		}

		if ( ( COLLISION_TOP & pInst->gridCollisionFlag ) == COLLISION_TOP )
		{
			SnapToCell( &pInst->posCurr.y );
			pInst->velCurr.y = 0;
		}

		if ( ( COLLISION_LEFT & pInst->gridCollisionFlag ) == COLLISION_LEFT )
		{
			SnapToCell( &pInst->posCurr.x );
			pInst->velCurr.x = 0;
		}

		if ( ( COLLISION_RIGHT & pInst->gridCollisionFlag ) == COLLISION_RIGHT )
		{
			SnapToCell( &pInst->posCurr.x );
			pInst->velCurr.x = 0;
		}

	}
	//Checking for collision among object instances:
	//Hero against enemies
	//Hero against coins

	/**********
	for each game object instance
		Skip if it's inactive or if it's non collidable

		If it's an enemy
			If collision between the enemy instance and the hero (rectangle - rectangle)
				Decrement hero lives
				Reset the hero's position in case it has lives left, otherwise RESTART the level

		If it's a coin
			If collision between the coin instance and the hero (rectangle - rectangle)
				Remove the coin and decrement the coin counter.
				Quit the game level to the menu in case no more coins are left
	**********/

	for ( i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i )
	{
		pInst = sGameObjInstList + i;

		// skip non-active object
		if ( 0 == ( pInst->flag & FLAG_ACTIVE ) )
			continue;
		if ( ( pInst->flag & FLAG_NON_COLLIDABLE ) == FLAG_NON_COLLIDABLE )
			continue;
		if ( pInst->pObject->type == TYPE_OBJECT_ENEMY1 )
		{
			if ( CollisionIntersection_RectRect( pInst->boundingBox, pInst->velCurr, pHero->boundingBox, pHero->velCurr ) )
			{
				HeroLives--;
				if ( HeroLives > 0 )
				{
					pHero->posCurr = { ( float )Hero_Initial_X, ( float )Hero_Initial_Y };
				}
				else if ( HeroLives == 0 )
				{
					gGameStateNext = GS_RESTART;
				}
			}
		}
		if ( pInst->pObject->type == TYPE_OBJECT_COIN )
		{
			if ( CollisionIntersection_RectRect( pInst->boundingBox, pInst->velCurr, pHero->boundingBox, pHero->velCurr ) )
			{
				pInst->flag &= ~FLAG_ACTIVE;
				TotalCoins--;
				TotalCoinsCollected++;

				if ( TotalCoins <= 0 )
				{
					gGameStateNext = GS_MAINMENU;
				}
			}
		}
	}


	//Computing the transformation matrices of the game object instances
	for ( i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i )
	{
		AEMtx33 scale, rot, trans;
		pInst = sGameObjInstList + i;

		// skip non-active object
		if ( 0 == ( pInst->flag & FLAG_ACTIVE ) )
			continue;

		AEMtx33Scale( &scale, pInst->scale, pInst->scale );
		AEMtx33Rot( &rot, pInst->dirCurr );
		AEMtx33Trans( &trans, pInst->posCurr.x, pInst->posCurr.y );
		AEMtx33Concat( &scale, &rot, &scale );
		AEMtx33Concat( &pInst->transform, &trans, &scale );

	}


	if ( pHero && is_level2 )
	{
		AEMtx33 scale, trans;
		AEMtx33Trans( &trans, -pHero->posCurr.x, -pHero->posCurr.y );
		AEMtx33Scale( &scale, 30, 30 );
		AEMtx33Concat( &MapTransform, &scale, &trans );
	}

}
/******************************************************************************/
/*!

*/
/******************************************************************************/
void GameStatePlatformDraw( void )
{
	AEGfxSetRenderMode( AEGfxRenderMode::AE_GFX_RM_COLOR );

	//Drawing the tile map (the grid)
	int i, j;
	AEMtx33 cellTranslation, cellFinalTransformation;
	/*UNREFERENCED_PARAMETER( cellTranslation );
	UNREFERENCED_PARAMETER( cellFinalTransformation );*/

	//Drawing the tile map

	/******REMINDER*****
	You need to concatenate MapTransform with the transformation matrix
	of any object you want to draw. MapTransform transform the instance
	from the normalized coordinates system of the binary map
	*******************/

	/*********
	for each array element in BinaryCollisionArray (2 loops)
		Compute the cell's translation matrix acoording to its
		X and Y coordinates and save it in "cellTranslation"
		Concatenate MapTransform with the cell's transformation
		and save the result in "cellFinalTransformation"
		Send the resultant matrix to the graphics manager using "AEGfxSetTransform"

		Draw the instance's shape depending on the cell's value using "AEGfxMeshDraw"
			Use the black instance in case the cell's value is TYPE_OBJECT_EMPTY
			Use the white instance in case the cell's value is TYPE_OBJECT_COLLISION
	*********/
	for ( i = 0; i < BINARY_MAP_WIDTH; ++i )
	{
		for ( j = 0; j < BINARY_MAP_HEIGHT; ++j )
		{
			AEMtx33Trans( &cellTranslation, ( float )i + 0.5f, ( float )j + 0.5f );
			AEMtx33Concat( &cellFinalTransformation, &MapTransform, &cellTranslation );
			AEGfxSetTransform( cellFinalTransformation.m );
			if ( BinaryCollisionArray[i][j] == TYPE_OBJECT_EMPTY )
			{
				AEGfxMeshDraw( pBlackInstance->pObject->pMesh, AE_GFX_MDM_TRIANGLES );
			}
			if ( BinaryCollisionArray[i][j] == TYPE_OBJECT_COLLISION )
			{
				AEGfxMeshDraw( pWhiteInstance->pObject->pMesh, AE_GFX_MDM_TRIANGLES );
			}
		}
	}

	//Drawing the object instances
	/**********
	For each active and visible object instance
		Concatenate MapTransform with its transformation matrix
		Send the resultant matrix to the graphics manager using "AEGfxSetTransform"
		Draw the instance's shape using "AEGfxMeshDraw"
	**********/
	for ( i = 0; i < GAME_OBJ_INST_NUM_MAX; i++ )
	{
		GameObjInst* pInst = sGameObjInstList + i;

		// skip non-active object
		if ( 0 == ( pInst->flag & FLAG_ACTIVE ) || 0 == ( pInst->flag & FLAG_VISIBLE ) )
			continue;

		if ( pInst->pObject->type != TYPE_OBJECT_HERO  /*&& pInst->pObject->type != TYPE_OBJECT_COLLISION */)
		{
			//Don't forget to concatenate the MapTransform matrix with the transformation of each game object instance
			AEMtx33 TempMapTransform;
			AEMtx33Concat( &TempMapTransform, &MapTransform, &pInst->transform );
			AEGfxSetTransform( TempMapTransform.m );
			AEGfxMeshDraw( pInst->pObject->pMesh, AE_GFX_MDM_TRIANGLES );
		}
		
		if ( pInst->pObject->type == TYPE_OBJECT_HERO  )
		{
			pInst->pObject->DrawSprite.SetPosition( pHero->posCurr );
			pInst->pObject->DrawSprite.Draw( MapTransform );
		}



	}

	char strBuffer[100];
	memset( strBuffer, 0, 100 * sizeof( char ) );
	sprintf_s( strBuffer, "Lives:  %i", HeroLives );
	AEGfxGetPrintSize( fontId, strBuffer, 1.0f, TextWidth, TextHeight );
	AEGfxPrint( fontId, strBuffer, 0.90f - TextWidth, 0.99f - TextHeight, 1.0f, 1.f, 1.f, 1.f );



	sprintf_s( strBuffer, "Score: %d", ( TotalCoinsCollected > 0 ? TotalCoinsCollected : 0 ) );
	AEGfxGetPrintSize( fontId, strBuffer, 1.0f, TextWidth, TextHeight );
	AEGfxPrint( fontId, strBuffer, 0.66f - TextWidth, 0.99f - TextHeight, 1.0f, 1.f, 1.f, 1.f );

	ps.Draw( MapTransform );
}

/******************************************************************************/
/*!

*/
/******************************************************************************/
void GameStatePlatformFree( void )
{
	// kill all object in the list
	for ( unsigned int i = 0; i < GAME_OBJ_INST_NUM_MAX; i++ )
		gameObjInstDestroy( sGameObjInstList + i );

	ps.Unload();

}

/******************************************************************************/
/*!

*/
/******************************************************************************/
void GameStatePlatformUnload( void )
{
	// free all CREATED mesh
	for ( u32 i = 0; i < sGameObjNum; i++ )
	{
		if ( sGameObjList[i].type == TYPE_OBJECT_HERO )
		{
			sGameObjList[i].DrawSprite.Unload();
		}
		AEGfxMeshFree( sGameObjList[i].pMesh );
	}
		



	free( sGameObjList );
	free( sGameObjInstList );

	/*********
	Free the map data
	*********/
	FreeMapData();
}

/******************************************************************************/
/*!

*/
/******************************************************************************/
GameObjInst* gameObjInstCreate( unsigned int type, float scale,
								AEVec2* pPos, AEVec2* pVel,
								float dir, enum STATE startState )
{
	AEVec2 zero;
	AEVec2Zero( &zero );

	AE_ASSERT_PARM( type < sGameObjNum );

	// loop through the object instance list to find a non-used object instance
	for ( unsigned int i = 0; i < GAME_OBJ_INST_NUM_MAX; i++ )
	{
		GameObjInst* pInst = sGameObjInstList + i;

		// check if current instance is not used
		if ( pInst->flag == 0 )
		{
			// it is not used => use it to create the new instance
			pInst->pObject = sGameObjList + type;
			pInst->flag = FLAG_ACTIVE | FLAG_VISIBLE;
			pInst->scale = scale;
			pInst->posCurr = pPos ? *pPos : zero;
			pInst->velCurr = pVel ? *pVel : zero;
			pInst->dirCurr = dir;
			pInst->pUserData = 0;
			pInst->gridCollisionFlag = 0;
			pInst->state = startState;
			pInst->innerState = INNER_STATE_ON_ENTER;
			pInst->counter = 0;

			// return the newly created instance
			return pInst;
		}
	}

	return 0;
}

/******************************************************************************/
/*!

*/
/******************************************************************************/
void gameObjInstDestroy( GameObjInst* pInst )
{
	// if instance is destroyed before, just return
	if ( pInst->flag == 0 )
		return;

	// zero out the flag
	pInst->flag = 0;
}

/******************************************************************************/
/*!
	This function retrieves the value of the element (X;Y) in BinaryCollisionArray.
	Before retrieving the value, it should check that the supplied X and Y values
	are not out of bounds (in that case return 0)
 */
 /******************************************************************************/
int GetCellValue( int X, int Y )
{
	return ( X >= BINARY_MAP_WIDTH || X < 0 || Y >= BINARY_MAP_HEIGHT || Y < 0 ) ? 0 : BinaryCollisionArray[X][Y];
}

/******************************************************************************/
/*!
	This function creates 2 hot spots on each side of the object instance,
	and checks if each of these hot spots is in a collision area (which means
	the cell if falls in has a value of 1).
	At the beginning of the function, a "Flag" integer should be initialized to 0.
	Each time a hot spot is in a collision area, its corresponding bit
	in "Flag" is set to 1.
	Finally, the function returns the integer "Flag"
	The position of the object instance is received as PosX and PosY
	The size of the object instance is received as scaleX and scaleY

	Note: This function assume the object instance's size is 1 by 1
		  (the size of 1 tile)

	Creating the hotspots:
		-Handle each side separately.
		-2 hot spots are needed for each collision side.
		-These 2 hot spots should be positioned on 1/4 above the center
		and 1/4 below the center

	Example: Finding the hots spots on the left side of the object instance

	float x1, y1, x2, y2;

	-hotspot 1
	x1 = PosX + scaleX/2	To reach the right side
	y1 = PosY + scaleY/4	To go up 1/4 of the height

	-hotspot 2
	x2 = PosX + scaleX/2	To reach the right side
	y2 = PosY - scaleY/4	To go down 1/4 of the height
 */
 /******************************************************************************/

int CheckInstanceBinaryMapCollision( float PosX, float PosY, float scaleX, float scaleY )
{
	//At the end of this function, "Flag" will be used to determine which sides
	//of the object instance are colliding. 2 hot spots will be placed on each side.

	int Flag = 0;

	int x_rh = static_cast< int >( PosX + scaleX / 2.0f );
	int x_lh = static_cast< int >( PosX - scaleX / 2.0f );

	int x_rq = static_cast< int >( PosX + scaleX / 4.0f );
	int x_lq = static_cast< int >( PosX - scaleX / 4.0f );

	int y_th = static_cast< int >( PosY + scaleY / 2.0f );
	int y_bh = static_cast< int >( PosY - scaleY / 2.0f );

	int y_tq = static_cast< int >( PosY + scaleY / 4.0f );
	int y_bq = static_cast< int >( PosY - scaleY / 4.0f );


	if ( GetCellValue( x_lh, y_tq ) == TYPE_OBJECT_COLLISION || GetCellValue( x_lh, y_bq ) == TYPE_OBJECT_COLLISION )
	{
		Flag |= COLLISION_LEFT;
	}
	if ( GetCellValue( x_rq, y_th ) == TYPE_OBJECT_COLLISION || GetCellValue( x_lq, y_th ) == TYPE_OBJECT_COLLISION )
	{
		Flag |= COLLISION_TOP;
	}
	if ( GetCellValue( x_rh, y_tq ) == TYPE_OBJECT_COLLISION || GetCellValue( x_rh, y_bq ) == TYPE_OBJECT_COLLISION )
	{
		Flag |= COLLISION_RIGHT;
	}
	if ( GetCellValue( x_rq, y_bh ) == TYPE_OBJECT_COLLISION || GetCellValue( x_lq, y_bh ) == TYPE_OBJECT_COLLISION )
	{
		Flag |= COLLISION_BOTTOM;
	}

	return Flag;
}

/******************************************************************************/
/*!
	This function snaps the value sent as parameter to the center of the cell.
	It is used when a sprite is colliding with a collision area from one
	or more side.
	To snap the value sent by "Coordinate", find its integral part by type
	casting it to an integer, then add 0.5 (which is half the cell's width
	or height)
 */
 /******************************************************************************/
void SnapToCell( float* Coordinate )
{
	int temp = ( int )( *Coordinate );
	*Coordinate = ( float )temp + 0.50f;

}

/******************************************************************************/
/*!
	This function opens the file name "FileName" and retrieves all the map data.
	It allocates memory for the 2 arrays: MapData & BinaryCollisionArray
	The first line in this file is the width of the map.
	The second line in this file is the height of the map.
	The remaining part of the file is a series of numbers
	Each number represents the ID (or value) of a different element in the
	double dimensionaly array.
*/
/******************************************************************************/
int ImportMapDataFromFile( char* FileName )
{
	std::ifstream file( FileName );

	if ( file )
	{
		int int_input;
		std::string string_input;

		file >> string_input >> BINARY_MAP_WIDTH >> string_input >> BINARY_MAP_HEIGHT;

		MapData = new int* [BINARY_MAP_WIDTH];
		BinaryCollisionArray = new int* [BINARY_MAP_WIDTH];
		for ( int i = 0; i < BINARY_MAP_WIDTH; i++ )
		{
			MapData[i] = new int[BINARY_MAP_HEIGHT];
			BinaryCollisionArray[i] = new int[BINARY_MAP_HEIGHT];
		}

		for ( int i = 0; i < BINARY_MAP_HEIGHT; ++i )
		{
			for ( int j = 0; j < BINARY_MAP_WIDTH; ++j )
			{
				file >> int_input;
				MapData[j][i] = int_input;
				BinaryCollisionArray[j][i] = int_input == TYPE_OBJECT_COLLISION ? 1 : 0;
			}
		}
		return 1;
	}
	return 0;
}

/******************************************************************************/
/*!
	This function frees the memory that was allocated for the 2 arrays MapData
	& BinaryCollisionArray which was allocated in the "ImportMapDataFromFile"
	function
 */
 /******************************************************************************/
void FreeMapData( void )
{
	for ( int j = 0; j < BINARY_MAP_WIDTH; j++ )
	{
		delete[] BinaryCollisionArray[j];
		delete[] MapData[j];
	}
	delete[] BinaryCollisionArray;
	delete[] MapData;
}

/******************************************************************************/
/*!

*/
/******************************************************************************/
void EnemyStateMachine( GameObjInst* pInst )
{
	/***********
	This state machine has 2 states: STATE_GOING_LEFT and STATE_GOING_RIGHT
	Each state has 3 inner states: INNER_STATE_ON_ENTER, INNER_STATE_ON_UPDATE, INNER_STATE_ON_EXIT
	Use "switch" statements to determine which state and inner state the enemy is currently in.


	STATE_GOING_LEFT
		INNER_STATE_ON_ENTER
			Set velocity X to -MOVE_VELOCITY_ENEMY
			Set inner state to "on update"

		INNER_STATE_ON_UPDATE3
			If collision on left side OR bottom left cell is non collidable
				Initialize the counter to ENEMY_IDLE_TIME
				Set inner state to on exit
				Set velocity X to 0


		INNER_STATE_ON_EXIT
			Decrement counter by frame time
			if counter is less than 0 (sprite's idle time is over)
				Set state to "going right"
				Set inner state to "on enter"

	STATE_GOING_RIGHT is basically the same, with few modifications.

	***********/

	bool check = false;
	switch ( pInst->state )
	{
		case  ( STATE_GOING_LEFT ):

			switch ( pInst->innerState )
			{
				case ( INNER_STATE_ON_ENTER ):
					pInst->velCurr.x = -MOVE_VELOCITY_ENEMY;
					pInst->innerState = INNER_STATE_ON_UPDATE;
					break;
				case ( INNER_STATE_ON_UPDATE ):
					check = ( pInst->posCurr.x - ( int )pInst->posCurr.x <= 0.5f ) ? !GetCellValue( ( int )pInst->posCurr.x - 1, ( int )pInst->posCurr.y - 1 ) : false;
					if ( ( pInst->gridCollisionFlag & COLLISION_LEFT ) == COLLISION_LEFT || check )
					{
						pInst->counter = ENEMY_IDLE_TIME;
						pInst->innerState = INNER_STATE_ON_EXIT;
						pInst->velCurr.x = 0;
					}
					break;
				case ( INNER_STATE_ON_EXIT ):
					pInst->counter -= g_dt;
					if ( pInst->counter <= 0 )
					{

						pInst->state = STATE_GOING_RIGHT;
						pInst->innerState = INNER_STATE_ON_ENTER;
					}
					break;
			}
			break;

		case ( STATE_GOING_RIGHT ):


			switch ( pInst->innerState )
			{
				case ( INNER_STATE_ON_ENTER ):
					pInst->velCurr.x = MOVE_VELOCITY_ENEMY;
					pInst->innerState = INNER_STATE_ON_UPDATE;
					break;
				case ( INNER_STATE_ON_UPDATE ):
					check = ( pInst->posCurr.x - ( int )pInst->posCurr.x >= 0.5f ) ? !GetCellValue( ( int )pInst->posCurr.x + 1, ( int )pInst->posCurr.y - 1 ) : false ;
					if ( ( pInst->gridCollisionFlag & COLLISION_RIGHT ) == COLLISION_RIGHT || check )
					{
						pInst->counter = ENEMY_IDLE_TIME;
						pInst->innerState = INNER_STATE_ON_EXIT;
						pInst->velCurr.x = 0;
					}
					break;
				case ( INNER_STATE_ON_EXIT ):
					pInst->counter -= g_dt;
					if ( pInst->counter <= 0 )
					{
						pInst->state = STATE_GOING_LEFT;
						pInst->innerState = INNER_STATE_ON_ENTER;
					}
					break;
			}
			break;


	}

	UNREFERENCED_PARAMETER( pInst );
}