#pragma once
#include "Sprite.h"
#include "WinApp.h"

// フェード状態
enum class FadeState {
    None,       // フェードなし
    FadeIn,     // フェードイン中（徐々に明るくなる）
    FadeOut,    // フェードアウト中（徐々に暗くなる）
    Complete    // フェード完了
};

// フェード管理クラス（シングルトン）
class FadeManager {
private:
    // シングルトン
    FadeManager();
    ~FadeManager();
    FadeManager(const FadeManager&) = delete;
    FadeManager& operator=(const FadeManager&) = delete;

public:
    // インスタンス取得
    static FadeManager* GetInstance();

    // 初期化
    void Initialize();

    // 更新
    void Update();

    // 描画
    void Draw();

    // 終了
    void Finalize();

    // フェードイン開始
    void StartFadeIn(float speed = 0.02f);

    // フェードアウト開始
    void StartFadeOut(float speed = 0.02f);

    // フェード状態取得
    FadeState GetFadeState() const { return fadeState_; }

    // フェード完了したかどうか
    bool IsFadeComplete() const { return fadeState_ == FadeState::Complete; }

    // フェード中かどうか
    bool IsFading() const { return fadeState_ == FadeState::FadeIn || fadeState_ == FadeState::FadeOut; }

private:
    // フェード用スプライト
    Sprite* fadeSprite_ = nullptr;

    // フェード状態
    FadeState fadeState_ = FadeState::None;

    // フェード速度
    float fadeSpeed_ = 0.02f;

    // フェード量（0.0f〜1.0f）
    float fadeAlpha_ = 0.0f;
};
