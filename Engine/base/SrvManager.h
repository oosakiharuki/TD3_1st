#pragma once
#include "DirectXCommon.h"
class SrvManager{
public:
	static SrvManager* GetInstance();
	void Finalize();

	void Initialize(DirectXCommon* dxCommon);
	uint32_t Allocate();
	bool Max();
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);
	
	//texture用
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels);
	void CreateSRVforStructureBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);
	void PreDraw();
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return descriptorHeap; }

	//最大SRV数
	static const uint32_t kMaxSRVCount;

private:
	DirectXCommon* directXCommon = nullptr;

	//
	uint32_t descriptorSize;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	uint32_t useIndex = 0;


	static SrvManager* instance;

	SrvManager() = default;
	~SrvManager() = default;
	SrvManager(SrvManager&) = default;
	SrvManager& operator=(SrvManager&) = default;

	static uint32_t kSRVIndexTop;
};