#pragma once
#include "Block.h"
#include "Door.h"
#include "Goal.h" 
#include "MoveTile.h"
#include "Key.h"
#include "Player.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

// マップオブジェクトの種類を表す列挙型
enum class MapObjectType {
	Key,
	Door,
	Block,
	Tile,
	ColorWall,
	Goal // Goalタイプを追加
};

// MoveTileの動きプリセット
enum class TileMovementPreset {
	Normal,     // 標準設定
	Slow,       // ゆっくり動く
	Fast,       // 速く動く
	SmallRange, // 小さい範囲で動く
	WideRange,  // 広い範囲で動く
	High,       // 高い位置
	Low,        // 低い位置
	Custom      // カスタム設定（数値指定）
};

// CSVから読み込んだオブジェクトデータの構造体
struct MapObjectData {
	Vector3 position;
	float rotate;
	MapObjectType type;
	ColorType color;
	Vector3 size;
	int id = 0;
	// MoveTile用のパラメータを追加
	float moveSpeed = 1.0f;
	float moveRange = 15.0f;
	float initialY = 92.0f;
	TileMovementPreset movePreset = TileMovementPreset::Normal;
	// Doorの回転パラメータ
	float doorOpenAngle = 90.0f; // ドアの開閉角度（度数法）
	float doorSpeed = 2.0f;     // ドアの回転速度（度/フレーム）
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

	//オブジェクトのパーティクル描画
	void DrawP();

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

	// ゴーストブロックリストへのアクセス
	const std::vector<GhostBlock*>& GetGhostBlockList() const { return ghostBlocks_; }

	// ドアリストのアクセス
	const std::vector<Door*>& GetDoorList() const { return doors_; }

	// 鍵リストへのアクセス - 追加
	const std::vector<Key*>& GetKeys() const { return keys_; }

	// Goalへのアクセス（追加）
	Goal* GetGoal() const { return goal_ ? goal_ : nullptr; }

	// CSV保存機能
	bool SaveMapData(const std::string& csvPath);

	// ImGui編集用
	void UpdateImGui();

	// 現在のCSVファイルパス
	const std::string& GetCurrentCSVPath() const { return currentCSVPath_; }

	// MoveTileリストへのアクセス
	std::vector<MoveTile*>& GetTiles() { return tiles_; }

	// オブジェクトの動的追加・削除
	void AddKey(const Vector3& position);
	void AddDoor(const Vector3& position, float rotation = 0.0f);
	void AddBlock(const Vector3& position, const Vector3& size);
	void AddTile(const Vector3& position, float speed = 1.0f, float range = 15.0f);
	void AddGhostBlock(const Vector3& position, ColorType color, const Vector3& size = Vector3(10.0f, 10.0f, 10.0f));
	void AddGoal(const Vector3& position);

	// オブジェクトの削除
	void RemoveKey(int index);
	void RemoveDoor(int index);
	void RemoveBlock(int index);
	void RemoveTile(int index);
	void RemoveGhostBlock(int index);

	// プレイヤーの参照を保持
	void SetPlayer(Player* player) { player_ = player; }

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

	// 生成されたタイルのリスト
	std::vector<MoveTile*> tiles_;

	// 生成されたゴーストブロックのリスト
	std::vector<GhostBlock*> ghostBlocks_;

	// Goal（追加）
	Goal* goal_ = nullptr;
	Model* goalModel_ = nullptr;

	// CSVから座標とオブジェクトタイプを解析
	bool ParseCSVLine(const std::string& line, MapObjectData& data);

	// 文字列からTileMovementPresetを解析
	TileMovementPreset ParseTileMovementPreset(const std::string& presetStr);

	// プリセットに基づいてMoveTileのパラメータを設定
	void ApplyTileMovementPreset(MapObjectData& data);

	// リソースのクリーンアップ
	void ClearResources();

	// 現在のCSVファイルパス
	std::string currentCSVPath_;

	// プレイヤー参照
	Player* player_ = nullptr;
};