#include "EnemyLoader.h"
#include <algorithm>

EnemyLoader::EnemyLoader() {}

EnemyLoader::~EnemyLoader() { ClearResources(); }

bool EnemyLoader::LoadEnemyData(const std::string& csvPath) {
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