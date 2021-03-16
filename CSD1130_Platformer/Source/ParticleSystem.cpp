#include "ParticleSystem.h"
#include <iostream>

Particle::Particle( AEVec2 position )
{
	float rand_angle = AERandFloat();
	velocity = { ( float )( cos( rand_angle * PI ) ), ( float )( sin( rand_angle * PI ) ) };
	Particle_Png = AEGfxTextureLoad( "../Resources/Textures/Heart.png" );
	Particle_Sprite.Load( Particle_Png );
	Particle_Sprite.SetPosition( position );
	lifespan = 5.0f;
}

void Particle::Draw( AEMtx33 maptransform )
{
	Particle_Sprite.Draw( maptransform );
}

void Particle::Update( const float& dt )
{
	AEVec2& TempPosition = Particle_Sprite.GetPosition();
	TempPosition.x += velocity.x * dt;
	TempPosition.y += velocity.y * dt;
	lifespan -= dt;
}

void Particle::Unload()
{
	Particle_Sprite.Unload();
}

ParticleSystem::ParticleSystem()
{
	AllParticles.reserve( 500 );
}

void ParticleSystem::Draw( AEMtx33 maptransform )
{
	for ( auto& Particle : AllParticles )
	{
		if ( Particle.lifespan > 0.0f )
		{
			Particle.Draw( maptransform );
		}
	}
}

void ParticleSystem::Update( const float& dt )
{
	for ( int i = 0 ; i < AllParticles.size() ; i++)
	{
		AllParticles[i].Update( dt );
		if ( AllParticles[i].lifespan < 0.0f )
		{
			--_count;
			
		}
	}

	if ( _timer < 0.0f && _count < 5)
	{
		AllParticles.emplace_back( _position );
		_timer = 1.5f;
	}
	else
	{
		_timer -= dt;
	}
}

void ParticleSystem::Unload()
{
	for ( auto& Particle : AllParticles )
	{
		Particle.Unload();
	}
	AllParticles.clear();
}
