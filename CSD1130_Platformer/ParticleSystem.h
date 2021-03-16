#pragma once
#include "Sprite.h"
#include "AEEngine.h"
#include <vector>

class Particle
{
public:
	AEGfxTexture* Particle_Png;
	AEVec2 Particle_Size{ 1.0f , 1.0f };
	AEVec2 Particle_Pos{ 0 };
	Sprite Particle_Sprite{ Particle_Size, Particle_Pos };
	AEVec2 velocity{ 0 };
	float lifespan{ 0 };
public :
	Particle( AEVec2 position );
	void Draw( AEMtx33 maptransform );
	void Update(const float& dt);
	void Unload();
};

class ParticleSystem
{
public :
	int _count{ 0 };
	AEVec2 _position{ 0.0f,0.0f };
	float _timer = 0.0f;
	std::vector <Particle> AllParticles;
	ParticleSystem();
	void Draw( AEMtx33 maptransform );
	void Update( const float& dt );
	void Unload();
};