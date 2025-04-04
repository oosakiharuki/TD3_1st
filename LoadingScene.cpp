#include "LoadingScene.h"
#include "Input.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "SpriteCommon.h"
#include <cmath>

// イージング関数の定義
namespace Easing {
    // イーズアウト関数（徐々に減速）
    float EaseOutQuad(float t) {
        return 1.0f - (1.0f - t) * (1.0f - t);
    }

    // イーズインアウト関数（最初と最後が緩やかで中間が速い）
    float EaseInOutQuad(float t) {
        return t < 0.5f ? 2.0f * t * t : 1.0f - pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
    }

    // バウンス効果（跳ね返り）
    float EaseOutBounce(float t) {
        const float n1 = 7.5625f;
        const float d1 = 2.75f;

        if (t < 1.0f / d1) {
            return n1 * t * t;
        }
        else if (t < 2.0f / d1) {
            t -= 1.5f / d1;
            return n1 * t * t + 0.75f;
        }
        else if (t < 2.5f / d1) {
            t -= 2.25f / d1;
            return n1 * t * t + 0.9375f;
        }
        else {
            t -= 2.625f / d1;
            return n1 * t * t + 0.984375f;
        }
    }
}

LoadingScene::LoadingScene() :
    loadProgress(0.0f),
    displayProgress(0.0f), // 表示用の進捗率（イージング適用）
    loadState(LoadState::NotStarted),
    gameScene(nullptr),
    waitTimer(0.0f),
    currentLoadStep(0),
    totalLoadSteps(7), // ロードステップの総数（モデル、マップ、敵等の読み込み）
    progressSpeed(2.0f) // 進行の速度係数（大きいほど早く追いつく）
{
}

LoadingScene::~LoadingScene() {
    Finalize();
}

void LoadingScene::Initialize() {
    // ロード画面用スプライトの初期化
    backgroundSprite = new Sprite();
    backgroundSprite->Initialize("loading_bg.png"); // ロード画面背景
    backgroundSprite->SetPosition({ 0, 0 });
    backgroundSprite->SetSize({ static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight) });

    loadingBarBgSprite = new Sprite();
    loadingBarBgSprite->Initialize("loading_bar_bg.png"); // ロードバー背景
    loadingBarBgSprite->SetPosition({ WinApp::kClientWidth / 2.0f - 400.0f, WinApp::kClientHeight * 0.7f });
    loadingBarBgSprite->SetSize({ 800.0f, 40.0f });

    loadingBarFgSprite = new Sprite();
    loadingBarFgSprite->Initialize("loading_bar_fg.png"); // ロードバー前景
    loadingBarFgSprite->SetPosition({ WinApp::kClientWidth / 2.0f - 400.0f, WinApp::kClientHeight * 0.7f });
    loadingBarFgSprite->SetSize({ 0.0f, 40.0f }); // 進捗に応じて幅を変更

    // ゲームシーンの作成（初期化はここではまだ行わない）
    gameScene = new GameScene();

    // アニメーションの初期化
    loadProgress = 0.0f;
    displayProgress = 0.0f;

    // ロード処理開始
    loadState = LoadState::InProgress;
    currentLoadStep = 0;
}

void LoadingScene::ExecuteLoadStep() {
    // ロードステップを順番に実行
    switch (currentLoadStep) {
    case 0: // モデルリソースの読み込み Part 1
        ModelManager::GetInstance()->LoadModel("cannon");
        ModelManager::GetInstance()->LoadModel("cube");
        ModelManager::GetInstance()->LoadModel("door");
        break;

    case 1: // モデルリソースの読み込み Part 2
        ModelManager::GetInstance()->LoadModel("EnemyBullet");
        ModelManager::GetInstance()->LoadModel("EnemyGhost");
        ModelManager::GetInstance()->LoadModel("goal");
        break;

    case 2: // モデルリソースの読み込み Part 3
        ModelManager::GetInstance()->LoadModel("key");
        ModelManager::GetInstance()->LoadModel("player");
        ModelManager::GetInstance()->LoadModel("space");
        break;

    case 3: // モデルリソースの読み込み Part 4
        ModelManager::GetInstance()->LoadModel("Spring");
        ModelManager::GetInstance()->LoadModel("stage1");
        ModelManager::GetInstance()->LoadModel("stage2");
        ModelManager::GetInstance()->LoadModel("stage3");
        break;

    case 4: // ゲームシーンのカメラと基本オブジェクト初期化
        if (!gameScene->InitializeLoadingStep1()) {
            // 初期化ステップが失敗した場合は進行しない
            return;
        }
        break;

    case 5: // マップとプレイヤー初期化
        if (!gameScene->InitializeLoadingStep2()) {
            return;
        }
        break;

    case 6: // 敵と当たり判定の初期化
        if (!gameScene->InitializeLoadingStep3()) {
            return;
        }
        break;

    default:
        // 全ステップ完了
        loadState = LoadState::Completed;
        waitTimer = 0.5f; // 完了後少し待機
        return;
    }

    // 次のステップへ
    currentLoadStep++;

    // 進捗率の更新
    loadProgress = static_cast<float>(currentLoadStep) / static_cast<float>(totalLoadSteps);
}

void LoadingScene::Update() {
    // デルタタイム (1/60秒を想定)
    const float deltaTime = 1.0f / 60.0f;

    // スプライトの更新
    backgroundSprite->Update();
    loadingBarBgSprite->Update();

    // 表示用進捗率を実際の進捗率に向かって滑らかに更新
    if (displayProgress < loadProgress) {
        // イージングを適用して滑らかに追いつく
        displayProgress += (loadProgress - displayProgress) * progressSpeed * deltaTime;

        // 誤差が小さくなったら等しくする
        if (loadProgress - displayProgress < 0.001f) {
            displayProgress = loadProgress;
        }
    }

    // イージング関数の適用
    float easedProgress = Easing::EaseOutQuad(displayProgress);

    // ロードバーの幅を進捗に応じて更新（イージング適用済み）
    float barWidth = 800.0f * easedProgress;
    loadingBarFgSprite->SetSize({ barWidth, 40.0f });
    loadingBarFgSprite->Update();

    // ロード状態に応じた処理
    switch (loadState) {
    case LoadState::NotStarted:
        // 初期化されていない場合は何もしない
        break;

    case LoadState::InProgress:
        // ロードステップを実行
        ExecuteLoadStep();
        break;

    case LoadState::Completed:
        // ロード完了後、少し待ってからシーン切り替え
        waitTimer -= deltaTime;
        if (waitTimer <= 0.0f) {
            // GameSceneへ遷移
            sceneNo = Game;
            // GameManagerがシーン変更時にこのシーンを破棄するため、
            // gameSceneがFinalize()で二重解放されないようnullptrに設定
            gameScene = nullptr;
        }
        break;
    }
}

void LoadingScene::Draw() {
    // スプライト描画
    SpriteCommon::GetInstance()->Command();

    backgroundSprite->Draw();
    loadingBarBgSprite->Draw();
    loadingBarFgSprite->Draw();
}

void LoadingScene::Finalize() {
    // スプライトの解放
    delete backgroundSprite;
    backgroundSprite = nullptr;

    delete loadingBarBgSprite;
    loadingBarBgSprite = nullptr;

    delete loadingBarFgSprite;
    loadingBarFgSprite = nullptr;

    // ロード中のゲームシーンを解放
    if (gameScene) {
        delete gameScene;
        gameScene = nullptr;
    }
}