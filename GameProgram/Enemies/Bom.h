#pragma once
#include "WorldTransform.h"
#include "Object3d.h"
#include "AABB.h"
#include "collision.h"

#include "Mymath.h"

class Bom{
public:
	Bom();
	~Bom();
	void Init(Vector3 position,Vector3 velocity);
	void Update();
	void Draw();
	bool IsDaed() { return isDead; }

	AABB GetAABB();
	void OnCollision();

private:
	WorldTransform worldTransform_;
	Object3d* model_ = nullptr;
	Vector3 position_;
	bool onGround_ = true;

	Vector3 velocity_;
	float deadTimer = 3.0f;
	bool isDead = false;
};