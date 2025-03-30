#pragma once

#include "Sprite.h"
#include "Object3d.h"
#include "particle.h" 
#include "Audio.h"
#include "MyMath.h"
#include "Framework.h"
#include "IScene.h"

class GameScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
private:


	Camera* camera = nullptr;
	Vector3 cameraRotate = { 0.0f,0.0f,0.0f };
	Vector3 cameraTranslate = { 0.0f,0.0f,-15.0f };

	//ゲームループ終了(while文から抜ける)
	bool isRequst = false;

	XINPUT_STATE state,preState;
};