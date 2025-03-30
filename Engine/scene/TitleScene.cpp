#include "TitleScene.h"

void TitleScene::Initialize() {
	sprite = new Sprite();
	sprite->Initialize("monsterBall.png");

	sprite->SetPosition({ 64,64 });
	sprite->SetSize({ 128,128 });
}

void TitleScene::Update() {
	sprite->Update();

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		sceneNo = Game;
	}

}

void TitleScene::Draw() {
	SpriteCommon::GetInstance()->Command();

	sprite->Draw();
}

void TitleScene::Finalize() {
	delete sprite;
}