#pragma once
//#include "Vector2.h"
//#include "Vector3.h"
//#include "Vector4.h"
#include "MyMath.h"
//ComPtr
#include <wrl.h>

#include "DirectXCommon.h"

class SpriteCommon;

class Sprite{
public:
	void Initialize(std::string textureFilePath);
	void Update();
	void Draw();


	const Vector2& GetPosition()const { return position; }
	void SetPosition(const Vector2& position) { this->position = position; }

	float GetRotate() { return rotation; }
	void SetRotate(float rotation) { this->rotation = rotation; }

	const Vector4& GetColor()const { return materialData->color; }
	void SetColor(const Vector4& color) { materialData->color = color; }

	const Vector2& GetSize()const { return size; }
	void SetSize(const Vector2& size) { this->size = size; }


	const Vector2& GetAnchorPoint()const { return anchorPoint; }
	void SetAnchorPoint(const Vector2& anchorPoint) { this->anchorPoint = anchorPoint; }

	const bool& GetFlipX() const { return isFlipX_; }
	const bool& GetFlipY() const { return isFlipY_; }
	
	void SetFlipX(const bool& isFlipX) { this->isFlipX_ = isFlipX; }
	void SetFlipY(const bool& isFlipY) { this->isFlipY_ = isFlipY; }


	const Vector2& GetTextureLT() const { return textureLeftTop; }
	const Vector2& GetTextureSize() const { return textureSize; }

	void SetTextureLT(const Vector2& textureLeftTop) { this->textureLeftTop = textureLeftTop; }
	void SetTextureSize(const Vector2& textureSize) { this->textureSize = textureSize; }

private:
	SpriteCommon* spriteCommon_ = nullptr;


	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;

	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	//頂点バッファービュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;


	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	//マテリアルにデータを書き込む
	Material* materialData = nullptr;


	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;

	TransformationMatrix* transformationMatrixData = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU;

	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	
	Transform uvTransform{
	{ 1.0f,1.0f,1.0f },
	{ 0.0f,0.0f,0.0f },
	{ 0.0f,0.0f,0.0f }
	};


	//座標
	Vector2 position = { 0.0f,0.0f };
	
	float rotation = 0.0f;

	Vector2 size = { 100.0f,100.0f };


	uint32_t textureIndex = 0;

	//アンカーポイント ココが中心点になる
	Vector2 anchorPoint = { 0.0f,0.0f };
	
	//フリップ テクスチャ反転
	bool isFlipX_ = false;
	bool isFlipY_ = false;

	//左上から情報を得る
	Vector2 textureLeftTop = { 0.0f,0.0f };
	//切り取りサイズ
	Vector2 textureSize = { 64.0f,64.0f };

	//本来のテクスチャの大きさにする
	void AdjustTextureSize();

	std::string filePath;
};
