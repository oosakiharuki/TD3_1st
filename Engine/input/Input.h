#pragma once

#include <windows.h>
#include <wrl.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Xinput.h>

#include "WinApp.h"
class Input{
public:
	static Input* GetInstance();
	void Finalize();

	//using namespace Microsoft::WRL の代わり
	template <class T> using ComPtr =  Microsoft::WRL::ComPtr<T>;

	void Initialize(WinApp* winApp);
	void Update();

	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);

	bool GetJoystickState(uint32_t num,XINPUT_STATE& state);
	bool GetJoystickStatePrevious(uint32_t num, XINPUT_STATE& state);

	//ボタン情報の移行する
	XINPUT_STATE GetState() { return copyState_; }
	XINPUT_STATE GetPreState() { return copyPreState_; }

	//前のシーンのボタン情報の取得
	void SetStates(XINPUT_STATE state, XINPUT_STATE preState);

private:
	ComPtr<IDirectInputDevice8> keyboard;
	ComPtr<IDirectInput8> directInput;
	BYTE key[256] = {};
	BYTE keyPre[256] = {};
	WinApp* winApp_ = nullptr; //協力関係

	static Input* instance;

	Input() = default;
	~Input() = default;
	Input(Input&) = default;
	Input& operator=(Input&) = default;

	static uint32_t kSRVIndexTop;
	
	XINPUT_STATE prevState = {};

	XINPUT_STATE copyState_ = {};
	XINPUT_STATE copyPreState_ = {};
};