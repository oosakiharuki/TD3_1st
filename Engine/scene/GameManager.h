#pragma once

#include <memory>
#include "IScene.h"
#include "GameScene.h"     // ゲーム
#include "TitleScene.h"    // タイトル
#include "LoadingScene.h"  // ロード画面

class GameManager {
public:
	GameManager();
	~GameManager();

	void Initialize();
	void Update();
	void Draw();
	void Finalize();

private:

	void SceneChange(int prev, int current);//シーン入れ替え

	//ステージの最大数
	static const uint32_t SceneNum = 3; // ロード画面を追加したので3に増やす

	IScene* sceneArr_[SceneNum];

	int currentSceneNo_;//現在シーン
	int prevSceneNo_;//前シーン

	 Player* player_ = nullptr;

    // 敵いろいろ
    std::vector<Enemy*> enemies_;
    std::vector<CannonEnemy*> cannonEnemies_;
    std::vector<SpringEnemy*> springEnemies_;

};