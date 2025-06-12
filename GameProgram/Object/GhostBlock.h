#pragma once
#include "AABB.h"
#include "WorldTransform.h"
#include "Object3d.h"
#include "GhostColor.h"

class GhostBlock {
public:
	GhostBlock();
	~GhostBlock();
	void Init();
	void Update();
    void Draw();

    bool IsActive() const { return isActive_; } // アクティブ状態を取得
    void SetActive(bool active) { isActive_ = active; } // アクティブ状態を設定

    AABB GetAABB() const; // AABBの取得

    void SetPosition(Vector3 position) { worldTransform_.translation_ = position; }
    
    ColorType GetColor() { return colorType; }
    void SetColor(ColorType color) { colorType = color; }
    void SetSize(const Vector3& size);

    // MapLoader用のメソッド
    Vector3 GetTranslation() const { return worldTransform_.translation_; }
    ColorType GetColorType() const { return colorType; }
    Vector3 GetSize() const { return size_; }
private:
    WorldTransform worldTransform_;

    Object3d* model_ = nullptr;
    bool isActive_ = true; // ブロックが有効かどうか
    uint32_t texturehandle_ = 0;
    Vector3 size_;
    ColorType colorType = ColorType::Blue;
};