#pragma once
#include "Sprite.h"
#include "Framework.h"
#include "IScene.h"

class LoadingScene : public IScene {
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    Sprite* loadingText = nullptr;  // ロード中テキスト表示
    Sprite* loadingIcon = nullptr;  // 回転するロードアイコン
    Sprite* background = nullptr;   // 背景画像
    float rotationAngle = 0.0f;     // アイコン回転角度
    float loadingProgress = 0.0f;   // ロード進行度
    const float loadingSpeed = 0.015f;  // ロード速度（値が大きいほど速く完了）
    bool isLoadingComplete = false;  // ロード完了フラグ
    int loadingStage = 0;           // ロードステージ（段階的にロードを行う）
};