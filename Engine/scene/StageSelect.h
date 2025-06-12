#pragma once  
#include "Sprite.h"
#include "Framework.h"
#include "IScene.h"
#include "GameData.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Object3d.h"

#include "Audio.h"

class StageSelect : public IScene {
public:
	void Initialize();
	void Update();
	void Draw();
	void Finalize();

private:

	Sprite* backGround;
	Sprite* shadow;
	Sprite* StageSelectName;
	Sprite* stageNumber;
	Sprite* selectBar;
	Sprite* buttonSprite;

	XINPUT_STATE state = {}, preState = {}; // 初期化を追加	

	uint32_t stageNum = 0;
	float selectLT = 96;

	float steackCount = 0.0f;
	float steackMax = 0.25f;

	Object3d* stageObject_ = nullptr;
	
	WorldTransform worldTransform_;

	Camera* camera_ = nullptr;

	Vector3 cameraPosition = { -0.8f,1.5f,-4.0f };
	Vector3 cameraRotate = { 0.35f,0,0 };
	
	//SE
	Audio* audio_ = nullptr;
	SoundData selectSound_;
	SoundData decisionSound_;

	///決定した時
	bool isDecision = false;
};