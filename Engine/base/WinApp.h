#include<Windows.h>
#include <cstdint>

#pragma once

class WinApp
{	
public://静寂メンバ関数
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public://メンバ関数
	void Initialize();
	void Update();
	void Finalize(); //API終了処理　(delete)


	//クライアント領域のサイズ　横　縦
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

	HWND GetHwnd()const { return hwnd; }
	HINSTANCE GetHInstance() const { return wc.hInstance; }
	bool ProcessMessage();

private:
	WNDCLASS wc{};
	HWND hwnd = nullptr;
};