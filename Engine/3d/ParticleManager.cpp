#include "ParticleManager.h"
#include "ModelManager.h"
#include <cassert>

using namespace MyMath;

using namespace StringUtility;

ParticleManager* ParticleManager::instance = nullptr;

uint32_t ParticleManager::kSRVIndexTop = 1;

ParticleManager* ParticleManager::GetInstance() {
	if (instance == nullptr) {
		instance = new ParticleManager();
	}
	return instance;
}

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {
	particleCommon = ParticleCommon::GetInstance(); 
	this->srvManager = srvManager;
	particleCommon->Initialize(dxCommon);
}

void ParticleManager::Finalize() {
	particleCommon->Finalize();
	delete instance;
	instance = nullptr;
}


void ParticleManager::CreateParticleGroup(const std::string name, const std::string textureFilePath) {
	//読み込み済み
	if (particleGroups.contains(name)) {
		return;
	}

	assert(srvManager->Max());

	ParticleGroup& particleG = particleGroups[name];


	particleG.textureFile = textureFilePath;

	//仮のモデル
	particleG.modelData.vertices.push_back({ {1.0f,1.0f,0.0f,1.0f},{0.0f,0.0f},{0.0f,0.0f,1.0f} });
	particleG.modelData.vertices.push_back({ {-1.0f,1.0f,0.0f,1.0f},{1.0f,0.0f},{0.0f,0.0f,1.0f} });
	particleG.modelData.vertices.push_back({ {1.0f,-1.0f,0.0f,1.0f},{0.0f,1.0f},{0.0f,0.0f,1.0f} });
	particleG.modelData.vertices.push_back({ {1.0f,-1.0f,0.0f,1.0f},{0.0f,1.0f},{0.0f,0.0f,1.0f} });
	particleG.modelData.vertices.push_back({ {-1.0f,1.0f,0.0f,1.0f},{1.0f,0.0f},{0.0f,0.0f,1.0f} });
	particleG.modelData.vertices.push_back({ {-1.0f,-1.0f,0.0f,1.0f},{1.0f,1.0f},{0.0f,0.0f,1.0f} });

	particleG.modelData.material.textureFilePath = textureFilePath;


	particleG.resource = particleCommon->GetDxCommon()->CreateBufferResource(sizeof(ParticleForGPU) * particleG.kNumInstance);



	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = particleG.kNumInstance;
	srvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);

	particleG.srvIndex = srvManager->Allocate();
	particleG.srvHandleCPU = srvManager->GetCPUDescriptorHandle(particleG.srvIndex);
	particleG.srvHandleGPU = srvManager->GetGPUDescriptorHandle(particleG.srvIndex);
	

	//SRVの生成
	particleCommon->GetDxCommon()->GetDevice()->CreateShaderResourceView(particleG.resource.Get(), &srvDesc, particleG.srvHandleCPU);


	//particleEmit->Emit();

	emitter.transform.rotate = { 0.0f,0.0f,0.0f };
	emitter.transform.scale = { 1.0f,1.0f,1.0f };
	emitter.frequency = 0.5f;
	emitter.frequencyTime = 0.0f;
}

D3D12_GPU_DESCRIPTOR_HANDLE  ParticleManager::GetSrvHandleGPU(const std::string filePath) {
	assert(srvManager->Max());

	ParticleGroup& particleG = particleGroups[filePath];
	return particleG.srvHandleGPU;
}

ModelData ParticleManager::GetModelData(const std::string filePath) {
	assert(srvManager->Max());

	ParticleGroup& particleG = particleGroups[filePath];
	return particleG.modelData;
}

std::string ParticleManager::GetTextureHandle(const std::string filePath) {
	assert(srvManager->Max());

	ParticleGroup& particleG = particleGroups[filePath];
	return particleG.textureFile;
}

Microsoft::WRL::ComPtr<ID3D12Resource> ParticleManager::GetResource(const std::string filePath) {
	assert(srvManager->Max());

	ParticleGroup& particleG = particleGroups[filePath];
	return particleG.resource;
}

std::list<Particles> ParticleManager::GetParticle(const std::string filePath) {
	assert(srvManager->Max());

	ParticleGroup& particleG = particleGroups[filePath];
	return particleG.particles;
}



void ParticleManager::Emit(const std::string name, const Vector3& position, uint32_t count) {
	ParticleGroup& particleG = particleGroups[name];


	emitter.transform.translate = position;
	emitter.count = count;

	//
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	particleG.particles = particleEmit.MakeEmit(emitter, randomEngine);
	//emitter.frequencyTime -= emitter.frequency;
	
}
