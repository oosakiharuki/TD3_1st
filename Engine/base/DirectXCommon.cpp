#include "DirectXCommon.h"
#include <cassert>
#include <format>
#include "externals/DirectXTex/DirectXTex.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxcompiler.lib")

#include <thread>
#include "SrvManager.h"
#include "ImGuiManager.h"

using namespace Microsoft::WRL;
using namespace Logger;
using namespace StringUtility;

const uint32_t DirectXCommon::kMaxSRVCount = 512;


DirectXCommon* DirectXCommon::instance = nullptr;

uint32_t DirectXCommon::kSRVIndexTop = 1;

DirectXCommon* DirectXCommon::GetInstance() {
	if (instance == nullptr) {
		instance = new DirectXCommon;
	}
	return instance;
}

void DirectXCommon::Initialize() {

	InitializeFixFPS();

	assert(winApp_);

	this->winApp_ = winApp_;

	Device();
	Command();
	SwapChain();
	ZBuffer();//?
	DescriptorHeap();
	RTV();
	DSV();//深度ステンシルビュー
	Fence();
	ViewPort();
	Siccer();
	DXC();
}


void DirectXCommon::Device() {

#pragma endregion

#ifdef _DEBUG
	Microsoft::WRL::ComPtr <ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		//デバッグレイヤーを有効にする
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}

#endif

#pragma region Factoryの生成
	//DZGIファクトリーの生成
	
	//HREUSLTはWindouws系のエラーコード
	//関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	//初期化の標本的な部分でエラーが出た場合はぷろぐらむがまちがっているか、
	// どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));//甲であることを保証　そうでないと止まる

	log("Hello,DirectX\n");

#pragma endregion

#pragma region アダプタの作成

	//使用するアダプタ用の変数。最初にnullptrを入れておく
	Microsoft::WRL::ComPtr < IDXGIAdapter4> useAdapter = nullptr;
	//良い順にアダプタを読む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; i++) {
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));
		//ソフトウェアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//採用したアダプタの情報をログに出力。wstringのほうなので注意
			log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;
	}
	//適切なアダプタが見つからないので起動しない
	//適切なアダプタが見つからなかったら起動できなくする
	assert(useAdapter != nullptr);

#pragma endregion

#pragma region Deviceの生成

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12,0" };

	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
		if (SUCCEEDED(hr)) {
			log(std::format("FEatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}

	assert(device != nullptr);
	log("Complete create D3D12Device!!!\n");

#pragma endregion

#ifdef _DEBUG
	Microsoft::WRL::ComPtr < ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		//やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//緊急時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumCategories = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		infoQueue->PushStorageFilter(&filter);

	}


#endif
}

void DirectXCommon::Command(){

#pragma region コマンドキュー

	//コマンドキュー生成
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	HRESULT hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//生成できない場合
	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region コマンドアロケータ
	//コマンドアロケータ生成
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//生成できない場合
	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region コマンドリスト
	//コマンドリスト生成
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	//生成できない場合
	assert(SUCCEEDED(hr));
#pragma endregion

}

void DirectXCommon::SwapChain() {

#pragma region Swap Chainの生成
	//スワップチェイン生成
	swapChainDesc.Width = WinApp::kClientWidth;
	swapChainDesc.Height = WinApp::kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	HRESULT hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp_->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast <IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));

	//SwapchainからResourceを引っ張ってくる

	for (uint32_t i = 0; i < MaxResource; ++i) {
		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&swapChainResources[i]));
		assert(SUCCEEDED(hr));
	}

#pragma endregion
}

void DirectXCommon::ZBuffer() {

	resourceDesc.Width = WinApp::kClientWidth;
	resourceDesc.Height = WinApp::kClientHeight;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//二次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//DepthStrencil

	//利用するHeap

	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//VRAN上で作る


	//深度値のクリア設定

	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;


	//resourceの生成
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

}


void DirectXCommon::DescriptorHeap() {
#pragma region ディスクリプターヒープの生成

	descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


	rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);
	dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

#pragma endregion
}

Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDesciptors, bool shaderVisible)
{
	//ディスクリプターヒープの生成

	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDesciptors;

	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}



D3D12_CPU_DESCRIPTOR_HANDLE  DirectXCommon::GetCPUDescriptorHandle(Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE  DirectXCommon::GetGPUDescriptorHandle(Microsoft::WRL::ComPtr < ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}


//SRVHandle
D3D12_CPU_DESCRIPTOR_HANDLE  DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index) {
	return GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
}
D3D12_GPU_DESCRIPTOR_HANDLE  DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index) {
	return GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
}

