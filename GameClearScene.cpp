#include "GameClearScene.h"
#include "GameData.h"

void GameClearScene::Initialize() {
	sprite = new Sprite();
	sprite->Initialize("GameClearName.png");
	sprite->SetPosition({ 0,-200 });

	backGround = new Sprite();
	backGround->Initialize("GameClear_background.png");
	backGround->SetPosition({ 0,0 });
}

void GameClearScene::Update() {
	sprite->Update();
	backGround->Update();

	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);

	//if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A)) ||
	//	Input::GetInstance()->TriggerKey(DIK_SPACE)) {
	//	// GameOverからTitleに変更
	//	sceneNo = Title;
	//}


	//次のステージに行くとき
	if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A)) ||
		Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		GameData::selectedStage += 1;
		sceneNo = Game;
	}

	//ステージセレクトに戻るとき
	if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A)) ||
		Input::GetInstance()->TriggerKey(DIK_F1)) {
		sceneNo = Select;
	}

	//タイトルに戻るとき
	if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A)) ||
		Input::GetInstance()->TriggerKey(DIK_F2)) {
		sceneNo = Title;
	}
}

void GameClearScene::Draw() {
	SpriteCommon::GetInstance()->Command();
	
	backGround->Draw();
	sprite->Draw();
}

void GameClearScene::Finalize() {
	delete sprite;
	delete backGround;
}