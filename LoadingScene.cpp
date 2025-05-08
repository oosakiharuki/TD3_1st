#include "LoadingScene.h"
// WinApp.hとTextureManager.hは既に読み込まれている

void LoadingScene::Initialize() {
    // デバッグ出力
    OutputDebugStringA("LoadingScene::Initialize() が実行されました\n");
    
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

    // ローディング画面用のアセットを確実にロード
    if (!TextureManager::GetInstance()->CheckTextureExist("resource/Sprite/loading/loading_text.png")) {
        TextureManager::GetInstance()->LoadTexture("resource/Sprite/loading/loading_text.png");
    }
    if (!TextureManager::GetInstance()->CheckTextureExist("resource/Sprite/loading/loading_icon.png")) {
        TextureManager::GetInstance()->LoadTexture("resource/Sprite/loading/loading_icon.png");
    }
    if (!TextureManager::GetInstance()->CheckTextureExist("resource/Sprite/title_background.png")) {
        TextureManager::GetInstance()->LoadTexture("resource/Sprite/title_background.png");
    }
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
            // ロードステージ1開始
            OutputDebugStringA("ロードステージ1: 基本モデルとUIのロード開始\n");
            
            // ステージ1: 基本モデルのロード
            ModelManager::GetInstance()->LoadModel("cannon");
            ModelManager::GetInstance()->LoadModel("cube");
            ModelManager::GetInstance()->LoadModel("door");
            ModelManager::GetInstance()->LoadModel("tile");
            ModelManager::GetInstance()->LoadModel("EnemyBullet");
            ModelManager::GetInstance()->LoadModel("EnemyGhost");
            ModelManager::GetInstance()->LoadModel("GhostRespown");

            // タイトル・ゲームオーバー関連テクスチャをロード
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/TitleName.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/ui/press_Botton.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/GameOverName.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/GameOver_background.png");

            loadingProgress = 0.0f;
            loadingStage = 1;
        }
        else if (loadingProgress >= 1.0f && loadingStage == 1) {
            // ロードステージ2開始
            OutputDebugStringA("ロードステージ2: 追加モデルとUIロード開始\n");

            // ステージ2: 追加モデルのロード
            ModelManager::GetInstance()->LoadModel("goal");
            ModelManager::GetInstance()->LoadModel("key");
            ModelManager::GetInstance()->LoadModel("player");
            ModelManager::GetInstance()->LoadModel("space");
            ModelManager::GetInstance()->LoadModel("Spring");

            // ステージセレクト関連テクスチャをロード
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/StageSelectName.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/StageSelect_background.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/stageNumber.png");

            // ゲーム中のUI要素をロード
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/ui/hp_bar_bg.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/ui/hp_bar_fill.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/ui/hp_icon.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/ui/controller_guide.png");

            loadingProgress = 0.0f;
            loadingStage = 2;
        }
        else if (loadingProgress >= 1.0f && loadingStage == 2) {
            // ロードステージ3開始
            OutputDebugStringA("ロードステージ3: ステージモデルとキャラクターロード開始\n");

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

            // キャラクター用テクスチャロード
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/BlueGhost.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/RedGhost.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/GreenGhost.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/player.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/Bullet.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/bane1.png");

            loadingProgress = 0.0f;
            loadingStage = 3;
        }
        else if (loadingProgress >= 1.0f && loadingStage == 3) {
            // ロードステージ4開始 - 最終ステージ
            OutputDebugStringA("ロードステージ4: 最終ロード処理開始\n");

            // ステージ4: おけるものをすべてロード
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/key.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/get_key.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/Goal.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/Cube.png");

            // 全てのチュートリアル画像をロード
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/ui/tutorial01.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/ui/tutorial02.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/ui/tutorial03.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/ui/tutorial04.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/ui/tutorial05.png");
            TextureManager::GetInstance()->LoadTexture("resource/Sprite/ui/tutorial06.png");
            
            // 動くテクスチャを確認するためのチェック
            TextureManager::GetInstance()->CheckAllTextureLoaded();
            // ModelManagerの照合チェック
            ModelManager::GetInstance()->CheckAllModelsLoaded();

            // すべてのロードが完了
            isLoadingComplete = true;
            loadingProgress = 1.0f;

            // タイトル画面へ遷移
            OutputDebugStringA("ロード完了: タイトル画面へ遷移します\n");
            sceneNo = Title;
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