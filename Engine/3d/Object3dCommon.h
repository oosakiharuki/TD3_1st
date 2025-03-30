#pragma once
#include "DirectXCommon.h"
class Camera;

class Object3dCommon {
public:
	static Object3dCommon* GetInstance();
	void Finalize();

	void Initialize(DirectXCommon* dxCommon);
	DirectXCommon* GetDirectXCommon()const { return dxCommon_; }

	void Command();	
	
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	Camera* GetDefaultCamera() const { return defaultCamera; }
private:
	//PSO
	void RootSignature();
	void GraphicsPipeline();


	DirectXCommon* dxCommon_;

	//RootSignature
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};


	//バイナリを元に生成
	Microsoft::WRL::ComPtr < ID3D12RootSignature> rootSignature = nullptr;

	//PSO
	Microsoft::WRL::ComPtr < ID3D12PipelineState> graphicsPipelineState = nullptr;

	Camera* defaultCamera = nullptr;

	static Object3dCommon* instance;

	Object3dCommon() = default;
	~Object3dCommon() = default;
	Object3dCommon(Object3dCommon&) = default;
	Object3dCommon& operator=(Object3dCommon&) = default;


	static uint32_t kSRVIndexTop;



};