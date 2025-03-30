#pragma once
#include <string>
#include "externals/DirectXTex/DirectXTex.h"
#include <wrl.h>
#include <d3d12.h>
#include <unordered_map>

#include "DirectXCommon.h"
#include "SrvManager.h"

class TextureManager {
public:
	static TextureManager* GetInstance();
	void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager);
	void Finalize();

	void LoadTexture(const std::string& filePath);

	uint32_t GetSrvIndex(const std::string filePath);

	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string filePath);

	const DirectX::TexMetadata& GetMetaData(const std::string filePath);
private:
	static TextureManager* instance;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = default;
	TextureManager& operator=(TextureManager&) = default;

	struct TextureData {
		DirectX::TexMetadata metadata; //width,height
		Microsoft::WRL::ComPtr<ID3D12Resource>resource; // テクスチャリソース
		uint32_t srvIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};

	std::unordered_map<std::string, TextureData> textureDatas;


	static uint32_t kSRVIndexTop;

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager = nullptr;
};