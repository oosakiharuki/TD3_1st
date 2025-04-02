#include "TextureManager.h"

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
	// 読み込み済みテクスチャを検索
	if (textureDatas.contains(filePath)) {
		return;
	}

	// SRVが最大数を超えていないか確認
	if (!srvManager->Max()) {
		OutputDebugStringA("SRVの最大数を超えています\n");
		return;
	}

	// TextureDataの準備（最後尾を取得）
	TextureData& textureData = textureDatas[filePath];

	// 初期化
	DirectX::ScratchImage image{};
	DirectX::ScratchImage mipImages{};
	bool loadFailed = false;

	try {
		// テクスチャファイル読み込み
		std::wstring filePathW = ConvertString(filePath);
		HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);

		if (FAILED(hr)) {
			OutputDebugStringA(("テクスチャ読み込み失敗: " + filePath + "\n").c_str());
			loadFailed = true;
		}
		else {
			// ミップマップ生成 (読み込み成功時のみ)
			hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);

			if (FAILED(hr)) {
				OutputDebugStringA(("MIPマップ生成失敗: " + filePath + "\n").c_str());
				// MIPマップ生成失敗時は元の画像を使用
				mipImages = std::move(image);
			}
		}
	}
	catch (...) {
		OutputDebugStringA(("テクスチャ処理中に例外発生: " + filePath + "\n").c_str());
		loadFailed = true;
	}

	// 読み込み失敗時はダミーテクスチャ(1x1の白)を生成
	if (loadFailed || image.GetImageCount() == 0) {
		OutputDebugStringA("ダミーテクスチャを生成します\n");

		// 既存のイメージをクリア
		image.Release();
		mipImages.Release();

		// 1x1白テクスチャを作成
		image.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1);
		uint32_t* pPixel = reinterpret_cast<uint32_t*>(image.GetPixels());
		if (pPixel) {
			*pPixel = 0xFFFFFFFF; // 白色
		}

		// mipImagesを初期化
		mipImages = std::move(image);
	}

	// メタデータの取得と設定
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	textureData.metadata = metadata;

	try {
		// テクスチャリソース作成
		textureData.resource = dxCommon_->CreateTextureResource(textureData.metadata);
		if (!textureData.resource) {
			throw std::runtime_error("テクスチャリソース作成失敗");
		}

		// テクスチャデータのアップロード
		dxCommon_->UploadTextureData(textureData.resource, mipImages);

		// SRVの設定
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = metadata.format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);

		// SRVインデックスの割り当て
		textureData.srvIndex = srvManager->Allocate();
		textureData.srvHandleCPU = srvManager->GetCPUDescriptorHandle(textureData.srvIndex);
		textureData.srvHandleGPU = srvManager->GetGPUDescriptorHandle(textureData.srvIndex);

		// SRVの生成
		dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

		OutputDebugStringA(("テクスチャ読み込み成功: " + filePath + "\n").c_str());
	}
	catch (const std::exception& e) {
		OutputDebugStringA(("SRV作成中にエラー: " + std::string(e.what()) + "\n").c_str());
		// エラー時はテクスチャデータを削除
		textureDatas.erase(filePath);
	}
	catch (...) {
		OutputDebugStringA("SRV作成中に不明なエラー\n");
		textureDatas.erase(filePath);
	}
}

uint32_t TextureManager::GetSrvIndex(const std::string filePath) {
	assert(srvManager->Max());

	TextureData& textureData = textureDatas[filePath];
	return textureData.srvIndex;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string filePath) {
	assert(srvManager->Max());

	TextureData& textureData = textureDatas[filePath];
	return textureData.srvHandleGPU;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string filePath) {
	assert(srvManager->Max());

	TextureData& textureData = textureDatas[filePath];
	return textureData.metadata;
}
