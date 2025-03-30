#pragma once
#include "Input.h"
#include "DirectXCommon.h"
#include "SrvManager.h"

#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "ParticleCommon.h"

#include "ModelManager.h"
#include "ParticleManager.h"
#include "TextureManager.h"

#include "ImGuiManager.h"
#include "D3DResorceLeakChecker.h"


class Framework{
public:
	virtual void Initialize();
	virtual void Update();
	virtual void Draw() = 0;
	virtual void Finalize();
	virtual bool IsEndRequst() { return isRequst; }

	virtual ~Framework() = default;

	void Run();
private:
	WinApp* winApp_ = nullptr;
	Input* input_;
	DirectXCommon* dxCommon = nullptr;
	SrvManager* srvManager = nullptr;

	SpriteCommon* spriteCommon = nullptr;
	Object3dCommon* object3dCommon = nullptr;
	ModelCommon* modelCommon = nullptr;
	ParticleCommon* particleCommon = nullptr;

	bool isRequst = false;
};