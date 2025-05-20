#include "GameScene.h"
#include "ParticleNumber.h"
#include "ImGuiManager.h"
#include <filesystem>

GameScene::GameScene() {}

GameScene::~GameScene() {}

void GameScene::Finalize() {
	delete camera_;
	delete player_;
	delete mapLoader_;
	delete enemyLoader_;
	delete stage;
	delete skydome_;

	delete uiManager;

	allObstacles_.clear();
	ParticleManager::GetInstance()->ParticleListReset();//パーテイクルのコンテナをリセット
}

void GameScene::Initialize() {

	currentStage_ = GameData::selectedStage;
	
	// デバッグ出力 - GameSceneの初期化時のステージ番号
	{
		std::string initDebugMsg = "GameScene Initialize: Stage " + std::to_string(currentStage_) + "\n";
		OutputDebugStringA(initDebugMsg.c_str());
	}

	camera_ = new Camera();
	
	Object3dCommon::GetInstance()->SetDefaultCamera(camera_);
	ParticleCommon::GetInstance()->SetDefaultCamera(camera_);

	worldTransform_.Initialize();

	// Playerの生成と初期化
	player_ = new Player();
	player_->Init(camera_);

	stage = new Object3d();
	stage->Initialize();
	stage->SetModelFile("stage" + std::to_string(currentStage_));

	// 天球の生成
	skydome_ = new Skydome();
	// 天球の初期化
	skydome_->Initialize();

	// MapLoaderの初期化
	mapLoader_ = new MapLoader();
	std::string objectsFile = "resource/objects" + std::to_string(currentStage_) + ".csv";
	if (mapLoader_->LoadMapData(objectsFile)) {
		mapLoader_->CreateObjects(player_);
	}

	Vector3 StartPosition;
	//各ステージのプレイヤー初期位置
	StartPosition = PlayerPosition::stage[currentStage_];

	player_->SetPosition(StartPosition);

	// 障害物情報の読み込み
	LoadStage("resource/Object/stage" + std::to_string(currentStage_) + "/stage" + std::to_string(currentStage_) + ".obj");

	// EnemyLoaderの生成と初期化
	enemyLoader_ = new EnemyLoader();
	std::string enemiesFile = "resource/enemies" + std::to_string(currentStage_) + ".csv";
	// CSVから敵の情報を読み込み
	if (enemyLoader_->LoadEnemyData(enemiesFile)) {
		// 敵を生成
		enemyLoader_->CreateEnemies(player_, allObstacles_);
	}

	// 各種敵リストをプレイヤーに設定
	player_->SetGhostEnemies(enemyLoader_->GetGhostList());

	// キャノン敵への参照をプレイヤーに設定
	player_->SetCannonEnemies(enemyLoader_->GetCannonEnemyList());

	// バネ敵への参照をプレイヤーに設定
	player_->SetSpringEnemies(enemyLoader_->GetSpringEnemyList());

	// プレイヤーにブロックリストを設定（更新: 単一ブロックではなくリスト全体を渡す）
	const std::vector<Block*>& blocks = mapLoader_->GetBlockList();
	player_->SetBlocks(blocks);

	// キャノン敵にもブロックリストを設定
	for (CannonEnemy* cannon : enemyLoader_->GetCannonEnemyList()) {
		cannon->SetBlocks(blocks);
	}

	const std::vector<GhostBlock*>& ghostBlocks = mapLoader_->GetGhostBlockList();
	player_->SetGhostBlocks(ghostBlocks);


	const std::vector<Door*>& doors = mapLoader_->GetDoorList();
	player_->SetDoor(doors);


	// 障害物リストを Player にセット
	for (const auto& obstacles : allObstacles_) {
		player_->SetObstacleList(obstacles);
	}

	// プレイヤーにGoalへの参照を設定
	if (mapLoader_ && mapLoader_->GetGoal()) {
		player_->SetGoal(mapLoader_->GetGoal());
	}

	uiManager = new UIManager();
	uiManager->Initialize();

	// 現在のステージに応じたBGM読み込みと再生
	audio_ = Audio::GetInstance();
	
	// BGMファイル名の作成
	std::string bgmFile;
	
	// ステージ番号に基づいてBGMを切り替え
	switch (currentStage_) {
	case 1:
		bgmFile = "sound/stage1.wav";
		break;
	case 2:
		bgmFile = "sound/stage2.wav";
		break;
	case 3:
		bgmFile = "sound/stage3.wav";
		break;
	case 4:
		bgmFile = "sound/stage4.wav"; // stage4.wavがない場合の代替
		break;
	case 5:
		bgmFile = "sound/stage5.wav"; // stage5.wavがない場合の代替
		break;
	case 6:
		bgmFile = "sound/stage7.wav"; // stage6.wavがない場合の代替
		break;
	case 7:
		bgmFile = "sound/stage7.wav"; // stage7.wavがない場合の代替
		break;
	default:
		bgmFile = "sound/stage1.wav"; // デフォルトはステージ1のBGM
		break;
	}
	
	// デバッグ出力 - 現在のステージとBGMファイル
	{
		std::string bgmDebugMsg = "BGM Load: Stage " + std::to_string(currentStage_) + ", File: " + bgmFile + "\n";
		OutputDebugStringA(bgmDebugMsg.c_str());
	}
	
	BGMSound = audio_->LoadWave(bgmFile.c_str());
	audio_->SoundPlayWave(BGMSound, 0.25f, true);

	//パーテイクルのメモリ対策でリセット
	ParticleNum::number = 0;

	//前のシーンのボタン情報の取得
	state = Input::GetInstance()->GetState();
	preState = Input::GetInstance()->GetPreState();
}

