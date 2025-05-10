#pragma once
#include "Sprite.h"
#include "Framework.h"
#include "IScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "FadeManager.h"

class TitleScene : public IScene{
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
private:
	Sprite* sprite;
	Sprite* backGround;
	
	Sprite* bottonSprite;

	XINPUT_STATE state = {}, preState = {}; // 初期化を追加	
};