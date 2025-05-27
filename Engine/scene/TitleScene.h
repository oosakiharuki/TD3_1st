#pragma once
#include "Sprite.h"
#include "Framework.h"
#include "IScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "FadeManager.h"
#include "Audio.h"

class TitleScene : public IScene{
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
private:
	Sprite* sprite;
	Sprite* backGround;
	
	Sprite* bottonSprite;

	XINPUT_STATE state = {}, preState = {}; // 初期化を追加	
	///決定した時
	bool isDecision = false;

	// オーディオ
	Audio* audio_ = nullptr;
	SoundData titleBGM_;
	SoundData decisionSound_;

	// アニメーション用
	float animationTime_ = 0.0f;
	float buttonBlinkTimer_ = 0.0f;
};