void GameScene::Update() {


	Input::GetInstance()->GetJoystickState(0, state);
	Input::GetInstance()->GetJoystickStatePrevious(0, preState);


	// ポーズのトグル（ESCキーまたはStartボタン）
	if (Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
		isPaused_ = !isPaused_;
	}

	// ImGuiの更新
	UpdateImGui();

	if (Input::GetInstance()->TriggerKey(DIK_F1)) { //シーンが切り替わる
		audio_->StopWave(BGMSound);
		sceneNo = Title;
	}

	// ポーズ中はUIのみ更新し、ゲーム処理をスキップ
	if (isPaused_) {
		uiManager->Update(); // 必要ならポーズ画面のUI更新

		if (Input::GetInstance()->TriggerKey(DIK_W)) {
			pauseCount_--;
			if (pauseCount_ < 1) pauseCount_ = 1;
		}
		if (Input::GetInstance()->TriggerKey(DIK_S)) {
			pauseCount_++;
			if (pauseCount_ > 2) pauseCount_ = 2;
		}

		if (pauseCount_ == 1 && Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			Finalize();
			audio_->StopWave(BGMSound);
			Initialize();
			isPaused_ = !isPaused_;
		}

		if (pauseCount_ == 2 && Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			audio_->StopWave(BGMSound);
			sceneNo = Select;
		}

		if (Input::GetInstance()->TriggerKey(DIK_F1)) {
			audio_->StopWave(BGMSound);
			sceneNo = GameClear;
		}

		if (Input::GetInstance()->TriggerKey(DIK_F2)) {
			audio_->StopWave(BGMSound);
			sceneNo = GameOver;
		}

		return;
	}

	// リスタート処理
	if (Input::GetInstance()->PushKey(DIK_R) || ((state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) && (preState.Gamepad.wButtons & XINPUT_GAMEPAD_Y))) {
		longPress -= 1.0f / 60.0f;
	}
	else {
		longPress = RestartTimer;
	}
	// 0になった時リスタート / 押しなおさないと更新されなくする
	if (longPress < 0 && longPress > -0.017f) {
		Finalize();
		audio_->StopWave(BGMSound);
		Initialize();
	}


	// MapLoaderが管理するGoalの状態をチェック
	if (mapLoader_ && mapLoader_->GetGoal() && mapLoader_->GetGoal()->IsClear()) {
		audio_->StopWave(BGMSound);
		sceneNo = GameClear;
		return; // ゴールクリア状態なら更新処理をスキップ
	}

	// プレイヤーのHPが0になった場合、GameOverSceneに移行
	if (player_->GetHp() <= 0) {
		audio_->StopWave(BGMSound);
		sceneNo = GameOver; // GameOverSceneに遷移
	}

	player_->Update();

	// EnemyLoaderの更新
	if (enemyLoader_) {
		enemyLoader_->Update();
	}

	// MapLoaderの更新
	if (mapLoader_) {
		mapLoader_->Update();
	}

	player_->DrawUI();
	skydome_->Update();

	// 　↓　ゴールしたら1と2ステージループするようになってる、切り替え処理2を消すとステージ3に進む

	//if (mapLoader_ && mapLoader_->IsDoorOpened()) {
	//	// 次のステージ番号を計算
	//	int nextStage = currentStage_ + 1;
	//	GameData::selectedStage += 1;

	//	// 次のステージに応じてプレイヤーの座標を設定
	//	Vector3 newPosition;
	//	newPosition = PlayerPosition::stage[nextStage];

	//	// プレイヤーの座標を変更
	//	player_->SetPosition(newPosition);

	//	// ステージを切り替え
	//	ChangeStage(nextStage);
	//}

	camera_->Update();

	if (GameData::selectedStage == 0) {
		uiManager->TutorialPos(player_->GetWorldPosition());
	}

	uiManager->Update();
}

