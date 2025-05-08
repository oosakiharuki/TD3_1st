#pragma once
#include "Sprite.h"
#include "Framework.h"
#include "IScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "WinApp.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <future>
#include <vector>
#include <string>

class LoadingScene : public IScene {
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    // 非同期ローディング関連のメソッド
    void StartAsyncLoading();
    void LoadEssentialResources();  // 最初に必要な基本リソースのロード
    void LoadStage1Resources();     // ステージ1（基本モデルとUI）
    void LoadStage2Resources();     // ステージ2（追加モデルとUI）
    void LoadStage3Resources();     // ステージ3（ステージモデルとキャラクター）
    void LoadStage4Resources();     // ステージ4（その他全てのリソース）

    // UI要素
    Sprite* loadingText = nullptr;  // ロード中テキスト表示
    Sprite* loadingIcon = nullptr;  // 回転するロードアイコン
    Sprite* background = nullptr;   // 背景画像
    Sprite* progressBar = nullptr;  // 進行度バー
    Sprite* progressBarBg = nullptr; // 進行度バーの背景
    
    // かわいいキラキラエフェクト用
    static const int kMaxStars = 10;
    Sprite* stars[kMaxStars] = { nullptr };
    float starRotations[kMaxStars] = { 0.0f };
    float starScales[kMaxStars] = { 0.0f };
    float starAlphas[kMaxStars] = { 0.0f };
    
    float rotationAngle = 0.0f;     // アイコン回転角度
    float posX = 0.0f;            // アイコンのX座標
    float posY = 0.0f;            // アイコンのY座標
    std::atomic<float> loadingProgress = 0.0f;   // ロード進行度（スレッド間で共有）
    const float loadingSpeed = 0.05f;  // ロード速度（0.015fから高速化）
    std::atomic<bool> isLoadingComplete = false;  // ロード完了フラグ
    std::atomic<int> loadingStage = 0;           // ロードステージ（段階的にロードを行う）
    
    // 非同期ローディング用
    std::thread loadingThread;
    std::mutex loadingMutex;
    bool isAsyncLoadingStarted = false;

    // ローディング順序を制御するためのリソースキュー
    struct ResourceInfo {
        std::string name;
        bool isModel; // trueならモデル、falseならテクスチャ
    };
    std::vector<ResourceInfo> resourceQueue;

    // 各ステージの進行状況
    const int totalStages = 4;
    float stageProgress[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};