#include "GhostBlock.h"
#include "MyMath.h"

using namespace MyMath;

GhostBlock::GhostBlock() {}
GhostBlock::~GhostBlock() { delete model_; }

void GhostBlock::Init() {
    model_ = new Object3d();
    model_->Initialize();
	model_->SetModelFile("cube");

    worldTransform.Initialize();
    worldTransform.translation_ = { 5, 2, 3 };
	worldTransform.UpdateMatrix();
}

void GhostBlock::Update() {
    worldTransform.UpdateMatrix();
}

void GhostBlock::Draw() {
    if (!isActive_) return; // 非アクティブなら描画しない
    model_->Draw(worldTransform);
}

AABB GhostBlock::GetAABB() const {
    AABB aabb;
    aabb.min = worldTransform.translation_ - Vector3(1.0f, 1.0f, 1.0f);
    aabb.max = worldTransform.translation_ + Vector3(1.0f, 1.0f, 1.0f);
    return aabb;
}