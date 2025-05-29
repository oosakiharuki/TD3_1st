#include "MapLoader.h"
#include <algorithm>
#include <iostream>
#include "ImGuiManager.h"
#ifdef _DEBUG
#include <windows.h>
#endif

MapLoader::MapLoader() {}

MapLoader::~MapLoader() { ClearResources(); }

bool MapLoader::LoadMapData(const std::string& csvPath) {
	// 現在のCSVパスを保存
	currentCSVPath_ = csvPath;
	
	// 以前のデータをクリア
	mapObjectsData_.clear();

	// CSVファイルを開く
	std::ifstream file(csvPath);
	if (!file.is_open()) {
		return false;
	}

	std::string line;
	// ヘッダー行をスキップする場合はここでgetline(file, line);を追加

	// CSVの各行を読み込む
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == '#') // コメント行もスキップ
			continue;

		MapObjectData data;
		if (ParseCSVLine(line, data)) {
			mapObjectsData_.push_back(data);
		}
	}

	file.close();
	return true;
}

TileMovementPreset MapLoader::ParseTileMovementPreset(const std::string& presetStr) {
	// 大文字小文字を区別しない比較のため、小文字に変換する
	std::string lowerPreset = presetStr;
	std::transform(lowerPreset.begin(), lowerPreset.end(), lowerPreset.begin(),
		[](unsigned char c) { return std::tolower(c); });

	if (lowerPreset == "normal") return TileMovementPreset::Normal;
	if (lowerPreset == "slow") return TileMovementPreset::Slow;
	if (lowerPreset == "fast") return TileMovementPreset::Fast;
	if (lowerPreset == "smallrange" || lowerPreset == "small") return TileMovementPreset::SmallRange;
	if (lowerPreset == "widerange" || lowerPreset == "wide") return TileMovementPreset::WideRange;
	if (lowerPreset == "high") return TileMovementPreset::High;
	if (lowerPreset == "low") return TileMovementPreset::Low;
	if (lowerPreset == "custom") return TileMovementPreset::Custom;

	// 数値かどうか確認（数値ならカスタム設定として扱う）
	try {
		float value = std::stof(presetStr); // 戻り値を変数に格納
		return TileMovementPreset::Custom;
	}
	catch (...) {
		// デフォルト値として標準設定を返す
		std::cerr << "Unknown tile movement preset: " << presetStr << ". Using Normal instead." << std::endl;
		return TileMovementPreset::Normal;
	}
}

void MapLoader::ApplyTileMovementPreset(MapObjectData& data) {
	// プリセットに基づいてパラメータを設定
	switch (data.movePreset) {
	case TileMovementPreset::Normal:
		data.moveSpeed = 1.0f;
		data.moveRange = 15.0f;
		data.initialY = 92.0f;
		break;
	case TileMovementPreset::Slow:
		data.moveSpeed = 0.5f;
		data.moveRange = 15.0f;
		data.initialY = 92.0f;
		break;
	case TileMovementPreset::Fast:
		data.moveSpeed = 2.0f;
		data.moveRange = 15.0f;
		data.initialY = 92.0f;
		break;
	case TileMovementPreset::SmallRange:
		data.moveSpeed = 1.0f;
		data.moveRange = 5.0f;
		data.initialY = 92.0f;
		break;
	case TileMovementPreset::WideRange:
		data.moveSpeed = 1.0f;
		data.moveRange = 30.0f;
		data.initialY = 92.0f;
		break;
	case TileMovementPreset::High:
		data.moveSpeed = 1.0f;
		data.moveRange = 15.0f;
		data.initialY = 150.0f;
		break;
	case TileMovementPreset::Low:
		data.moveSpeed = 1.0f;
		data.moveRange = 15.0f;
		data.initialY = 50.0f;
		break;
	case TileMovementPreset::Custom:
		// カスタム設定の場合は、既に設定されている値を使用するため、何もしない
		break;
	}
}