void GameScene::Draw() {

	//スプライト描画処理(背景用)
	SpriteCommon::GetInstance()->Command();


	//モデル描画処理
	Object3dCommon::GetInstance()->Command();

	skydome_->Draw();

	stage->Draw(worldTransform_);

	player_->Draw();

	// EnemyLoaderで読み込んだ敵の描画
	if (enemyLoader_) {
		enemyLoader_->Draw();
	}

	// MapLoaderで読み込んだオブジェクト（鍵、ドア、ブロック、ゴール）の描画
	if (mapLoader_) {
		mapLoader_->Draw();
	}



	//パーティクル描画処理
	ParticleCommon::GetInstance()->Command();


	player_->DrawP();
	
	for(CannonEnemy* it : enemyLoader_->GetCannonEnemyList()){
		it->DrawP();
	}

	for (Block* block : mapLoader_->GetBlockList()) {
		block->DrawP();
	}

	if (mapLoader_) {
		mapLoader_->DrawP();
	}

	//スプライト描画処理(UI用)
	SpriteCommon::GetInstance()->Command();
	if (mapLoader_) {
		mapLoader_->Draw2D();
	}

	// プレイヤーのHPを表示
	uiManager->Draw(player_->GetHp());

	// 鍵の数を表示
	if (mapLoader_) {
		// 鍵の総数を数える
		int totalKeys = 0;
		int remainingKeys = 0;

		const auto& keys = mapLoader_->GetKeys();
		if (!keys.empty()) {
			totalKeys = static_cast<int>(keys.size());
			
			// 残りの鍵の数を数える
			remainingKeys = totalKeys; // 初期値は全部の鍵
			for (const auto* key : keys) {
				if (key && key->IsKeyObtained()) {
					remainingKeys--; // 取得済みの鍵は残数から減らす
				}
			}

			// 鍵の情報をUIManagerに渡して表示
			uiManager->DrawKeyCount(remainingKeys, totalKeys);
		}
	}

	if (isPaused_) {
		uiManager->DrawPause();

		if (pauseCount_ == 1) {
			uiManager->DrawPauseRestart();  // pause_restartを描画
		}
		else if (pauseCount_ == 2) {
			uiManager->DrawPauseSelect();   // pause_selectを描画
		}
	}
}


