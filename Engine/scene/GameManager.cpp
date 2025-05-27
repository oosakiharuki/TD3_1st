#include "GameManager.h"

GameManager::GameManager() {
	// 最初はローディングシーンから開始
	sceneArr_[Loading] = new LoadingScene();

	prevSceneNo_ = 0;
	currentSceneNo_ = Loading;
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
		sceneArr_[current] = new LoadingScene(); // 新しいローディングシーン
		break;
	case Select:
		sceneArr_[current] = new StageSelect();
		break;
	case Game:
		sceneArr_[current] = new GameScene();
		break;
	case GameClear:
		sceneArr_[current] = new GameClearScene();
		break;
	case GameOver:
		sceneArr_[current] = new GameOverScene();
		break;
	}
}
void GameManager::Initialize() {
	sceneArr_[currentSceneNo_]->Initialize();
}

void GameManager::Update() {

	prevSceneNo_ = currentSceneNo_;
	currentSceneNo_ = sceneArr_[currentSceneNo_]->GetSceneNo();

	// シーン遷移が発生した場合
	if (prevSceneNo_ != currentSceneNo_) {
		// ローディングからの遷移以外ではフェード処理を行う
		// Loadingシーンからの遷移は、LoadingScene内で独自にフェード処理を行っているため
		if (prevSceneNo_ != Loading) {
			 //フェードアウト・インを行う
			FadeManager::GetInstance()->StartFadeIn(0.03f);
		}

		SceneChange(prevSceneNo_, currentSceneNo_);
		sceneArr_[currentSceneNo_]->Initialize();
	}

	sceneArr_[currentSceneNo_]->Update();
}

void GameManager::Draw() {
	sceneArr_[currentSceneNo_]->Draw();
}