#pragma once 
#include "Framework.h"
#include "GameManager.h"

class MyGame : public Framework{
public:
	void Initialize() override;
	void Finalize() override;
	void Update() override;
	void Draw() override;

private:
	GameManager* gameScene = nullptr;
};