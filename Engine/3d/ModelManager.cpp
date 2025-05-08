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