#include "Minimap.h"
#include "TextureManager.h"

Minimap::Minimap()
    : position_({ 1030.0f, 470.0f }), size_({ 250.0f, 250.0f }),
    scaleFactor_(0.5f) {
}

Minimap::~Minimap() {
    if (border_) delete border_;
    if (background_) delete background_;
    if (playerIcon_) delete playerIcon_;
    if (keyIcon_) delete keyIcon_;
    if (doorIcon_) delete doorIcon_;
    if (goalIcon_) delete goalIcon_;
}

void Minimap::Initialize() {
    // ミニマップの背景と枠線（白い正方形）
    background_ = new Sprite();
    background_->Initialize("white1x1.png");
    background_->SetPosition(position_);
    background_->SetSize(size_);
    background_->SetColor(Vector4(0.1f, 0.1f, 0.1f, 0.8f)); // 半透明の暗めの背景

    // 枠線用のスプライト（サイズを少し大きくして背景の周りに表示）
    border_ = new Sprite();
    border_->Initialize("white1x1.png");
    border_->SetPosition(Vector2(position_.x - 2.0f, position_.y - 2.0f));
    border_->SetSize(Vector2(size_.x + 4.0f, size_.y + 4.0f));
    border_->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.8f)); // 白い枠線

    // プレイヤーアイコン
    playerIcon_ = new Sprite();
    playerIcon_->Initialize("player.png");
    playerIcon_->SetSize(Vector2(14.4f, 14.4f)); // 12.0 × 1.2倍
    playerIcon_->SetColor(Vector4(1.0f, 0.5f, 0.0f, 1.0f)); // オレンジ色で目立たせる

    // 鍵アイコン（単一のアイコンを使いまわす）
    keyIcon_ = new Sprite();
    keyIcon_->Initialize("key.png");
    keyIcon_->SetSize(Vector2(19.2f, 19.2f)); // 16 × 1.2倍

    // ドアアイコン
    doorIcon_ = new Sprite();
    doorIcon_->Initialize("door.png");
    doorIcon_->SetSize(Vector2(16.0f, 16.0f));

    // ゴールアイコン
    goalIcon_ = new Sprite();
    goalIcon_->Initialize("white1x1.png");
    goalIcon_->SetSize(Vector2(16.0f, 16.0f));
    goalIcon_->SetColor(Vector4(0.0f, 1.0f, 0.0f, 1.0f)); // ゴールは緑色で表示
}

void Minimap::Update() {
    if (!player_) return;

    // プレイヤーの位置を取得
    Vector3 playerPosition = player_->GetWorldPosition();

    // 背景と枠線の更新
    border_->Update();
    background_->Update();

    // プレイヤーアイコンの位置を更新
    Vector2 playerPos2D = WorldToMinimap(playerPosition);
    playerIcon_->SetPosition(playerPos2D);
    playerIcon_->Update();

    // ゴールの位置を更新
    if (goal_) {
        Vector3 goalPosition = goal_->GetPosition();
        Vector2 goalPos2D = WorldToMinimap(goalPosition);
        goalIcon_->SetPosition(goalPos2D);
        goalIcon_->Update();
    }

    // ドアの位置を更新
    for (const auto& door : doors_) {
        if (door) {
            // ドアの位置を取得
            Vector3 doorPosition = door->GetPosition();
            Vector2 doorPos2D = WorldToMinimap(doorPosition);
            doorIcon_->SetPosition(doorPos2D);
            doorIcon_->Update();
        }
    }
}

void Minimap::Draw() {
    // 枠線描画
    border_->Draw();

    // 背景描画
    background_->Draw();

    // ゴールの描画
    if (goal_) {
        goalIcon_->Draw();
    }

    // ドアの描画
    for (const auto& door : doors_) {
        if (door && !door->IsDoorOpened()) {
            doorIcon_->Draw();
        }
    }

    // 鍵の描画
    for (const auto& key : keys_) {
        if (key) {
            // 鍵の位置を取得
            Vector3 keyPosition = key->GetPosition();
            Vector2 keyPos2D = WorldToMinimap(keyPosition);
            keyIcon_->SetPosition(keyPos2D);

            // 取得済みなら薄く表示
            if (key->IsKeyObtained()) {
                keyIcon_->SetColor(Vector4(0.5f, 0.5f, 0.0f, 0.5f)); // 薄い黄色
            }
            else {
                keyIcon_->SetColor(Vector4(1.0f, 1.0f, 0.0f, 1.0f)); // 通常の黄色
            }

            keyIcon_->Update();
            keyIcon_->Draw();
        }
    }

    // プレイヤーの描画（最後に描画して前面に表示）
    playerIcon_->Draw();
}

Vector2 Minimap::WorldToMinimap(const Vector3& worldPos) {
    if (!player_) return Vector2(0, 0);

    // プレイヤーの位置を取得
    Vector3 playerPosition = player_->GetWorldPosition();

    // ミニマップの中心座標
    float centerX = position_.x + size_.x / 2.0f;
    float centerY = position_.y + size_.y / 2.0f;

    // プレイヤーを基準にした相対座標を計算（X-Zプレーン）
    float relativeX = (worldPos.x - playerPosition.x) * scaleFactor_;
    float relativeZ = (worldPos.z - playerPosition.z) * scaleFactor_;

    // カメラの回転を無視してマップに反映（プレイヤーの向きに関わらずマップは北が上）
    // 相対座標をミニマップ上の座標に変換
    float minimapX = centerX + relativeX;
    float minimapY = centerY - relativeZ; // Z軸は反転して表示（奥がミニマップ上部）

    return Vector2(minimapX, minimapY);
}