bool MapLoader::ParseCSVLine(const std::string& line, MapObjectData& data) {
	std::istringstream iss(line);
	std::string token;

	// x座標を読み込む
	if (std::getline(iss, token, ',')) {
		data.position.x = std::stof(token);
	}
	else {
		return false;
	}

	// y座標を読み込む
	if (std::getline(iss, token, ',')) {
		data.position.y = std::stof(token);
	}
	else {
		return false;
	}

	// z座標を読み込む
	if (std::getline(iss, token, ',')) {
		data.position.z = std::stof(token);
	}
	else {
		return false;
	}

	// オブジェクトタイプを読み込む
	if (std::getline(iss, token, ',')) {
		if (token == "key") {
			data.type = MapObjectType::Key;
		}
		else if (token == "door") {
			data.type = MapObjectType::Door;

			// モデルの向き（normal_値）
			if (std::getline(iss, token, ',')) {
				data.rotate = std::stof(token);
			}
			else {
				data.rotate = 0.0f;
			}

			// 開閉角度
			if (std::getline(iss, token, ',')) {
				data.doorOpenAngle = std::stof(token);
			}
			else {
				data.doorOpenAngle = 90.0f; // デフォルト90度
			}

			// 回転速度
			if (std::getline(iss, token, ',')) {
				data.doorSpeed = std::stof(token);
			}
			else {
				data.doorSpeed = 2.0f; // デフォルト2.0度/フレーム
			}
		}
		else if (token == "block") {
			data.type = MapObjectType::Block;

			//ブロックのサイズ変更
			// x座標を読み込む
			if (std::getline(iss, token, ',')) {
				data.size.x = std::stof(token);
			}
			else {
				//しない場合は元々のサイズ
				data.size.x = 1.0f;
			}

			// y座標を読み込む
			if (std::getline(iss, token, ',')) {
				data.size.y = std::stof(token);
			}
			else {
				data.size.y = 1.0f;
			}

			// z座標を読み込む
			if (std::getline(iss, token, ',')) {
				data.size.z = std::stof(token);
			}
			else {
				data.size.z = 1.0f;
			}

		}
		else if (token == "colorwall" || token == "colorWall") {
			data.type = MapObjectType::ColorWall;
			// 色タイプを読み込む	
			if (std::getline(iss, token, ',')) {
				if (token == "blue" || token == "Blue") {
					data.color = ColorType::Blue;
				}
				else if (token == "green" || token == "Green") {
					data.color = ColorType::Green;
				}
				else if (token == "red" || token == "Red") {
					data.color = ColorType::Red;
				}
				else {
					return false; // 未知の色タイプ
				}
				
				// サイズを読み込む（オプション）
				if (std::getline(iss, token, ',')) {
					data.size.x = std::stof(token);
					if (std::getline(iss, token, ',')) {
						data.size.y = std::stof(token);
						if (std::getline(iss, token, ',')) {
							data.size.z = std::stof(token);
						} else {
							data.size.z = data.size.x; // デフォルト
						}
					} else {
						data.size.y = data.size.x; // デフォルト
						data.size.z = data.size.x;
					}
				} else {
					// デフォルトサイズを設定
					data.size = {10.0f, 10.0f, 10.0f};
				}
			}
			else {
				return false;
			}
		}
		else if (token == "goal") { // goalタイプを追加
			data.type = MapObjectType::Goal;
		}
		else if (token == "tile") {
			data.type = MapObjectType::Tile;

			// デフォルト値を設定
			data.moveSpeed = 1.0f;
			data.moveRange = 15.0f;
			data.initialY = 92.0f;

			// 次のパラメータを読み込む（customまたはプリセット名）
			if (std::getline(iss, token, ',')) {
				if (token == "custom") {
					// カスタムパラメータを読み込む
					data.movePreset = TileMovementPreset::Custom;
					
					// moveSpeedを読み込む
					if (std::getline(iss, token, ',')) {
						data.moveSpeed = std::stof(token);
					}
					
					// moveRangeを読み込む
					if (std::getline(iss, token, ',')) {
						data.moveRange = std::stof(token);
					}
					
					// initialYを読み込む
					if (std::getline(iss, token, ',')) {
						data.initialY = std::stof(token);
					}

#ifdef _DEBUG
					// デバッグ出力
					OutputDebugStringA(("MoveTile CSV read - Speed: " + std::to_string(data.moveSpeed) + 
						", Range: " + std::to_string(data.moveRange) + 
						", InitialY: " + std::to_string(data.initialY) + "\n").c_str());
#endif
				}
				else {
					// プリセット名として扱う
					data.movePreset = ParseTileMovementPreset(token);
					// プリセットに基づいてパラメータを設定
					ApplyTileMovementPreset(data);
				}
			}
			else {
				// パラメータが指定されていない場合はデフォルト値
				data.movePreset = TileMovementPreset::Normal;
			}
		}
		else {
			return false; // 未知のオブジェクトタイプ
		}
	}
	else {
		return false;
	}

	// オプションのIDフィールドを読み込む
	if (std::getline(iss, token, ',')) {
		// ID値が存在する場合、読み込む
		data.id = std::stoi(token);
	}
	else {
		// IDが指定されていない場合はデフォルト値0
		data.id = 0;
	}

	return true;
}

