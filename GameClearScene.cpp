#include "GameClearScene.h"
#include "GameData.h"

void GameClearScene::Initialize() {

	backGround = new Sprite();
	backGround->Initialize("scene/background.png");
	backGround->SetPosition({ 0,0 });

	stageClear = new Sprite();
	//最後のステージをクリアした時
	if (GameData::selectedStage == 6) {
		stageClear->Initialize("scene/StageClear_All.png");
		//nextStageを選べなくする
		gameClearCount_++;
		allClear = true;
	}
	else {
		stageClear->Initialize("scene/StageClear.png");
	}
	stageClear->SetPosition({ 0,0 });

	Arrow  = new Sprite();
	Arrow->Initialize("scene/arrow.png");
	Arrow->SetPosition({ 400,352 });


	//切り替え時長押しにならないように
	state = Input::GetInstance()->GetState();
	preState = Input::GetInstance()->GetPreState();
}

void GameClearScene::Update() {

	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);

	//フェード中操作させない
	if (FadeManager::GetInstance()->IsFadeComplete()) {
		//キーボード操作
		if (Input::GetInstance()->TriggerKey(DIK_W)) {
			gameClearCount_--;
			if (gameClearCount_ < 1) gameClearCount_ = 1;

			if (gameClearCount_ < 2 && allClear) gameClearCount_ = 2;
		}
		if (Input::GetInstance()->TriggerKey(DIK_S)) {
			gameClearCount_++;
			if (gameClearCount_ > 3) gameClearCount_ = 3;
		}

		//コントローラ操作
		if (Input::GetInstance()->GetJoystickState(0, state)) {
			float y = static_cast<float>(state.Gamepad.sThumbLY) / 32768.0f;

			if (y >= 0.7f && !stopSteck) {
				gameClearCount_--;
				stopSteck = true;
				if (gameClearCount_ < 1) gameClearCount_ = 1;
				
				if (gameClearCount_ < 2 && allClear) gameClearCount_ = 2;
			}
			else if (y <= -0.7f && !stopSteck) {
				gameClearCount_++;
				stopSteck = true;
				if (gameClearCount_ > 3) gameClearCount_ = 3;
			}
			else if(y < 0.7f && y > -0.7f){
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
		
		if (gameClearCount_ == 1) {
			//次のステージに移動
			GameData::selectedStage++;
			sceneNo = Game;
		}
		else if (gameClearCount_ == 2) {
			sceneNo = Select;
		}
		else if (gameClearCount_ == 3) {
			sceneNo = Title;
		}
	}

	stageClear->Update();

	const float deltaTime = 1.0f / 60.0f;

	// 矢印のアニメーション
	animationTime_ += deltaTime;
	float floatOffset = sin(animationTime_ * 2.0f) * 10.0f;

	//矢印の位置変更
	arrowPosY = 352 + (138 * (float(gameClearCount_) - 1));
	Arrow->SetPosition({ 400 + floatOffset,arrowPosY });

	Arrow->Update();

	backGround->Update();

	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);
}

void GameClearScene::Draw() {
	SpriteCommon::GetInstance()->Command();

	backGround->Draw();
	Arrow->Draw();
	stageClear->Draw();
}

void GameClearScene::Finalize() {
	delete stageClear;
	delete Arrow;
	delete backGround;
}