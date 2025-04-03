#include "GhostBlock.h"
#include "MyMath.h"

using namespace MyMath;

GhostBlock::GhostBlock() {}
GhostBlock::~GhostBlock() { delete model_; }

void GhostBlock::Init() {
    model_ = new Object3d();
    model_->Initialize();
	model_->SetModelFile("EnemyBullet");

    worldTransform_.Initialize();
    //worldTransform_.translation_ = { 5, 2, 3 };
	worldTransform_.UpdateMatrix();
}

void GhostBlock::Update() {
    worldTransform_.UpdateMatrix();
}

void GhostBlock::Draw() {
    if (!isActive_) return; // 非アクティブなら描画しない

    switch (colorType)
    {
    case ColorType::Blue:
        model_->Draw(worldTransform_, "resource/Sprite/BlueGhost.png");
        break;
    case ColorType::Green:
        model_->Draw(worldTransform_, "resource/Sprite/GreenGhost.png");
        break;
    case ColorType::Red:
        model_->Draw(worldTransform_, "resource/Sprite/RedGhost.png");
        break;
    default:
        break;
    }
}

AABB GhostBlock::GetAABB() const {
    AABB aabb;
    aabb.min = worldTransform_.translation_ - Vector3(1.0f, 1.0f, 1.0f);
    aabb.max = worldTransform_.translation_ + Vector3(1.0f, 1.0f, 1.0f);
    return aabb;
}