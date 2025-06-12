#include "LoadingScene.h"
#include "ResourceManager.h"
#include "PerformanceMonitor.h"
#include "math/Vector4.h"
#include <algorithm>

void LoadingScene::Initialize() {
    OutputDebugStringA("LoadingScene::Initialize() 開始\n");
    
    // パフォーマンスモニター初期化
    PerformanceMonitor::GetInstance()->BeginSection("LoadingSceneInit");
    
    // ResourceManager初期化
    ResourceManager::GetInstance()->Initialize();
    
    // 音声の初期化と再生
    loadingSound = Audio::GetInstance()->LoadWave("sound/load.wav");
    Audio::GetInstance()->SoundPlayWave(loadingSound, 0.5f, true);
    isSoundStopped = false;
    
    // フェードマネージャー初期化
    FadeManager::GetInstance()->Initialize();
    FadeManager::GetInstance()->StartFadeIn(0.05f);  // より高速なフェードイン
    
    // 必須リソースの即座ロード（ローディング画面表示用）
    ResourceManager::GetInstance()->LoadResourceImmediate("resource/Sprite/loading/loading_text.png", ResourceType::Texture);
    ResourceManager::GetInstance()->LoadResourceImmediate("resource/Sprite/loading/loading_icon.png", ResourceType::Texture);
    ResourceManager::GetInstance()->LoadResourceImmediate("resource/Sprite/scene/BackGround.png", ResourceType::Texture);
    ResourceManager::GetInstance()->LoadResourceImmediate("resource/Sprite/ui/hp_bar_bg.png", ResourceType::Texture);
    ResourceManager::GetInstance()->LoadResourceImmediate("resource/Sprite/ui/hp_bar_fill.png", ResourceType::Texture);
    ResourceManager::GetInstance()->LoadResourceImmediate("resource/Sprite/circle.png", ResourceType::Texture);
    
    // UI要素の初期化
    InitializeUI();
    
    // リソースキューの準備
    PrepareResourceQueue();
    
    // バッチローディング開始
    ResourceManager::GetInstance()->StartBatchLoading();
    
    PerformanceMonitor::GetInstance()->EndSection("LoadingSceneInit");
    OutputDebugStringA("LoadingScene::Initialize() 完了\n");
}

void LoadingScene::InitializeUI() {
    // 背景
    background = new Sprite();
    background->Initialize("scene/BackGround.png");
    background->SetPosition({ 0, 0 });
    
    // ローディングテキスト
    loadingText = new Sprite();
    loadingText->Initialize("loading/loading_text.png");
    float textX = (WinApp::kClientWidth - 400.0f) / 2.0f;
    float textY = WinApp::kClientHeight / 2.0f - 100.0f;
    loadingText->SetPosition({ textX, textY });
    loadingText->SetSize({ 400.0f, 80.0f });
    
    // ローディングアイコン
    loadingIcon = new Sprite();
    loadingIcon->Initialize("loading/loading_icon.png");
    posX = WinApp::kClientWidth / 2.0f;
    posY = WinApp::kClientHeight / 2.0f;
    loadingIcon->SetPosition({ posX, posY });
    loadingIcon->SetSize({ 100.0f, 100.0f });
    loadingIcon->SetAnchorPoint({ 0.5f, 0.5f });
    
    // プログレスバー
    progressBar = new Sprite();
    progressBarBg = new Sprite();
    progressBar->Initialize("ui/hp_bar_fill.png");
    progressBarBg->Initialize("ui/hp_bar_bg.png");
    
    float barWidth = 500.0f;
    float barHeight = 30.0f;
    float barX = (WinApp::kClientWidth - barWidth) / 2.0f;
    float barY = WinApp::kClientHeight / 2.0f + 120.0f;
    
    progressBarBg->SetPosition({ barX, barY });
    progressBarBg->SetSize({ barWidth, barHeight });
    progressBar->SetPosition({ barX, barY });
    progressBar->SetSize({ 0.0f, barHeight });
    
    // キラキラエフェクト
    for (int i = 0; i < kMaxStars; i++) {
        stars[i] = new Sprite();
        stars[i]->Initialize("circle.png");
        
        float randX = static_cast<float>(rand() % WinApp::kClientWidth);
        float randY = static_cast<float>(rand() % WinApp::kClientHeight);
        stars[i]->SetPosition({ randX, randY });
        
        starScales[i] = 0.05f + (static_cast<float>(rand()) / RAND_MAX) * 0.15f;
        float size = 30.0f * starScales[i];
        stars[i]->SetSize({ size, size });
        stars[i]->SetAnchorPoint({ 0.5f, 0.5f });
        
        starRotations[i] = static_cast<float>(rand() % 360);
        starAlphas[i] = 0.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.5f;
        stars[i]->SetColor({ 0.9f, 0.95f, 1.0f, starAlphas[i] });
    }
}

