#include "TitleScene.h"

void TitleScene::Initialize() {
	sprite = new Sprite();
	sprite->Initialize("monsterBall.png");

	sprite->SetPosition({ 64,64 });
	sprite->SetSize({ 128,128 });
}

void TitleScene::Update() {
	sprite->Update();

	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);

	if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A)) ||
		Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		// ロード画面に遷移するように変更
		sceneNo = Loading;
	}

}

void TitleScene::Draw() {
	SpriteCommon::GetInstance()->Command();

	sprite->Draw();
}

void TitleScene::Finalize() {
	delete sprite;
}