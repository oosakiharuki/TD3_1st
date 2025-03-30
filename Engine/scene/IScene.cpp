#include "IScene.h"

int IScene::sceneNo = Title;

IScene::~IScene(){}

int IScene::GetSceneNo() { return sceneNo; }