void MapLoader::CreateObjects(Player* player) {
	// プレイヤー参照を保存
	player_ = player;
	
	// 既存のオブジェクトをクリア
	ClearResources();

	// キーのIDカウンターを初期化
	int keyIdCounter = 0;

	// ドアのIDカウンターを初期化
	int doorIdCounter = 0;

	// ゴールを見つけたかどうかのフラグ
	bool foundGoal = false;

	// 読み込んだデータに基づいてオブジェクトを生成
	for (const auto& objectData : mapObjectsData_) {
		if (objectData.type == MapObjectType::Key) {
			Key* key = new Key();
			key->Init();
			key->SetPosition(objectData.position);
			key->SetPlayer(player);

			// CSVからIDが指定されている場合はそれを使用、そうでなければ自動採番
			if (objectData.id > 0) {
				key->SetKeyID(objectData.id);
			}
			else {
				key->SetKeyID(++keyIdCounter);
			}

			keys_.push_back(key);
		}
		else if (objectData.type == MapObjectType::Door) {
		Door* door = new Door();
		
		// IDの設定
		if (objectData.id > 0) {
		door->SetDoorID(objectData.id);
		} else {
		door->SetDoorID(doorIdCounter++);
		}
		
		// 初期化前にnormal_値を設定 - この順番が重要
		door->SetRotateY(objectData.rotate);
		
		// 開閉パラメータを設定
		door->SetDoorOpenParams(objectData.doorOpenAngle, objectData.doorSpeed);
		
		// ドアの初期化
		door->Init();
		door->SetPosition(objectData.position);
		 door->SetPlayer(player);
			doors_.push_back(door);
		}
		else if (objectData.type == MapObjectType::Block) {
			Block* block = new Block();
			block->Init();
			block->SetPosition(objectData.position);
			block->SetSize(objectData.size);
			blocks_.push_back(block);
		}
		else if (objectData.type == MapObjectType::ColorWall) {
			GhostBlock* ghostBlock = new GhostBlock();
			ghostBlock->Init();
			ghostBlock->SetPosition(objectData.position);
			ghostBlock->SetColor(objectData.color);
			ghostBlock->SetSize(objectData.size);
			ghostBlocks_.push_back(ghostBlock);
		}
		else if (objectData.type == MapObjectType::Goal) {
			// Goalの生成
			if (goal_) {
				delete goal_;
				goal_ = nullptr;
			}
			goal_ = new Goal();
			goal_->Init();
			goal_->SetPosition(objectData.position);
			foundGoal = true;
		}
		else if (objectData.type == MapObjectType::Tile) {
			MoveTile* tile = new MoveTile();
			tile->Init();
			tile->SetPosition(objectData.position);
			tile->SetPlayer(player);

			// CSVから読み込んだパラメータを設定
			tile->SetMoveSpeed(objectData.moveSpeed);
			tile->SetMoveRange(objectData.moveRange);
			tile->SetInitialY(objectData.initialY);
			
			// カスタム設定かどうかを設定
			tile->SetIsCustom(objectData.movePreset == TileMovementPreset::Custom);

#ifdef _DEBUG
			// デバッグ出力
			OutputDebugStringA(("MoveTile created - Speed: " + std::to_string(objectData.moveSpeed) + 
				", Range: " + std::to_string(objectData.moveRange) + 
				", InitialY: " + std::to_string(objectData.initialY) + "\n").c_str());
#endif

			tiles_.push_back(tile);
		}
	}

	// 鍵とドアの相互参照を設定
	SetupObjectReferences();
}


