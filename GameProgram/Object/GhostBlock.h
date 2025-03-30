#pragma once
#include "AABB.h"

class GhostBlock {
public:
	GhostBlock();
	~GhostBlock();
	void Init(Camera* camera_);
	void Update();
    void Draw();

    bool IsActive() const { return isActive_; } // アクティブ状態を取得
    void SetActive(bool active) { isActive_ = active; } // アクティブ状態を設定

    AABB GetAABB() const; // AABBの取得

private:
    WorldTransform worldTransform;
    Camera* Camera_ = nullptr;
    Model* model_ = nullptr;
    bool isActive_ = true; // ブロックが有効かどうか
    uint32_t texturehandle_ = 0;
};