void LoadingScene::PrepareResourceQueue() {
    // 優先度順にリソースをキューに追加
    
    // Essential: UI関連（タイトル画面で必要）
    std::vector<std::string> essentialTextures = {
        "resource/Sprite/scene/TitleName.png",
        "resource/Sprite/ui/press_Button.png",
        "resource/Sprite/ui/press_start.png"
    };
    ResourceManager::GetInstance()->QueueResources(essentialTextures, ResourceType::Texture, ResourcePriority::Essential);
    
    // High: ゲームプレイに必要な基本リソース
    std::vector<std::string> highPriorityModels = {
        "player", "playerHead", "playerFoot",
        "cube", "tile", "door", "key", "goal"
    };
    ResourceManager::GetInstance()->QueueResources(highPriorityModels, ResourceType::Model, ResourcePriority::High);
    
    std::vector<std::string> highPriorityTextures = {
        "resource/Sprite/player.png",
        "resource/Sprite/key.png",
        "resource/Sprite/key_Icon.png",
        "resource/Sprite/noKey_Icon.png",
        "resource/Sprite/ui/hp_icon.png"
    };
    ResourceManager::GetInstance()->QueueResources(highPriorityTextures, ResourceType::Texture, ResourcePriority::High);
    
    // Medium: 敵、ステージ関連
    std::vector<std::string> mediumPriorityModels = {
        "cannon", "Spring", "EnemyBullet", "EnemyGhost",
        "BlueGhost", "GhostRespown"
    };
    ResourceManager::GetInstance()->QueueResources(mediumPriorityModels, ResourceType::Model, ResourcePriority::Medium);
    
    std::vector<std::string> mediumPriorityTextures = {
        "resource/Sprite/Bullet.png",
        "resource/Sprite/cannon.png",
        "resource/Sprite/bane1.png",
        "resource/Sprite/EnemyGhost.png",
        "resource/Sprite/BlueGhost.png",
        "resource/Sprite/RedGhost.png",
        "resource/Sprite/GreenGhost.png"
    };
    ResourceManager::GetInstance()->QueueResources(mediumPriorityTextures, ResourceType::Texture, ResourcePriority::Medium);
    
    // Low: ステージモデル、装飾
    std::vector<std::string> lowPriorityModels = {
        "stage0", "stage1", "stage2", "stage3", "stage4", "stage5","stage6",
        "space", "sky"
    };
    ResourceManager::GetInstance()->QueueResources(lowPriorityModels, ResourceType::Model, ResourcePriority::Low);
    
    std::vector<std::string> lowPriorityTextures = {
        "resource/Sprite/ui/tutorial01.png",
        "resource/Sprite/ui/tutorial02.png",
        "resource/Sprite/ui/tutorial03.png",
        "resource/Sprite/ui/tutorial04.png",
        "resource/Sprite/ui/tutorial05.png",
        "resource/Sprite/ui/tutorial06.png",
        "resource/Sprite/scene/GameOverName.png",
        "resource/Sprite/scene/GameOver_Continue.png",
        "resource/Sprite/scene/GameOver_Title.png",
        "resource/Sprite/scene/GameClearName.png",
        "resource/Sprite/scene/GameClear_NextStage.png",
        "resource/Sprite/scene/GameClear_Select.png",
        "resource/Sprite/scene/StageSelectName.png",
        "resource/Sprite/scene/stageNumber.png"
    };
    ResourceManager::GetInstance()->QueueResources(lowPriorityTextures, ResourceType::Texture, ResourcePriority::Low);
}

