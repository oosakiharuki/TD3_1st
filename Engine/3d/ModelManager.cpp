#include "ModelManager.h"
#include <vector>
#include <string>

ModelManager* ModelManager::instance = nullptr;


ModelManager* ModelManager::GetInstance() {
	if (instance == nullptr) {
		instance = new ModelManager;
	}
	return instance;
}

void ModelManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void ModelManager::Initialize(DirectXCommon* dxCommon) {
	modelCommon = new ModelCommon;
	modelCommon->Initialize(dxCommon);
}

void ModelManager::LoadModel(const std::string& filePath) {
	if (models.contains(filePath)) {
		return;
	}

	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(modelCommon, "resource", filePath);//model,file名,OBJ本体

	models.insert(std::make_pair(filePath, std::move(model)));
}

// 非同期でモデルを読み込む
void ModelManager::LoadModelAsync(const std::string& filePath, std::function<void()> callback) {
    // スレッドを使ってモデルをロード
    std::thread loadThread([this, filePath, callback]() {
        // ロック範囲を最小限に
        {
            std::lock_guard<std::mutex> lock(modelMutex);
            
            // 既に読み込まれている場合は何もしない
            if (models.contains(filePath)) {
                if (callback) callback();
                return;
            }
        }
        
        try {
            // モデル生成
            std::unique_ptr<Model> model = std::make_unique<Model>();
            model->Initialize(modelCommon, "resource", filePath);
            
            // モデルをマップに追加するためのロック
            {
                std::lock_guard<std::mutex> lock(modelMutex);
                if (!models.contains(filePath)) {
                    models.insert(std::make_pair(filePath, std::move(model)));
                }
            }
            
            OutputDebugStringA(("モデル非同期読み込み完了: " + filePath + "\n").c_str());
        }
        catch (std::exception& e) {
            OutputDebugStringA(("モデル読み込みエラー: " + std::string(e.what()) + "\n").c_str());
        }
        
        // コールバック実行
        if (callback) callback();
    });
    
    // スレッドをデタッチ
    loadThread.detach();
}

// プリロードリストにモデルを追加
void ModelManager::AddToPreloadList(const std::vector<std::string>& filePathList) {
    std::lock_guard<std::mutex> lock(modelMutex);
    preloadList.insert(preloadList.end(), filePathList.begin(), filePathList.end());
}

// プリロードリストからモデルを読み込む (1回の呼び出しで1つのモデルを読み込む)
float ModelManager::PreloadModels() {
    if (preloadIndex >= preloadList.size()) {
        return 1.0f; // 全て読み込み完了
    }
    
    // 次に読み込むべきモデルパス
    std::string nextModel = preloadList[preloadIndex];
    
    // 既に読み込まれている場合はスキップ
    if (!CheckModelExist(nextModel)) {
        try {
            LoadModel(nextModel);
            OutputDebugStringA(("プリロードモデル読み込み: " + nextModel + "\n").c_str());
        }
        catch (std::exception& e) {
            OutputDebugStringA(("モデル読み込みエラー: " + std::string(e.what()) + "\n").c_str());
        }
    }
    
    // インデックスを進める
    preloadIndex++;
    
    // 進捗率を返す (0.0f～1.0f)
    return static_cast<float>(preloadIndex) / static_cast<float>(preloadList.size());
}

Model* ModelManager::FindModel(const std::string& filePath) {
	if(models.contains(filePath)){
		return models.at(filePath).get();
	}

	//ファイル一致なし
	return nullptr;
}

bool ModelManager::CheckModelExist(const std::string& filePath) {
	return models.contains(filePath);
}

void ModelManager::CheckAllModelsLoaded() {
	// ロードされたモデルの数を出力
	OutputDebugStringA(("ロードされたモデル数: " + std::to_string(models.size()) + "\n").c_str());

	// 基本的なモデルがロードされているかチェック
	const std::vector<std::string> essentialModels = {
		"player",
		"stage1",
		"cube",
		"EnemyGhost",
		"cannon"
	};

	for (const auto& model : essentialModels) {
		if (!CheckModelExist(model)) {
			OutputDebugStringA(("重要なモデルがロードされていません: " + model + "\n").c_str());
		}
	}
}