#include "Particle.h"
#include "TextureManager.h"
#include "ParticleManager.h"
#include "Camera.h"
#include <fstream>
#include <sstream>
#include <mutex>

#include <numbers>
#include "ModelManager.h"

#include <string>
#include "ParticleNumber.h"

using namespace MyMath;

Particle::~Particle() {
	// リソースのアンマップ
	if (vertexResource && vertexData) {
		vertexResource->Unmap(0, nullptr);
		vertexData = nullptr;
	}
	if (materialResource && materialData) {
		materialResource->Unmap(0, nullptr);
		materialData = nullptr;
	}
	if (wvpResource && wvpData) {
		wvpResource->Unmap(0, nullptr);
		wvpData = nullptr;
	}
	if (directionalLightSphereResource && directionalLightSphereData) {
		directionalLightSphereResource->Unmap(0, nullptr);
		directionalLightSphereData = nullptr;
	}
}

void Particle::Initialize(std::string textureFile) {
	this->particleCommon = ParticleCommon::GetInstance();
	this->camera = particleCommon->GetDefaultCamera();
	
	//パーティクルの発生源数を増やす
	const uint32_t MAX_PARTICLE_GROUPS = 1000; // パーティクルグループの最大数
	
	// スレッドセーフなインクリメント
	static std::mutex particleNumMutex;
	{
		std::lock_guard<std::mutex> lock(particleNumMutex);
		ParticleNum::number++;
		// パーティクル番号が上限に達した場合はリセット
		if (ParticleNum::number >= MAX_PARTICLE_GROUPS) {
			char errorMsg[256];
			sprintf_s(errorMsg, "Particle::Initialize - Particle group limit reached (%d). Resetting to 1.\n", MAX_PARTICLE_GROUPS);
			OutputDebugStringA(errorMsg);
			ParticleNum::number = 1;
		}
		number = ParticleNum::number;
	}

	char debugMsg[256];
	sprintf_s(debugMsg, "Particle::Initialize - Creating particle group with number: %d, texture: %s\n", number, textureFile.c_str());
	OutputDebugStringA(debugMsg);
	
	ParticleManager::GetInstance()->CreateParticleGroup(std::to_string(number),textureFile);

	this->fileName = std::to_string(number);

	this->textureFile = textureFile;

	modelData = ParticleManager::GetInstance()->GetModelData(fileName);
	
	// モデルデータが空の場合は初期化失敗
	if (modelData.vertices.empty()) {
		OutputDebugStringA("Particle::Initialize - Failed to get model data from ParticleManager\n");
		isInitialized = false;
		return;
	}

	vertexResource = particleCommon->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	if (!vertexResource) {
		OutputDebugStringA("Failed to create vertexResource\n");
		return;
	}

	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);


	// HRESULTを一度だけ宣言
	HRESULT hr;
	
	wvpResource = ParticleManager::GetInstance()->GetResource(fileName);
	if (!wvpResource) {
		OutputDebugStringA("Failed to get wvpResource from ParticleManager\n");
		isInitialized = false;
		return;
	}
	hr = wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	if (FAILED(hr)) {
		OutputDebugStringA("Failed to map wvpResource\n");
		isInitialized = false;
		return;
	}
	
	for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		wvpData[index].World = MakeIdentity4x4();
		wvpData[index].WVP = MakeIdentity4x4();
		wvpData[index].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	}


	hr = vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	if (FAILED(hr)) {
		OutputDebugStringA("Failed to map vertexResource\n");
		isInitialized = false;
		return;
	}
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());


	//Particle用マテリアル
	//マテリアル用のリソース
	materialResource = particleCommon->GetDxCommon()->CreateBufferResource(sizeof(Material));
	if (!materialResource) {
		OutputDebugStringA("Failed to create materialResource\n");
		return;
	}
	//書き込むためのアドレス
	hr = materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	if (FAILED(hr)) {
		OutputDebugStringA("Failed to map materialResource\n");
		isInitialized = false;
		return;
	}
	//色の設定
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();

	//テクスチャ読み込み
	if (!modelData.material.textureFilePath.empty()) {
		char debugMsg[256];
		sprintf_s(debugMsg, "Particle::Initialize - Loading texture: %s\n", modelData.material.textureFilePath.c_str());
		OutputDebugStringA(debugMsg);
		
		TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
		modelData.material.textureIndex = TextureManager::GetInstance()->GetSrvIndex(modelData.material.textureFilePath);
	} else {
		OutputDebugStringA("Particle::Initialize - Warning: Empty texture file path in modelData\n");
	}


	//ライト用のリソース
	directionalLightSphereResource = particleCommon->GetDxCommon()->CreateBufferResource(sizeof(DirectionalLight));
	//リソースの作成チェック
	if (!directionalLightSphereResource) {
		// リソース作成に失敗した場合はエラーログを出力して早期リターン
		OutputDebugStringA("Failed to create directionalLightSphereResource\n");
		return;
	}
	//書き込むためのアドレス
	hr = directionalLightSphereResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightSphereData));
	if (FAILED(hr)) {
		// Mapに失敗した場合
		OutputDebugStringA("Failed to map directionalLightSphereResource\n");
		isInitialized = false;
		return;
	}
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
	
	// 初期化成功
	isInitialized = true;
}

void Particle::Update() {
	// 初期化されていない場合は処理をスキップ
	if (!isInitialized) {
		return;
	}

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

		// wvpDataのnullチェック
		if (wvpData) {
			wvpData[numInstance].World = worldMatrix;

			wvpData[numInstance].color = (*particleIterator).color;
			wvpData[numInstance].color.s = alpha;

			if (numInstance < kNumMaxInstance) {
				wvpData[numInstance].WVP = WorldViewProjectionMatrix;
				++numInstance;
			}
		}
		++particleIterator;
	}

	// directionalLightSphereDataのnullチェック
	if (directionalLightSphereData) {
		directionalLightSphereData->direction = Normalize(directionalLightSphereData->direction);
	}

}

void Particle::Draw() {
	// 初期化されていない場合は描画をスキップ
	if (!isInitialized) {
		return;
	}
	
	// リソースのnullチェック
	if (!vertexResource || !materialResource || !wvpResource || !directionalLightSphereResource) {
		// いずれかのリソースがnullの場合は描画をスキップ
		return;
	}
	
	particleCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	particleCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress()); //rootParameterの配列の0番目 [0]
	particleCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	// テクスチャが存在するか確認
	if (!modelData.material.textureFilePath.empty() && TextureManager::GetInstance()->CheckTextureExist(textureFile)) {
		particleCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureFile));
	} else {
		// テクスチャが存在しない場合は白いテクスチャを使用するか、スキップ
		return; // 今回は描画をスキップ
	}
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
