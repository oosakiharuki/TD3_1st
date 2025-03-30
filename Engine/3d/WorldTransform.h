#pragma once
#include"Vector3.h"
#include "Matrix4x4.h"
#include "Camera.h"

// 定数バッファ用データ構造体
struct ConstBufferDataWorldTransform {
	Matrix4x4 matWorld; // ローカル → ワールド変換行列
};

class WorldTransform{
public:
	
	Vector3 scale_;

	Vector3 rotation_ ;

	Vector3 translation_ ;

	Matrix4x4 matWorld_;

	const WorldTransform* parent_ = nullptr;

	WorldTransform() = default;
	~WorldTransform() = default;


	void Initialize();

	//void CreateConstBuffer();

	//void Map();

	void UpdateMatrix();

	//void TransferMatrix();

private:

};