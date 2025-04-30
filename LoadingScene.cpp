#include "LoadingScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "WinApp.h" // ウィンドウサイズ取得用

void LoadingScene::Initialize() {
    // 背景の読み込み
    background = new Sprite();
    background->Initialize("title_background.png");
    background->SetPosition({ 0,0 });

    // ロードテキストの設定
    loadingText = new Sprite();
    loadingText->Initialize("loading/loading_text.png");
    loadingText->SetPosition({ 0, 20 });

    // ロードアイコン（回転するスピナー）の設定 - 右下に配置
    loadingIcon = new Sprite();
    loadingIcon->Initialize("loading/loading_icon.png");

    // アイコンサイズを大きく設定
    float iconSize = 150.0f; // より大きなサイズに変更
    float padding = 30.0f;  // 画面端からの余白

    // 右下の座標計算 - WinAppのクライアントサイズを使用
    float posX = (WinApp::kClientWidth / 2.0f) - padding;
    float posY = (WinApp::kClientHeight / 2.0f) - padding;

    // 位置とサイズを設定
    loadingIcon->SetPosition({ posX, posY });
    loadingIcon->SetSize({ iconSize, iconSize });
    loadingIcon->SetAnchorPoint({ 0.5f, 0.5f }); // 中心を軸に回転させるためのアンカーポイント設定

    // ロード状態のリセット
    loadingProgress = 0.0f;
    isLoadingComplete = false;
    loadingStage = 0;
    rotationAngle = 0.0f;

    // 一部のアセットを事前にロードして実際のロードを高速化
    TextureManager::GetInstance()->LoadTexture("resource/Sprite/loading/loading_text.png");
    TextureManager::GetInstance()->LoadTexture("resource/Sprite/loading/loading_icon.png");
}

void LoadingScene::Update() {
    background->Update();
    loadingText->Update();

    // ロードアイコンを回転 - 回転速度を調整
    rotationAngle += 0.05f;
    loadingIcon->SetRotate(rotationAngle);
    loadingIcon->Update();

    // ロード処理のシミュレーション
    if (!isLoadingComplete) {
        loadingProgress += loadingSpeed;

        // ロードステージ - 各ステージで異なるアセットをロード
        if (loadingProgress >= 1.0f && loadingStage == 0) {
            // ステージ1: 基本モデルのロード
            ModelManager::GetInstance()->LoadModel("cannon");
            ModelManager::GetInstance()->LoadModel("cube");
            ModelManager::GetInstance()->LoadModel("door");
            ModelManager::GetInstance()->LoadModel("tile");
            ModelManager::GetInstance()->LoadModel("EnemyBullet");
            ModelManager::GetInstance()->LoadModel("EnemyGhost");

            loadingProgress = 0.0f;
            loadingStage = 1;
        }
        else if (loadingProgress >= 1.0f && loadingStage == 1) {
            // ステージ2: 追加モデルのロード
            ModelManager::GetInstance()->LoadModel("goal");
            ModelManager::GetInstance()->LoadModel("key");
            ModelManager::GetInstance()->LoadModel("player");
            ModelManager::GetInstance()->LoadModel("space");
            ModelManager::GetInstance()->LoadModel("Spring");

            ParticleManager::GetInstance()->CreateParticleGroup("01", "resource/Sprite/circle.png");
            ParticleManager::GetInstance()->CreateParticleGroup("02", "resource/Sprite/circle.png");

            loadingProgress = 0.0f;
            loadingStage = 2;
        }
        else if (loadingProgress >= 1.0f && loadingStage == 2) {
            // ステージ3: ステージモデルのロード
            ModelManager::GetInstance()->LoadModel("stage0");
            ModelManager::GetInstance()->LoadModel("stage1");
            ModelManager::GetInstance()->LoadModel("stage2");
            ModelManager::GetInstance()->LoadModel("stage3");
            ModelManager::GetInstance()->LoadModel("stage4");
            ModelManager::GetInstance()->LoadModel("stage5");
            ModelManager::GetInstance()->LoadModel("stage6");
            ModelManager::GetInstance()->LoadModel("stage7");
            ModelManager::GetInstance()->LoadModel("BlueGhost");

            loadingProgress = 0.0f;
            loadingStage = 3;
        }
        else if (loadingProgress >= 1.0f && loadingStage == 3) {
            // すべてのロードが完了
            isLoadingComplete = true;
            loadingProgress = 1.0f;

            // ゲームシーンへ遷移
            sceneNo = Select;
        }
    }
}

void LoadingScene::Draw() {
    SpriteCommon::GetInstance()->Command();

    background->Draw();
    loadingText->Draw();
    loadingIcon->Draw();

    // 必要に応じてプログレスバーを追加することも可能
}

void LoadingScene::Finalize() {
    delete background;
    delete loadingText;
    delete loadingIcon;
}