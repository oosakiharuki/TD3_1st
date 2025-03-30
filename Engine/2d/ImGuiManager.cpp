#include "ImGuiManager.h"

ImGuiManager* ImGuiManager::instance = nullptr;

uint32_t ImGuiManager::kSRVIndexTop = 1;

ImGuiManager* ImGuiManager::GetInstance() {
	if (instance == nullptr) {
		instance = new ImGuiManager();
	}
	return instance;
}

void ImGuiManager::Initialize([[maybe_unused]]WinApp* winApp, DirectXCommon* dxCommon, [[maybe_unused]]SrvManager* srvManager) {
#ifdef  USE_IMGUI

	dxCommon_ = dxCommon;

	uint32_t srvIndex = srvManager->Allocate();
	srvHeap_ = srvManager->GetDescriptorHeap();

	//ImGui初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHwnd());
	ImGui_ImplDX12_Init(
		dxCommon_->GetDevice(), 
		static_cast<int>(dxCommon_->GetSwapChainResourceNum()),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		srvHeap_.Get(),
		srvManager->GetCPUDescriptorHandle(srvIndex),
		srvManager->GetGPUDescriptorHandle(srvIndex));


#endif //  USE_IMGUI
}

void ImGuiManager::Begin() {
#ifdef  USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

#endif //  USE_IMGUI
}

void ImGuiManager::End() {
#ifdef  USE_IMGUI
	ImGui::Render();
#endif //  USE_IMGUI
}

void ImGuiManager::Draw(){
#ifdef  USE_IMGUI
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	ID3D12DescriptorHeap* ppHeaps[] = { srvHeap_.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#endif //  USE_IMGUI

}

void ImGuiManager::Finalize() {
#ifdef  USE_IMGUI

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	delete instance;
	instance = nullptr;

#endif //  USE_IMGUI
}
