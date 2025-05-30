#pragma once  
#include "Sprite.h"
#include "Framework.h"
#include "IScene.h"  

class GameClearScene : public IScene {
public:
	void Initialize()override;
	void Update()override;
	void Draw()override;
	void Finalize()override;

private:
	Sprite* stageClear;
	Sprite* Arrow;
	float arrowPosY = 0.0f;
	bool stopSteck = false;
	bool allClear = false;

	Sprite* backGround;
	XINPUT_STATE state = {}, preState = {}; // 初期化を追加	

	int gameClearCount_ = 1;
	bool isDecision = false;

	//SE
	Audio* audio_ = nullptr;
	SoundData selectSound_;
	SoundData decisionSound_;

	float animationTime_ = 0.0f;
};

