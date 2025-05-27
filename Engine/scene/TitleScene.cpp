#include "TitleScene.h"
#include "FadeManager.h"
#include "Audio.h"
#include <cmath>

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

	// オーディオ初期化
	audio_ = Audio::GetInstance();
	titleBGM_ = audio_->LoadWave("sound/title.wav");
	decisionSound_ = audio_->LoadWave("sound/decision.wav");

	// BGM再生
	audio_->SoundPlayWave(titleBGM_, 0.3f, true);

	//切り替え時長押しにならないように
	state = Input::GetInstance()->GetState();
	preState = Input::GetInstance()->GetPreState();
}

void TitleScene::Update() {
	const float deltaTime = 1.0f / 60.0f;

	// タイトルロゴのアニメーション
	animationTime_ += deltaTime;
	
	// フローティングアニメーション
	float floatOffset = sin(animationTime_ * 2.0f) * 10.0f;
	sprite->SetPosition({0, -200 + floatOffset});
	
	// ボタンの点滅アニメーション
	buttonBlinkTimer_ += deltaTime;
	float alpha = (sin(buttonBlinkTimer_ * 3.0f) + 1.0f) * 0.5f; // 0～1の範囲で変化
	// bottonSpriteにSetAlphaメソッドがある場合は使用
	// bottonSprite->SetAlpha(alpha);

	sprite->Update();
	backGround->Update();
	bottonSprite->Update();

	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);

	if (((state.Gamepad.wButtons & XINPUT_GAMEPAD_A && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A)) ||
		Input::GetInstance()->TriggerKey(DIK_SPACE)) && !isDecision) {
		//押した瞬間のボタン情報の取得
		Input::GetInstance()->SetStates(state, preState);
		
		// 決定音を再生
		audio_->SoundPlayWave(decisionSound_, 0.8f);
		
		// フェードアウト開始
		FadeManager::GetInstance()->StartFadeOut(0.03f);
		isDecision = true;
	}
	
	if (FadeManager::GetInstance()->IsFadeComplete() && isDecision) {
		// BGMを停止
		audio_->StopWave(titleBGM_);
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