void MapLoader::SetupObjectReferences() {
	// すべてのドアに対して、すべての鍵への参照を設定
	for (auto* door : doors_) {
		// 鍵のリストを設定
		door->SetKeys(keys_);

		// 必要なキーの数をセット（デフォルトではすべてのキーが必要）
		door->SetRequiredKeyCount(static_cast<int>(keys_.size()));
	}
}

void MapLoader::Update() {
	// すべての鍵を更新
	for (auto* key : keys_) {
		key->Update();
	}

	// すべてのドアを更新
	for (auto* door : doors_) {
		door->Update();
	}

	// すべてのブロックを更新
	for (auto* block : blocks_) {
		block->Update();
	}

	// すべてのタイルを更新
	for (auto* tile : tiles_) {
		tile->Update();
	}

	//全てのゴーストブロックの更新
	for (auto* ghostBlock : ghostBlocks_) {
		ghostBlock->Update();
	}

	// Goalの更新
	if (goal_) {
		goal_->Update();
	}
}

void MapLoader::Draw() {
	// すべての鍵を描画
	for (auto* key : keys_) {
		key->Draw();
	}

	// すべてのドアを描画
	for (auto* door : doors_) {
		door->Draw();
	}

	// すべてのブロックを描画
	for (auto* block : blocks_) {
		block->Draw();
	}

	//全てのゴーストブロックの更新
	for (auto* ghostBlock : ghostBlocks_) {
		ghostBlock->Draw();
	}

	// Goalを描画（存在する場合のみ）
	if (goal_) {
		goal_->Draw();
	}

	// すべてのタイルを描画
	for (auto* tile : tiles_) {
		tile->Draw();
	}
}

void MapLoader::Draw2D() {
	if (goal_) {
		goal_->Text();
	}
}

void MapLoader::DrawP() {
	// すべての鍵を描画
	for (auto* key : keys_) {
		key->DrawP();
	}
}

// スプライトの描画
void MapLoader::DrawSprites([[maybe_unused]] ID3D12GraphicsCommandList* commandList) {
	// Goalのスプライト描画（ゴールクリア表示）
	if (goal_ && goal_->IsClear()) {
		goal_->Text();
	}
}

bool MapLoader::IsKeyObtained() const {
	// いずれかの鍵が取得されているかチェック
	return std::any_of(keys_.begin(), keys_.end(), [](const Key* key) { return key->IsKeyObtained(); });
}

bool MapLoader::IsDoorOpened() const {
	// いずれかのドアが開いているかチェック
	return std::any_of(doors_.begin(), doors_.end(), [](const Door* door) { return door->IsDoorOpened(); });
}

void MapLoader::ClearResources() {
	// 鍵のリソースを解放
	for (auto* key : keys_) {
		delete key;
	}
	keys_.clear();

	// ドアのリソースを解放
	for (auto* door : doors_) {
		delete door;
	}
	doors_.clear();

	// ブロックのリソースを解放
	for (auto* block : blocks_) {
		delete block;
	}
	blocks_.clear();

	//全てのゴーストブロックの更新
	for (auto* ghostBlock : ghostBlocks_) {
		delete ghostBlock;
	}
	ghostBlocks_.clear();

	// Goalのリソースを解放
	if (goal_) {
		delete goal_;
		goal_ = nullptr;
	}

	// タイルのリソースを解放
	for (auto* tile : tiles_) {
		delete tile;
	}
	tiles_.clear();
}

void MapLoader::ChangeStage(int stageNumber, Player* player) {
	// 既存のオブジェクトを削除
	ClearResources();

	// ステージ番号に応じたCSVファイル名を決定
	std::string csvPath = "resource/objects" + std::to_string(stageNumber) + ".csv";

	// マップデータを読み込み
	if (LoadMapData(csvPath)) {
		// 新しいオブジェクトを作成
		CreateObjects(player);
	}
	else {
		std::cerr << "Failed to load map data: " << csvPath << std::endl;
	}
}

