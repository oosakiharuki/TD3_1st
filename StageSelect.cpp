#include "StageSelect.h"

void StageSelect::Initialize() {
	sprite = new Sprite();
	sprite->Initialize("StageSelectName.png");
	sprite->SetPosition({ 0,-200 });

	backGround = new Sprite();
	backGround->Initialize("StageSelect_background.png");
	backGround->SetPosition({ 0,0 });

	stageNumber = new Sprite();
	stageNumber->Initialize("StageNumber.png");
	stageNumber->SetPosition({ 100,200 });	

	stageNumber->SetTextureSize({ 64,64 });

}

void StageSelect::Update() {
	sprite->Update();
	backGround->Update();
	stageNumber->Update();

	stageNumber->SetTextureLT({ selectLT,0 });

	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);

	bool isPrassNum = false;
	bool isMinusNum = false;

	if (Input::GetInstance()->GetJoystickState(0, state)) {
		float x = static_cast<float>(state.Gamepad.sThumbLX) / 32768.0f;

		if (x >= 0.7f) {
			isPrassNum = true;
		}
		else if (x <= -0.7f) {
			isMinusNum = true;
		}
	}

	if (Input::GetInstance()->PushKey(DIK_D) && stageNum < 6) {
		isPrassNum = true;
	}

	if (Input::GetInstance()->PushKey(DIK_A) && stageNum > 0) {
		isMinusNum = true;
	}

	const float deltaTimer = 1.0f / 60.0f;
	
	if (steackCount > 0) 
		steackCount -= deltaTimer;
	

	if (isPrassNum && stageNum < 6 && steackCount <= 0) {
		stageNum += 1;
		selectLT += 64.0f;
		steackCount = steackMax;
	}
	
	if (isMinusNum && stageNum > 0 && steackCount <= 0) {
		stageNum -= 1;
		selectLT -= 64.0f;
		steackCount = steackMax;
	}

	if (Input::GetInstance()->TriggerKey(DIK_SPACE) ||
		((state.Gamepad.wButtons & XINPUT_GAMEPAD_A) && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A))) {
		GameData::selectedStage = stageNum;
		sceneNo = Game;
	}
}

void StageSelect::Draw() {
	SpriteCommon::GetInstance()->Command();

	backGround->Draw();
	sprite->Draw();
	stageNumber->Draw();
}

void StageSelect::Finalize() {
	delete sprite;
	delete backGround;
	delete stageNumber;
}