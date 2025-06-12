#include "EnemyLoader.h"
#include <algorithm>
#include "ImGuiManager.h"
#ifdef _DEBUG
#include <windows.h>
#endif

EnemyLoader::EnemyLoader() {}

EnemyLoader::~EnemyLoader() { ClearResources(); }

bool EnemyLoader::LoadEnemyData(const std::string& csvPath) {
	// 現在のCSVパスを保存
	currentCSVPath_ = csvPath;
	
	// 以前のデータをクリア
	enemyData_.clear();


	// CSVファイルを開く
	std::ifstream file(csvPath);
	if (!file.is_open()) {
		return false;
	}

	std::string line;
	// ヘッダー行をスキップする場合はここでgetline(file, line);を追加

	// CSVの各行を読み込む
	while (std::getline(file, line)) {
		if (line.empty())
			continue;

		EnemyData data;
		if (ParseCSVLine(line, data)) {
			enemyData_.push_back(data);
		}
	}

	file.close();
	return true;
}

bool EnemyLoader::ParseCSVLine(const std::string& line, EnemyData& data) {
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

	// 敵タイプを読み込む
	if (std::getline(iss, token, ',')) {
		if (token == "ghost") {
			data.type = EnemyType::Ghost;
		}
		else if (token == "cannon") {
			data.type = EnemyType::Cannon;
		}
		else if (token == "spring") {
			data.type = EnemyType::Spring;
		}
		else {
			return false; // 未知の敵タイプ
		}
	}
	else {
		return false;
	}

	if (data.type == EnemyType::Ghost) {
		// 敵タイプを読み込む
		if (std::getline(iss, token, ',')) {
			if (token == "Blue") {
				data.colorType = ColorType::Blue;
			}
			else if (token == "Green") {
				data.colorType = ColorType::Green;
			}
			else if (token == "Red") {
				data.colorType = ColorType::Red;
			}
			else {
				return false; // 未知の敵タイプ
			}
		}
		else {
			return false;
		}
	}
	return true;
}

void EnemyLoader::CreateEnemies(Player* player, const std::vector<std::vector<AABB>>& obstacles) {
	// プレイヤーと障害物の参照を保存
	player_ = player;
	obstacles_ = obstacles;
	
	// 既存の敵をクリア
	ClearResources();

	// 読み込んだデータに基づいて敵を生成
	for (const auto& data : enemyData_) {
		switch (data.type) {
		case EnemyType::Ghost: {
			GhostEnemy* ghost = new GhostEnemy();
			ghost->Init();
			ghost->SetPosition(data.position);
			ghost->SetTarget(player);
			ghost->SetColor(data.colorType);

			// 障害物リストを設定
			for (const auto& obstacleList : obstacles) {
				ghost->SetObstacleList(obstacleList);
			}

			ghostEnemies_.push_back(ghost);
			break;
		}
		case EnemyType::Cannon: {
			CannonEnemy* cannon = new CannonEnemy();
			cannon->Init();
			cannon->SetPosition(data.position);
			cannon->SetPlayer(player);

			// 障害物リストを設定
			for (const auto& obstacleList : obstacles) {
				cannon->SetObstacleList(obstacleList);
			}

			cannonEnemies_.push_back(cannon);
			break;
		}
		case EnemyType::Spring: {
			SpringEnemy* spring = new SpringEnemy();
			spring->Init();
			spring->SetPosition(data.position);
			spring->SetPlayer(player);

			// 障害物リストを設定
			for (const auto& obstacleList : obstacles) {
				spring->SetObstacleList(obstacleList);
			}

			springEnemies_.push_back(spring);
			break;
		}
		}
	}

	// すべてのゴーストに他のゴーストへの参照を設定
	if (!ghostEnemies_.empty()) {
		for (auto& ghost : ghostEnemies_) {
			ghost->SetOtherGhosts(&ghostEnemies_);

			// フィールド境界を適切に設定
			ghost->SetFieldBoundaries({ -150.0f, -50.0f, -150.0f }, { 150.0f, 100.0f, 150.0f });
		}
	}
}

void EnemyLoader::Update() {
	// 通常の敵の更新
	for (auto* ghost : ghostEnemies_) {
		ghost->Update();
	}

	// 大砲敵の更新
	for (auto* cannon : cannonEnemies_) {
		cannon->Update();
	}

	// バネ敵の更新
	for (auto* spring : springEnemies_) {
		spring->Update();
	}
}

void EnemyLoader::Draw() {
	// 通常の敵の描画
	for (auto* ghost : ghostEnemies_) {
		ghost->Draw();
	}

	// 大砲敵の描画
	for (auto* cannon : cannonEnemies_) {
		cannon->Draw();
	}

	// バネ敵の描画
	for (auto* spring : springEnemies_) {
		spring->Draw();
	}
}