void LoadingScene::Update() {
    PerformanceMonitor::GetInstance()->BeginSection("LoadingSceneUpdate");
    
    // UIアニメーション更新
    UpdateAnimations();
    
    // プログレスバー更新
    float progress = ResourceManager::GetInstance()->GetLoadingProgress();
    UpdateProgressBar(progress);
    
    // ロード完了チェック
    if (ResourceManager::GetInstance()->IsLoadingComplete()) {
        HandleLoadingComplete();
    }
    
    // デバッグ情報出力（10フレームごと）
    static int debugCounter = 0;
    if (++debugCounter % 10 == 0) {
        ResourceManager::GetInstance()->PrintDebugInfo();
    }
    
    PerformanceMonitor::GetInstance()->EndSection("LoadingSceneUpdate");
}

void LoadingScene::UpdateAnimations() {
    // 背景更新
    background->Update();
    progressBarBg->Update();
    
    // アイコン回転（2秒で1回転）
    // 2秒で2πラジアン = 1秒でπラジアン = 60FPSでπ/60ラジアン/フレーム
    const float PI = 3.14159265359f;
    const float rotationSpeed = PI / 60.0f;  // 2秒で1回転
    rotationAngle += rotationSpeed;
    float wobble = sin(rotationAngle * 0.02f) * 5.0f;
    loadingIcon->SetPosition({ posX, posY + wobble });
    loadingIcon->SetRotate(rotationAngle);
    loadingIcon->Update();
    
    // テキストアニメーション
    static float textAnimTime = 0.0f;
    textAnimTime += 0.03f;
    float textWobble = sin(textAnimTime) * 8.0f;
    float textX = (WinApp::kClientWidth - 400.0f) / 2.0f;
    float textY = WinApp::kClientHeight / 2.0f - 100.0f + textWobble;
    loadingText->SetPosition({ textX, textY });
    loadingText->Update();
    
    // キラキラエフェクト更新
    for (int i = 0; i < kMaxStars; i++) {
        // 回転
        starRotations[i] += 1.0f * (i % 3 + 1);
        stars[i]->SetRotate(starRotations[i]);
        
        // 透明度アニメーション
        float phase = rotationAngle * 0.01f + i * 0.8f;
        float alpha = 0.2f + (sin(phase) * 0.5f + 0.5f) * 0.5f;
        stars[i]->SetColor({ 0.9f, 0.95f, 1.0f, alpha });
        
        // サイズアニメーション
        float scale = starScales[i] * (0.8f + sin(phase * 1.5f) * 0.2f);
        float size = 30.0f * scale;
        stars[i]->SetSize({ size, size });
        
        // ゆっくり移動
        Vector2 pos = stars[i]->GetPosition();
        pos.y -= 0.5f;
        if (pos.y < -50.0f) {
            pos.y = WinApp::kClientHeight + 50.0f;
            pos.x = static_cast<float>(rand() % WinApp::kClientWidth);
        }
        stars[i]->SetPosition(pos);
        
        stars[i]->Update();
    }
}