void GameScene::ChangeStage(int nextStage) {
	allObstacles_.clear();
	Command.str("");
	Command.clear();

	// 前のステージのBGMを停止
	audio_->StopWave(BGMSound);

	currentStage_ = nextStage;
	
	// 新しいステージのBGMを読み込んで再生
	std::string bgmFile;
	
	// ステージ番号に基づいてBGMを切り替え
	switch (currentStage_) {
	case 1:
		bgmFile = "sound/stage1.wav";
		break;
	case 2:
		bgmFile = "sound/stage2.wav";
		break;
	case 3:
		bgmFile = "sound/stage3.wav";
		break;
	case 4:
		bgmFile = "sound/stage1.wav"; // stage4.wavがない場合の代替
		break;
	case 5:
		bgmFile = "sound/stage2.wav"; // stage5.wavがない場合の代替
		break;
	case 6:
		bgmFile = "sound/stage3.wav"; // stage6.wavがない場合の代替
		break;
	case 7:
		bgmFile = "sound/stage1.wav"; // stage7.wavがない場合の代替
		break;
	default:
		bgmFile = "sound/stage1.wav"; // デフォルトはステージ1のBGM
		break;
	}
	
	// デバッグ出力 - 現在のステージとBGMファイル
	{
		std::string changeDebugMsg = "BGM Change: Stage " + std::to_string(currentStage_) + ", File: " + bgmFile + "\n";
		OutputDebugStringA(changeDebugMsg.c_str());
	}
	
	BGMSound = audio_->LoadWave(bgmFile.c_str());
	audio_->SoundPlayWave(BGMSound, 0.25f, true);

	//前ステージの削除
	delete stage;
	stage = nullptr;
	stage = new Object3d();
	stage->Initialize();
	stage->SetModelFile("stage" + std::to_string(currentStage_));

	// **プレイヤーと敵の障害物リストをクリア**
	player_->ClearObstacleList();
	if (enemyLoader_) {
		for (auto& enemy : enemyLoader_->GetGhostList()) {
			enemy->ClearObstacleList();
		}
	}

	// 新しいオブジェクトデータをロード
	if (mapLoader_) {
		std::string objectsFile = "resource/objects" + std::to_string(currentStage_) + ".csv";
		mapLoader_->ChangeStage(currentStage_, player_);
		if (mapLoader_->LoadMapData(objectsFile)) {
			mapLoader_->CreateObjects(player_);
		}
	}

	// **新しい障害物データをロード**
	std::string stageFile = "resource/Object/stage" + std::to_string(currentStage_) + "/stage" + std::to_string(currentStage_) + ".obj";
	LoadStage(stageFile);

	// バネ
	for (auto& springEnemy : enemyLoader_->GetSpringEnemyList()) {
		springEnemy->ClearObstacleList();
	}

	// **プレイヤーに新しい障害物リストを設定**
	for (const auto& obstacles : allObstacles_) {
		player_->SetObstacleList(obstacles);
	}

	// **敵の当たり判定も再設定**
	if (enemyLoader_) {
		delete enemyLoader_;
	}
	enemyLoader_ = new EnemyLoader();

	std::string enemiesFile = "resource/enemies" + std::to_string(currentStage_) + ".csv";
	if (enemyLoader_->LoadEnemyData(enemiesFile)) {
		enemyLoader_->CreateEnemies(player_, allObstacles_);
	}

	// 敵の当たり判定リストを再設定
	for (auto& enemy : enemyLoader_->GetGhostList()) {
		for (const auto& obstacles : allObstacles_) {
			enemy->SetObstacleList(obstacles);
		}
	}

	// プレイヤーに敵リストを設定
	player_->SetGhostEnemies(enemyLoader_->GetGhostList());
	player_->SetSpringEnemies(enemyLoader_->GetSpringEnemyList());

	// キャノン敵への参照をプレイヤーに設定
	player_->SetCannonEnemies(enemyLoader_->GetCannonEnemyList());

	// プレイヤーにブロックリストを設定（更新：単一ブロックではなくリスト全体を渡す）
	const std::vector<Block*>& blocks = mapLoader_->GetBlockList();
	player_->SetBlocks(blocks);

	// キャノン敵にもブロックリストを設定
	for (CannonEnemy* cannon : enemyLoader_->GetCannonEnemyList()) {
		cannon->SetBlocks(blocks);
	}

	// プレイヤーにゴーストブロックの追加
	const std::vector<GhostBlock*>& ghostBlocks = mapLoader_->GetGhostBlockList();
	player_->SetGhostBlocks(ghostBlocks);

	// プレイヤーにGoalへの参照を再設定
	if (mapLoader_ && mapLoader_->GetGoal()) {
		player_->SetGoal(mapLoader_->GetGoal());
	}
}

// AddObstacle、LoadStage、UpdateStageAABBメソッドはそのまま以前の実装を使用
void GameScene::AddObstacle(std::vector<std::vector<AABB>>& allObstacles, const Vector3& min, const Vector3& max) {
	AABB obstacle;
	obstacle.min = min;
	obstacle.max = max;
	if (allObstacles.empty() || allObstacles.back().size() >= 500) { // 100個の障害物を追加
		allObstacles.emplace_back();
	}
	allObstacles.back().push_back(obstacle);
}

