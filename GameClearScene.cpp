#include "GameClearScene.h"
#include "GameData.h"

void GameClearScene::Initialize() {

	sprite = new Sprite();
	sprite->Initialize("winScene.png");
	sprite->SetPosition({ 0,0 });

	backGround = new Sprite();
	backGround->Initialize("scene/background.png");
	backGround->SetPosition({ 0,0 });

	gameClearNextStage = new Sprite();
	gameClearNextStage->Initialize("scene/GameClear_NextStage.png");
	gameClearNextStage->SetPosition({ 0,0 });

	gameClearTitle = new Sprite();
	gameClearTitle->Initialize("scene/GameClear_Title.png");
	gameClearTitle->SetPosition({ 0,0 });

	//切り替え時長押しにならないように
	state = Input::GetInstance()->GetState();
	preState = Input::GetInstance()->GetPreState();
}

void GameClearScene::Update() {

	if (Input::GetInstance()->TriggerKey(DIK_W)) {
		gameClearCount_--;
		if (gameClearCount_ < 1) gameClearCount_ = 1;
	}
	if (Input::GetInstance()->TriggerKey(DIK_S)) {
		gameClearCount_++;
		if (gameClearCount_ > 2) gameClearCount_ = 2;
	}

	if (gameClearCount_ == 1 && Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		//audio_->StopWave(BGMSound);
		sceneNo = Game;
	}

	if (gameClearCount_ == 2 && Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		//audio_->StopWave(BGMSound);
		sceneNo = Title;
	}

	gameClearNextStage->Update();
	gameClearTitle->Update();
	backGround->Update();

	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);
}

void GameClearScene::Draw() {
	SpriteCommon::GetInstance()->Command();

	backGround->Draw();
	
	if (gameClearCount_ == 1) {
		gameClearNextStage->Draw();
	}
	else if (gameClearCount_ == 2) {
		gameClearTitle->Draw();
	}
}

void GameClearScene::Finalize() {
	delete gameClearNextStage;
	delete gameClearTitle;
	delete backGround;
}