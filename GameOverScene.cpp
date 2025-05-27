#include "GameOverScene.h"

void GameOverScene::Initialize() {
	gameOverContinue = new Sprite();
	gameOverContinue->Initialize("scene/GameOver_Continue.png");
	gameOverContinue->SetPosition({ 0,0 });

	gameOverTitle = new Sprite();
	gameOverTitle->Initialize("scene/GameOver_Title.png");
	gameOverTitle->SetPosition({ 0,0 });

	//切り替え時長押しにならないように
	state = Input::GetInstance()->GetState();
	preState = Input::GetInstance()->GetPreState();
}

void GameOverScene::Update() {
	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);

	//フェード中操作させない
	if (FadeManager::GetInstance()->IsFadeComplete()) {
		//キーボード操作
		if (Input::GetInstance()->TriggerKey(DIK_W)) {
			gameOverCount_--;
			if (gameOverCount_ < 1) gameOverCount_ = 1;
		}
		if (Input::GetInstance()->TriggerKey(DIK_S)) {
			gameOverCount_++;
			if (gameOverCount_ > 2) gameOverCount_ = 2;
		}

		//コントローラ操作
		if (Input::GetInstance()->GetJoystickState(0, state)) {
			float y = static_cast<float>(state.Gamepad.sThumbLY) / 32768.0f;

			if (y >= 0.7f) {
				gameOverCount_--;
				if (gameOverCount_ < 1) gameOverCount_ = 1;
			}

			if (y <= -0.7f) {
				gameOverCount_++;
				if (gameOverCount_ > 2) gameOverCount_ = 2;
			}
		}

		if ((Input::GetInstance()->TriggerKey(DIK_SPACE) ||
			((state.Gamepad.wButtons & XINPUT_GAMEPAD_A) && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A)))) {
			isDecision = true;
			FadeManager::GetInstance()->StartFadeOut(0.03f);
		}
	}

	if (FadeManager::GetInstance()->IsFadeComplete() && isDecision) {
		if (gameOverCount_ == 1) {
			sceneNo = Game;
		}

		if (gameOverCount_ == 2) {
			sceneNo = Title;
		}
	}

	gameOverContinue->Update();
	gameOverTitle->Update();

	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);
}

void GameOverScene::Draw() {
	SpriteCommon::GetInstance()->Command();

	if (gameOverCount_ == 1) {
		gameOverContinue->Draw();
	}
	else if (gameOverCount_ == 2) {
		gameOverTitle->Draw();
	}
}

void GameOverScene::Finalize() {
	delete gameOverContinue;
	delete gameOverTitle;
}