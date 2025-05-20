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

	if (Input::GetInstance()->TriggerKey(DIK_W)) {
		gameOverCount_--;
		if (gameOverCount_ < 1) gameOverCount_ = 1;
	}
	if (Input::GetInstance()->TriggerKey(DIK_S)) {
		gameOverCount_++;
		if (gameOverCount_ > 2) gameOverCount_ = 2;
	}

	if (gameOverCount_ == 1 && Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		sceneNo = Game;
	}

	if (gameOverCount_ == 2 && Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		sceneNo = Title;
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