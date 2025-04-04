#pragma once
#include "Sprite.h"
#include "Player.h"
#include "Key.h"
#include "Door.h"
#include "Goal.h"
#include <vector>

class Minimap {
public:
    Minimap();
    ~Minimap();

    // 初期化
    void Initialize();

    // 更新
    void Update();

    // 描画
    void Draw();

    // プレイヤーの参照を設定
    void SetPlayer(Player* player) { player_ = player; }

    // 鍵リストを設定
    void SetKeys(const std::vector<Key*>& keys) { keys_ = keys; }

    // ドアリストを設定
    void SetDoors(const std::vector<Door*>& doors) { doors_ = doors; }

    // ゴールを設定
    void SetGoal(Goal* goal) { goal_ = goal; }

private:
    // 3D座標をミニマップ座標に変換する関数
    Vector2 WorldToMinimap(const Vector3& worldPos);

    // ミニマップの背景と枠線
    Sprite* background_ = nullptr;
    Sprite* border_ = nullptr;

    // プレイヤーのアイコン
    Sprite* playerIcon_ = nullptr;

    // 鍵のアイコン
    Sprite* keyIcon_ = nullptr;

    // ドアのアイコン
    Sprite* doorIcon_ = nullptr;

    // ゴールのアイコン
    Sprite* goalIcon_ = nullptr;

    // ミニマップの位置とサイズ
    Vector2 position_;
    Vector2 size_;

    // オブジェクトの参照
    Player* player_ = nullptr;
    std::vector<Key*> keys_;
    std::vector<Door*> doors_;
    Goal* goal_ = nullptr;

    // マップのスケーリング設定
    float scaleFactor_;
};