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

	// 鍵表示機能を追加
	void DrawKeyCount(int remainingKeys, int totalKeys);

	// 入力デバイスの検出
	void DetectInputDevice();

	// 操作ガイドの表示
	void DrawControlGuide();

	void TutorialPos(Vector3 playerPos);

	// ポーズ画面の表示
	void DrawPause();
	void DrawPauseRestart();
	void DrawPauseSelect();

private:
	// HP表示関連
	Sprite* hpBarBgSprite_ = nullptr;   // HP背景
	Sprite* hpBarFillSprite_ = nullptr; // HPバー 
	Sprite* hpIconSprite_ = nullptr;    // HPアイコン
	Sprite* hpTextSprite_ = nullptr;    // HPテキスト

	// 鍵表示関連
	static const int MAX_KEYS = 10;     // 最大表示可能な鍵の数
	Sprite* keyIcons_[MAX_KEYS] = {};   // 鍵アイコンの配列
	Sprite* No_keyIcons_[MAX_KEYS] = {}; // 持ってない鍵アイコンの配列
	int currentTotalKeys_ = 0;          // 現在のステージの鍵の総数

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

	Sprite* tutorial = nullptr;

	bool isTutorialEnd = false;
	bool isTextureEnd[6] = { false };//次の説明で変れないように

	// ポーズ画面
	Sprite* pauseRestartSprite_ = nullptr;
	Sprite* pauseSelectSprite_ = nullptr;
	Sprite* pauseSprite_ = nullptr;
};