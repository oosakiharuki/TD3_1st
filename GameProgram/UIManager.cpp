#include "UIManager.h"
#include <algorithm> // min/maxのために追加
#include <TextureManager.h>
#include "WinApp.h"
#include <cassert>
#include <GameData.h>

UIManager::UIManager() {}

UIManager::~UIManager() {
	// スプライトの解放
	if (hpBarBgSprite_)
		delete hpBarBgSprite_;
	if (hpBarFillSprite_)
		delete hpBarFillSprite_;
	if (hpIconSprite_)
		delete hpIconSprite_;
	if (hpTextSprite_)
		delete hpTextSprite_;
	
	// 鍵アイコン配列の解放
	for (int i = 0; i < MAX_KEYS; i++) {
		if (keyIcons_[i]) {
			delete keyIcons_[i];
			keyIcons_[i] = nullptr;
		}
	}
	
	if (keyboardGuideSprite_)
		delete keyboardGuideSprite_;
	if (controllerGuideSprite_)
		delete controllerGuideSprite_;

	delete tutorial;
}

void UIManager::Initialize() {

	// スプライト生成
	// HPバー背景
	hpBarBgSprite_ = new Sprite();
	hpBarBgSprite_->Initialize("ui/hp_bar_bg.png");
	hpBarBgSprite_->SetPosition({ 20, WinApp::kClientHeight - 50 });
	hpBarBgSprite_->SetSize({300, 30});

	// HPバー前景
	hpBarFillSprite_ = new Sprite();
	hpBarFillSprite_->Initialize("ui/hp_bar_fill.png");
	hpBarFillSprite_->SetPosition({ 20, WinApp::kClientHeight - 50 });
	hpBarFillSprite_->SetSize({300, 30});

	// HPアイコン
	hpIconSprite_ = new Sprite();
	hpIconSprite_->Initialize("ui/hp_icon.png");
	hpIconSprite_->SetPosition({ 20, WinApp::kClientHeight - 80 });
	hpIconSprite_->SetSize({50, 50});

	// HPテキスト
	hpTextSprite_ = new Sprite();
	hpTextSprite_->Initialize("ui/hp_text.png");
	hpTextSprite_->SetPosition({ 70, WinApp::kClientHeight - 75 });
	hpTextSprite_->SetSize({50, 30});

	// 鍵アイコンの初期化（最大10個まで）
	for (int i = 0; i < MAX_KEYS; i++) {
		keyIcons_[i] = new Sprite();
		keyIcons_[i]->Initialize("key.png");
		// 鍵アイコンを横並びにする（HPバーの上に配置）
		float xPos = 10.0f + (i * 15.0f); // 各鍵アイコンの間隔を35ピクセルに設定
		keyIcons_[i]->SetPosition({ xPos, WinApp::kClientHeight - 90 });
		keyIcons_[i]->SetSize({60, 60});
	}

	// キーボード操作ガイド
	keyboardGuideSprite_ = new Sprite();
	keyboardGuideSprite_->Initialize("ui/keyboard_guide.png");
	keyboardGuideSprite_->SetPosition({ 20, 20 });
	keyboardGuideSprite_->SetSize({300, 150});

	// コントローラー操作ガイド
	controllerGuideSprite_ = new Sprite();
	controllerGuideSprite_->Initialize("ui/controller_guide.png");
	controllerGuideSprite_->SetPosition({ 20, 20 });
	controllerGuideSprite_->SetSize({300, 150});

	if (GameData::selectedStage == 0) {
		tutorial = new Sprite();
		tutorial->Initialize("ui/tutorial01.png");
		tutorial->SetPosition({ 0,-50 });
	}
	
	// 現在のステージの鍵の総数を初期化
	currentTotalKeys_ = 0;
}

void UIManager::Update() {
	hpBarBgSprite_->Update();
	hpBarFillSprite_->Update();
	hpIconSprite_->Update();
	hpTextSprite_->Update();
	
	// 鍵アイコンの更新
	for (int i = 0; i < MAX_KEYS; i++) {
		if (keyIcons_[i]) {
			keyIcons_[i]->Update();
		}
	}
	
	keyboardGuideSprite_->Update();
	controllerGuideSprite_->Update();

	if (GameData::selectedStage == 0) {
		tutorial->Update();
	}
	
	// 入力デバイスの検出
	DetectInputDevice();

	// 入力検出タイマーの更新
	inputDetectionTimer_ -= 1.0f / 60.0f;
	if (inputDetectionTimer_ < 0.0f) {
		inputDetectionTimer_ = 0.0f;
	}
}