void EnemyLoader::ClearResources() {
	// 通常の敵のリソースを解放
	for (auto* ghost : ghostEnemies_) {
		delete ghost;
	}
	ghostEnemies_.clear();

	// 大砲敵のリソースを解放
	for (auto* cannon : cannonEnemies_) {
		delete cannon;
	}
	cannonEnemies_.clear();

	// バネ敵のリソースを解放
	for (auto* spring : springEnemies_) {
		delete spring;
	}
	springEnemies_.clear();
}

bool EnemyLoader::SaveEnemyData(const std::string& csvPath) {
	std::ofstream file(csvPath);
	if (!file.is_open()) {
		return false;
	}

	// 各敵をCSV形式で書き込む
	for (const auto& ghost : ghostEnemies_) {
		file << ghost->GetPosition().x << ","
			 << ghost->GetPosition().y << ","
			 << ghost->GetPosition().z << ",ghost,"
			 << (ghost->GetColorType() == ColorType::Red ? "Red" :
				 ghost->GetColorType() == ColorType::Blue ? "Blue" : "Green") << "\n";
	}

	for (const auto& cannon : cannonEnemies_) {
		file << cannon->GetPosition().x << ","
			 << cannon->GetPosition().y << ","
			 << cannon->GetPosition().z << ",cannon\n";
	}

	for (const auto& spring : springEnemies_) {
		file << spring->GetPosition().x << ","
			 << spring->GetPosition().y << ","
			 << spring->GetPosition().z << ",spring\n";
	}

	file.close();
	return true;
}

