#pragma once
#include "Sprite.h"
#include "Input.h"
#include <vector>


// 検出された入力デバイスタイプ
enum class InputDeviceType { None, Keyboard, Controller };

class UIManager {
public:
	UIManager();
	~UIManager();

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画 - intで統一
	void Draw(int playerHP);

	// 入力デバイスの検出
	void DetectInputDevice();

	// 操作ガイドの表示
	void DrawControlGuide();

private:
	// HP表示関連
	Sprite* hpBarBgSprite_ = nullptr;   // HP背景
	Sprite* hpBarFillSprite_ = nullptr; // HPバー
	Sprite* hpIconSprite_ = nullptr;    // HPアイコン
	Sprite* hpTextSprite_ = nullptr;    // HPテキスト

	// 操作ガイド関連
	Sprite* keyboardGuideSprite_ = nullptr;   // キーボード操作ガイド
	Sprite* controllerGuideSprite_ = nullptr; // コントローラー操作ガイド

	// 検出された入力デバイス
	InputDeviceType detectedDevice_ = InputDeviceType::None;

	// 入力検出タイマー
	float inputDetectionTimer_ = 0.0f;
	bool keyboardInputDetected_ = false;
	bool controllerInputDetected_ = false;

	// 前回の入力状態
	XINPUT_STATE prevState_ = {};
	XINPUT_STATE currentState_ = {};
};