bool MapLoader::SaveMapData(const std::string& csvPath) {
	std::ofstream file(csvPath);
	if (!file.is_open()) {
		return false;
	}

	// 各オブジェクトをCSV形式で書き込む
	for (const auto& key : keys_) {
		file << key->GetWorldTransform().translation_.x << ","
			 << key->GetWorldTransform().translation_.y << ","
			 << key->GetWorldTransform().translation_.z << ",key\n";
	}

	for (const auto& door : doors_) {
		file << door->GetTranslation().x << ","
			 << door->GetTranslation().y << ","
			 << door->GetTranslation().z << ",door";
		
		// ドアの回転角度を常に保存（度単位で）
		float rotationY = door->GetRotation().y;
		file << "," << rotationY;
		
		file << "\n";
	}

	for (const auto& block : blocks_) {
		file << block->GetTranslation().x << ","
			 << block->GetTranslation().y << ","
			 << block->GetTranslation().z << ",block,"
			 << block->GetScale().x << ","
			 << block->GetScale().y << ","
			 << block->GetScale().z << "\n";
	}

	for (const auto& tile : tiles_) {
		file << tile->GetWorldTransform().translation_.x << ","
			 << tile->GetWorldTransform().translation_.y << ","
			 << tile->GetWorldTransform().translation_.z << ",tile";
		
		// カスタム設定の場合のみパラメータを保存
		if (tile->IsCustom()) {
			file << ",custom," 
				 << tile->GetSpeed() << ","
				 << tile->GetRange() << ","
				 << tile->GetInitialY();
		}
		// それ以外はデフォルト（プリセット）として保存
		
		file << "\n";
	}

	for (const auto& ghostBlock : ghostBlocks_) {
		file << ghostBlock->GetTranslation().x << ","
			 << ghostBlock->GetTranslation().y << ","
			 << ghostBlock->GetTranslation().z << ",colorwall,"
			 << (ghostBlock->GetColorType() == ColorType::Red ? "red" :
				 ghostBlock->GetColorType() == ColorType::Blue ? "blue" : "green") << ","
			 << ghostBlock->GetSize().x << ","
			 << ghostBlock->GetSize().y << ","
			 << ghostBlock->GetSize().z << "\n";
	}

	if (goal_) {
		file << goal_->GetWorldTransform().translation_.x << ","
			 << goal_->GetWorldTransform().translation_.y << ","
			 << goal_->GetWorldTransform().translation_.z << ",goal\n";
	}

	file.close();
	return true;
}

