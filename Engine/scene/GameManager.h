#pragma once

#include <memory>
#include "IScene.h"
#include "GameScene.h"//ゲーム
#include "TitleScene.h"//タイトル
#include "LoadingScene.h"//ローディング
#include "GameOverScene.h"//ゲームオーバー
#include "GameClearScene.h"//ゲームクリア

class GameManager {
public:
	GameManager();
	~GameManager();

	void Initialize();
	void Update();
	void Draw();
	//void Finalize();

private:

	void SceneChange(int prev, int current);//シーン入れ替え

	//ゲームオーバー,クリア追加のため+2
	static const uint32_t SceneNum = 5;

	IScene* sceneArr_[SceneNum];

	int currentSceneNo_;//現在シーン
	int prevSceneNo_;//前シーン

};