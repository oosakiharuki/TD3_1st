#include "TextureManager.h"
#include <vector>
#include <string>

using namespace StringUtility;

TextureManager* TextureManager::instance = nullptr;

uint32_t TextureManager::kSRVIndexTop = 1;


TextureManager* TextureManager::GetInstance() {
	if (instance == nullptr) {
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {
	dxCommon_ = dxCommon;
	this->srvManager = srvManager;
	textureDatas.reserve(SrvManager::kMaxSRVCount);
}


void TextureManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void TextureManager::LoadTexture(const std::string& filePath) {
	//読み込み済みテクスチャを検索
	if (textureDatas.contains(filePath)) {
		return;
	}

	assert(srvManager->Max());

	//テクスチャファイル // byte関連
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	if (FAILED(hr)) {
		char errorMsg[256];
		sprintf_s(errorMsg, "TextureManager::LoadTexture - Failed to load texture file: %s (HRESULT: 0x%08X)\n", filePath.c_str(), hr);
		OutputDebugStringA(errorMsg);
		return;
	}

	//ミップマップ　//拡大縮小で使う
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	if (FAILED(hr)) {
		char errorMsg[256];
		sprintf_s(errorMsg, "TextureManager::LoadTexture - Failed to generate mipmaps for: %s (HRESULT: 0x%08X)\n", filePath.c_str(), hr);
		OutputDebugStringA(errorMsg);
		return;
	}
	
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	
	// メタデータの検証
	if (metadata.width == 0 || metadata.height == 0) {
		char errorMsg[256];
		sprintf_s(errorMsg, "TextureManager::LoadTexture - Invalid texture metadata for: %s (width=%zu, height=%zu)\n", 
			filePath.c_str(), metadata.width, metadata.height);
		OutputDebugStringA(errorMsg);
		return;
	}
	
	//最後尾を取得
	TextureData& textureData = textureDatas[filePath];

	textureData.metadata = metadata;
	textureData.resource = dxCommon_->CreateTextureResource(textureData.metadata);
	if (!textureData.resource) {
		// リソース作成に失敗した場合はエントリを削除
		textureDatas.erase(filePath);
		return;
	}
	dxCommon_->UploadTextureData(textureData.resource, mipImages);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	textureData.srvIndex = srvManager->Allocate();
	textureData.srvHandleCPU = srvManager->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager->GetGPUDescriptorHandle(textureData.srvIndex);

	
	//SRVの生成
	dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);
}

// 非同期でテクスチャを読み込む
void TextureManager::LoadTextureAsync(const std::string& filePath, std::function<void()> callback) {
    // スレッドを使ってテクスチャをロード
    std::thread loadThread([this, filePath, callback]() {
        // ロック範囲を最小限に
        {
            std::lock_guard<std::mutex> lock(textureMutex);
            
            // 既に読み込まれている場合は何もしない
            if (textureDatas.contains(filePath)) {
                if (callback) callback();
                return;
            }
        }
        
        // テクスチャ読み込み
        //テクスチャファイル // byte関連
        DirectX::ScratchImage image{};
        std::wstring filePathW = ConvertString(filePath);
        HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
        if (FAILED(hr)) {
            OutputDebugStringA(("テクスチャ読み込み失敗: " + filePath + "\n").c_str());
            if (callback) callback();
            return;
        }

        //ミップマップ　//拡大縮小で使う
        DirectX::ScratchImage mipImages{};
        hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
        if (FAILED(hr)) {
            OutputDebugStringA(("ミップマップ生成失敗: " + filePath + "\n").c_str());
            if (callback) callback();
            return;
        }
        
        const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
        
        TextureData textureData;
        textureData.metadata = metadata;
        textureData.resource = dxCommon_->CreateTextureResource(textureData.metadata);
        dxCommon_->UploadTextureData(textureData.resource, mipImages);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = metadata.format;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

        // テクスチャデータをマップに追加するためのロック
        {
            std::lock_guard<std::mutex> lock(textureMutex);
            if (textureDatas.contains(filePath)) {
                // 別スレッドで既に追加された場合
                if (callback) callback();
                return;
            }
            
            textureData.srvIndex = srvManager->Allocate();
            textureData.srvHandleCPU = srvManager->GetCPUDescriptorHandle(textureData.srvIndex);
            textureData.srvHandleGPU = srvManager->GetGPUDescriptorHandle(textureData.srvIndex);
            
            //SRVの生成
            dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);
            
            // テクスチャデータを登録
            textureDatas[filePath] = textureData;
        }
        
        // コールバック実行
        if (callback) callback();
    });
    
    // スレッドをデタッチ
    loadThread.detach();
}

