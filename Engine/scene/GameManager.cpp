#include "GameManager.h"

GameManager::GameManager() {
	sceneArr_[Title] = new TitleScene();
	sceneArr_[Loading] = nullptr; // ロードシーンは必要時に作成

	prevSceneNo_ = 0;
	currentSceneNo_ = Title;
}

GameManager::~GameManager() {
	sceneArr_[currentSceneNo_]->Finalize();
	delete sceneArr_[currentSceneNo_];
}

void GameManager::SceneChange(int prev, int current) {

	//前のシーンの解放
	sceneArr_[prev]->Finalize();
	delete sceneArr_[prev];
	sceneArr_[prev] = nullptr;

	//scene_ = current;
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
	sceneArr_[currentSceneNo_]->Finalize();
	delete sceneArr_[currentSceneNo_];
	sceneArr_[currentSceneNo_] = nullptr;
	for (int i = 0; i < SceneNum; i++) {
		if (sceneArr_[i]) {
			sceneArr_[i]->Finalize();
			delete sceneArr_[i];
			sceneArr_[i] = nullptr;
		}
	}
}