void LoadingScene::UpdateProgressBar(float progress) {
    // スムーズな進行（イージング）
    static float smoothProgress = 0.0f;
    float easeSpeed = 0.08f;
    smoothProgress += (progress - smoothProgress) * easeSpeed;
    
    // プログレスバーのサイズ更新
    float barMaxWidth = 500.0f;
    progressBar->SetSize({ barMaxWidth * smoothProgress, 30.0f });
    
    // 進行度に応じた色変化（青→紫→ピンク→白）
    Vector4 color;
    if (smoothProgress < 0.33f) {
        // 青→紫
        float t = smoothProgress / 0.33f;
        color = {
            0.3f + 0.5f * t,  // R: 0.3 -> 0.8
            0.5f + 0.2f * t,  // G: 0.5 -> 0.7
            1.0f,             // B: 1.0
            1.0f
        };
    } else if (smoothProgress < 0.66f) {
        // 紫→ピンク
        float t = (smoothProgress - 0.33f) / 0.33f;
        color = {
            0.8f + 0.2f * t,  // R: 0.8 -> 1.0
            0.7f - 0.1f * t,  // G: 0.7 -> 0.6
            1.0f - 0.2f * t,  // B: 1.0 -> 0.8
            1.0f
        };
    } else {
        // ピンク→白
        float t = (smoothProgress - 0.66f) / 0.34f;
        color = {
            1.0f,             // R: 1.0
            0.6f + 0.4f * t,  // G: 0.6 -> 1.0
            0.8f + 0.2f * t,  // B: 0.8 -> 1.0
            1.0f
        };
    }
    
    progressBar->SetColor(color);
    progressBar->Update();
}

void LoadingScene::HandleLoadingComplete() {
    // 音声停止
    if (!isSoundStopped) {
        Audio::GetInstance()->StopWave(loadingSound);
        isSoundStopped = true;
        OutputDebugStringA("ロード完了: BGM停止\n");
    }
    
    // フェードアウト開始
    static bool fadeOutStarted = false;
    if (!fadeOutStarted) {
        FadeManager::GetInstance()->StartFadeOut(0.05f);
        fadeOutStarted = true;
        OutputDebugStringA("ロード完了: フェードアウト開始\n");
    }
    
    // シーン遷移
    if (FadeManager::GetInstance()->IsFadeComplete()) {
        static int waitCounter = 0;
        if (++waitCounter > 10) {  // 約0.17秒待機
            OutputDebugStringA("タイトル画面へ遷移\n");
            
            // パフォーマンス情報を最終出力
            ResourceManager::GetInstance()->PrintDebugInfo();
            PerformanceMonitor::GetInstance()->LogPerformance();
            
            sceneNo = Title;
        }
    }
}

void LoadingScene::Draw() {
    PerformanceMonitor::GetInstance()->BeginSection("LoadingSceneDraw");
    
    SpriteCommon::GetInstance()->Command();
    
    // 背景
    background->Draw();
    
    // キラキラエフェクト（背景の上）
    for (int i = 0; i < kMaxStars; i++) {
        stars[i]->Draw();
    }
    
    // プログレスバー
    progressBarBg->Draw();
    progressBar->Draw();
    
    // テキストとアイコン
    loadingText->Draw();
    loadingIcon->Draw();
    
    PerformanceMonitor::GetInstance()->EndSection("LoadingSceneDraw");
}

void LoadingScene::Finalize() {
    OutputDebugStringA("LoadingScene::Finalize() 開始\n");
    
    // 音声停止
    if (!isSoundStopped) {
        Audio::GetInstance()->StopWave(loadingSound);
    }
    
    // スプライト削除
    delete background;
    delete loadingText;
    delete loadingIcon;
    delete progressBar;
    delete progressBarBg;
    
    for (int i = 0; i < kMaxStars; i++) {
        delete stars[i];
    }
    
    OutputDebugStringA("LoadingScene::Finalize() 完了\n");
}

// 互換性のために残すメソッド（空実装）
void LoadingScene::StartAsyncLoading() {}
void LoadingScene::LoadEssentialResources() {}
void LoadingScene::LoadStage1Resources() {}
void LoadingScene::LoadStage2Resources() {}
void LoadingScene::LoadStage3Resources() {}
void LoadingScene::LoadStage4Resources() {}
