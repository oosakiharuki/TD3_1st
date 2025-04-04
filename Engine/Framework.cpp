#include "Framework.h"

void Framework::Initialize() {

    winApp_ = new WinApp();
    winApp_->Initialize();

    input_ = Input::GetInstance();
    input_->Initialize(winApp_);

    dxCommon = DirectXCommon::GetInstance();
    dxCommon->SetWinApp(winApp_);
    dxCommon->Initialize();

    srvManager = SrvManager::GetInstance();
    srvManager->Initialize(dxCommon);

    // ImGuiをFramework内で初期化
    ImGuiManager::GetInstance()->Initialize(winApp_, dxCommon, srvManager);

    spriteCommon = SpriteCommon::GetInstance();
    spriteCommon->Initialize(dxCommon);
    TextureManager::GetInstance()->Initialize(dxCommon, srvManager);

    object3dCommon = Object3dCommon::GetInstance();
    object3dCommon->Initialize(dxCommon);

    modelCommon = new ModelCommon();
    modelCommon->Initialize(dxCommon);
    ModelManager::GetInstance()->Initialize(dxCommon);

    particleCommon = ParticleCommon::GetInstance();
    particleCommon->Initialize(dxCommon);
    ParticleManager::GetInstance()->Initialize(dxCommon, srvManager);
}

void Framework::Update() {
    if (winApp_->ProcessMessage()) {
        isRequst = true;
    }
    else {
        input_->Update();
    }
}

// Finalizeの順番を整理してみる
void Framework::Finalize() {
    // リソースリークチェック
    D3DResourceLeakChecker leakCheck;

    // 1) まずImGuiを解放
    ImGuiManager::GetInstance()->Finalize();

    // 2) Input解放 (必要ならDeleteも)
    if (input_) {
        input_->Finalize();
        //delete input_; // Singletonならやらない方がいい
        //input_ = nullptr;
    }

    // 3) 各ManagerやCommonを解放
    TextureManager::GetInstance()->Finalize();
    ModelManager::GetInstance()->Finalize();
    ParticleManager::GetInstance()->Finalize();

    if (spriteCommon) {
        spriteCommon->Finalize();
        // spriteCommon = nullptr; // 気になるならこれも
    }
    if (object3dCommon) {
        object3dCommon->Finalize();
        // object3dCommon = nullptr;
    }

    // newしたやつはdelete
    if (modelCommon) {
        delete modelCommon;
        modelCommon = nullptr;
    }
    if (particleCommon) {
        particleCommon->Finalize();
        // particleCommon = nullptr; // シングルトンじゃないなら
    }

    // 4) SRVマネージャ解放
    if (srvManager) {
        srvManager->Finalize();
        // srvManager = nullptr; // シングルトンなら削除はしない
    }

    // 5) 最後にdxCommon解放
    if (dxCommon) {
        dxCommon->Finalize();
        // dxCommon = nullptr; // シングルトンでなければdelete
    }

    // 6) WinApp解放
    if (winApp_) {
        winApp_->Finalize();
        delete winApp_;
        winApp_ = nullptr;
    }
}

void Framework::Run() {
    // 初期化
    Initialize();

    // メインループ
    while (true) {
        Update();
        if (IsEndRequst()) {
            break;
        }
        Draw();
    }

    // 終了処理
    Finalize();
}