//RTVHandle
D3D12_CPU_DESCRIPTOR_HANDLE  DirectXCommon::GetRTVCPUDescriptorHandle(uint32_t index) {
	return GetCPUDescriptorHandle(rtvDescriptorHeap, descriptorSizeRTV, index);
}
D3D12_GPU_DESCRIPTOR_HANDLE  DirectXCommon::GetRTVGPUDescriptorHandle(uint32_t index) {
	return GetGPUDescriptorHandle(rtvDescriptorHeap, descriptorSizeRTV, index);
}

//DSVHandle
D3D12_CPU_DESCRIPTOR_HANDLE  DirectXCommon::GetDSVCPUDescriptorHandle(uint32_t index) {
	return GetCPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, index);
}
D3D12_GPU_DESCRIPTOR_HANDLE  DirectXCommon::GetDSVGPUDescriptorHandle(uint32_t index) {
	return GetGPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, index);
}

void DirectXCommon::RTV() {
	//RTV
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (uint32_t i = 0; i < MaxResource; ++i) {
		rtvHandles[i] = rtvStartHandle;
		if (i > 0) {
			rtvHandles[i].ptr = rtvHandles[i - 1].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}
		device->CreateRenderTargetView(swapChainResources[i].Get(), &rtvDesc, rtvHandles[i]);
	}
}

void DirectXCommon::DSV() {
	//DSV生成
	D3D12_DEPTH_STENCIL_VIEW_DESC dscDesc{};
	dscDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dscDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	//void zBuffer()の部分
	//Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device, WinApp::kClientWidth, WinApp::kClientHeight);

	//DSVHeapの先頭
	device->CreateDepthStencilView(*&resource, &dscDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

}

void DirectXCommon::Fence() {
	//初期化で0でFenceを作る
	HRESULT hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);
}

void DirectXCommon::ViewPort() {
	//ビューポート
	viewport.Width = WinApp::kClientWidth;
	viewport.Height = WinApp::kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

void DirectXCommon::Siccer() {
	//シザー
	scissorRect.left = 0;
	scissorRect.right = WinApp::kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WinApp::kClientHeight;

}


//ComplierShader関数
Microsoft::WRL::ComPtr<IDxcBlob> DirectXCommon::CompileShader(
	const std::wstring& filePath,
	const wchar_t* profile)
{
	//1.hlslファイル
	log(ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));

	Microsoft::WRL::ComPtr <IDxcBlobEncoding> shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);

	assert(SUCCEEDED(hr));

	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	//2.Complie
	LPCWSTR arguments[] = {
		filePath.c_str(),
		L"-E",L"main",
		L"-T",profile,
		L"-Zi",L"-Qembed_debug",
		L"-Od",
		L"-Zpr",
	};

	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,
		arguments,
		_countof(arguments),
		*&includeHandler,
		IID_PPV_ARGS(&shaderResult));

	assert(SUCCEEDED(hr));

	//3.警告エラー

	Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0)
	{
		log(shaderError->GetStringPointer());
		//警告エラーダメ絶対
		assert(false);
	}
	//4.Complie結果
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));

	log(ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));

	return shaderBlob;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResource(size_t sizeInBytes) {
	// デバイスの有効性チェック
	if (!device) {
		OutputDebugStringA("DirectXCommon::CreateBufferResource - Device is null\n");
		return nullptr;
	}
	
	//VertexResource
	//頂点シェーダを作る
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC vertexResourceDesc{};

	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;

	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;

	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	
	if (FAILED(hr)) {
		// エラーの詳細情報を出力
		char errorMsg[256];
		sprintf_s(errorMsg, "DirectXCommon::CreateBufferResource - Failed to create buffer resource. HRESULT: 0x%08X, Size: %zu bytes\n", hr, sizeInBytes);
		OutputDebugStringA(errorMsg);
		
		// デバイスロストの可能性をチェック
		if (hr == DXGI_ERROR_DEVICE_REMOVED) {
			HRESULT removedReason = device->GetDeviceRemovedReason();
			sprintf_s(errorMsg, "Device was removed. Reason: 0x%08X\n", removedReason);
			OutputDebugStringA(errorMsg);
		}
		
		return nullptr;
	}

	return vertexResource;
}



Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateTextureResource(const DirectX::TexMetadata& metadata) {
	// デバイスの有効性チェック
	if (!device) {
		OutputDebugStringA("DirectXCommon::CreateTextureResource - Device is null\n");
		return nullptr;
	}

	// メタデータの検証
	if (metadata.width == 0 || metadata.height == 0) {
		char errorMsg[256];
		sprintf_s(errorMsg, "DirectXCommon::CreateTextureResource - Invalid texture dimensions: width=%zu, height=%zu\n", 
			metadata.width, metadata.height);
		OutputDebugStringA(errorMsg);
		return nullptr;
	}
	if (metadata.mipLevels == 0) {
		char errorMsg[256];
		sprintf_s(errorMsg, "DirectXCommon::CreateTextureResource - Invalid mip levels: %zu\n", metadata.mipLevels);
		OutputDebugStringA(errorMsg);
		return nullptr;
	}

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);//幅
	resourceDesc.Height = UINT(metadata.height);//高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);//数
	resourceDesc.DepthOrArraySize = UINT(metadata.arraySize);//奥行き　Textureの配置数
	resourceDesc.Format = metadata.format;//format
	resourceDesc.SampleDesc.Count = 1;//サンプリングカウント(1固定)
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);// textureの次元数


	//利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;


	//Resouceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource)
	);
	
	if (FAILED(hr)) {
		// エラーの詳細情報を出力
		char errorMsg[512];
		sprintf_s(errorMsg, "DirectXCommon::CreateTextureResource - Failed to create texture resource.\n"
			"HRESULT: 0x%08X\n"
			"Width: %u, Height: %u, MipLevels: %u, ArraySize: %u\n"
			"Format: %d, Dimension: %d\n", 
			hr, 
			UINT(metadata.width), UINT(metadata.height), UINT16(metadata.mipLevels), UINT(metadata.arraySize),
			metadata.format, metadata.dimension);
		OutputDebugStringA(errorMsg);
		
		// デバイスロストの可能性をチェック
		if (hr == DXGI_ERROR_DEVICE_REMOVED) {
			HRESULT removedReason = device->GetDeviceRemovedReason();
			sprintf_s(errorMsg, "Device was removed. Reason: 0x%08X\n", removedReason);
			OutputDebugStringA(errorMsg);
		}
		
		return nullptr;
	}
	
	return resource;

}

