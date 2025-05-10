#pragma once
#include <string>
#include "externals/DirectXTex/DirectXTex.h"
#include <wrl.h>
#include <d3d12.h>
#include <unordered_map>
#include <future>
#include <mutex>
#include <functional>

#include "DirectXCommon.h"
#include "SrvManager.h"

class TextureManager {
public:
	static TextureManager* GetInstance();
	void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager);
	void Finalize();

	// 同期的にテクスチャを読み込む
	void LoadTexture(const std::string& filePath);

	// 非同期でテクスチャを読み込む
	void LoadTextureAsync(const std::string& filePath, std::function<void()> callback = nullptr);

	// プリロードするテクスチャのリストを追加する
	void AddToPreloadList(const std::vector<std::string>& filePathList);
	
	// プリロードリストからテクスチャを読み込み、進行状況を返す (0.0f～1.0f)
	float PreloadTextures();

	uint32_t GetSrvIndex(const std::string filePath);

	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string filePath);

	// 存在チェック用
	bool CheckTextureExist(const std::string& filePath);

	// 全てのテクスチャが正しくロードされているか確認
	void CheckAllTextureLoaded();

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

	// 非同期ローディング用
	std::mutex textureMutex;
	std::vector<std::string> preloadList;
	size_t preloadIndex = 0;

	static uint32_t kSRVIndexTop;

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager = nullptr;
};