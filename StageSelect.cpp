#include "StageSelect.h"
#include <iostream>
#include  <string>

void StageSelect::Initialize() {
	// デバッグ出力
	OutputDebugStringA("StageSelect::Initialize() が実行されました\n");

	stageNum = GameData::selectedStage;


	//背景
	backGround = new Sprite();
	backGround->Initialize("StageSelectName.png");
	backGround->SetPosition({ 0,0 });

	//ステージのナンバー
	stageNumber = new Sprite();
	stageNumber->Initialize("StageNumber.png");
	stageNumber->SetPosition({ 100,200 });	

	stageNumber->SetTextureSize({ 64,64 });
	stageNumber->SetTextureLT({ selectLT * stageNum,0 });

	// ロード状態確認
	TextureManager::GetInstance()->CheckAllTextureLoaded();
	ModelManager::GetInstance()->CheckAllModelsLoaded();
	camera_ = new Camera();

	Object3dCommon::GetInstance()->SetDefaultCamera(camera_);


	//ステージの全体像
	stageObject_ = new Object3d();
	stageObject_->Initialize();

	//切り替え時長押しにならないように
	state = Input::GetInstance()->GetState();
	preState = Input::GetInstance()->GetPreState();
}

void StageSelect::Update() {
	backGround->Update();
	stageNumber->Update();


	//ステージごとのモデル
	stageObject_->SetModelFile("stage" + std::to_string(stageNum));
	//回して見やすくする
	//worldTransform_.translation_.x = 2.0f;
	worldTransform_.rotation_.y += 0.01f;
	worldTransform_.scale_ = { 0.005f,0.005f,0.005f };
	

	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);

	bool isPlus = false;  //数字が増える 
	bool isMinus = false; //数字が減る
	
	const float deltaTimer = 1.0f / 60.0f;
	
	if (Input::GetInstance()->GetJoystickState(0, state)) {
		float x = static_cast<float>(state.Gamepad.sThumbLX) / 32768.0f;

		if (x >= 0.7f) {
			isPlus = true; 
		}
		else if (x <= -0.7f) {
			isMinus = true; 
		}
	}

	if (Input::GetInstance()->PushKey(DIK_D)) {
		isPlus = true;
	}

	if (Input::GetInstance()->PushKey(DIK_A)) {
		isMinus = true;
	}

	if (isPlus || isMinus) {
		steackCount -= deltaTimer;		
	}
	else {
		steackCount = 0.01f; // 離した後、次に押すときスムーズに切り替わる
	}

	//ステージの数字が大きいほうに進む
	if (isPlus && stageNum < 7 && steackCount <= 0) {
		stageNum += 1;
		steackCount = steackMax;
	}
	
	//ステージの数字が小さいほうに進む
	if (isMinus && stageNum > 0 && steackCount <= 0) {
		stageNum -= 1;
		steackCount = steackMax;
	}

	stageNumber->SetTextureLT({ selectLT * stageNum,0 });

	//決定ボタン
	if (Input::GetInstance()->TriggerKey(DIK_SPACE) ||
		((state.Gamepad.wButtons & XINPUT_GAMEPAD_A) && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A))) {
		GameData::selectedStage = stageNum;
		sceneNo = Game;
	}


	camera_->SetTranslate(cameraPosition);
	camera_->SetRotate(cameraRotate);

	camera_->Update();

#pragma region デバッグ表示
#ifdef _DEBUG
	ImGui::Begin("camera");

	ImGui::DragFloat3("Translate",&cameraPosition.x,0.01f);
	ImGui::DragFloat3("Rotate", &cameraRotate.x, 0.01f);

	ImGui::End();
#endif
#pragma endregion


	worldTransform_.UpdateMatrix();
}

void StageSelect::Draw() {
	SpriteCommon::GetInstance()->Command();

	backGround->Draw();
	stageNumber->Draw();

	Object3dCommon::GetInstance()->Command();

	stageObject_->Draw(worldTransform_);
}

void StageSelect::Finalize() {
	delete backGround;
	delete stageNumber;
	delete stageObject_;
}