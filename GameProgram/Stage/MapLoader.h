#pragma once
#include "Block.h"
#include "Door.h"
#include "Goal.h" 
#include "Key.h"
#include "Player.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// マップオブジェクトの種類を表す列挙型
enum class MapObjectType {
	Key,
	Door,
	Block,
	Goal // Goalタイプを追加
};

// CSVから読み込んだオブジェクトデータの構造体
struct MapObjectData {
	Vector3 position;
	MapObjectType type;
	int id = 0;
};

class MapLoader {
public:
	MapLoader();
	~MapLoader();

	// CSVファイルからマップデータを読み込む
	bool LoadMapData(const std::string& csvPath);

	// 読み込んだデータに基づいてオブジェクトを生成・初期化
	void CreateObjects(Player* player);

	// オブジェクトの更新
	void Update();

	// オブジェクトの描画
	void Draw();

	//オブジェクトのテクスチャ描画
	void Draw2D();

	void DrawSprites(ID3D12GraphicsCommandList* commandList);

	// 鍵とドアの相互参照を設定
	void SetupObjectReferences();

	// 鍵が取得されたかどうかを確認
	bool IsKeyObtained() const;

	// ドアが開いたかどうかを確認
	bool IsDoorOpened() const;

	// ステージ切り替え
	void ChangeStage(int stageNumber, Player* player);

	void GLoadStage(std::string objFile);

	// ブロックリストへのアクセス
	const std::vector<Block*>& GetBlockList() const { return blocks_; }

	// Goalへのアクセス（追加）
	Goal* GetGoal() const { return goal_ ? goal_ : nullptr; }

	// キーリストへのアクセサ
	const std::vector<Key*>& GetKeys() const { return keys_; }

	// ドアリストへのアクセサ
	const std::vector<Door*>& GetDoors() const { return doors_; }

private:
	DirectXCommon* dxCommon_ = nullptr;

	// 読み込んだマップオブジェクトデータのリスト
	std::vector<MapObjectData> mapObjectsData_;

	// 生成された鍵のリスト
	std::vector<Key*> keys_;

	// 生成されたドアのリスト
	std::vector<Door*> doors_;

	// 生成されたブロックのリスト
	std::vector<Block*> blocks_;

	// Goal（追加）
	Goal* goal_ = nullptr;
	Model* goalModel_ = nullptr;

	// CSVから座標とオブジェクトタイプを解析
	bool ParseCSVLine(const std::string& line, MapObjectData& data);

	// リソースのクリーンアップ
	void ClearResources();
};