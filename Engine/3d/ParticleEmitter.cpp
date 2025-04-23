#include "ParticleEmitter.h"
#include <numbers>

using namespace MyMath;

ParticleEmitter* ParticleEmitter::instance = nullptr;

uint32_t ParticleEmitter::kSRVIndexTop = 1;

ParticleEmitter* ParticleEmitter::GetInstance() {
	if (instance == nullptr) {
		instance = new ParticleEmitter();
	}
	return instance;
}

Particles ParticleEmitter::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate) {
	//random
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);//position用
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);//color用
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);

	Particles particle;
	particle.transform.scale = { 1.0f,1.0f,1.0f };
	particle.transform.rotate = { 0.0f,0.0f,0.0f };

	Vector3 randomTranslate{ distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };
	particle.transform.translate = translate;

	particle.velocity = { distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };
	particle.color = { distColor(randomEngine),distColor(randomEngine),distColor(randomEngine),1.0f };

	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;

	return particle;
}
Particles ParticleEmitter::MakeNewParticlePlane(std::mt19937& randomEngine, const Vector3& translate) {
	//random
	std::uniform_real_distribution<float> distRotate(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);
	std::uniform_real_distribution<float> distScale(2.5f, 4.5f);

	Particles particle;
	particle.transform.scale = { 0.05f,distScale(randomEngine),1.0f };
	particle.transform.rotate = { 0.0f,0.0f,distRotate(randomEngine) };

	particle.transform.translate = translate;

	particle.velocity = { 0.0f,0.0f,0.0f };
	particle.color = { 1.0f,1.0f,1.0f,1.0f };

	particle.lifeTime = 0.5f;
	particle.currentTime = 0;

	return particle;
}

std::list<Particles> ParticleEmitter::MakeEmit(const Emitter& emitter, std::mt19937& randomEngine, ParticleType Type) {
	std::list<Particles> particles;

	switch (Type)
	{
	case ParticleType::Normal:
		for (uint32_t count = 0; count < emitter.count; ++count) {
			particles.push_back(MakeNewParticle(randomEngine, emitter.transform.translate));
		}
		break;
	case ParticleType::Plane:
		for (uint32_t count = 0; count < emitter.count; ++count) {
			particles.push_back(MakeNewParticlePlane(randomEngine, emitter.transform.translate));
		}
		break;
	default:
		break;
	}


	return particles;
}
