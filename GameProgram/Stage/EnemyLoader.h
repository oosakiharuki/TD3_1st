#pragma once
#include "CannonEnemy.h"
//#include "Enemy.h"
#include "GhostEnemy.h"
#include "Player.h"
#include "SpringEnemy.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// 敵オブジェクトの種類を表す列挙型
enum class EnemyType {
	Ghost, // イロオバケ敵
	Cannon, // 大砲敵
	Spring  // バネ敵
};

// CSVから読み込んだ敵データの構造体
struct EnemyData {
	Vector3 position;
	EnemyType type;
	ColorType colorType;
};

class EnemyLoader {
public:
	EnemyLoader();
	~EnemyLoader();

	// CSVファイルから敵データを読み込む
	bool LoadEnemyData(const std::string& csvPath);

	// 読み込んだデータに基づいて敵オブジェクトを生成・初期化
	void CreateEnemies(Player* player, const std::vector<std::vector<AABB>>& obstacles);

	// 敵の更新
	void Update();

	// 敵の描画
	void Draw();

	// 各種敵リストへのアクセッサ
	//const std::vector<Enemy*>& GetEnemyList() const { return enemies_; }
	const std::vector<GhostEnemy*>& GetGhostList() const { return ghostEnemies_; }
	const std::vector<CannonEnemy*>& GetCannonEnemyList() const { return cannonEnemies_; }
	const std::vector<SpringEnemy*>& GetSpringEnemyList() const { return springEnemies_; }

	// CSV保存機能
	bool SaveEnemyData(const std::string& csvPath);

	// ImGui編集用
	void UpdateImGui();

	// 現在のCSVファイルパス
	const std::string& GetCurrentCSVPath() const { return currentCSVPath_; }

	// 敵の動的追加
	void AddGhostEnemy(const Vector3& position, ColorType color);
	void AddCannonEnemy(const Vector3& position);
	void AddSpringEnemy(const Vector3& position);

	// 敵の削除
	void RemoveGhostEnemy(int index);
	void RemoveCannonEnemy(int index);
	void RemoveSpringEnemy(int index);

	// プレイヤー参照を保持
	void SetPlayer(Player* player) { player_ = player; }
	void SetObstacles(const std::vector<std::vector<AABB>>& obstacles) { obstacles_ = obstacles; }

private:
	// 読み込んだ敵データのリスト
	std::vector<EnemyData> enemyData_;

	// 生成された各種敵のリスト
	//std::vector<Enemy*> enemies_;
	std::vector<GhostEnemy*> ghostEnemies_;
	std::vector<CannonEnemy*> cannonEnemies_;
	std::vector<SpringEnemy*> springEnemies_;

	// CSVから座標と敵タイプを解析
	bool ParseCSVLine(const std::string& line, EnemyData& data);

	// リソースのクリーンアップ
	void ClearResources();

	// 現在のCSVファイルパス
	std::string currentCSVPath_;

	// プレイヤーと障害物の参照
	Player* player_ = nullptr;
	std::vector<std::vector<AABB>> obstacles_;
};