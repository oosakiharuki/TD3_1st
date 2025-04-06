#include "TitleScene.h"

void TitleScene::Initialize() {
	sprite = new Sprite();
	sprite->Initialize("TitleName.png");
	sprite->SetPosition({ 0,-200 });

	backGround = new Sprite();
	backGround->Initialize("title_background.png");
	backGround->SetPosition({0,0});
}

void TitleScene::Update() {
	sprite->Update();
	backGround->Update();

	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);

	if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A)) || 
		Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		sceneNo = Game;
	}

}

void TitleScene::Draw() {
	SpriteCommon::GetInstance()->Command();

	backGround->Draw();
	sprite->Draw();	
}

void TitleScene::Finalize() {
	delete sprite;
}