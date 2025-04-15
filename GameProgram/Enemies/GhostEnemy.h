#pragma once
#include "3d/Camera.h"
#include "Object3d.h"
#include "Player.h"
#include "3d/WorldTransform.h"
#include "AABB.h"
#include "CameraController.h"
#include "Collision.h"
#include "math/Vector3.h"
#include <vector>
#include "input/input.h"
#include "Mymath.h"
#include "GhostColor.h"

class Player;
class GhostEnemy {
public:
    GhostEnemy();
    ~GhostEnemy();

    void Init();
    void Update();
    void Draw();

    // 障害物リスト（AABB）の設定／追加
    void SetObstacleList(const std::vector<AABB>& obstacles);
    void AddObstacle(const AABB& obstacle);

    // AABBを取得するメソッドを追加
    AABB GetAABB() const;

    void SetParent(const WorldTransform* parent) { worldTransform_.parent_ = parent; }
    void ContralPlayer();
    void ReMove(const Vector3& position_);
    bool GetPlayerCtrl() { return isPlayer; }

    const Vector3& GetWorldTranslate() { return worldTransform_.translation_; }

    // 位置を設定するメソッドを追加
    void SetPosition(const Vector3& position);

    void SetTarget(Player* target); // Ensure Player is defined

    Vector3 GetWorldPosition();

    void Enemystop(const Vector3 stop) { velocity_ = stop; }

    void SetVelocity(const Vector3& velocity1) { velocity_ = velocity1; }

    // Playerとの衝突を検出するメソッドを追加
    bool CheckCollisionWithPlayer();

    void SetChaseRadius(float radius); // 追尾範囲を設定するメソッド

    void ClearObstacleList();

    ColorType GetColor() { return colorType; }
    void SetColor(ColorType color) { colorType = color; }

    // 他のゴーストへの参照を設定
    void SetOtherGhosts(std::vector<GhostEnemy*>* others) { otherGhosts_ = others; }

    // フィールド境界を設定
    void SetFieldBoundaries(const Vector3& min, const Vector3& max);

    // 新たに追加：ゴースト同士の衝突判定と解決
    void CheckAndResolveGhostCollisions();

    // 友達クラスにアクセス権を与える（衝突解決用）
    friend class GhostEnemy;

private:
    // ランダム移動関連のメソッド
    void UpdateRandomMovement(float deltaTime); // ランダム移動の更新
    void SetNewRandomDestination();            // 新しいランダムな目的地を設定
    void EnforceFieldBoundaries();            // フィールド境界を強制する
    void CheckCollisionWithObstacles();       // 障害物との衝突チェック

    WorldTransform worldTransform_; // Fix the error by ensuring the type is defined
    Camera* camera_ = nullptr;
    Object3d* model_ = nullptr;
    Vector3 position = { 0, 0, -20 };
    bool onGround_ = true;
    float velocityY_ = 0.0f;
    Vector3 velocity_;
    float kChaseSpeed = 0.1f;  // 追跡速度
    float kSeparationSpeed = 0.15f;  // 分離速度（他のゴーストから離れる速度）

    // 障害物リスト
    std::vector<AABB> obstacleList_;

    Vector3 velocity;
    //bool IsJump = false;

    XINPUT_STATE state, preState;
    const float speed = 0.2f;

    // 移動
    bool Normal = true;
    float timer = 0.0f;
    const float corveTime = 1.0f;

    // スタン
    bool isStan = false;
    float timerS = 0.0f;
    const float stanTime = 3.0f;

    bool isPlayer = false;

    bool MoveNot = false;

    Player* player_ = nullptr;

    float radius = 30.0f;

    float chaseRadius_ = 30.0f;   // 追尾範囲の初期値
    float minGhostDistance_ = 5.0f; // ゴースト同士の最小距離
    bool IsPlayerInChaseRadius(); // 追尾範囲内にPlayerがいるかどうかをチェックするメソッド
    ColorType colorType = ColorType::Blue;

    // 他のゴーストへの参照
    std::vector<GhostEnemy*>* otherGhosts_ = nullptr;

    // ランダム移動関連のパラメータ
    Vector3 randomDestination_ = { 0.0f, 0.0f, 0.0f }; // ランダム移動の目的地
    float randomMoveTimer_ = 0.0f;                    // ランダム移動のタイマー
    float randomMoveDuration_ = 7.0f;                 // 一つの目的地に向かう最大時間（秒）
    float randomMoveRadius_ = 50.0f;                  // ランダム移動の半径（この範囲内に目的地を設定）
    float randomMoveSpeed_ = 0.1f;                    // ランダム移動の速度
    float destinationThreshold_ = 3.0f;               // 目的地に到達したとみなす距離

    // フィールド境界
    Vector3 fieldMin_ = { -100.0f, -50.0f, -100.0f }; // フィールドの最小範囲
    Vector3 fieldMax_ = { 100.0f, 100.0f, 100.0f };   // フィールドの最大範囲
    float bounceBackDistance_ = 20.0f;               // 境界から反発する距離
};