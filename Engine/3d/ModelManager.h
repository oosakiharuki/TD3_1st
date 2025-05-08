#pragma once
#include <map>
#include <string>
#include <memory>
#include <mutex>
#include <future>
#include <vector>
#include <functional>
#include "Model.h"

class ModelManager{
public:
	static ModelManager* GetInstance();
	void Finalize();
	std::map<std::string, std::unique_ptr<Model>> models;

	void Initialize(DirectXCommon* dxCommon);

	// 同期的にモデルを読み込む
	void LoadModel(const std::string& filePath);
	
	// 非同期でモデルを読み込む
	void LoadModelAsync(const std::string& filePath, std::function<void()> callback = nullptr);
	
	// プリロードするモデルのリストを追加する
	void AddToPreloadList(const std::vector<std::string>& filePathList);
	
	// プリロードリストからモデルを読み込み、進行状況を返す (0.0f～1.0f)
	float PreloadModels();
	
	Model* FindModel(const std::string& filePath);

	// モデルが存在するかチェック
	bool CheckModelExist(const std::string& filePath);

	// 全てのモデルが正しくロードされているか確認
	void CheckAllModelsLoaded();

private:
	static ModelManager* instance;

	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(ModelManager&) = delete;
	ModelManager& operator=(ModelManager&) = delete;

	ModelCommon* modelCommon = nullptr;
	
	// 非同期ローディング用
	std::mutex modelMutex;
	std::vector<std::string> preloadList;
	size_t preloadIndex = 0;
};