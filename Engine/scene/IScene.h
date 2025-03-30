#pragma once
#include <cstdint>

enum SCENE{
	Title,
	Game,
	Clear,
	GameOver,
};


class IScene{
protected:
	static int sceneNo;

public:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void Finalize() = 0;

	virtual ~IScene();

	int GetSceneNo();
};