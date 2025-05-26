#include "Block.h"
#include "MyMath.h"

using namespace MyMath;

Block::Block() {}

Block::~Block() { 
	delete model_;
	delete particle;
}

void Block::Init() {

	// worldTransformを確実に初期化
	worldTransform.Initialize();

	// モデルの読み込み
	model_ = new Object3d();
	model_->Initialize();
	model_->SetModelFile("cube");
	// 行列の更新
	worldTransform.UpdateMatrix();

	//ブロックに攻撃した時のパーテイクル
	particle = new Particle();
	particle->Initialize("resource/Sprite/break_block.png");
	particle->ChangeMode(BornParticle::Stop);
	particle->ChangeType(ParticleType::Normal);
}

void Block::Update() {
	particle->Update();
	particle->SetScale({ 0.8f,0.8f ,0.8f });


	worldTransform.UpdateMatrix();
}

void Block::Draw() {
	// 条件式の修正: !isActive_ ではなく isActive_ のときに描画
	if (isActive_) {
		model_->Draw(worldTransform);
	}
	// 非アクティブなら描画しない (ここでreturnを入れるなら上のifブロックの外に出す)
}

void Block::DrawP() {
	particle->Draw();
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

void Block::OnCollision() {
	hp--;

	particle->SetParticleCount(3);
	particle->SetTranslate(particlePosition);
	particle->ChangeMode(BornParticle::MomentMode);

	if (hp <= 0) {
		isActive_ = false;
	}
}
