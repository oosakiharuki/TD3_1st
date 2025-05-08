#include "IScene.h"

int IScene::sceneNo = Loading; // ゲーム開始時はローディングから開始

IScene::~IScene(){}

int IScene::GetSceneNo() { return sceneNo; }