void MapLoader::UpdateImGui() {
#ifdef _DEBUG
	if (ImGui::Begin("Map Object Editor")) {
		ImGui::Text("Current CSV: %s", currentCSVPath_.c_str());
		ImGui::Separator();

		// 新規オブジェクト配置UI
		if (ImGui::CollapsingHeader("Add New Objects")) {
			static Vector3 newPos = {0.0f, 0.0f, 0.0f};
			static Vector3 newSize = {10.0f, 10.0f, 10.0f};
			static float newRotation = 0.0f;
			static int colorIndex = 0;
			const char* colorNames[] = {"Red", "Blue", "Green"};
			
			// MoveTile用パラメータ
			static float tileSpeed = 1.0f;
			static float tileRange = 15.0f;
			
			ImGui::DragFloat3("Position", &newPos.x, 1.0f);
			ImGui::Separator();
			
			if (ImGui::Button("Add Key", ImVec2(150, 30))) {
				AddKey(newPos);
			}
			ImGui::SameLine();
			if (ImGui::Button("Add Goal", ImVec2(150, 30))) {
				AddGoal(newPos);
			}
			
			ImGui::DragFloat("Door Rotation", &newRotation, 1.0f, -180.0f, 180.0f);
			if (ImGui::Button("Add Door", ImVec2(150, 30))) {
				AddDoor(newPos, newRotation);
			}
			
			ImGui::DragFloat3("Block Size", &newSize.x, 1.0f, 1.0f, 100.0f);
			if (ImGui::Button("Add Block", ImVec2(150, 30))) {
				AddBlock(newPos, newSize);
			}
			
			ImGui::Text("Moving Tile Parameters:");
			ImGui::DragFloat("Tile Speed", &tileSpeed, 0.1f, 0.1f, 10.0f);
			ImGui::DragFloat("Tile Range", &tileRange, 1.0f, 1.0f, 100.0f);
			if (ImGui::Button("Add Moving Tile", ImVec2(150, 30))) {
				AddTile(newPos, tileSpeed, tileRange);
			}
			
			ImGui::Combo("Ghost Block Color", &colorIndex, colorNames, 3);
			ImGui::DragFloat3("Ghost Block Size", &newSize.x, 1.0f, 1.0f, 100.0f);
			if (ImGui::Button("Add Ghost Block", ImVec2(150, 30))) {
				ColorType color = colorIndex == 0 ? ColorType::Red : 
								  colorIndex == 1 ? ColorType::Blue : ColorType::Green;
				AddGhostBlock(newPos, color, newSize);
			}
		}

		ImGui::Separator();

		// 鍵の編集
		if (ImGui::CollapsingHeader("Keys")) {
			for (size_t i = 0; i < keys_.size(); i++) {
				ImGui::PushID(static_cast<int>(i));
				if (ImGui::TreeNode(("Key " + std::to_string(i)).c_str())) {
					Vector3 pos = keys_[i]->GetWorldTransform().translation_;
					Vector3 scale = keys_[i]->GetWorldTransform().scale_;
					
					if (ImGui::DragFloat3("Position", &pos.x, 1.0f)) {
						keys_[i]->GetWorldTransform().translation_ = pos;
					}
					if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.1f, 10.0f)) {
						keys_[i]->GetWorldTransform().scale_ = scale;
					}
					
					// 削除ボタン
					if (ImGui::Button(("Delete##Key" + std::to_string(i)).c_str())) {
						RemoveKey(static_cast<int>(i));
						ImGui::TreePop();
						ImGui::PopID();
						break; // イテレータが無効になるため
					}
					
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}

		// ドアの編集
		if (ImGui::CollapsingHeader("Doors")) {
			for (size_t i = 0; i < doors_.size(); i++) {
				ImGui::PushID(static_cast<int>(i + 1000));
				if (ImGui::TreeNode(("Door " + std::to_string(i)).c_str())) {
					Vector3 pos = doors_[i]->GetTranslation();
					Vector3 scale = doors_[i]->GetScale();
					Vector3 rotation = doors_[i]->GetRotation(); // すでに度単位
					
					if (ImGui::DragFloat3("Position", &pos.x, 1.0f)) {
						doors_[i]->SetTranslation(pos);
					}
					if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.1f, 10.0f)) {
						doors_[i]->SetScale(scale);
					}
					if (ImGui::DragFloat("Rotation Y", &rotation.y, 1.0f, -180.0f, 180.0f)) {
						doors_[i]->SetRotateY(rotation.y); // SetRotateYは度単位を受け取る
					}
					
					// 削除ボタン
					if (ImGui::Button(("Delete##Door" + std::to_string(i)).c_str())) {
						RemoveDoor(static_cast<int>(i));
						ImGui::TreePop();
						ImGui::PopID();
						break;
					}
					
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}

		// ブロックの編集
		if (ImGui::CollapsingHeader("Blocks")) {
			for (size_t i = 0; i < blocks_.size(); i++) {
				ImGui::PushID(static_cast<int>(i + 2000));
				if (ImGui::TreeNode(("Block " + std::to_string(i)).c_str())) {
					Vector3 pos = blocks_[i]->GetTranslation();
					Vector3 scale = blocks_[i]->GetScale();
					
					if (ImGui::DragFloat3("Position", &pos.x, 1.0f)) {
						blocks_[i]->SetTranslation(pos);
					}
					if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.1f, 100.0f)) {
						blocks_[i]->SetScale(scale);
					}
					
					// 削除ボタン
					if (ImGui::Button(("Delete##Block" + std::to_string(i)).c_str())) {
						RemoveBlock(static_cast<int>(i));
						ImGui::TreePop();
						ImGui::PopID();
						break;
					}
					
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}

		// タイルの編集
		if (ImGui::CollapsingHeader("Moving Tiles")) {
			for (size_t i = 0; i < tiles_.size(); i++) {
				ImGui::PushID(static_cast<int>(i + 3000));
				if (ImGui::TreeNode(("Tile " + std::to_string(i)).c_str())) {
					Vector3 pos = tiles_[i]->GetWorldTransform().translation_;
					Vector3 scale = tiles_[i]->GetWorldTransform().scale_;
					
					if (ImGui::DragFloat3("Position", &pos.x, 1.0f)) {
						tiles_[i]->GetWorldTransform().translation_ = pos;
						// 位置が変更されたらinitialYも更新
						tiles_[i]->SetInitialY(pos.y);
					}
					if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.1f, 10.0f)) {
						tiles_[i]->GetWorldTransform().scale_ = scale;
					}
					
					// MoveTileのパラメータも編集可能に
					float speed = tiles_[i]->GetSpeed();
					float range = tiles_[i]->GetRange();
					float initialY = tiles_[i]->GetInitialY();
					
					if (ImGui::DragFloat("Speed", &speed, 0.1f, 0.1f, 10.0f)) {
						tiles_[i]->SetSpeed(speed);
						tiles_[i]->SetIsCustom(true); // 編集したらカスタム設定にする
					}
					if (ImGui::DragFloat("Range", &range, 1.0f, 1.0f, 100.0f)) {
						tiles_[i]->SetRange(range);
						tiles_[i]->SetIsCustom(true); // 編集したらカスタム設定にする
					}
					if (ImGui::DragFloat("Initial Y", &initialY, 1.0f)) {
						tiles_[i]->SetInitialY(initialY);
						tiles_[i]->SetIsCustom(true); // 編集したらカスタム設定にする
					}
					
					// 削除ボタン
					if (ImGui::Button(("Delete##Tile" + std::to_string(i)).c_str())) {
						RemoveTile(static_cast<int>(i));
						ImGui::TreePop();
						ImGui::PopID();
						break;
					}
					
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}

		// ゴーストブロックの編集
		if (ImGui::CollapsingHeader("Ghost Blocks")) {
			for (size_t i = 0; i < ghostBlocks_.size(); i++) {
				ImGui::PushID(static_cast<int>(i + 4000));
				if (ImGui::TreeNode(("Ghost Block " + std::to_string(i)).c_str())) {
					Vector3 pos = ghostBlocks_[i]->GetTranslation();
					Vector3 size = ghostBlocks_[i]->GetSize();
					
					if (ImGui::DragFloat3("Position", &pos.x, 1.0f)) {
						ghostBlocks_[i]->SetPosition(pos);
					}
					
					if (ImGui::DragFloat3("Scale", &size.x, 0.1f, 0.1f, 100.0f)) {
						ghostBlocks_[i]->SetSize(size);
					}
					
					// 色の選択
					const char* colorNames[] = {"Red", "Blue", "Green"};
					int currentColor = ghostBlocks_[i]->GetColorType() == ColorType::Red ? 0 :
									   ghostBlocks_[i]->GetColorType() == ColorType::Blue ? 1 : 2;
					if (ImGui::Combo("Color", &currentColor, colorNames, 3)) {
						ColorType newColor = currentColor == 0 ? ColorType::Red :
										     currentColor == 1 ? ColorType::Blue : ColorType::Green;
						ghostBlocks_[i]->SetColor(newColor);
					}
					
					// 削除ボタン
					if (ImGui::Button(("Delete##GhostBlock" + std::to_string(i)).c_str())) {
						RemoveGhostBlock(static_cast<int>(i));
						ImGui::TreePop();
						ImGui::PopID();
						break;
					}
					
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}

		// ゴールの編集
		if (goal_ && ImGui::CollapsingHeader("Goal")) {
			Vector3 pos = goal_->GetWorldTransform().translation_;
			Vector3 scale = goal_->GetWorldTransform().scale_;
			
			if (ImGui::DragFloat3("Position", &pos.x, 1.0f)) {
				goal_->GetWorldTransform().translation_ = pos;
			}
			if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.1f, 10.0f)) {
				goal_->GetWorldTransform().scale_ = scale;
			}
		}

		ImGui::Separator();
		
		// 保存ボタン
		if (ImGui::Button("Save to CSV", ImVec2(200, 40))) {
			if (SaveMapData(currentCSVPath_)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "Saved successfully!");
			} else {
				ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to save!");
			}
		}
		
		ImGui::End();
	}
