#pragma once

#include "Sprite.h"
#include "Object3d.h"
#include "particle.h" 
#include "Audio.h"
#include "MyMath.h"
#include "Framework.h"
#include "IScene.h"

#include "Camera.h"
#include "Block.h"
#include "CannonEnemy.h"
#include "Door.h"
#include "Enemy.h"
#include "EnemyLoader.h"
#include "GhostBlock.h"
#include "Goal.h"
#include "Key.h"
#include "MapLoader.h"
#include "Player.h"
#include "Skydome.h"
#include "SpringEnemy.h"

#include "UIManager.h"

#include "Audio.h"

#include "GameData.h"

class GameScene : public IScene {
public:
	// コンストラクタ
	GameScene();

	// デストラクタ
	~GameScene();

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

	// ステージを変更する
	void ChangeStage(int nextStage);

private:
	void AddObstacle(std::vector<std::vector<AABB>>& allObstacles, const Vector3& min, const Vector3& max);
	void LoadStage(std::string objFile);
	void UpdateStageAABB();

	// 現在のステージ番号
	int currentStage_ = 0;

	WorldTransform worldTransform_;
	Camera* camera_ = nullptr;
	Vector3 cameraRotate = { 0.0f,0.0f,0.0f };
	Vector3 cameraTranslate = { 0.0f,0.0f,-15.0f };
	Player* player_ = nullptr;

	// 障害物リスト
	std::vector<std::vector<AABB>> allObstacles_;

	uint32_t textureHandle = 0;

	Object3d* stage = nullptr;
	std::stringstream Command;

	// MapLoaderのインスタンス
	MapLoader* mapLoader_ = nullptr;

	// EnemyLoaderのインスタンス
	EnemyLoader* enemyLoader_ = nullptr;

	//Block* block_ = nullptr;
	//Model* modelBlock_ = nullptr;

	//GhostBlock* ghostBlock_ = nullptr;
	//Model* modelGhostBlock_ = nullptr;

	// 天球
	Skydome* skydome_ = nullptr;
	//Model* modelSkydome_ = nullptr;

	float longPress = 1.0f;
	const float RestartTimer = 1.0f;
	XINPUT_STATE state = {}, preState = {}; // 初期化を追加	


	// BGM関連
	Audio* audio_ = nullptr;
	SoundData BGMSound;
	int stageBGMHandle_ = 0;
	int stageBGMID_ = -1; // 再生IDを保持するために必要


	UIManager* uiManager = nullptr;

	//ゲームループ終了(while文から抜ける)
	bool isRequst = false;
};