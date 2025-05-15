#include "LoadingScene.h"
#include <thread>
#include <chrono>
#include "math/Vector4.h"

void LoadingScene::Initialize() {
    // デバッグ出力
    OutputDebugStringA("LoadingScene::Initialize() が実行されました\n");
    
    // フェードマネージャーを初期化し、フェードイン開始
    FadeManager::GetInstance()->Initialize();
    FadeManager::GetInstance()->StartFadeIn(0.03f);
    
    // 基本的なUIのみを先にロード
    LoadEssentialResources();
    
    // 背景の読み込み
    background = new Sprite();
    background->Initialize("scene/background.png");
    background->SetPosition({ 0,0 });

    // ロードテキストの設定 - 画面中央に配置
    loadingText = new Sprite();
    loadingText->Initialize("loading/loading_text.png");
    loadingText->SetPosition({ WinApp::kClientWidth / 2.0f - 200.0f, WinApp::kClientHeight / 2.0f });
    loadingText->SetSize({ 400.0f, 100.0f }); // テキストを大きく表示
    
    // デバッグ出力
    OutputDebugStringA("Loading text initialized successfully.\n");
    char debugPosStr[100];
    sprintf_s(debugPosStr, "Initial position: X=%.2f, Y=%.2f\n", 
             WinApp::kClientWidth / 2.0f - 200.0f, WinApp::kClientHeight / 3.0f);
    OutputDebugStringA(debugPosStr);

    // ロードアイコン（回転するスピナー）の設定
    loadingIcon = new Sprite();
    loadingIcon->Initialize("loading/loading_icon.png");

    // アイコンサイズを設定
    float iconSize = 120.0f; 

    // 画面中央の座標計算
    posX = WinApp::kClientWidth / 2.0f;
    posY = WinApp::kClientHeight / 2.0f;

    // 位置とサイズを設定
    loadingIcon->SetPosition({ posX, posY });
    loadingIcon->SetSize({ iconSize, iconSize });
    loadingIcon->SetAnchorPoint({ 0.5f, 0.5f }); // 中心を軸に回転させるためのアンカーポイント設定

    // プログレスバーの初期化
    progressBar = new Sprite();
    progressBarBg = new Sprite();
    
    // プログレスバーのテクスチャはすでにロード済みのUI素材を使用
    progressBar->Initialize("ui/hp_bar_fill.png"); // 既存HPバーを流用
    progressBarBg->Initialize("ui/hp_bar_bg.png"); // 既存HPバー背景を流用
    
    // プログレスバーの位置とサイズを設定（画面中央の下）
    float barWidth = 400.0f;
    float barHeight = 20.0f;
    float barY = WinApp::kClientHeight / 2.0f + 100.0f; // アイコンの下に配置
    
    progressBarBg->SetPosition({ (WinApp::kClientWidth - barWidth) / 2.0f, barY });
    progressBarBg->SetSize({ barWidth, barHeight });
    
    progressBar->SetPosition({ (WinApp::kClientWidth - barWidth) / 2.0f, barY });
    progressBar->SetSize({ 0.0f, barHeight }); // 幅は0から開始（更新時に変更）

    // ロード状態のリセット
    loadingProgress = 0.0f;
    isLoadingComplete = false;
    loadingStage = 0;
    rotationAngle = 0.0f;
    
    // キラキラエフェクトの初期化
    for (int i = 0; i < kMaxStars; i++) {
        stars[i] = new Sprite();
        stars[i]->Initialize("circle.png"); // 円形テクスチャを使用
        
        // 画面全体にランダム配置
        float randX = static_cast<float>(rand() % WinApp::kClientWidth);
        float randY = static_cast<float>(rand() % WinApp::kClientHeight);
        stars[i]->SetPosition({ randX, randY });
        
        // 小さめのサイズで初期化
        starScales[i] = 0.1f + (static_cast<float>(rand()) / RAND_MAX) * 0.2f;
        stars[i]->SetSize({ 20.0f * starScales[i], 20.0f * starScales[i] });
        stars[i]->SetAnchorPoint({ 0.5f, 0.5f }); // 中心を軸に回転
        
        // 回転角度もランダム
        starRotations[i] = static_cast<float>(rand() % 360);
        stars[i]->SetRotate(starRotations[i]);
        
        // 透明度もランダム・白から薄い水色に
        starAlphas[i] = 0.3f + (static_cast<float>(rand()) / RAND_MAX) * 0.7f;
        stars[i]->SetColor({ 0.8f, 0.9f, 1.0f, starAlphas[i] });
    }
    
    // 非同期ローディングの開始
    isAsyncLoadingStarted = false;
}