void GameScene::LoadStage(std::string objFile) {
	// ステージデータの読み込み
	std::ifstream file;
	file.open(objFile);
	assert(file.is_open());

	// Commandをリセット
	Command.str("");
	Command.clear();

	Command << file.rdbuf();
	file.close();

	// 障害物データをクリア
	allObstacles_.clear();

	UpdateStageAABB();

	player_->ClearObstacleList(); // 古い障害物リストを削除
	// 新しい障害物リスト
	for (const auto& obstacles : allObstacles_) {
		player_->SetObstacleList(obstacles);
	}

	//たまにバグる
	//if (enemyLoader_) {
	//	for (auto& enemy : enemyLoader_->GetEnemyList()) {
	//		enemy->ClearObstacleList();
	//		for (const auto& obstacles : allObstacles_) {
	//			enemy->SetObstacleList(obstacles);
	//		}
	//	}
	//}
}

void GameScene::UpdateImGui() {
#ifdef _DEBUG
	// 経過時間を更新(ハイライト表示用)
	if (objectRotations_.lastModifiedTime > 0) {
		objectRotations_.lastModifiedTime -= 1.0f / 60.0f; // 1秒間ハイライト表示
	}

	// ImGuiウィンドウを作成
	ImGui::Begin("Object Rotations");

	// ドアの回転を調整
	if (ImGui::CollapsingHeader("Doors")) {
		// 全てのドアのXYZ軸回転を制御するスライダー
		ImGui::Text("Global Door Rotation Controls:");
		bool anyRotationChanged = false;
		
		// X軸回転を調整
		if (ImGui::SliderFloat("X Rotation (All Doors)", &objectRotations_.doorRotation.x, 0.0f, 360.0f)) {
			// 全てのドアに適用
			const std::vector<Door*>& doors = mapLoader_->GetDoorList();
			for (Door* door : doors) {
				if (door) {
					door->SetRotateX(objectRotations_.doorRotation.x);
				}
			}
			anyRotationChanged = true;
		}

		// Y軸回転を調整
		if (ImGui::SliderFloat("Y Rotation (All Doors)", &objectRotations_.doorRotation.y, 0.0f, 360.0f)) {
			// 全てのドアに適用
			const std::vector<Door*>& doors = mapLoader_->GetDoorList();
			for (Door* door : doors) {
				if (door) {
					door->SetRotateY(objectRotations_.doorRotation.y);
				}
			}
			anyRotationChanged = true;
		}

		// Z軸回転を調整
		if (ImGui::SliderFloat("Z Rotation (All Doors)", &objectRotations_.doorRotation.z, 0.0f, 360.0f)) {
			// 全てのドアに適用
			const std::vector<Door*>& doors = mapLoader_->GetDoorList();
			for (Door* door : doors) {
				if (door) {
					door->SetRotateZ(objectRotations_.doorRotation.z);
				}
			}
			anyRotationChanged = true;
		}

		if (anyRotationChanged) {
			// 全てのドアの変更をハイライトする
			objectRotations_.lastModifiedDoorId = -1; // -1は全ドアを意味する
			objectRotations_.lastModifiedTime = 1.0f; // 1秒間ハイライト
		}

		ImGui::Separator();
		ImGui::Text("Individual Door Rotation Controls:");

		// 各ドアの個別回転
		const std::vector<Door*>& doors = mapLoader_->GetDoorList();
		for (int i = 0; i < doors.size(); i++) {
			if (doors[i]) {
				// ハイライトカラーを設定
				bool isHighlighted = (objectRotations_.lastModifiedDoorId == i) && 
								   (objectRotations_.lastModifiedTime > 0);

				// ハイライト表示
				if (isHighlighted) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.5f, 0.1f, 0.1f, 0.5f));
				}

				ImGui::PushID(i * 3);
				ImGui::Text("Door %d:", i+1);
				
				// 現在の回転値を取得
				Vector3 rotation = doors[i]->GetRotation();
				
				// X軸回転
				bool rotXChanged = false;
				ImGui::PushID(0);
				if (ImGui::SliderFloat("X##Door", &rotation.x, 0.0f, 360.0f)) {
					doors[i]->SetRotateX(rotation.x);
					objectRotations_.lastModifiedDoorId = i;
					objectRotations_.lastModifiedTime = 1.0f;
					rotXChanged = true;
				}
				ImGui::PopID();
				
				// Y軸回転
				bool rotYChanged = false;
				ImGui::PushID(1);
				if (ImGui::SliderFloat("Y##Door", &rotation.y, 0.0f, 360.0f)) {
					doors[i]->SetRotateY(rotation.y);
					objectRotations_.lastModifiedDoorId = i;
					objectRotations_.lastModifiedTime = 1.0f;
					rotYChanged = true;
				}
				ImGui::PopID();
				
				// Z軸回転
				bool rotZChanged = false;
				ImGui::PushID(2);
				if (ImGui::SliderFloat("Z##Door", &rotation.z, 0.0f, 360.0f)) {
					doors[i]->SetRotateZ(rotation.z);
					objectRotations_.lastModifiedDoorId = i;
					objectRotations_.lastModifiedTime = 1.0f;
					rotZChanged = true;
				}
				ImGui::PopID();

				// 変更があった場合はグローバル値も更新
				if (rotXChanged) objectRotations_.doorRotation.x = rotation.x;
				if (rotYChanged) objectRotations_.doorRotation.y = rotation.y;
				if (rotZChanged) objectRotations_.doorRotation.z = rotation.z;
				
				ImGui::PopID(); // i * 3

				// ハイライトを元に戻す
				if (isHighlighted) {
					ImGui::PopStyleColor(2);
				}
				
				// ドアの位置情報を表示
				AABB aabb = doors[i]->GetAABB();
				Vector3 center = {
					(aabb.min.x + aabb.max.x) * 0.5f,
					(aabb.min.y + aabb.max.y) * 0.5f,
					(aabb.min.z + aabb.max.z) * 0.5f
				};
				ImGui::Text("Position: X=%.2f Y=%.2f Z=%.2f", center.x, center.y, center.z);
				ImGui::Separator();
			}
		}
	}

	// TODO: 他のオブジェクトにもSetRotateX/Y/Zメソッドを追加する必要があります
	// 下記のクラスにはこれらのメソッドが実装されていません
	// Block, Key, GhostBlock, Enemy/GhostEnemy, CannonEnemy, SpringEnemy, Player, Goal

	ImGui::End();
