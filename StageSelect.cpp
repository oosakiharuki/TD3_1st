#include "StageSelect.h"
#include "FadeManager.h"
#include <iostream>
#include  <string>

void StageSelect::Initialize() {
	// デバッグ出力
	OutputDebugStringA("StageSelect::Initialize() が実行されました\n");

	stageNum = GameData::selectedStage;


	//背景
	backGround = new Sprite();
	backGround->Initialize("scene/background.png");
	backGround->SetPosition({ 0,0 });

	// 黒背景
	shadow = new Sprite();
	shadow->Initialize("ui/black.png");
	shadow->SetPosition({ 0,0 });
	shadow->SetColor({ 0.0f, 0.0f, 0.0f, 0.8f });

	// ステージセレクトの名前
	StageSelectName = new Sprite();
	StageSelectName->Initialize("scene/StageSelectName.png");
	StageSelectName->SetPosition({ 0,0 });

	//ステージのナンバー
	stageNumber = new Sprite();
	stageNumber->Initialize("scene/StageNumber.png");
	stageNumber->SetPosition({ 200,350 });	

	// 選択バー
	selectBar = new Sprite();
	selectBar->Initialize("scene/selectBar.png");
	selectBar->SetPosition({ 100,350 });

	// ボタン
	buttonSprite = new Sprite();
	buttonSprite->Initialize("ui/press_Button.png");
	buttonSprite->SetPosition({ 384 ,560 });

	stageNumber->SetTextureSize({ 96,96 });
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
	shadow->Update();
	StageSelectName->Update();
	stageNumber->Update();
	selectBar->Update();
	buttonSprite->Update();


	//ステージごとのモデル
	stageObject_->SetModelFile("stage" + std::to_string(stageNum));
	//回して見やすくする
	worldTransform_.translation_.x = -0.3f;
	worldTransform_.rotation_.y += 0.01f;
	worldTransform_.scale_ = { 0.005f,0.007f,0.005f };
		
	if (FadeManager::GetInstance()->IsFadeComplete() && isDecision) {
		GameData::selectedStage = stageNum;

		// デバッグ出力 - ステージ選択時のステージ番号
		std::string debugMsg = "Stage Selected: " + std::to_string(GameData::selectedStage) + "\n";
		OutputDebugStringA(debugMsg.c_str());

		sceneNo = Game;
	}

	//フェードイン中に操作させないようにする
	if (FadeManager::GetInstance()->IsFadeComplete()) {

		Input::GetInstance()->GetJoystickState(0, state);
		Input::GetInstance()->GetJoystickStatePrevious(0, preState);

		bool isPlus = false;  //数字が増える 
		bool isMinus = false; //数字が減る

		const float deltaTimer = 1.0f / 60.0f;

		if (Input::GetInstance()->GetJoystickState(0, state)) {
			// 左スティック
			float x = static_cast<float>(state.Gamepad.sThumbLX) / 32768.0f;

			if (x >= 0.7f) {
				isPlus = true;
			}
			else if (x <= -0.7f) {
				isMinus = true;
			}

			// 十字キー
			if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)) {
				isPlus = true;
				steackCount = 0.01f; // 即座に反応
			}
			else if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)) {
				isMinus = true;
				steackCount = 0.01f; // 即座に反応
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
	if (isPlus && stageNum < 5 && steackCount <= 0) {
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
		if ((Input::GetInstance()->TriggerKey(DIK_SPACE) ||
			((state.Gamepad.wButtons & XINPUT_GAMEPAD_A) && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A))) && !isDecision) {
			// フェードアウト開始
			FadeManager::GetInstance()->StartFadeOut(0.03f);
			isDecision = true;
		}
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
	shadow->Draw();
	StageSelectName->Draw();
	stageNumber->Draw();
	selectBar->Draw();
	buttonSprite->Draw();

	Object3dCommon::GetInstance()->Command();

	stageObject_->Draw(worldTransform_);
}

void StageSelect::Finalize() {
	delete backGround;
	delete stageNumber;
	delete shadow;
	delete StageSelectName;
	delete selectBar;
	delete buttonSprite;
	delete stageObject_;
}