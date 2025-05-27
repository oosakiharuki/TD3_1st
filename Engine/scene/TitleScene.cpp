#include "TitleScene.h"
#include "FadeManager.h"

void TitleScene::Initialize() {
	// デバッグ出力
	OutputDebugStringA("TitleScene::Initialize() が実行されました\n");

	sprite = new Sprite();
	sprite->Initialize("scene/TitleName.png");
	sprite->SetPosition({ 0,-200 });

	backGround = new Sprite();
	backGround->Initialize("scene/background.png");
	backGround->SetPosition({ 0,0 });

	bottonSprite = new Sprite();
	bottonSprite->Initialize("ui/press_Button.png");
	bottonSprite->SetPosition({ 384 ,560 });

	// ロード状態確認
	TextureManager::GetInstance()->CheckAllTextureLoaded();
	ModelManager::GetInstance()->CheckAllModelsLoaded();

	// フェードイン開始
	FadeManager::GetInstance()->StartFadeIn(0.03f);

	//切り替え時長押しにならないように
	state = Input::GetInstance()->GetState();
	preState = Input::GetInstance()->GetPreState();
}

void TitleScene::Update() {
	sprite->Update();
	backGround->Update();
	bottonSprite->Update();

	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);

	//フェード中操作させない
	if (FadeManager::GetInstance()->IsFadeComplete()) {
		if (((state.Gamepad.wButtons & XINPUT_GAMEPAD_A && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A)) ||
			Input::GetInstance()->TriggerKey(DIK_SPACE)) && !isDecision) {
			//押した瞬間のボタン情報の取得
			Input::GetInstance()->SetStates(state, preState);
			// フェードアウト開始
			FadeManager::GetInstance()->StartFadeOut(0.03f);
			isDecision = true;
		}
	}

	if (FadeManager::GetInstance()->IsFadeComplete() && isDecision) {
		// 既にロード済みなので、直接Selectシーンに遷移
		sceneNo = Select;
	}
}

void TitleScene::Draw() {
	SpriteCommon::GetInstance()->Command();

	backGround->Draw();
	sprite->Draw();
	bottonSprite->Draw();
}

void TitleScene::Finalize() {
	delete sprite;
	delete backGround;
	delete bottonSprite;
}