void LoadingScene::LoadEssentialResources() {
    // ローディング画面用の最小限のアセットを確実にロード
    if (!TextureManager::GetInstance()->CheckTextureExist("resource/Sprite/loading/loading_text.png")) {
        TextureManager::GetInstance()->LoadTexture("resource/Sprite/loading/loading_text.png");
    }
    if (!TextureManager::GetInstance()->CheckTextureExist("resource/Sprite/loading/loading_icon.png")) {
        TextureManager::GetInstance()->LoadTexture("resource/Sprite/loading/loading_icon.png");
    }
    if (!TextureManager::GetInstance()->CheckTextureExist("resource/Sprite/scene/background.png")) {
        TextureManager::GetInstance()->LoadTexture("resource/Sprite/scene/background.png");
    }
    if (!TextureManager::GetInstance()->CheckTextureExist("resource/Sprite/ui/hp_bar_bg.png")) {
        TextureManager::GetInstance()->LoadTexture("resource/Sprite/ui/hp_bar_bg.png");
    }
    if (!TextureManager::GetInstance()->CheckTextureExist("resource/Sprite/ui/hp_bar_fill.png")) {
        TextureManager::GetInstance()->LoadTexture("resource/Sprite/ui/hp_bar_fill.png");
    }
    if (!TextureManager::GetInstance()->CheckTextureExist("resource/Sprite/circle.png")) {
        TextureManager::GetInstance()->LoadTexture("resource/Sprite/circle.png");
    }
    
    // フェードマネージャーはcircle.pngを使用するので別途ロードは不要
}

void LoadingScene::StartAsyncLoading() {
    // 各ステージのリソースをロードするための非同期スレッドを開始
    loadingThread = std::thread([this]() {
        try {
            // ステージ1のリソースロード
            LoadStage1Resources();
            {
                std::lock_guard<std::mutex> lock(loadingMutex);
                loadingStage = 1;
                stageProgress[0] = 1.0f;
            }
            
            // ステージ2のリソースロード
            LoadStage2Resources();
            {
                std::lock_guard<std::mutex> lock(loadingMutex);
                loadingStage = 2;
                stageProgress[1] = 1.0f;
            }
            
            // ステージ3のリソースロード
            LoadStage3Resources();
            {
                std::lock_guard<std::mutex> lock(loadingMutex);
                loadingStage = 3;
                stageProgress[2] = 1.0f;
            }
            
            // ステージ4のリソースロード
            LoadStage4Resources();
            {
                std::lock_guard<std::mutex> lock(loadingMutex);
                loadingStage = 4;
                stageProgress[3] = 1.0f;
                isLoadingComplete = true;
            }
            
            OutputDebugStringA("非同期ローディング完了\n");
        }
        catch (std::exception& e) {
            OutputDebugStringA(("ローディングエラー: " + std::string(e.what()) + "\n").c_str());
        }
    });
    
    // スレッドをデタッチ
    loadingThread.detach();
}