void EnemyLoader::UpdateImGui() {
#ifdef _DEBUG
	bool isOpen = ImGui::Begin("Enemy Editor");
	
	if (isOpen) {
		ImGui::Text("Current CSV: %s", currentCSVPath_.c_str());
		ImGui::Separator();

		// 新規敵配置UI
		if (ImGui::CollapsingHeader("Add New Enemies")) {
			static Vector3 newPos = {0.0f, 0.0f, 0.0f};
			static int colorIndex = 0;
			const char* colorNames[] = {"Red", "Blue", "Green"};
			
			ImGui::DragFloat3("Position", &newPos.x, 1.0f);
			ImGui::Separator();
			
			// Ghost Enemy
			ImGui::Combo("Ghost Color", &colorIndex, colorNames, 3);
			if (ImGui::Button("Add Ghost Enemy", ImVec2(150, 30))) {
				ColorType color = colorIndex == 0 ? ColorType::Red : 
								  colorIndex == 1 ? ColorType::Blue : ColorType::Green;
				AddGhostEnemy(newPos, color);
			}
			
			// Cannon Enemy
			if (ImGui::Button("Add Cannon Enemy", ImVec2(150, 30))) {
				AddCannonEnemy(newPos);
			}
			
			// Spring Enemy
			if (ImGui::Button("Add Spring Enemy", ImVec2(150, 30))) {
				AddSpringEnemy(newPos);
			}
		}

		ImGui::Separator();

		// Ghost Enemies編集
		if (ImGui::CollapsingHeader("Ghost Enemies")) {
			for (size_t i = 0; i < ghostEnemies_.size(); i++) {
				ImGui::PushID(static_cast<int>(i));
				if (ImGui::TreeNode(("Ghost " + std::to_string(i)).c_str())) {
					Vector3 pos = ghostEnemies_[i]->GetPosition();
					
					if (ImGui::DragFloat3("Position", &pos.x, 1.0f)) {
						ghostEnemies_[i]->SetPosition(pos);
					}
					
					// 色の選択
					const char* colorNames[] = {"Red", "Blue", "Green"};
					int currentColor = ghostEnemies_[i]->GetColorType() == ColorType::Red ? 0 :
									   ghostEnemies_[i]->GetColorType() == ColorType::Blue ? 1 : 2;
					if (ImGui::Combo("Color", &currentColor, colorNames, 3)) {
						ColorType newColor = currentColor == 0 ? ColorType::Red :
											 currentColor == 1 ? ColorType::Blue : ColorType::Green;
						ghostEnemies_[i]->SetColor(newColor);
					}
					
					// 削除ボタン
					if (ImGui::Button(("Delete##Ghost" + std::to_string(i)).c_str())) {
						RemoveGhostEnemy(static_cast<int>(i));
						ImGui::TreePop();
						ImGui::PopID();
						break;
					}
					
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}

		// Cannon Enemies編集
		if (ImGui::CollapsingHeader("Cannon Enemies")) {
			for (size_t i = 0; i < cannonEnemies_.size(); i++) {
				ImGui::PushID(static_cast<int>(i + 1000));
				if (ImGui::TreeNode(("Cannon " + std::to_string(i)).c_str())) {
					Vector3 pos = cannonEnemies_[i]->GetPosition();
					
					if (ImGui::DragFloat3("Position", &pos.x, 1.0f)) {
						cannonEnemies_[i]->SetPosition(pos);
					}
					
					// 削除ボタン
					if (ImGui::Button(("Delete##Cannon" + std::to_string(i)).c_str())) {
						RemoveCannonEnemy(static_cast<int>(i));
						ImGui::TreePop();
						ImGui::PopID();
						break;
					}
					
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}

		// Spring Enemies編集
		if (ImGui::CollapsingHeader("Spring Enemies")) {
			for (size_t i = 0; i < springEnemies_.size(); i++) {
				ImGui::PushID(static_cast<int>(i + 2000));
				if (ImGui::TreeNode(("Spring " + std::to_string(i)).c_str())) {
					Vector3 pos = springEnemies_[i]->GetPosition();
					
					if (ImGui::DragFloat3("Position", &pos.x, 1.0f)) {
						springEnemies_[i]->SetPosition(pos);
					}
					
					// 削除ボタン
					if (ImGui::Button(("Delete##Spring" + std::to_string(i)).c_str())) {
						RemoveSpringEnemy(static_cast<int>(i));
						ImGui::TreePop();
						ImGui::PopID();
						break;
					}
					
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}

		ImGui::Separator();
		
		// 保存ボタン
		if (ImGui::Button("Save to CSV", ImVec2(200, 40))) {
			if (SaveEnemyData(currentCSVPath_)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "Saved successfully!");
			} else {
				ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed to save!");
			}
		}
	}
	
	ImGui::End();
#endif
}

// 敵の追加メソッド
void EnemyLoader::AddGhostEnemy(const Vector3& position, ColorType color) {
	GhostEnemy* ghost = new GhostEnemy();
	ghost->Init();
	ghost->SetPosition(position);
	ghost->SetColor(color);
	if (player_) {
		ghost->SetTarget(player_);
	}
	// 障害物リストを設定
	for (const auto& obstacleList : obstacles_) {
		ghost->SetObstacleList(obstacleList);
	}
	// フィールド境界を設定
	ghost->SetFieldBoundaries({ -150.0f, -50.0f, -150.0f }, { 150.0f, 100.0f, 150.0f });
	
	ghostEnemies_.push_back(ghost);
	
	// 新しく追加した後、全てのゴーストに他のゴーストへの参照を更新
	for (auto& g : ghostEnemies_) {
		g->SetOtherGhosts(&ghostEnemies_);
	}
}

void EnemyLoader::AddCannonEnemy(const Vector3& position) {
	CannonEnemy* cannon = new CannonEnemy();
	cannon->Init();
	cannon->SetPosition(position);
	if (player_) {
		cannon->SetPlayer(player_);
	}
	// 障害物リストを設定
	for (const auto& obstacleList : obstacles_) {
		cannon->SetObstacleList(obstacleList);
	}
	cannonEnemies_.push_back(cannon);
}

void EnemyLoader::AddSpringEnemy(const Vector3& position) {
	SpringEnemy* spring = new SpringEnemy();
	spring->Init();
	spring->SetPosition(position);
	if (player_) {
		spring->SetPlayer(player_);
	}
	// 障害物リストを設定
	for (const auto& obstacleList : obstacles_) {
		spring->SetObstacleList(obstacleList);
	}
	springEnemies_.push_back(spring);
}

// 敵の削除メソッド
void EnemyLoader::RemoveGhostEnemy(int index) {
	if (index >= 0 && index < static_cast<int>(ghostEnemies_.size())) {
		delete ghostEnemies_[index];
		ghostEnemies_.erase(ghostEnemies_.begin() + index);
		
		// 削除後、残りのゴーストに他のゴーストへの参照を更新
		for (auto& g : ghostEnemies_) {
			g->SetOtherGhosts(&ghostEnemies_);
		}
	}
}

void EnemyLoader::RemoveCannonEnemy(int index) {
	if (index >= 0 && index < static_cast<int>(cannonEnemies_.size())) {
		delete cannonEnemies_[index];
		cannonEnemies_.erase(cannonEnemies_.begin() + index);
	}
}

void EnemyLoader::RemoveSpringEnemy(int index) {
	if (index >= 0 && index < static_cast<int>(springEnemies_.size())) {
		delete springEnemies_[index];
		springEnemies_.erase(springEnemies_.begin() + index);
	}
}