#include "MapLoader.h"
#include <algorithm>
#include <iostream>
#include "ImGuiManager.h"

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
		else if (token == "colorWall") {
			data.type = MapObjectType::ColorWall;
			// 敵タイプを読み込む	
			if (std::getline(iss, token, ',')) {
				if (token == "Blue") {
					data.color = ColorType::Blue;
				}
				else if (token == "Green") {
					data.color = ColorType::Green;
				}
				else if (token == "Red") {
					data.color = ColorType::Red;
				}
				else {
					return false; // 未知の敵タイプ
				}
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
			else {
				return false;
			}
		}
		else if (token == "goal") { // goalタイプを追加
			data.type = MapObjectType::Goal;
		}
		else if (token == "tile") {
			data.type = MapObjectType::Tile;

			// MoveTileのパラメータを読み込む
			bool isPresetSpecified = false;

			// 次のパラメータを読み込む
			if (std::getline(iss, token, ',')) {
				// 文字列か数値かを判断
				bool isNumeric = true;
				try {
					data.moveSpeed = std::stof(token);
				}
				catch (...) {
					isNumeric = false;
				}

				if (!isNumeric) {
					// プリセット名として扱う
					data.movePreset = ParseTileMovementPreset(token);
					isPresetSpecified = true;

					// プリセットに基づいてパラメータを設定
					ApplyTileMovementPreset(data);
				}
			}
			else {
				// パラメータが指定されていない場合はデフォルト値
				data.movePreset = TileMovementPreset::Normal;
				ApplyTileMovementPreset(data);
				return true;
			}

			// プリセットが指定されていない場合（数値指定の場合）は残りのパラメータも読み込む
			if (!isPresetSpecified) {
				// moveRangeを読み込む
				if (std::getline(iss, token, ',')) {
					data.moveRange = std::stof(token);
				}
				else {
					// デフォルト値を設定
					data.moveRange = 15.0f;
				}

				// initialYを読み込む
				if (std::getline(iss, token, ',')) {
					data.initialY = std::stof(token);
				}
				else {
					// デフォルト値を設定
					data.initialY = 92.0f;
				}

				// カスタム設定としてマーク
				data.movePreset = TileMovementPreset::Custom;
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
		
		// ドアの回転角度も保存
		float rotationY = door->GetRotation().y * (180.0f / 3.14159265359f);
		if (abs(rotationY) > 0.01f) {
			file << "," << rotationY;
		}
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
		
		// カスタム設定の場合はパラメータも保存
		if (tile->GetSpeed() != 1.0f || tile->GetRange() != 15.0f || tile->GetInitialY() != 92.0f) {
			file << ",custom," 
				 << tile->GetSpeed() << ","
				 << tile->GetRange() << ","
				 << tile->GetInitialY();
		}
		file << "\n";
	}

	for (const auto& ghostBlock : ghostBlocks_) {
		file << ghostBlock->GetTranslation().x << ","
			 << ghostBlock->GetTranslation().y << ","
			 << ghostBlock->GetTranslation().z << ",colorwall,"
			 << (ghostBlock->GetColorType() == ColorType::Red ? "red" :
				 ghostBlock->GetColorType() == ColorType::Blue ? "blue" : "green") << "\n";
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
					Vector3 rotation = doors_[i]->GetRotation();
					rotation.y *= (180.0f / 3.14159265359f); // ラジアンから度に変換
					
					if (ImGui::DragFloat3("Position", &pos.x, 1.0f)) {
						doors_[i]->SetTranslation(pos);
					}
					if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.1f, 10.0f)) {
						doors_[i]->SetScale(scale);
					}
					if (ImGui::DragFloat("Rotation Y", &rotation.y, 1.0f, -180.0f, 180.0f)) {
						rotation.y *= (3.14159265359f / 180.0f); // 度からラジアンに変換
						doors_[i]->SetRotation(rotation);
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
					}
					if (ImGui::DragFloat("Range", &range, 1.0f, 1.0f, 100.0f)) {
						tiles_[i]->SetRange(range);
					}
					if (ImGui::DragFloat("Initial Y", &initialY, 1.0f)) {
						tiles_[i]->SetInitialY(initialY);
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