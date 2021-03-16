#include "Sprite.h"
#include <stdexcept>   



Sprite::Sprite( const AEVec2& scale, const AEVec2& position )
	:
	_scale( scale ),
	_position( position )
{
	CheckPosition( position );
}

void Sprite::Load( AEGfxTexture* texture )
{
	_texture = texture;
	AE_ASSERT_MESG( _texture, "Failed to create texture!" );

	AEGfxMeshStart();

	AEGfxTriAdd(
		-0.5f, -0.5f, 0x00FF00FF, 0.0f, 1.0f,
		0.5f, -0.5f, 0x00FFFF00, 1.0f, 1.0f,
		-0.5f, 0.5f, 0x0000FFFF, 0.0f, 0.0f );

	AEGfxTriAdd(
		0.5f, -0.5f, 0x00FFFFFF, 1.0f, 1.0f,
		0.5f, 0.5f, 0x00FFFFFF, 1.0f, 0.0f,
		-0.5f, 0.5f, 0x00FFFFFF, 0.0f, 0.0f );


	_mesh = AEGfxMeshEnd();


	AE_ASSERT_MESG( _mesh, "Failed to create mesh !!" );

}

Sprite::~Sprite()
{

}

void Sprite::Draw()
{
	AEMtx33 scale, translate, transform;
	AEMtx33Scale( &scale, _scale.x, _scale.y );
	AEMtx33Trans( &translate, _position.x, _position.y );
	AEMtx33Concat( &transform, &translate, &scale );
	AEGfxSetRenderMode( AE_GFX_RM_TEXTURE );
	AEGfxSetBlendMode( AE_GFX_BM_BLEND );
	AEGfxSetTransparency( 1.0f );
	AEGfxSetTintColor( 1.0f, 1.0f, 1.0f, 1.0f );
	AEGfxSetTransform( transform.m );
	AEGfxTextureSet( _texture, 0.0f, 0.0f );
	AEGfxMeshDraw( _mesh, AE_GFX_MDM_TRIANGLES );
}

void Sprite::Draw( AEMtx33 maptransform )
{
	AEMtx33 scale, translate, transform;
	AEMtx33Scale( &scale, _scale.x, _scale.y );
	AEMtx33Trans( &translate, _position.x, _position.y );
	AEMtx33Concat( &transform, &translate, &scale );
	AEMtx33Concat( &transform, &maptransform, &transform );
	AEGfxSetRenderMode( AE_GFX_RM_TEXTURE );
	AEGfxSetBlendMode( AE_GFX_BM_BLEND );
	AEGfxSetTransparency( 1.0f );
	AEGfxSetTintColor( 1.0f, 1.0f, 1.0f, 1.0f );
	AEGfxSetTransform( transform.m );
	AEGfxTextureSet( _texture, 0.0f, 0.0f );
	AEGfxMeshDraw( _mesh, AE_GFX_MDM_TRIANGLES );
}

void Sprite::SetPosition( const AEVec2& position )
{
	CheckPosition( position );
	_position = position;
}

AEVec2& Sprite::GetPosition()
{
	return _position;
}

void Sprite::SetScale( const AEVec2& scaling )
{
	_scale = scaling;
}

void Sprite::CheckPosition( const AEVec2& position )
{
	if ( position.x < ( -800.0f / 2.0f ) || position.x >( 800.0f / 2.0f ) )
		throw std::out_of_range( "PositionX - Row out of range" );

	if ( position.y < ( -400.0f/ 2.0f ) || position.y >( 400.0f / 2.0f ) )
		throw std::out_of_range( "PositionY - Row out of range" );
}

void Sprite::Unload()
{
	AEGfxTextureUnload( _texture );
	AEGfxMeshFree( _mesh );
}
