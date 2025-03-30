#include "Input.h"

#include <cassert>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"Xinput.lib")

Input* Input::instance = nullptr;

uint32_t Input::kSRVIndexTop = 1;

Input* Input::GetInstance() {
	if (instance == nullptr) {
		instance = new Input;
	}
	return instance;
}

void Input::Finalize() {
	delete instance;
	instance = nullptr;
}

void Input::Initialize(WinApp* winApp) {

	//メンバ変数のwinApp <-代入- ローカル変数のwinAppの値
	this->winApp_ = winApp;

	HRESULT result;

	result = DirectInput8Create(winApp_->GetHInstance(), DIRECTINPUT_VERSION,
		IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	result = keyboard->SetCooperativeLevel(winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

}

void Input::Update() {
	HRESULT result;

	memcpy(keyPre, key, sizeof(key));

	result = keyboard->Acquire();

	result = keyboard->GetDeviceState(sizeof(key), key);
}

bool Input::PushKey(BYTE keyNumber) {
	
	if (key[keyNumber]) {
		return true;
	}

	return false;
}

bool Input::TriggerKey(BYTE keyNumber) {
	
	if (key[keyNumber] && !keyPre[keyNumber]) {
		return true;
	}

	return false;
}

bool Input::GetJoyStickState(uint32_t num, XINPUT_STATE& state) {
	DWORD dwResult;

	prevState = state;

	ZeroMemory(&state, sizeof(XINPUT_STATE));
	
	// Simply get the state of the controller from XInput.
	dwResult = XInputGetState(num, &state);

	//コントローラが作動してるか
	if (dwResult == ERROR_SUCCESS)
	{
		// Controller is connected
		return true;
	}
	else
	{
		// Controller is not connected
		return false;
	}

	return false;
}

bool Input::GetJoystickStatePrevious(uint32_t num, XINPUT_STATE& state) {
	DWORD dwResult;

	ZeroMemory(&state, sizeof(XINPUT_STATE));

	// Simply get the state of the controller from XInput.
	dwResult = XInputGetState(num, &state);

	//コントローラが作動してるか
	if (dwResult == ERROR_SUCCESS)
	{
		// Controller is connected	

		//現在の状態から前回の状態に
		state = prevState;

		return true;
	}
	else
	{
		// Controller is not connected		
		return false;
	}

	return false;
}
