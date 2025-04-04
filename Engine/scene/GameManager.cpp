#include "GameManager.h"

GameManager::GameManager() {
	sceneArr_[Title] = new TitleScene();
	sceneArr_[Loading] = nullptr; // ロードシーンは必要時に作成
	sceneArr_[Game] = nullptr;

	prevSceneNo_ = 0;
	currentSceneNo_ = Title;
}

GameManager::~GameManager() {
	// デストラクタでは単にFinalize()を呼び出す
	Finalize();
}

void GameManager::SceneChange(int prev, int current) {
	// 前のシーンの解放
	if (sceneArr_[prev]) {
		sceneArr_[prev]->Finalize();
		delete sceneArr_[prev];
		sceneArr_[prev] = nullptr;
	}

	// 新しいシーンの作成
	switch (current)
	{
	case Title:
		sceneArr_[current] = new TitleScene();
		break;
	case Loading:
		sceneArr_[current] = new LoadingScene();
		break;
	case Game:
		sceneArr_[current] = new GameScene();
		break;
	}
}

void GameManager::Initialize() {
	sceneArr_[currentSceneNo_]->Initialize();
}

void GameManager::Update() {
	prevSceneNo_ = currentSceneNo_;
	currentSceneNo_ = sceneArr_[currentSceneNo_]->GetSceneNo();

	if (prevSceneNo_ != currentSceneNo_) {
		SceneChange(prevSceneNo_, currentSceneNo_);
		sceneArr_[currentSceneNo_]->Initialize();
	}

	sceneArr_[currentSceneNo_]->Update();
}

void GameManager::Draw() {
	sceneArr_[currentSceneNo_]->Draw();
}

void GameManager::Finalize() {
	// ダブルデリートを避けるための修正
	// 各シーンは一度だけ解放する
	for (int i = 0; i < SceneNum; i++) {
		if (sceneArr_[i]) {
			sceneArr_[i]->Finalize();
			delete sceneArr_[i];
			sceneArr_[i] = nullptr;
		}
	}
}