#endif
}

// オブジェクトの追加メソッド
void MapLoader::AddKey(const Vector3& position) {
	Key* key = new Key();
	key->Init();
	key->SetPosition(position);
	if (player_) {
		key->SetPlayer(player_);
	}
	key->SetKeyID(static_cast<int>(keys_.size()) + 1);
	keys_.push_back(key);
}

void MapLoader::AddDoor(const Vector3& position, float rotation) {
	Door* door = new Door();
	door->SetDoorID(static_cast<int>(doors_.size()));
	door->Init();
	door->SetPosition(position);
	door->SetRotateY(rotation);
	if (player_) {
		door->SetPlayer(player_);
	}
	door->SetKeys(keys_); // 現在のすべての鍵を関連付け
	doors_.push_back(door);
}

void MapLoader::AddBlock(const Vector3& position, const Vector3& size) {
	Block* block = new Block();
	block->Init();
	block->SetPosition(position);
	block->SetSize(size);
	blocks_.push_back(block);
}

void MapLoader::AddTile(const Vector3& position, float speed, float range) {
	MoveTile* tile = new MoveTile();
	tile->Init();
	tile->SetPosition(position);
	tile->SetInitialY(position.y); // 明示的にinitialYを設定
	tile->SetSpeed(speed);
	tile->SetRange(range);
	tile->SetIsCustom(true); // 新規追加はカスタム設定
	if (player_) {
		tile->SetPlayer(player_);
	}
	tiles_.push_back(tile);
}

