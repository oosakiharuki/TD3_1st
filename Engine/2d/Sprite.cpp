#include "Sprite.h"
#include "SpriteCommon.h"
#include "externals/imgui/imgui.h"
#include "TextureManager.h"

using namespace MyMath;

void Sprite::Initialize(std::string textureFilePath) {
	this->spriteCommon_ = SpriteCommon::GetInstance();
	filePath = "resource/Sprite/" + textureFilePath;

	TextureManager::GetInstance()->LoadTexture(filePath);

	//textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

	//Sprite
	vertexResource = spriteCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(VertexData) * 4);

	//リソースの先頭アドレス
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	//頂点サイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);


	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	
	float left = 0.0f - anchorPoint.x;
	float right = 1.0f - anchorPoint.x;
	float top = 0.0f - anchorPoint.y;
	float bottom = 1.0f - anchorPoint.y;

	//左右反転
	if (isFlipX_) {
		left = -left;
		right = -right;
	}

	//上下反転
	if (isFlipY_) {
		top = -top;
		bottom = -bottom;
	}

	AdjustTextureSize();
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(filePath);
	float tex_left = textureLeftTop.x / metadata.width;
	float tex_right = textureLeftTop.x + textureSize.x / metadata.width;
	float tex_top = textureLeftTop.y / metadata.height;
	float tex_bottom = textureLeftTop.y + textureSize.y / metadata.height;

	vertexData[0].position = { left,bottom,0.0f,1.0f };//0
	vertexData[0].texcoord = { tex_left,tex_bottom };
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };

	vertexData[1].position = { left,top,0.0f,1.0f };//1,3
	vertexData[1].texcoord = { tex_left,tex_top };
	vertexData[1].normal = { 0.0f,0.0f,-1.0f };


	vertexData[2].position = { right,bottom,0.0f,1.0f };//2,5
	vertexData[2].texcoord = { tex_right,tex_bottom };
	vertexData[2].normal = { 0.0f,0.0f,-1.0f };


	vertexData[3].position = { right,top,0.0f,1.0f };//4
	vertexData[3].texcoord = { tex_right,tex_top };
	vertexData[3].normal = { 0.0f,0.0f,-1.0f };
	
	//Index
	indexResource = spriteCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(uint32_t) * 6);

	//リソースの先頭アドレス
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	//使用するリソースサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	//頂点サイズ
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 1;
	indexData[4] = 3;
	indexData[5] = 2;


	//spriteのリソース
	materialResource = spriteCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Material));

	//書き込むためのアドレス
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//色の設定
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();




	//座標変換行列	
	
	transformationMatrixResource = spriteCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();

}

void Sprite::AdjustTextureSize() {
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(filePath);

	textureSize.x = static_cast<float>(metadata.width);
	textureSize.y = static_cast<float>(metadata.height);

	size = textureSize;
}


void Sprite::Update() {

	transform.translate = { position.x,position.y,0.0f };
	transform.rotate = { 0.0f,0.0f,rotation };
	transform.scale = { size.x,size.y,1.0f };

	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, (float)WinApp::kClientWidth, (float)WinApp::kClientHeight, 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;
}

void Sprite::Draw() {
	spriteCommon_->GetDirectXCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	spriteCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress()); //rootParameterの配列の0番目 [0]
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(filePath));
	spriteCommon_->GetDirectXCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}
