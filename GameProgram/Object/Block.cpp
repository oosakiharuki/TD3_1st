#include "Block.h"
#include "MyMath.h"

using namespace MyMath;

Block::Block() {}

Block::~Block() { delete model_; }

void Block::Init() {

	// worldTransformを確実に初期化
	worldTransform.Initialize();

	// モデルの読み込み
	model_ = new Object3d();
	model_->Initialize();
	model_->SetModelFile("cube");
	// 行列の更新
	worldTransform.UpdateMatrix();
}

void Block::Update() {
	worldTransform.UpdateMatrix();
}

void Block::Draw() {
	// 条件式の修正: !isActive_ ではなく isActive_ のときに描画
	if (isActive_) {
		model_->Draw(worldTransform);
	}
	// 非アクティブなら描画しない (ここでreturnを入れるなら上のifブロックの外に出す)
}

AABB Block::GetAABB() const {
	AABB aabb;
	aabb.min = worldTransform.translation_ - size_;
	aabb.max = worldTransform.translation_ + size_;
	return aabb;
}

void Block::SetPosition(const Vector3& pos) {
	worldTransform.translation_ = pos;
	worldTransform.UpdateMatrix();
}

void Block::SetSize(const Vector3& size) {
	worldTransform.scale_ = size;
	size_ = size;
	worldTransform.UpdateMatrix();
}
