#pragma once  
#include "Sprite.h"
#include "Framework.h"
#include "IScene.h"
#include "GameData.h"
#include "TextureManager.h"
#include "ModelManager.h"

class StageSelect : public IScene {
public:
	void Initialize();
	void Update();
	void Draw();
	void Finalize();

private:
	Sprite* sprite;
	Sprite* backGround;
	Sprite* stageNumber;
	XINPUT_STATE state = {}, preState = {}; // 初期化を追加	

	uint32_t stageNum = 0;
	float selectLT = 0;

	float steackCount = 0.0f;
	float steackMax = 0.25f;
};