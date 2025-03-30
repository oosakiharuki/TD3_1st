#pragma once
#include "MyMath.h"
#include "Particle.h"


class ParticleEmitter{
public:
	void Emit() {
		emitter.transform.translate = { 0.0f,0.0f,0.0f };
		emitter.transform.rotate = { 0.0f,0.0f,0.0f };
		emitter.transform.scale = { 1.0f,1.0f,1.0f };
		emitter.count = 3;
		emitter.frequency = 0.5f;
		emitter.frequencyTime = 0.0f;
	}

	Particles MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);
	std::list<Particles> MakeEmit(const Emitter& emitter, std::mt19937& randomEngine);
private:
	Emitter emitter{};
};