#endif
}

void GameScene::UpdateStageAABB() {
	std::string line;
	uint32_t cornerNumber = 0;

	Vector3 max;
	Vector3 min;

	// AABB stageAABB;
	bool start = false;
	bool reverse = false;

	while (getline(Command, line)) {
		std::istringstream line_stream(line);

		std::string word;

		getline(line_stream, word, ' ');

		if (word.find("vn") == 0) {
			break; //vを読み取ったら終了
		}

		if (word.find("v") == 0) {
			cornerNumber++;
		}
		else {
			continue;
		}

		if (cornerNumber > 0) {

			getline(line_stream, word, ' ');
			float x = (float)std::atof(word.c_str());

			getline(line_stream, word, ' ');
			float y = (float)std::atof(word.c_str());

			getline(line_stream, word, ' ');
			float z = (float)std::atof(word.c_str());

			if (!start) {
				max = { x, y, z };
				min = { x, y, z };
				start = true;
			}
			else {

				// 前よりも大きいとき
				if (max.x <= x) {
					max.x = x;
				}
				// 前よりも小さいとき
				if (min.x > x) {
					min.x = x;
				}

				if (max.y <= y) {
					max.y = y;
				}

				if (min.y > y) {
					min.y = y;
				}

				if (max.z <= z) {
					max.z = z;
				}

				if (min.z > z) {
					min.z = z;
				}
			}
		}

		if (cornerNumber == 8) {
			if (!reverse) {
				AddObstacle(allObstacles_, min, max); // 結合した基盤となるobj
			}
			else {

				float minX;
				float maxX;
				maxX = -(max.x);
				minX = -(min.x);

				min.x = maxX;
				max.x = minX;
				AddObstacle(allObstacles_, { min.x, min.y, min.z }, { max.x, max.y, max.z }); // それ以外のすべてobj
			}

			cornerNumber = 0;
			start = false;
			reverse = true;
		}
	}
}