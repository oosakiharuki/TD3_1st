#pragma once

#include <memory>
#include "IScene.h"
#include "GameScene.h"//geme
#include "TitleScene.h"//title

class GameManager {
public:
	GameManager();
	~GameManager();

	void Initialize();
	void Update();
	void Draw();
	//void Finalize();

private:
	
	void SceneChange(int prev,int current);//シーン入れ替え

	//ステージの最大数
	static const uint32_t SceneNum = 2;

	IScene* sceneArr_[SceneNum];

	int currentSceneNo_;//現在シーン
	int prevSceneNo_;//前シーン

};