void DirectXCommon::UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages) {

	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
		HRESULT hr = texture->WriteToSubresource(
			UINT(mipLevel),
			nullptr,			 //全領域へコピー
			img->pixels,		 //元データアドレス
			UINT(img->rowPitch), //1ラインサイズ
			UINT(img->slicePitch)//1枚サイズ
		);
		assert(SUCCEEDED(hr));
	}
}


void DirectXCommon::DXC() {
	//DXC
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

}


//更新前
void DirectXCommon::PreDraw() {


	//　これから書き込みバックバッファのインデックスを取得
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();


	//今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを貼る対象のリソース。現在のバッファに対して行う
	barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();//こいつ
	//前の(現在の)ResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	//後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);


	// 描画先のRTVの設定をする
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
	//指定した色で画面をクリアする　
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
	//コマンド蓄積
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

	//描画用のDescriptorHeap
	Microsoft::WRL::ComPtr < ID3D12DescriptorHeap> descriptorHeaps[] = { srvDescriptorHeap };
	commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());

	//DSV
	dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);



	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	SrvManager::GetInstance()->PreDraw();
}

//更新後
void DirectXCommon::PostDraw() {

	//　これから書き込みバックバッファのインデックスを取得
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	//画面に描く処理はすべて終わり、画面に映すので、状況をそうい
	//今回はResourceTargetからPresentにする
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);



	//コマンドリストの内容を確定させる。全てのコマンドを積んでからclearする
	HRESULT hr = commandList->Close();
	assert(SUCCEEDED(hr));


	//GPUにコマンドリストの実行を行わせる
	Microsoft::WRL::ComPtr < ID3D12CommandList> commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(1, commandLists->GetAddressOf());
	//GPUとOSに画面の交換を行うように通知する
	swapChain->Present(1, 0);

	//// 出力ウィンドウへの文字出力
	//OutputDebugStringA("Hello DirectX!\n");
	//FENCEを更新する

	fenceValue++;
	//コマンドの実行完了まで待つ
	commandQueue->Signal(fence.Get(), fenceValue);

	if (fence->GetCompletedValue() != fenceValue) {
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}
	// FPS
	UpdateFixFPS();

	//次のフレームのコマンドリストを準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

void DirectXCommon::InitializeFixFPS() {
	//現在時間を記録
	reference_ = std::chrono::steady_clock::now();
}

void DirectXCommon::UpdateFixFPS() {
	//1/60秒ピッタリ
	const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));
	//1/60秒少し短い
	//vsyncまちの無駄時間軽減に使う
	const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));


	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

	if (elapsed < kMinCheckTime) {
		while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
	reference_ = std::chrono::steady_clock::now();
}

void DirectXCommon::Finalize() {
	CloseHandle(fenceEvent);
	delete instance;
	instance = nullptr;
}