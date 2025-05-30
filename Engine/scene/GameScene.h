#pragma once

// エンジン基本
#include "Framework.h"
#include "IScene.h"
#include "MyMath.h"

// 2D/3D描画
#include "Sprite.h"
#include "Object3d.h"
#include "particle.h"
#include "Camera.h"

// オーディオ
#include "Audio.h"

// ゲームオブジェクト
#include "Player.h"
#include "Enemy.h"
#include "CannonEnemy.h"
#include "SpringEnemy.h"
#include "Block.h"
#include "GhostBlock.h"
#include "Door.h"
#include "Goal.h"
#include "Key.h"
#include "Skydome.h"

// ローダー/マネージャー
#include "MapLoader.h"
#include "EnemyLoader.h"
#include "UIManager.h"

// ゲームデータ
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

	static void HandlePauseSelection(int selection);

private:
	// プライベートメンバ関数
	void AddObstacle(std::vector<std::vector<AABB>>& allObstacles, const Vector3& min, const Vector3& max);
	void LoadStage(std::string objFile);
	void UpdateStageAABB();
	void UpdateImGui();

	// ステージ管理
	int currentStage_ = 0;
	Object3d* stage = nullptr;
	std::vector<std::vector<AABB>> allObstacles_;

	// カメラ関連
	Camera* camera_ = nullptr;
	Vector3 cameraRotate = { 0.0f,0.0f,0.0f };
	Vector3 cameraTranslate = { 0.0f,0.0f,-15.0f };

	// ゲームオブジェクト
	Player* player_ = nullptr;
	Skydome* skydome_ = nullptr;

	// ローダー/マネージャー
	MapLoader* mapLoader_ = nullptr;
	EnemyLoader* enemyLoader_ = nullptr;
	UIManager* uiManager = nullptr;

	// オーディオ関連
	Audio* audio_ = nullptr;
	SoundData BGMSound;
	int stageBGMHandle_ = 0;
	int stageBGMID_ = -1;

	// ゲーム状態管理
	bool isClear = false;
	bool isOver = false;
	bool isPaused_ = false;
	int pauseCount_ = 1;
	//SE
	SoundData selectSound_;

	// 入力関連
	float longPress = 1.0f;
	const float RestartTimer = 1.0f;
	XINPUT_STATE state = {}, preState = {};

	// その他
	WorldTransform worldTransform_;
	uint32_t textureHandle = 0;
	std::stringstream Command;

	// ImGui用デバッグ変数
	struct ObjectRotations {
		struct {
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
		} doorRotation;
		
		int lastModifiedDoorId = -1;
		float lastModifiedTime = 0.0f;
	};
	ObjectRotations objectRotations_;
};
