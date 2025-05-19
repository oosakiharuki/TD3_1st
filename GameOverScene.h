#pragma once  
#include "Sprite.h"
#include "Framework.h"
#include "IScene.h"  

class GameOverScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

private:
	Sprite* gameOverContinue;
	Sprite* gameOverTitle;
	XINPUT_STATE state = {}, preState = {}; // 初期化を追加	

	int gameOverCount_ = 1;
};