#pragma once

#include "wrl.h"
#include "d3d12.h"
#include "dxgi1_2.h"

#ifdef  USE_IMGUI
#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
#endif //  USE_IMGUI

#include "WinApp.h"
#include "DirectXCommon.h"
#include "SrvManager.h"

class ImGuiManager {
public:
	void Initialize(WinApp* winApp, DirectXCommon* dxCommon, SrvManager* srvManager);

	static ImGuiManager* GetInstance();

	void Begin();

	void End();

	void Finalize();

	void Draw();

private:
	static ImGuiManager* instance;

	ImGuiManager() = default;
	~ImGuiManager() = default;
	ImGuiManager(ImGuiManager&) = default;
	ImGuiManager& operator=(ImGuiManager&) = default;

	static uint32_t kSRVIndexTop;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;


	DirectXCommon* dxCommon_ = nullptr;
};