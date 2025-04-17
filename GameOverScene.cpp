#include "GameOverScene.h"

void GameOverScene::Initialize() {
	sprite = new Sprite();
	sprite->Initialize("GameOverName.png");
	sprite->SetPosition({ 0,-200 });

	backGround = new Sprite();
	backGround->Initialize("GameOver_background.png");
	backGround->SetPosition({ 0,0 });
}

void GameOverScene::Update() {
	sprite->Update();
	backGround->Update();

	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);

	if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A)) ||
		Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		// GameOverからTitleに変更
		sceneNo = Title;
	}
}

void GameOverScene::Draw() {
	SpriteCommon::GetInstance()->Command();

	backGround->Draw();
	sprite->Draw();
}

void GameOverScene::Finalize() {
	delete sprite;
	delete backGround;
}