void LoadingScene::LoadStage1Resources() {
    OutputDebugStringA("ロードステージ1: 基本モデルとUIのロード開始\n");
    
    // モデルのロード（優先度の高いものから）
    std::vector<std::string> modelQueue = {
        "cube",
        "door",
        "tile",
        "EnemyBullet",
        "EnemyGhost",
        "GhostRespown",
        "cannon"
    };
    
    // 各モデルを順番にロード
    for (size_t i = 0; i < modelQueue.size(); ++i) {
        ModelManager::GetInstance()->LoadModel(modelQueue[i]);
        
        // 進捗状況の更新
        {
            std::lock_guard<std::mutex> lock(loadingMutex);
            stageProgress[0] = static_cast<float>(i + 1) / static_cast<float>(modelQueue.size());
        }
        
        // CPUの負荷を下げるために短い休止
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    // タイトル・ゲームオーバー関連テクスチャをロード
    std::vector<std::string> textureQueue = {
        "resource/Sprite/ui/press_Button.png",
        "resource/Sprite/scene/TitleName.png",
        "resource/Sprite/scene/GameOverName.png",
        "resource/Sprite/scene/background.png"
    };
    
    // 各テクスチャを順番にロード
    for (size_t i = 0; i < textureQueue.size(); ++i) {
        TextureManager::GetInstance()->LoadTexture(textureQueue[i]);
        
        // 進捗状況の更新（モデルのローディングが50%、テクスチャが50%と仮定）
        {
            std::lock_guard<std::mutex> lock(loadingMutex);
            stageProgress[0] = 0.5f + (static_cast<float>(i + 1) / static_cast<float>(textureQueue.size())) * 0.5f;
        }
        
        // CPUの負荷を下げるために短い休止
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void LoadingScene::LoadStage2Resources() {
    OutputDebugStringA("ロードステージ2: 追加モデルとUIロード開始\n");
    
    // モデルのロード
    std::vector<std::string> modelQueue = {
        "goal",
        "key",
        "player",
        "space",
        "Spring"
    };
    
    // 各モデルを順番にロード
    for (size_t i = 0; i < modelQueue.size(); ++i) {
        ModelManager::GetInstance()->LoadModel(modelQueue[i]);
        
        // 進捗状況の更新
        {
            std::lock_guard<std::mutex> lock(loadingMutex);
            stageProgress[1] = static_cast<float>(i + 1) / static_cast<float>(modelQueue.size()) * 0.5f;
        }
        
        // CPUの負荷を下げるために短い休止
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    // ステージセレクト関連テクスチャをロード
    std::vector<std::string> textureQueue = {
        "resource/Sprite/scene/StageSelectName.png",
        "resource/Sprite/scene/background.png",
        "resource/Sprite/scene/stageNumber.png",
        "resource/Sprite/ui/hp_bar_bg.png",
        "resource/Sprite/ui/hp_bar_fill.png",
        "resource/Sprite/ui/hp_icon.png",
        "resource/Sprite/ui/controller_guide.png"
    };
    
    // 各テクスチャを順番にロード
    for (size_t i = 0; i < textureQueue.size(); ++i) {
        if (!TextureManager::GetInstance()->CheckTextureExist(textureQueue[i])) {
            TextureManager::GetInstance()->LoadTexture(textureQueue[i]);
        }
        
        // 進捗状況の更新
        {
            std::lock_guard<std::mutex> lock(loadingMutex);
            stageProgress[1] = 0.5f + (static_cast<float>(i + 1) / static_cast<float>(textureQueue.size())) * 0.5f;
        }
        
        // CPUの負荷を下げるために短い休止
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void LoadingScene::LoadStage3Resources() {
    OutputDebugStringA("ロードステージ3: ステージモデルとキャラクターロード開始\n");
    
    // ステージモデルのロード
    std::vector<std::string> modelQueue = {
        "stage0",
        "stage1",
        "stage2",
        "stage3",
        "stage4",
        "stage5",
        "stage6",
        "stage7",
        "BlueGhost"
    };
    
    // 各モデルを順番にロード
    for (size_t i = 0; i < modelQueue.size(); ++i) {
        ModelManager::GetInstance()->LoadModel(modelQueue[i]);
        
        // 進捗状況の更新
        {
            std::lock_guard<std::mutex> lock(loadingMutex);
            stageProgress[2] = static_cast<float>(i + 1) / static_cast<float>(modelQueue.size()) * 0.6f;
        }
        
        // CPUの負荷を下げるために少し長めの休止
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // キャラクター用テクスチャロード
    std::vector<std::string> textureQueue = {
        "resource/Sprite/BlueGhost.png",
        "resource/Sprite/RedGhost.png",
        "resource/Sprite/GreenGhost.png",
        "resource/Sprite/player.png",
        "resource/Sprite/Bullet.png",
        "resource/Sprite/bane1.png"
    };
    
    // 各テクスチャを順番にロード
    for (size_t i = 0; i < textureQueue.size(); ++i) {
        if (!TextureManager::GetInstance()->CheckTextureExist(textureQueue[i])) {
            TextureManager::GetInstance()->LoadTexture(textureQueue[i]);
        }
        
        // 進捗状況の更新
        {
            std::lock_guard<std::mutex> lock(loadingMutex);
            stageProgress[2] = 0.6f + (static_cast<float>(i + 1) / static_cast<float>(textureQueue.size())) * 0.4f;
        }
        
        // CPUの負荷を下げるために短い休止
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void LoadingScene::LoadStage4Resources() {
    OutputDebugStringA("ロードステージ4: 最終ロード処理開始\n");
    
    // 最終ステージのテクスチャロード
    std::vector<std::string> textureQueue = {
        "resource/Sprite/key.png",
        "resource/Sprite/get_key.png",
        "resource/Sprite/Goal.png",
        "resource/Sprite/Cube.png",
        "resource/Sprite/ui/tutorial01.png",
        "resource/Sprite/ui/tutorial02.png",
        "resource/Sprite/ui/tutorial03.png",
        "resource/Sprite/ui/tutorial04.png",
        "resource/Sprite/ui/tutorial05.png",
        "resource/Sprite/ui/tutorial06.png"
    };
    
    // 各テクスチャを順番にロード
    for (size_t i = 0; i < textureQueue.size(); ++i) {
        if (!TextureManager::GetInstance()->CheckTextureExist(textureQueue[i])) {
            TextureManager::GetInstance()->LoadTexture(textureQueue[i]);
        }
        
        // 進捗状況の更新
        {
            std::lock_guard<std::mutex> lock(loadingMutex);
            stageProgress[3] = static_cast<float>(i + 1) / static_cast<float>(textureQueue.size());
        }
        
        // CPUの負荷を下げるために短い休止
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    // 動くテクスチャを確認するためのチェック
    TextureManager::GetInstance()->CheckAllTextureLoaded();
    
    // ModelManagerの照合チェック
    ModelManager::GetInstance()->CheckAllModelsLoaded();
    
    OutputDebugStringA("ロード完了: タイトル画面へ遷移準備完了\n");
}

void LoadingScene::Update() {
    // 非同期ローディングが開始されていない場合は開始
    if (!isAsyncLoadingStarted) {
        StartAsyncLoading();
        isAsyncLoadingStarted = true;
    }
    
    // UIの更新
    background->Update();
    progressBarBg->Update();
    
    // キラキラエフェクトの更新
    for (int i = 0; i < kMaxStars; i++) {
        // 星をゆっくり回転
        starRotations[i] += 0.02f * (i % 3 + 1);
        stars[i]->SetRotate(starRotations[i]);
        
        // 星の透明度を周期的に変化させる
        float alpha = 0.3f + sin(rotationAngle * 0.02f + i * 0.5f) * 0.3f + 0.4f;
        stars[i]->SetColor({ 0.8f, 0.9f, 1.0f, alpha });
        
        // 星の大きさも周期的に変化
        float scale = starScales[i] * (0.8f + sin(rotationAngle * 0.03f + i * 0.7f) * 0.2f);
        stars[i]->SetSize({ 20.0f * scale, 20.0f * scale });
        
        stars[i]->Update();
    }
    
    // ロードアイコンを回転 - かわいらしい動きに
    rotationAngle += 0.03f; // ゆっくり回転
    
    // ロード中のアイコンをゆらゆら動かす（かわいらしさ向上）
    float wobbleAmount = 8.0f;
    float wobbleSpeed = 0.04f;
    float offsetY = sin(rotationAngle * wobbleSpeed) * wobbleAmount;
    
    // テキストアニメーション（簡素化してシンプルなサインアニメーションに）
    static float textAnimTime = 0.0f;
    textAnimTime += 0.05f; // 2秒で一周するように調整
    
    // サイン値を計算（単純だが信頼性が高い）
    float sinValue = sinf(textAnimTime);
    
    // 上下動作の振幅を設定（12ピクセル）
    float amplitude = 12.0f;
    float newTextY = WinApp::kClientHeight / 3.0f + sinValue * amplitude;
    
    // ローディングテキストの位置を更新
    loadingText->SetPosition({ WinApp::kClientWidth / 2.0f - 200.0f, newTextY });
    loadingText->Update(); // 確実にアップデート
    
    // デバッグ出力
    char animDebugStr[100];
    sprintf_s(animDebugStr, "Text Animation: time=%.2f, sin=%.2f, newY=%.2f\n", 
             textAnimTime, sinValue, newTextY);
    OutputDebugStringA(animDebugStr);
    
    // ロードアイコンの位置を更新
    loadingIcon->SetPosition({ posX, posY + offsetY });
    loadingIcon->SetRotate(rotationAngle);
    loadingIcon->Update();

    // 全体の進行状況を計算（4つのステージの平均）
    float totalProgress = 0.0f;
    
    {
        std::lock_guard<std::mutex> lock(loadingMutex);
        for (int i = 0; i < totalStages; ++i) {
            totalProgress += stageProgress[i] / static_cast<float>(totalStages);
        }
    }
    
    // プログレスバーの幅を更新（イージングを適用してスムーズに）
    static float smoothProgress = 0.0f;
    // イージング関数（Smoothstep）でtotalProgressに追いつくように滑らかに変化
    float easeSpeed = 0.05f;
    smoothProgress += (totalProgress - smoothProgress) * easeSpeed;
    
    float barMaxWidth = 400.0f;
    progressBar->SetSize({ barMaxWidth * smoothProgress, progressBar->GetSize().y });
    
    // 進行度に応じてプログレスバーの色を変える（聖色と可愛さが増す）
    // 青からピンク、そして白へのグラデーション
    Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f }; // デフォルトは白
    
    if (smoothProgress < 0.5f) {
        // 0-50%: 青からピンクへ
        float r = 0.5f + 0.5f * (smoothProgress * 2.0f); // 0.5から1.0へ
        float g = 0.5f + 0.12f * (smoothProgress * 2.0f); // 0.5から0.62へ
        float b = 1.0f - 0.25f * (smoothProgress * 2.0f); // 1.0から0.75へ
        color = { r, g, b, 1.0f };
    } else {
        // 50-100%: ピンクから白へ
        float r = 1.0f; // 常に1.0
        float g = 0.62f + 0.38f * ((smoothProgress - 0.5f) * 2.0f); // 0.62から1.0へ
        float b = 0.75f + 0.25f * ((smoothProgress - 0.5f) * 2.0f); // 0.75から1.0へ
        color = { r, g, b, 1.0f };
    }
    
    progressBar->SetColor(color);
    progressBar->Update();
    


    // ロード完了したらシーン移行
    bool completed = false;
    {
        std::lock_guard<std::mutex> lock(loadingMutex);
        completed = isLoadingComplete;
    }
    
    if (completed) {
        // ロード完了時にフェードアウト開始
        static bool startedFadeOut = false;
        static int waitCounter = 0;
        
        if (!startedFadeOut) {
            FadeManager::GetInstance()->StartFadeOut(0.03f);
            startedFadeOut = true;
            OutputDebugStringA("ロード完了: フェードアウト開始\n");
        }
        
        // フェードアウト完了後にタイトル画面へ遷移
        if (FadeManager::GetInstance()->IsFadeComplete()) {
            // 少し待機してからタイトル画面へ
            if (waitCounter++ > 15) { // 15フレーム（約0.25秒）待機
                OutputDebugStringA("フェードアウト完了: タイトル画面へ遷移します\n");
                sceneNo = Title;
            }
        }
    }
}

void LoadingScene::Draw() {
    SpriteCommon::GetInstance()->Command();

    background->Draw();
    
    // 星を背景の上に描画
    for (int i = 0; i < kMaxStars; i++) {
        stars[i]->Draw();
    }
    
    progressBarBg->Draw();
    progressBar->Draw();
    
    // デバッグ出力（ローディングテキストが描画されていることを確認）
    OutputDebugStringA("\n=== Drawing Loading Text ===\n");
    loadingText->Draw();
    OutputDebugStringA("=== Loading Text Drawn ===\n");
    
    loadingIcon->Draw();
}

void LoadingScene::Finalize() {
    // スレッド関連のクリーンアップ
    OutputDebugStringA("LoadingScene::Finalize() - リソース解放開始\n");
    
    if (isAsyncLoadingStarted) {
        OutputDebugStringA("LoadingScene::Finalize() - 非同期ロードが開始されているので完全に終了するよう待機\n");
        // ローディングが完了するまで短時間待機
        int waitCount = 0;
        while (!isLoadingComplete && waitCount < 100) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            waitCount++;
            
            // 安全のためのタイムアウト
            if (waitCount >= 100) {
                OutputDebugStringA("LoadingScene::Finalize() - タイムアウトのため強制終了\n");
                break;
            }
        }
    }
    
    // オブジェクトの解放前に安全チェック
    OutputDebugStringA("LoadingScene::Finalize() - オブジェクトの解放開始\n");
    
    // 各オブジェクトのnullチェックをしてから解放
    if (background) {
        delete background;
        background = nullptr;
    }
    
    if (loadingText) {
        delete loadingText;
        loadingText = nullptr;
    }
    
    if (loadingIcon) {
        delete loadingIcon;
        loadingIcon = nullptr;
    }
    
    if (progressBar) {
        delete progressBar;
        progressBar = nullptr;
    }
    
    if (progressBarBg) {
        delete progressBarBg;
        progressBarBg = nullptr;
    }
    
    // 星を解放
    for (int i = 0; i < kMaxStars; i++) {
        if (stars[i]) {
            delete stars[i];
            stars[i] = nullptr;
        }
    }
    
    OutputDebugStringA("LoadingScene::Finalize() - 全てのリソース解放完了\n");
}