// プリロードリストにテクスチャを追加
void TextureManager::AddToPreloadList(const std::vector<std::string>& filePathList) {
    std::lock_guard<std::mutex> lock(textureMutex);
    preloadList.insert(preloadList.end(), filePathList.begin(), filePathList.end());
}

// プリロードリストからテクスチャを読み込む (1回の呼び出しで1つのテクスチャを読み込む)
float TextureManager::PreloadTextures() {
    if (preloadIndex >= preloadList.size()) {
        return 1.0f; // 全て読み込み完了
    }
    
    // 次に読み込むべきテクスチャパス
    std::string nextTexture = preloadList[preloadIndex];
    
    // 既に読み込まれている場合はスキップ
    if (!CheckTextureExist(nextTexture)) {
        try {
            LoadTexture(nextTexture);
            OutputDebugStringA(("プリロードテクスチャ読み込み: " + nextTexture + "\n").c_str());
        }
        catch (std::exception& e) {
            OutputDebugStringA(("テクスチャ読み込みエラー: " + std::string(e.what()) + "\n").c_str());
        }
    }
    
    // インデックスを進める
    preloadIndex++;
    
    // 進捗率を返す (0.0f～1.0f)
    return static_cast<float>(preloadIndex) / static_cast<float>(preloadList.size());
}

uint32_t TextureManager::GetSrvIndex(const std::string filePath) {
	assert(srvManager->Max());

	// テクスチャが存在しない場合はエラー
	auto it = textureDatas.find(filePath);
	if (it == textureDatas.end()) {
		char errorMsg[256];
		sprintf_s(errorMsg, "TextureManager::GetSrvIndex - Texture not found: %s\n", filePath.c_str());
		OutputDebugStringA(errorMsg);
		return 0; // 無効なインデックスを返す
	}

	return it->second.srvIndex;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string filePath) {
	assert(srvManager->Max());

	// テクスチャが存在しない場合はエラー
	auto it = textureDatas.find(filePath);
	if (it == textureDatas.end()) {
		char errorMsg[256];
		sprintf_s(errorMsg, "TextureManager::GetSrvHandleGPU - Texture not found: %s\n", filePath.c_str());
		OutputDebugStringA(errorMsg);
		D3D12_GPU_DESCRIPTOR_HANDLE emptyHandle = {};
		return emptyHandle;
	}

	return it->second.srvHandleGPU;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string filePath) {
	assert(srvManager->Max());

	// テクスチャが存在しない場合はエラー
	auto it = textureDatas.find(filePath);
	if (it == textureDatas.end()) {
		char errorMsg[256];
		sprintf_s(errorMsg, "TextureManager::GetMetaData - Texture not found: %s\n", filePath.c_str());
		OutputDebugStringA(errorMsg);
		// 静的な空のメタデータを返す
		static DirectX::TexMetadata emptyMetadata = {};
		// 初回のみデフォルト値を設定
		static bool initialized = false;
		if (!initialized) {
			emptyMetadata.width = 1;
			emptyMetadata.height = 1;
			emptyMetadata.depth = 1;
			emptyMetadata.arraySize = 1;
			emptyMetadata.mipLevels = 1;
			emptyMetadata.format = DXGI_FORMAT_R8G8B8A8_UNORM;
			emptyMetadata.dimension = DirectX::TEX_DIMENSION_TEXTURE2D;
			initialized = true;
		}
		return emptyMetadata;
	}

	return it->second.metadata;
}

bool TextureManager::CheckTextureExist(const std::string& filePath) {
	//読み込み済みテクスチャを検索
	return textureDatas.contains(filePath);
}

void TextureManager::CheckAllTextureLoaded() {
	// ロードされたテクスチャの数を出力
	OutputDebugStringA(("ロードされたテクスチャ数: " + std::to_string(textureDatas.size()) + "\n").c_str());

	// 基本的なテクスチャがロードされているかチェック
	const std::vector<std::string> essentialTextures = {
		"resource/Sprite/player.png",
		"resource/Sprite/Bullet.png",
		"resource/Sprite/title_background.png",
		"resource/Sprite/TitleName.png"
	};

	for (const auto& tex : essentialTextures) {
		if (!CheckTextureExist(tex)) {
			OutputDebugStringA(("重要なテクスチャがロードされていません: " + tex + "\n").c_str());
		}
	}
}