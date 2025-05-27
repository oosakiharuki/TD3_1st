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

	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);

	//選択したら変えられないようにする
	if (!isDecision) {
		//キーボード操作
		if (Input::GetInstance()->TriggerKey(DIK_W)) {
			gameClearCount_--;
			if (gameClearCount_ < 1) gameClearCount_ = 1;
		}
		if (Input::GetInstance()->TriggerKey(DIK_S)) {
			gameClearCount_++;
			if (gameClearCount_ > 2) gameClearCount_ = 2;
		}

		//コントローラ操作
		if (Input::GetInstance()->GetJoystickState(0, state)) {
			float y = static_cast<float>(state.Gamepad.sThumbLY) / 32768.0f;

			if (y >= 0.7f) {
				gameClearCount_--;
				if (gameClearCount_ < 1) gameClearCount_ = 1;
			}

			if (y <= -0.7f) {
				gameClearCount_++;
				if (gameClearCount_ > 2) gameClearCount_ = 2;
			}
		}
	}


	if ((Input::GetInstance()->TriggerKey(DIK_SPACE) || 
		((state.Gamepad.wButtons & XINPUT_GAMEPAD_A) && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A)))) {	
		isDecision = true;
		FadeManager::GetInstance()->StartFadeOut(0.03f);
	}

	if (FadeManager::GetInstance()->IsFadeComplete() && isDecision) {
		if (gameClearCount_ == 1) {
			//次のステージに移動
			GameData::selectedStage++;
			sceneNo = Game;
		}
		else if (gameClearCount_ == 2) {
			sceneNo = Select;
		}
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