void MapLoader::AddGhostBlock(const Vector3& position, ColorType color, const Vector3& size) {
	GhostBlock* ghostBlock = new GhostBlock();
	ghostBlock->SetColor(color);
	ghostBlock->Init();
	ghostBlock->SetPosition(position);
	ghostBlock->SetSize(size);
	ghostBlocks_.push_back(ghostBlock);
}

void MapLoader::AddGoal(const Vector3& position) {
	// すでにゴールが存在する場合は削除
	if (goal_) {
		delete goal_;
		goal_ = nullptr;
	}
	
	goal_ = new Goal();
	goal_->Init();
	goal_->SetPosition(position);
}

// オブジェクトの削除メソッド
void MapLoader::RemoveKey(int index) {
	if (index >= 0 && index < static_cast<int>(keys_.size())) {
		delete keys_[index];
		keys_.erase(keys_.begin() + index);
		
		// キーIDを再割り当て
		for (size_t i = 0; i < keys_.size(); i++) {
			keys_[i]->SetKeyID(static_cast<int>(i) + 1);
		}
	}
}

void MapLoader::RemoveDoor(int index) {
	if (index >= 0 && index < static_cast<int>(doors_.size())) {
		delete doors_[index];
		doors_.erase(doors_.begin() + index);
		
		// ドアIDを再割り当て
		for (size_t i = 0; i < doors_.size(); i++) {
			doors_[i]->SetDoorID(static_cast<int>(i));
		}
	}
}

void MapLoader::RemoveBlock(int index) {
	if (index >= 0 && index < static_cast<int>(blocks_.size())) {
		delete blocks_[index];
		blocks_.erase(blocks_.begin() + index);
	}
}

void MapLoader::RemoveTile(int index) {
	if (index >= 0 && index < static_cast<int>(tiles_.size())) {
		delete tiles_[index];
		tiles_.erase(tiles_.begin() + index);
	}
}

void MapLoader::RemoveGhostBlock(int index) {
	if (index >= 0 && index < static_cast<int>(ghostBlocks_.size())) {
		delete ghostBlocks_[index];
		ghostBlocks_.erase(ghostBlocks_.begin() + index);
	}
}