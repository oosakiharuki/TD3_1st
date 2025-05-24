#include "GameClearScene.h"
#include "GameData.h"

void GameClearScene::Initialize() {

	backGround = new Sprite();
	backGround->Initialize("scene/background.png");
	backGround->SetPosition({ 0,0 });

	gameClearNextStage = new Sprite();
	gameClearNextStage->Initialize("scene/GameClear_NextStage.png");
	gameClearNextStage->SetPosition({ 0,0 });

	gameClearSelect = new Sprite();
	gameClearSelect->Initialize("scene/GameClear_Select.png");
	gameClearSelect->SetPosition({ 0,0 });

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

	if (gameClearCount_ == 1 && Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		GameData::selectedStage++;
		sceneNo = Game;
	}

	if (gameClearCount_ == 2 && Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		sceneNo = Select;
	}

	gameClearNextStage->Update();
	gameClearSelect->Update();
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
		gameClearSelect->Draw();
	}
}

void GameClearScene::Finalize() {
	delete gameClearNextStage;
	delete gameClearSelect;
	delete backGround;
}