#include "Particle.h"
#include "TextureManager.h"
#include "ParticleManager.h"
#include "Camera.h"
#include <fstream>
#include <sstream>

#include <numbers>
#include "ModelManager.h"

using namespace MyMath;

void Particle::Initialize(ParticleCommon* ParticleCommon, const std::string& fileName) {
	this->particleCommon = ParticleCommon;
	this->camera = particleCommon->GetDefaultCamera();
	this->fileName = fileName;

	modelData = ParticleManager::GetInstance()->GetModelData(fileName);

	vertexResource = particleCommon->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());

	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);


	wvpResource = ParticleManager::GetInstance()->GetResource(fileName);
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	
	for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		wvpData[index].World = MakeIdentity4x4();
		wvpData[index].WVP = MakeIdentity4x4();
		wvpData[index].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	}


	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());


	//Particle用マテリアル
	//マテリアル用のリソース
	materialResource = particleCommon->GetDxCommon()->CreateBufferResource(sizeof(Material));
	//書き込むためのアドレス
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//色の設定
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();

	//テクスチャ読み込み
	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
	modelData.material.textureIndex = TextureManager::GetInstance()->GetSrvIndex(modelData.material.textureFilePath);


	//ライト用のリソース
	directionalLightSphereResource = particleCommon->GetDxCommon()->CreateBufferResource(sizeof(DirectionalLight));
	//書き込むためのアドレス
	directionalLightSphereResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightSphereData));
	//色の設定
	directionalLightSphereData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightSphereData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightSphereData->intensity = 1.0f;

	//エミッター
	emitter.transform.translate = { 0.0f,0.0f,-3.0f };
	emitter.transform.rotate = { 0.0f,0.0f,0.0f };
	emitter.transform.scale = { 1.0f,1.0f,1.0f };
	emitter.count = 3;
	emitter.frequency = 0.5f;
	emitter.frequencyTime = 0.0f;


	transform.translate = { 0.0f,0.0f,0.0f };
	transform.rotate = { 0.0f,0.0f,0.0f };
	transform.scale = { 1.0f,1.0f,1.0f };


	transformL = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

	//場
	accelerationField.acceleration = { 0.0f,15.0f,0.0f };
	accelerationField.area.min = { -1.0f,-1.0f,-1.0f };
	accelerationField.area.max = { 1.0f,1.0f,1.0f };
}

void Particle::Update() {

	
	const float kDeltaTime = 1.0f / 60.0f;
	emitter.frequencyTime += kDeltaTime;

	ParticleManager::GetInstance()->Emit(fileName,transform.translate,count);

	if (emitter.frequency <= emitter.frequencyTime) {
		particles.splice(particles.end(), ParticleManager::GetInstance()->GetParticle(fileName));
		emitter.frequencyTime -= emitter.frequency;
	}




	numInstance = 0;
	Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
	
	Matrix4x4 billboardMatrix = Multiply(backToFrontMatrix, camera->GetWorldMatrix());
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;


	for (std::list<Particles>::iterator particleIterator = particles.begin();
		particleIterator != particles.end(); ) {

		if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
			particleIterator = particles.erase(particleIterator);
			continue;
		}
	
		Matrix4x4 scaleMatrix = MakeScaleMatrix((*particleIterator).transform.scale);
		Matrix4x4 translateMatrix = MakeTranslateMatrix((*particleIterator).transform.translate);

		Matrix4x4 worldMatrix = Multiply(scaleMatrix, Multiply(billboardMatrix, translateMatrix));
		//Matrix4x4 worldMatrix = Multiply(billboardMatrix, MakeAffineMatrix((*particleIterator).transform.scale, (*particleIterator).transform.rotate, (*particleIterator).transform.translate));

		
		Matrix4x4 WorldViewProjectionMatrix;

		if (camera) {
			Matrix4x4 projectionMatrix = camera->GetViewProjectionMatrix();
			WorldViewProjectionMatrix = Multiply(worldMatrix, projectionMatrix);
		}
		else {
			WorldViewProjectionMatrix = worldMatrix;
		}

		const float kDeltaTime = 1.0f / 60.0f;
		float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);

		if (IsCollision(accelerationField.area, (*particleIterator).transform.translate)) {
			(*particleIterator).velocity += accelerationField.acceleration * kDeltaTime;
		}

		(*particleIterator).transform.translate += (*particleIterator).velocity * kDeltaTime;

		(*particleIterator).transform.rotate = transform.rotate;

		(*particleIterator).transform.scale = transform.scale;

	
		(*particleIterator).currentTime += kDeltaTime;

		wvpData[numInstance].World = worldMatrix;
		
		wvpData[numInstance].color = (*particleIterator).color;
		wvpData[numInstance].color.s = alpha;

		if (numInstance < kNumMaxInstance) {
			wvpData[numInstance].WVP = WorldViewProjectionMatrix;
			++numInstance;
		}
		++particleIterator;
	}
	
	directionalLightSphereData->direction = Normalize(directionalLightSphereData->direction);

}

void Particle::Draw() {
	
	particleCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	particleCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress()); //rootParameterの配列の0番目 [0]
	particleCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	particleCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelData.material.textureFilePath));
	particleCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightSphereResource->GetGPUVirtualAddress());

	//4のやつ particle専用
	particleCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(4, ParticleManager::GetInstance()->GetSrvHandleGPU(fileName));

	particleCommon->GetDxCommon()->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()),numInstance, 0, 0);
}

bool Particle::IsCollision(const AABB& aabb, const Vector3& point) {
	
	if ((aabb.min.x < point.x && aabb.max.x > point.x) &&
		(aabb.min.y < point.y && aabb.max.y > point.y) &&
		(aabb.min.z < point.z && aabb.max.z > point.z)) {
		return true;
	}

	return false;
}



//Particles Particle::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate){
//	//random
//	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);//position用
//	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);//color用
//	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);
//	
//	Particles particle;
//	particle.transform.scale = { 1.0f,1.0f,1.0f };
//	particle.transform.rotate = { 0.0f,3.0f,0.0f };
//	
//	Vector3 randomTranslate{ distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };
//	particle.transform.translate = translate + randomTranslate;
//	
//	particle.velocity = { distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };	
//	particle.color = { distColor(randomEngine),distColor(randomEngine),distColor(randomEngine),1.0f };
//	
//	particle.lifeTime = distTime(randomEngine);
//	particle.currentTime = 0;
//	
//	return particle;
//}
//
//std::list<Particles> Particle::MakeEmit(const Emitter& emitter, std::mt19937& randomEngine) {
//	std::list<Particles> particles;
//	for (uint32_t count = 0; count < emitter.count; ++count) {
//		particles.push_back(MakeNewParticle(randomEngine,emitter.transform.translate));
//	}
//	return particles;
//}
