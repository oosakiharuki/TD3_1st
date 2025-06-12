#include "MoveTile.h"
#ifdef _DEBUG
#include "ImGuiManager.h"
#endif

MoveTile::MoveTile() {}

MoveTile::~MoveTile() { delete model_; }

void MoveTile::Init() {
	worldTransform_.Initialize();

	model_ = new Object3d();
	model_->Initialize();
	model_->SetModelFile("tile");

	// 行列を更新
	worldTransform_.UpdateMatrix();
}

void MoveTile::Update() {
	// 現在の時間を取得（秒換算）
	float time = static_cast<float>(clock()) / CLOCKS_PER_SEC;

	// sin波を利用して上下移動
	worldTransform_.translation_.y = initialY_ + std::sin(time * moveSpeed_) * moveRange_;

	// プレイヤーとの当たり判定
	if (player_) {
		AABB playerAABB = player_->GetAABB();
		AABB moveTileAABB = GetAABB();

		// プレイヤーにもAABBを渡す
		player_->ResolveCollisionWithTile(moveTileAABB);

		if (IsCollisionAABB(playerAABB, moveTileAABB)) {
			// 衝突時の処理
		}
	}

	// ワールド座標を更新
	worldTransform_.UpdateMatrix();
}



void MoveTile::Draw() { model_->Draw(worldTransform_); }

AABB MoveTile::GetAABB() const {
	// ドアのローカル座標：
	//   min = (-3, 0, -1), max = (3, 6, 1)
	//   中心 = (0, 3, 0)
	//   半サイズ(=halfExtents) = (3, 3, 1)
	Vector3 localCenter = { 0.0f, 0.5f, 0.0f };
	Vector3 halfExtents = { 3.7f, 0.5f, 3.7f };

	// スケールを要素ごとに乗算
	Vector3 scaledCenter = { localCenter.x * worldTransform_.scale_.x, localCenter.y * worldTransform_.scale_.y, localCenter.z * worldTransform_.scale_.z };

	Vector3 scaledHalfExtents = { halfExtents.x * worldTransform_.scale_.x, halfExtents.y * worldTransform_.scale_.y, halfExtents.z * worldTransform_.scale_.z };

	// ドアのワールド中心 = ピボット位置(translation_) + scaledCenter
	Vector3 moveTileCenter = { worldTransform_.translation_.x + scaledCenter.x, worldTransform_.translation_.y + scaledCenter.y, worldTransform_.translation_.z + scaledCenter.z };

	// AABBを計算
	AABB moveTileAABB;
	moveTileAABB.min = { moveTileCenter.x - scaledHalfExtents.x, moveTileCenter.y - scaledHalfExtents.y, moveTileCenter.z - scaledHalfExtents.z };
	moveTileAABB.max = { moveTileCenter.x + scaledHalfExtents.x, moveTileCenter.y + scaledHalfExtents.y, moveTileCenter.z + scaledHalfExtents.z };
	return moveTileAABB;
}