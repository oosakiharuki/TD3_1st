#pragma once
#include "MyMath.h"
#include "ParticleCommon.h"
#include <random>


struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

struct Particles {
	Transform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};

struct Emitter {
	Transform transform;
	uint32_t count; //発生数
	float frequency; //発生頻度
	float frequencyTime; //頻度時刻
};

struct AccelerationField {
	Vector3 acceleration;
	AABB area;
};

enum class BornParticle {
	TimerMode, //タイマーで出てくる
	MomentMode,//瞬間的に出てくる
	Stop,
};


enum class ParticleType {
	Normal,
	Plane,
};

class Particle{
public:
	void Initialize(std::string textureFile);
	void Update();
	void Draw();

	//void SetModel(Model* model) { this->model = model; }
	//void SetModelFile(const std::string& filePath);

	void SetScale(const Vector3& scale) { emitter.transform.scale = scale; }
	const Vector3& GetScale() const { return emitter.transform.scale; }

	//未完
	//void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	//const Vector3& GetRotate() const { return transform.rotate; }

	const Vector3& GetTranslate()const { return emitter.transform.translate; }
	void SetTranslate(const Vector3& translate) { emitter.transform.translate = translate; }

	void SetFrequency(const float time) { emitter.frequency = time; }

	void SetCamera(Camera* camera) { this->camera = camera; }


	//static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
	//static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

	//Particles MakeNewParticle(std::mt19937& randomEngine,const Vector3& translate);
	//std::list<Particles> MakeEmit(const Emitter& emitter, std::mt19937& randomEngine);

	bool IsCollision(const AABB& aabb, const Vector3& point);

	void ChangeMode(BornParticle mode) { bornP = mode; }
	void ChangeType(ParticleType type) { particleType = type; }

	void SetParticleCount(uint32_t countnum) { emitter.count = countnum; }


	void Emit(ParticleType type);

private:
	ParticleCommon* particleCommon = nullptr;

	
	std::string fileName;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;


	VertexData* vertexData = nullptr;
	Material* materialData = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;




	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	ParticleForGPU* wvpData = nullptr;

	//ライト用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightSphereResource;


	//マテリアルにデータを書き込む
	DirectionalLight* directionalLightSphereData = nullptr;


	static const uint32_t kNumMaxInstance = 100;

	//Transform transform[kNumMaxInstance];

	//Particles particles[kNumMaxInstance];
	std::list<Particles> particles;
	uint32_t numInstance = 0;

	Transform transformL;

	Camera* camera = nullptr;

	ModelData modelData;
	Emitter emitter{};

	//std::list<Particles> MakeEmit(const Emitter& emitter, std::mt19937& randomEngine);

	AccelerationField accelerationField;

	BornParticle bornP = BornParticle::TimerMode;
	ParticleType particleType = ParticleType::Normal;

	uint32_t number = 0;
};