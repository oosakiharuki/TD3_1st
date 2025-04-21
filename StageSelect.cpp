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

	if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A)) ||
		Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		// SelectからLoadingに変更  - ローディング画面へ遷移
		sceneNo = Loading;
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