void UIManager::Draw(int playerHP) {
	// HPバー背景描画
	if (hpBarBgSprite_) {
		hpBarBgSprite_->Draw();
	}

	// HPバー前景描画 (HPに応じてサイズ変更)
	if (hpBarFillSprite_) {
		// HPの割合に応じてバーの横幅を変更 (最大HP 200を想定)
		float hpRatio = static_cast<float>(playerHP) / 200.0f;
		// std::clampの代わりに自前で範囲制限
		if (hpRatio < 0.0f)
			hpRatio = 0.0f;
		else if (hpRatio > 1.0f)
			hpRatio = 1.0f;

		// HPの割合に応じて色を変更
		Vector4 barColor = {1.0f, 1.0f, 1.0f, 1.0f};
		if (hpRatio < 0.3f) {
			// 低HP: 赤
			barColor = {1.0f, 0.3f, 0.3f, 1.0f};
		} else if (hpRatio < 0.5f) {
			// 中HP: 黄色
			barColor = {1.0f, 1.0f, 0.3f, 1.0f};
		} else {
			// 高HP: 緑
			barColor = {0.3f, 1.0f, 0.3f, 1.0f};
		}

		hpBarFillSprite_->SetSize({300 * hpRatio, 30});
		hpBarFillSprite_->SetColor(barColor);
		hpBarFillSprite_->Draw();
	}

	// HPアイコン描画
	if (hpIconSprite_) {
		hpIconSprite_->Draw();
	}

	// HPテキスト描画
	if (hpTextSprite_) {
		hpTextSprite_->Draw();
	}


	// 操作ガイド描画
	DrawControlGuide();

	if (GameData::selectedStage == 0 && !isTutorialEnd) {
		tutorial->Draw();
	}
}

void UIManager::DrawKeyCount(int remainingKeys, int totalKeys) {
	// ステージが変わったときに鍵の総数が変更されていれば、位置を再計算
	if (currentTotalKeys_ != totalKeys) {
		currentTotalKeys_ = totalKeys;
		
		// 鍵アイコンの位置を再計算（中央揃えにする）
		float startX = 5.0f; // 15
		float iconWidth = 30.0f;
		float iconSpacing = 5.0f;
		float totalWidth = (iconWidth + iconSpacing) * totalKeys - iconSpacing;
		
		for (int i = 0; i < totalKeys; i++) {
			float xPos = startX + i * (iconWidth + iconSpacing);
			keyIcons_[i]->SetPosition({ xPos, WinApp::kClientHeight - 140 });
		}
	}
	
	// remainingKeysの数だけ鍵アイコンを表示
	for (int i = 0; i < totalKeys; i++) {
		if (i < remainingKeys) {
			// まだ取得していない鍵（表示する）
			keyIcons_[i]->Draw();
		}
		// 取得済みの鍵は表示しない
	}
}

void UIManager::DetectInputDevice() {
	// キーボード入力の検出 - GetKeyStateの代わりにDIKキーチェック
	bool anyKeyPressed = false;
	// 一般的なゲーム操作キーをチェック - BYTEタイプを使用
	const BYTE keys[] = {DIK_W, DIK_A, DIK_S, DIK_D, DIK_UP, DIK_DOWN, DIK_LEFT, DIK_RIGHT, DIK_SPACE, DIK_RETURN, DIK_ESCAPE};

	for (BYTE key : keys) {
		if (Input::GetInstance()->PushKey(key)) {
			anyKeyPressed = true;
			break;
		}
	}

	if (anyKeyPressed) {
		keyboardInputDetected_ = true;
		controllerInputDetected_ = false;
		inputDetectionTimer_ = 2.0f; // 2秒間キーボード操作ガイドを表示
		detectedDevice_ = InputDeviceType::Keyboard;
	}

	// コントローラー入力の検出
	Input::GetInstance()->GetJoystickState(0, currentState_);
	if (currentState_.Gamepad.wButtons != 0 || abs(currentState_.Gamepad.sThumbLX) > 8000 || abs(currentState_.Gamepad.sThumbLY) > 8000 || abs(currentState_.Gamepad.sThumbRX) > 8000 ||
	    abs(currentState_.Gamepad.sThumbRY) > 8000) {

		keyboardInputDetected_ = false;
		controllerInputDetected_ = true;
		inputDetectionTimer_ = 2.0f; // 2秒間コントローラー操作ガイドを表示
		detectedDevice_ = InputDeviceType::Controller;
	}

	// 前回の状態を更新
	prevState_ = currentState_;
}

void UIManager::DrawControlGuide() {
	// 入力検出タイマーが0でない場合のみガイドを表示
	if (inputDetectionTimer_ <= 0.0f) {
		return;
	}

	// 検出されたデバイスに応じたガイドを表示
	if (detectedDevice_ == InputDeviceType::Keyboard && keyboardGuideSprite_) {
		keyboardGuideSprite_->Draw();
	} else if (detectedDevice_ == InputDeviceType::Controller && controllerGuideSprite_) {
		controllerGuideSprite_->Draw();
	}
}

void UIManager::TutorialPos(Vector3 playerPos) {
	if (playerPos.z >= -90) {
		tutorial->SetTextureFile("ui/tutorial02.png");
	}
	if (playerPos.z >= -80) {
		tutorial->SetTextureFile("ui/tutorial03.png");
	}
	if (playerPos.z >= -50) {
		tutorial->SetTextureFile("ui/tutorial04.png");
	}
	if (playerPos.z >= -30) {
		tutorial->SetTextureFile("ui/tutorial05.png");
	}
	if (playerPos.z >= 0) {
		tutorial->SetTextureFile("ui/tutorial06.png");
	}
	if (playerPos.z >= 60) {
		tutorial->SetTextureFile("ui/tutorial_key.png");
	}
	if (playerPos.z >= 100) {
		tutorial->SetTextureFile("ui/tutorial_goal.png");
	}
}