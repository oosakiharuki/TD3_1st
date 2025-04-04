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
    // リソースリークチェックを最後に実行するように変更
    // まずすべてのリソースを解放する

    // 1) まずImGuiを解放
    if (ImGuiManager::GetInstance()) {
        ImGuiManager::GetInstance()->Finalize();
    }

    // 2) Input解放
    if (input_) {
        input_->Finalize();
    }

    // 3) 各ManagerやCommonを解放
    if (TextureManager::GetInstance()) {
        TextureManager::GetInstance()->Finalize();
    }

    if (ModelManager::GetInstance()) {
        ModelManager::GetInstance()->Finalize();
    }

    if (ParticleManager::GetInstance()) {
        ParticleManager::GetInstance()->Finalize();
    }

    if (spriteCommon) {
        spriteCommon->Finalize();
    }

    if (object3dCommon) {
        object3dCommon->Finalize();
    }

    // newしたやつはdelete
    if (modelCommon) {
        delete modelCommon;
        modelCommon = nullptr;
    }

    // 4) SRVマネージャ解放
    if (srvManager) {
        srvManager->Finalize();
    }

    // 5) dxCommon解放
    if (dxCommon) {
        dxCommon->Finalize();
    }

    // 6) WinApp解放
    if (winApp_) {
        winApp_->Finalize();
        delete winApp_;
        winApp_ = nullptr;
    }

    // 最後にリソースリークチェックを実行
    // ここでどのようなリソースがリークしているかログに出力されます
    D3DResourceLeakChecker leakCheck;
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
