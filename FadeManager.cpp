#include "FadeManager.h"

// 静的メンバ変数の実体化
FadeManager* FadeManager::GetInstance() {
    static FadeManager instance;
    return &instance;
}

FadeManager::FadeManager() {
    fadeState_ = FadeState::None;
    fadeAlpha_ = 0.0f;
    fadeSpeed_ = 0.02f;
}

FadeManager::~FadeManager() {
    Finalize();
}

void FadeManager::Initialize() {
    // フェード用黒スプライトの生成
    fadeSprite_ = new Sprite();

    // 白色テクスチャを使用（後で色を黒に変更）
    fadeSprite_->Initialize("circle.png");

    // 画面全体を覆うサイズに設定
    fadeSprite_->SetSize({
        static_cast<float>(WinApp::kClientWidth),
        static_cast<float>(WinApp::kClientHeight)
        });

    // 左上原点
    fadeSprite_->SetPosition({ 0.0f, 0.0f });

    // 黒色に設定（アルファ値は可変）
    fadeSprite_->SetColor({ 0.0f, 0.0f, 0.0f, fadeAlpha_ });
}

void FadeManager::Update() {
    // フェード処理の更新
    if (fadeState_ == FadeState::FadeIn) {
        // フェードイン（α値減少）
        fadeAlpha_ -= fadeSpeed_;

        // 下限に達したらフェード完了
        if (fadeAlpha_ <= 0.0f) {
            fadeAlpha_ = 0.0f;
            fadeState_ = FadeState::Complete;
        }
    }
    else if (fadeState_ == FadeState::FadeOut) {
        // フェードアウト（α値増加）
        fadeAlpha_ += fadeSpeed_;

        // 上限に達したらフェード完了
        if (fadeAlpha_ >= 1.0f) {
            fadeAlpha_ = 1.0f;
            fadeState_ = FadeState::Complete;
        }
    }

    // スプライトの色情報更新
    fadeSprite_->SetColor({ 0.0f, 0.0f, 0.0f, fadeAlpha_ });

    // スプライトの更新
    fadeSprite_->Update();
}

void FadeManager::Draw() {
    // フェード中のみ描画（Complete状態では描画しない）
    if (fadeAlpha_ > 0.0f) {
        fadeSprite_->Draw();
    }
}

void FadeManager::Finalize() {
    if (fadeSprite_) {
        delete fadeSprite_;
        fadeSprite_ = nullptr;
    }
}

void FadeManager::StartFadeIn(float speed) {
    fadeState_ = FadeState::FadeIn;
    fadeSpeed_ = speed;
    // フェードイン開始時は完全不透明から始める
    fadeAlpha_ = 1.0f;
}

void FadeManager::StartFadeOut(float speed) {
    fadeState_ = FadeState::FadeOut;
    fadeSpeed_ = speed;
    // フェードアウト開始時は完全透明から始める
    fadeAlpha_ = 0.0f;
}
