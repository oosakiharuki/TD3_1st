#include "WorldTransform.h"
#include "MyMath.h"
#include "Object3dCommon.h"

using namespace MyMath;

void WorldTransform::UpdateMatrix() {
	// スケール、回転、平行移動を合成して行列を計算する
	matWorld_ = MakeAffineMatrix(scale_, rotation_, translation_);

	//親子関係
	if (parent_) {
		//matWorld_ = MatrixMultiply(matWorld_, parent_->matWorld_);
		matWorld_ = Multiply(matWorld_, parent_->matWorld_);
	}

	// 定数バッファに転送する
	//TransferMatrix();
}


void WorldTransform::Initialize() {
	scale_ = { 1.0f,1.0f,1.0f };

	rotation_ = { 0.0f,0.0f,0.0f };

	translation_ = { 0.0f,0.0f,0.0f };

	matWorld_ = MakeAffineMatrix(scale_, rotation_, translation_);

}


//void WorldTransform::TransferMatrix() {
//	//モデル
//	matWorld_ = MakeAffineMatrix(scale_, rotation_, translation_);
//	Matrix4x4 worldMatrix = MakeAffineMatrix(scale_, rotation_, translation_);
//	Matrix4x4 WorldViewProjectionMatrix;
//	if (camera) {
//		Matrix4x4 projectionMatrix = camera->GetViewProjectionMatrix();
//		WorldViewProjectionMatrix = Multiply(matWorld_, projectionMatrix);
//	}
//	else {
//		WorldViewProjectionMatrix = matWorld_;
//	}
//}

