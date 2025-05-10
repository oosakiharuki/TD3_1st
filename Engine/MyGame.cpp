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
#endif //  USE_IMGUI

	gameScene->Update();

#ifdef  USE_IMGUI
	ImGuiManager::GetInstance()->End();
#endif //  USE_IMGUI

}

void MyGame::Draw() {
	//描画開始
	DirectXCommon::GetInstance()->PreDraw();

	gameScene->Draw();

	// フェードの描画（全ての描画の一番上から）
	FadeManager::GetInstance()->Draw();

#ifdef  USE_IMGUI
	//ImGui描画処理
	ImGuiManager::GetInstance()->Draw();
#endif //  USE_IMGUI

	//描画終了
	DirectXCommon::GetInstance()->PostDraw();

}


void MyGame::Finalize() {
	//gameScene->Finalize();
	delete gameScene;

#ifdef  USE_IMGUI
	ImGuiManager::GetInstance()->Finalize();
#endif //  USE_IMGUI

	Framework::Finalize();
}
