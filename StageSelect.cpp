#include "StageSelect.h"

void StageSelect::Initialize() {
	sprite = new Sprite();
	sprite->Initialize("StageSelectName.png");
	sprite->SetPosition({ 0,-200 });

	backGround = new Sprite();
	backGround->Initialize("StageSelect_background.png");
	backGround->SetPosition({ 0,0 });
}

void StageSelect::Update() {
	sprite->Update();
	backGround->Update();

	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);

	if (Input::GetInstance()->TriggerKey(DIK_1)) {
		GameData::selectedStage = 0;
		sceneNo = Game;
	}
	else if (Input::GetInstance()->TriggerKey(DIK_2)) {
		GameData::selectedStage = 1;
		sceneNo = Game;
	}
	else if (Input::GetInstance()->TriggerKey(DIK_3)) {
		GameData::selectedStage = 2;
		sceneNo = Game;
	}
}

void StageSelect::Draw() {
	SpriteCommon::GetInstance()->Command();

	backGround->Draw();
	sprite->Draw();
}

void StageSelect::Finalize() {
	delete sprite;
	delete backGround;
}