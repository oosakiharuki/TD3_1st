#include "GameOverScene.h"

void GameOverScene::Initialize() {
	gameOver = new Sprite();
	gameOver->Initialize("scene/GameOver.png");
	gameOver->SetPosition({ 0,0 });

	Arrow = new Sprite();
	Arrow->Initialize("scene/arrow.png");
	Arrow->SetPosition({ 400,352 });

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
			if (gameOverCount_ > 3) gameOverCount_ = 3;
		}

		//コントローラ操作
		if (Input::GetInstance()->GetJoystickState(0, state)) {
			float y = static_cast<float>(state.Gamepad.sThumbLY) / 32768.0f;

			if (y >= 0.7f && !stopSteck) {
				gameOverCount_--;
				stopSteck = true;
				if (gameOverCount_ < 1) gameOverCount_ = 1;
			}
			else if (y <= -0.7f && !stopSteck) {
				gameOverCount_++;
				stopSteck = true;
				if (gameOverCount_ > 3) gameOverCount_ = 3;
			}
			else if (y < 0.7f && y > -0.7f) {
				stopSteck = false;
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
		else if (gameOverCount_ == 2) {
			sceneNo = Select;
		}
		else if (gameOverCount_ == 3) {
			sceneNo = Title;
		}
	}

	gameOver->Update();
	
	const float deltaTime = 1.0f / 60.0f;

	// 矢印のアニメーション
	animationTime_ += deltaTime;
	float floatOffset = sin(animationTime_ * 2.0f) * 10.0f;

	//矢印の位置変更
	arrowPosY = 352 + (138 * (float(gameOverCount_) - 1));
	Arrow->SetPosition({ 400 + floatOffset,arrowPosY });

	Arrow->Update();


	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);
}

void GameOverScene::Draw() {
	SpriteCommon::GetInstance()->Command();

	gameOver->Draw();
	Arrow->Draw();
}

void GameOverScene::Finalize() {
	delete gameOver;
	delete Arrow;
}