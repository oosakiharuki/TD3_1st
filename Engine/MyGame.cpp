#include "MyGame.h"

void MyGame::Initialize() {
    Framework::Initialize();

    gameScene = new GameManager();
    gameScene->Initialize();
}

void MyGame::Update() {
    Framework::Update();

#ifdef  USE_IMGUI
    ImGuiManager::GetInstance()->Begin();
#endif

    gameScene->Update();

#ifdef  USE_IMGUI
    ImGuiManager::GetInstance()->End();
#endif
}

void MyGame::Draw() {
    DirectXCommon::GetInstance()->PreDraw();

    gameScene->Draw();

#ifdef  USE_IMGUI
    ImGuiManager::GetInstance()->Draw();
#endif

    DirectXCommon::GetInstance()->PostDraw();
}

void MyGame::Finalize() {
    // シーン側の終了処理
    if (gameScene) {
        gameScene->Finalize();
        delete gameScene;
        gameScene = nullptr;
    }

    // ImGuiのFinalizeはFrameworkでやるように変更

    // 最後にFrameworkのFinalize
    Framework::Finalize();
}
