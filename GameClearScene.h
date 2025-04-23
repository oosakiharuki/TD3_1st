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
	Sprite* sprite;
	Sprite* backGround;
	XINPUT_STATE state = {}, preState = {}; // 初期化を追加	
};

