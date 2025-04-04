#pragma once
#include "IScene.h"
#include "Framework.h"
#include "Sprite.h"
#include "GameScene.h"

// ロード状態を表す列挙型
enum class LoadState {
    NotStarted,
    InProgress,
    Completed
};

class LoadingScene : public IScene {
public:
    LoadingScene();
    ~LoadingScene();

    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    // ロード画面用スプライト
    Sprite* backgroundSprite = nullptr;
    Sprite* loadingBarBgSprite = nullptr;
    Sprite* loadingBarFgSprite = nullptr;

    // 実際のロード進行度 (0.0f〜1.0f)
    float loadProgress;

    // 表示用の進行度（イージング適用）
    float displayProgress;

    // 進行の速度係数
    float progressSpeed;

    // ロード状態
    LoadState loadState;

    // ロード中のゲームシーン
    GameScene* gameScene;

    // ロード完了後の待機時間
    float waitTimer;

    // ロードステップ管理
    int currentLoadStep;
    int totalLoadSteps;

    // ロードステップ実行関数
    void ExecuteLoadStep();
};