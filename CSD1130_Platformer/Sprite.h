
#ifndef SPRITE_H
#define SPRITE_H

#include "AEEngine.h"

class Sprite
{
	AEVec2 _position{ 0 };
	AEVec2 _scale{ 1.0f ,1.0f };
	AEGfxVertexList* _mesh =  nullptr ;
	AEGfxTexture* _texture{ nullptr };
	

public:
	Sprite()=default;
	Sprite ( const AEVec2& scale, const AEVec2& position );
	void Load( AEGfxTexture* texture );
	~Sprite();
	void Draw();
	void Draw( AEMtx33 maptransform );
	void SetPosition( const AEVec2& position );
	AEVec2& GetPosition();
	void SetScale( const AEVec2& scale );
	void CheckPosition(const AEVec2 & position);
	void Unload();

};

#endif 