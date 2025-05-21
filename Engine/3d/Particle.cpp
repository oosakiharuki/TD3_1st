#include "Particle.h"
#include "TextureManager.h"
#include "ParticleManager.h"
#include "Camera.h"
#include <fstream>
#include <sstream>

#include <numbers>
#include "ModelManager.h"

#include <string>
#include "ParticleNumber.h"

using namespace MyMath;

void Particle::Initialize(std::string textureFile) {
	this->particleCommon = ParticleCommon::GetInstance();
	this->camera = particleCommon->GetDefaultCamera();
	
	//パーティクルの発生源数を増やす
	ParticleNum::number++;
	number = ParticleNum::number;

	ParticleManager::GetInstance()->CreateParticleGroup(std::to_string(number),textureFile);

	this->fileName = std::to_string(number);


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


	transformL = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

	//場
	accelerationField.acceleration = { 0.0f,15.0f,0.0f };
	accelerationField.area.min = { -1.0f,-1.0f,-1.0f };
	accelerationField.area.max = { 1.0f,1.0f,1.0f };
}

void Particle::Update() {

	const float kDeltaTime = 1.0f / 60.0f;

	switch (bornP)
	{
	case BornParticle::TimerMode:

		emitter.frequencyTime += kDeltaTime;

		if (emitter.frequency <= emitter.frequencyTime) {
			//発生処理
			Emit(particleType);
			//particles.splice(particles.end(), ParticleManager::GetInstance()->GetParticle(fileName));
			emitter.frequencyTime -= emitter.frequency;
		}
		break;
	case BornParticle::MomentMode:

		//発生処理
		Emit(particleType);
		//particles.splice(particles.end(), ParticleManager::GetInstance()->GetParticle(fileName));
		bornP = BornParticle::Stop;

		break;
	case BornParticle::Stop:
		break;
	}

	numInstance = 0;
	for (std::list<Particles>::iterator particleIterator = particles.begin();
		particleIterator != particles.end(); ) {

		if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
			particleIterator = particles.erase(particleIterator);
			continue;
		}




		float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);

		if (IsCollision(accelerationField.area, (*particleIterator).transform.translate)) {
			(*particleIterator).velocity += accelerationField.acceleration * kDeltaTime;
		}

		(*particleIterator).transform.translate += (*particleIterator).velocity * kDeltaTime;
	
		if (particleType == ParticleType::Normal) {
			if ((*particleIterator).transform.scale.x > 0) {
				(*particleIterator).transform.scale.x -= 0.5f * kDeltaTime;
			}
			if ((*particleIterator).transform.scale.y > 0) {
				(*particleIterator).transform.scale.y -= 0.5f * kDeltaTime;
			}
			if ((*particleIterator).transform.scale.z > 0) {
				(*particleIterator).transform.scale.z -= 0.5f * kDeltaTime;
			}
		}

		(*particleIterator).currentTime += kDeltaTime;

		Matrix4x4 scaleMatrix = MakeScaleMatrix((*particleIterator).transform.scale);
		Matrix4x4 translateMatrix = MakeTranslateMatrix((*particleIterator).transform.translate);

		//回転行列
		Matrix4x4 rotateX = MakeRotateXMatrix((*particleIterator).transform.rotate.x);
		Matrix4x4 rotateY = MakeRotateYMatrix((*particleIterator).transform.rotate.y);
		Matrix4x4 rotateZ = MakeRotateZMatrix((*particleIterator).transform.rotate.z);
		//全てまとめた
		Matrix4x4 rotateXYZ = Multiply(Multiply(rotateX, rotateY), rotateZ);

		//ビルボード
		Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);

		Matrix4x4 billboardMatrix = Multiply(Multiply(backToFrontMatrix, rotateXYZ), camera->GetWorldMatrix());
		billboardMatrix.m[3][0] = 0.0f;
		billboardMatrix.m[3][1] = 0.0f;
		billboardMatrix.m[3][2] = 0.0f;

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


void Particle::Emit(ParticleType type) {

	//
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

 	particles.splice(particles.end(), ParticleEmitter::GetInstance()->MakeEmit(emitter, randomEngine, type));

}
