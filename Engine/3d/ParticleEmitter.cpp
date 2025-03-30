#include "ParticleEmitter.h"

using namespace MyMath;

Particles ParticleEmitter::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate) {
	//random
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);//position用
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);//color用
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);

	Particles particle;
	particle.transform.scale = { 1.0f,1.0f,1.0f };
	particle.transform.rotate = { 0.0f,0.0f,0.0f };

	Vector3 randomTranslate{ distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };
	particle.transform.translate = translate + randomTranslate;

	particle.velocity = { distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };
	particle.color = { distColor(randomEngine),distColor(randomEngine),distColor(randomEngine),1.0f };

	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;

	return particle;
}

std::list<Particles> ParticleEmitter::MakeEmit(const Emitter& emitter, std::mt19937& randomEngine) {
	std::list<Particles> particles;
	for (uint32_t count = 0; count < emitter.count; ++count) {
		particles.push_back(MakeNewParticle(randomEngine, emitter.transform.translate));
	}
	return particles;
}
