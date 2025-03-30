#include "GameScene.h"

void GameScene::Initialize() {

	camera = new Camera();
	cameraRotate = { 1.4f,0.0f,0.0f };
	cameraTranslate = { 0.0f,30.0f,-8.0f };
	cameraRotate = { 0.0f,0.0f,0.0f };
	cameraTranslate = { 0.0f,0.0f,-15.0f };
	camera->SetRotate(cameraRotate);
	camera->SetTranslate(cameraTranslate);
	
	Object3dCommon::GetInstance()->SetDefaultCamera(camera);
	ParticleCommon::GetInstance()->SetDefaultCamera(camera);
}

void GameScene::Update() {

	if (Input::GetInstance()->PushKey(DIK_0)) {
		OutputDebugStringA("Hit 0\n");
	}

	if (Input::GetInstance()->TriggerKey(DIK_1)) {
		OutputDebugStringA("Hit 1\n");
	}

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		sceneNo = Title;
	}

	camera->Update();

#ifdef  USE_IMGUI

	//ここにテキストを入れられる

	//開発用UIの処理
	//ImGui::ShowDemoWindow();

	ImGui::Begin("camera");
	ImGui::Text("ImGuiText");

	//カメラ
	ImGui::SliderFloat3("cameraTranslate", &cameraTranslate.x, -30.0f, 30.0f);

	ImGui::SliderFloat("cameraRotateX", &cameraRotate.x, -10.0f, 10.0f);
	ImGui::SliderFloat("cameraRotateY", &cameraRotate.y, -10.0f, 10.0f);
	ImGui::SliderFloat("cameraRotateZ", &cameraRotate.z, -10.0f, 10.0f);
	camera->SetRotate(cameraRotate);
	camera->SetTranslate(cameraTranslate);

	ImGui::End();
#endif //  USE_IMGUI
}

void GameScene::Draw() {

	//スプライト描画処理(背景用)
	SpriteCommon::GetInstance()->Command();


	//モデル描画処理
	Object3dCommon::GetInstance()->Command();

	//パーティクル描画処理
	ParticleCommon::GetInstance()->Command();

	//スプライト描画処理(UI用)
	SpriteCommon::GetInstance()->Command();

}
void GameScene::Finalize() {
	delete camera;
}
