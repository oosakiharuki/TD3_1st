#pragma once  
#include "Sprite.h"
#include "Framework.h"
#include "IScene.h"  

class GameClearScene : public IScene {
public:
	void Initialize();
	void Update();
	void Draw();
	void Finalize();

private:
	Sprite* gameClearNextStage;
	Sprite* gameClearTitle;
	Sprite* backGround;
	XINPUT_STATE state = {}, preState = {}; // 初期化を追加	

	int gameClearCount_ = 1;
};

