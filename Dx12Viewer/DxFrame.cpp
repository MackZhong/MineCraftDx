//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "pch.h"
#include "DxFrame.h"

using namespace Microsoft::WRL;
using namespace DirectX;

DxFrame::DxFrame(UINT width, UINT height, std::wstring name) :
	m_width(width),
	m_height(height),
	m_title(name),
	m_useWarpDevice(false)
	, m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))
	, m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height))
	, m_rtvDescriptorSize(0)
{
	WCHAR assetsPath[512];
	GetAssetsPath(assetsPath, arraysize(assetsPath));
	m_assetsPath = assetsPath;

	m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	m_tearingSupported = CheckTearingSupport();
}

DxFrame::~DxFrame()
{
}

void DxFrame::EnableDebugLayer()
{
#if defined(_DEBUG)
	// Always enable the debug layer before doing anything DX12 related
	// so all possible errors generated while creating DX12 objects
	// are caught by the debug layer.
	ComPtr<ID3D12Debug> debugInterface;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	debugInterface->EnableDebugLayer();
#endif
}

ComPtr<IDXGIAdapter4> DxFrame::GetAdapter(bool useWarp)
{
	ComPtr<IDXGIFactory4> dxgiFactory;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));
	ComPtr<IDXGIAdapter1> dxgiAdapter1;
	ComPtr<IDXGIAdapter4> dxgiAdapter4;

	if (useWarp)
	{
		ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
		ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
	}
	else
	{
		SIZE_T maxDedicatedVideoMemory = 0;
		for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
			dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

			// Check to see if the adapter can create a D3D12 device without actually 
			// creating it. The adapter with the largest dedicated video memory
			// is favored.
			if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
				SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
					D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
				dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
			{
				maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
				ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
			}
		}
	}

	return dxgiAdapter4;
}

ComPtr<ID3D12Device2> DxFrame::CreateDevice(IDXGIAdapter4* adapter)
{
	ComPtr<ID3D12Device2> d3d12Device2;
	ThrowIfFailed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2)));

	// Enable debug messages in debug mode.
#if defined(_DEBUG)
	ComPtr<ID3D12InfoQueue> pInfoQueue;
	if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
	{
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// Suppress whole categories of messages
		//D3D12_MESSAGE_CATEGORY Categories[] = {};

		// Suppress messages based on their severity level
		D3D12_MESSAGE_SEVERITY Severities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO
		};

		// Suppress individual messages by their ID
		D3D12_MESSAGE_ID DenyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,	// I'm really not sure how to avoid this message.
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,							// This warning occurs when using capture frame while graphics debugging.
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,						// This warning occurs when using capture frame while graphics debugging.
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		//NewFilter.DenyList.NumCategories = _countof(Categories);
		//NewFilter.DenyList.pCategoryList = Categories;
		NewFilter.DenyList.NumSeverities = arraysize(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = arraysize(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
	}
#endif

	return d3d12Device2;
}

ComPtr<ID3D12CommandQueue> DxFrame::CreateCommandQueue(ID3D12Device2* device, D3D12_COMMAND_LIST_TYPE type)
{
	ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)));

	return d3d12CommandQueue;
}

ComPtr<IDXGISwapChain4> DxFrame::CreateSwapChain(HWND hWnd,
	ID3D12CommandQueue* commandQueue,
	UINT32 width, UINT32 height, UINT32 bufferCount)
{
	ComPtr<IDXGISwapChain4> dxgiSwapChain4;
	ComPtr<IDXGIFactory4> dxgiFactory4;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = bufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	// It is recommended to always allow tearing if tearing support is available.
	swapChainDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr<IDXGISwapChain1> swapChain1;
	ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
		commandQueue,
		hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain1));

	// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
	// will be handled manually.
	ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain1.As(&dxgiSwapChain4));

	return dxgiSwapChain4;
}

ComPtr<ID3D12DescriptorHeap> DxFrame::CreateDescriptorHeap(ID3D12Device2* device,
	D3D12_DESCRIPTOR_HEAP_TYPE type, UINT32 numDescriptors)
{
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;

	ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

	return descriptorHeap;
}

ComPtr<ID3D12CommandAllocator> DxFrame::CreateCommandAllocator(ID3D12Device2* device, D3D12_COMMAND_LIST_TYPE type)
{
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ThrowIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));

	return commandAllocator;
}

ComPtr<ID3D12GraphicsCommandList> DxFrame::CreateCommandList(ID3D12Device2* device, ID3D12CommandAllocator* commandAllocator, D3D12_COMMAND_LIST_TYPE type)
{
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ThrowIfFailed(device->CreateCommandList(0, type, commandAllocator, nullptr, IID_PPV_ARGS(&commandList)));

	ThrowIfFailed(commandList->Close());

	return commandList;
}

ComPtr<ID3D12Fence> DxFrame::CreateFence(ID3D12Device2* device)
{
	ComPtr<ID3D12Fence> fence;

	ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

	return fence;
}

HANDLE DxFrame::CreateEventHandle()
{
	HANDLE fenceEvent;

	fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent && "Failed to create fence event.");

	return fenceEvent;
}

void DxFrame::UpdateRenderTargetViews(ID3D12Device2* device,
	IDXGISwapChain4* swapChain, ID3D12DescriptorHeap* descriptorHeap)
{
	auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < FrameCount; ++i)
	{
		ComPtr<ID3D12Resource> backBuffer;
		ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

		device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

		m_renderTargets[i] = backBuffer;

		rtvHandle.Offset(rtvDescriptorSize);
	}
}

UINT64 DxFrame::Signal(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, UINT64 value) {
	UINT64 signalValue = ++value;
	HRESULT hr = commandQueue->Signal(fence, signalValue);

	return signalValue;
}

void DxFrame::WaitforFence(ID3D12Fence* fence, UINT64 value, HANDLE event, std::chrono::milliseconds duration) {
	if (fence->GetCompletedValue() < value) {
		HRESULT hr = fence->SetEventOnCompletion(value, event);
		::WaitForSingleObject(event, static_cast<DWORD>(duration.count()));
	}
}

void DxFrame::Flush(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, UINT64 value, HANDLE event) {
	UINT64 valueForSingle = Signal(commandQueue, fence, value);
	WaitforFence(fence, valueForSingle, event);
}

void DxFrame::SetFullscreen(bool fullscreen) {
	if (m_fullscreen == fullscreen) {
		return;
	}
	if (fullscreen) {
		::GetWindowRect(DxApplication::GetHwnd(), &m_rcWindow);
		UINT style = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
		::SetWindowLongW(DxApplication::GetHwnd(), GWL_STYLE, style);
		HMONITOR hMon = ::MonitorFromWindow(DxApplication::GetHwnd(), MONITOR_DEFAULTTONEAREST);
		MONITORINFOEX monitorInfo = {};
		monitorInfo.cbSize = sizeof(monitorInfo);
		::GetMonitorInfo(hMon, &monitorInfo);
		::SetWindowPos(DxApplication::GetHwnd(), HWND_TOPMOST,
			monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.top,
			monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);
		::ShowWindow(DxApplication::GetHwnd(), SW_MAXIMIZE);
	}
	else {
		::SetWindowLongW(DxApplication::GetHwnd(), GWL_STYLE, WS_OVERLAPPEDWINDOW);
		::SetWindowPos(DxApplication::GetHwnd(), HWND_TOPMOST,
			m_rcWindow.left,
			m_rcWindow.top,
			m_rcWindow.right - m_rcWindow.left,
			m_rcWindow.bottom - m_rcWindow.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);
		::ShowWindow(DxApplication::GetHwnd(), SW_NORMAL);
	}
	m_fullscreen = fullscreen;
}
void DxFrame::Init(HWND hwnd) {
	::GetWindowRect(hwnd, &m_rcWindow);

	ComPtr<IDXGIAdapter4> dxgiAdapter4 = this->GetAdapter(false);
	m_device = this->CreateDevice(dxgiAdapter4.Get());
	m_commandQueue = this->CreateCommandQueue(m_device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
	m_swapChain = this->CreateSwapChain(hwnd, m_commandQueue.Get(), m_width, m_height, FrameCount);
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	m_rtvHeap = this->CreateDescriptorHeap(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FrameCount);
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	this->UpdateRenderTargetViews(m_device.Get(), m_swapChain.Get(), m_rtvHeap.Get());

	for (UINT i = 0; i < FrameCount; i++) {
		m_commandAllocators[i] = this->CreateCommandAllocator(m_device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
	}
	m_commandList = this->CreateCommandList(m_device.Get(), m_commandAllocators[m_frameIndex].Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);

	m_fence = this->CreateFence(m_device.Get());
	m_fenceEvent = this->CreateEventHandle();

	//	// Create the root signature.
	//{
	//	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	//	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	//	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	//	if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	//	{
	//		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	//	}

	//	CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
	//	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	//	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

	//	CD3DX12_ROOT_PARAMETER1 rootParameters[2];
	//	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
	//	rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

	//	D3D12_STATIC_SAMPLER_DESC sampler = {};
	//	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	//	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	//	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	//	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	//	sampler.MipLODBias = 0;
	//	sampler.MaxAnisotropy = 0;
	//	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	//	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	//	sampler.MinLOD = 0.0f;
	//	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	//	sampler.ShaderRegister = 0;
	//	sampler.RegisterSpace = 0;
	//	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	//	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	//	rootSignatureDesc.Init_1_1(arraysize(rootParameters), rootParameters, 1, &sampler,
	//		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	//	ComPtr<ID3DBlob> signature;
	//	ComPtr<ID3DBlob> error;
	//	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));

	//	ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	//}

	//// Create the pipeline state, which includes compiling and loading shaders.
	//{
	//	ComPtr<ID3DBlob> vertexShader;
	//	ComPtr<ID3DBlob> pixelShader;
	//	vertexShader = CompileShader(L"shaders.hlsl", nullptr, "VSMain", "vs_5_0");
	//	pixelShader = CompileShader(L"shaders.hlsl", nullptr, "PSMain", "ps_5_0");
	//	//
	//	//#if defined(_DEBUG)
	//	//		// Enable better shader debugging with the graphics debugging tools.
	//	//		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	//	//#else
	//	//		UINT compileFlags = 0;
	//	//#endif

	//	//ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	//	//ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));
	//	//ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	//	//ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

	//	// Define the vertex input layout.
	//	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	//	{
	//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	//	};

	//	// Describe and create the graphics pipeline state object (PSO).
	//	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	//	psoDesc.InputLayout = { inputElementDescs, arraysize(inputElementDescs) };
	//	psoDesc.pRootSignature = m_rootSignature.Get();
	//	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	//	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	//	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	//	psoDesc.DepthStencilState.DepthEnable = FALSE;
	//	psoDesc.DepthStencilState.StencilEnable = FALSE;
	//	psoDesc.SampleMask = UINT_MAX;
	//	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//	psoDesc.NumRenderTargets = 1;
	//	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//	psoDesc.SampleDesc.Count = 1;
	//	ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	//}

	//// Create the command list.
	//ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

	//this->OnInit(m_device.Get(), m_commandList.Get());

	// Close the command list and execute it to begin the initial GPU setup.
	ThrowIfFailed(this->m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { this->m_commandList.Get() };
	this->m_commandQueue->ExecuteCommandLists(arraysize(ppCommandLists), ppCommandLists);

	m_matProjection = XMMatrixPerspectiveFovLH(75.0f * XM_PI / 180.f, m_aspectRatio, 0.1f, 1000.0f);

	m_initialized = true;
}

void DxFrame::ExecuteCommandLists(UINT numCommandLists, ID3D12CommandList* const* ppCommandLists) {
	this->m_commandQueue->ExecuteCommandLists(numCommandLists, ppCommandLists);
}

void DxFrame::Update() {
	static uint64_t frameCounter = 0;
	static double elapsedSeconds = 0.0;
	static std::chrono::high_resolution_clock clock;
	static auto t0 = clock.now();

	frameCounter++;
	auto t1 = clock.now();
	auto deltaTime = t1 - t0;
	t0 = t1;

	elapsedSeconds += deltaTime.count() * 1e-9;

	if (elapsedSeconds > 1.0)
	{
		wchar_t buffer[500];
		auto fps = frameCounter / elapsedSeconds;
		swprintf_s(buffer, 500, L"%s - FPS: %f\n", GetTitle(), fps);
		OutputDebugStringW(buffer);
		this->SetCustomWindowText(buffer);

		frameCounter = 0;
		elapsedSeconds = 0.0;
	}

	m_matView = XMMatrixLookAtLH({ -1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });

	this->OnUpdate(deltaTime.count());
}

void DxFrame::Render() {
	auto commandAllocator = m_commandAllocators[m_frameIndex];
	auto renderTarget = m_renderTargets[m_frameIndex];

	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(commandAllocator->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(m_commandList->Reset(commandAllocator.Get(), m_pipelineState.Get()));

	// Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);

	// Record commands.

	// clear back buffer
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// 
	this->OnRender(m_commandList.Get());

	// Indicate that the back buffer will now be used to present.
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &barrier);

	ThrowIfFailed(m_commandList->Close());

	// Execute the command list.
	ID3D12CommandList* const ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(arraysize(ppCommandLists), ppCommandLists);

	m_frameFenceValues[m_frameIndex] = Signal(m_commandQueue.Get(), m_fence.Get(), m_fenceValue);

	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(m_vSync ? 1 : 0,
		m_tearingSupported && !m_vSync ? DXGI_PRESENT_ALLOW_TEARING : 0));

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	WaitforFence(m_fence.Get(), m_frameFenceValues[m_frameIndex], m_fenceEvent);
}

void DxFrame::Destroy() {
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	Flush(m_commandQueue.Get(), m_fence.Get(), m_fenceValue, m_fenceEvent);

	CloseHandle(m_fenceEvent);

	this->OnDestroy();
}

void DxFrame::Resize(UINT32 width, UINT32 height) {
	if (m_width == width && m_height == height) {
		return;
	}

	m_width = std::max(1u, width);
	m_height = std::max(1u, height);

	Flush(m_commandQueue.Get(), m_fence.Get(), m_fenceValue, m_fenceEvent);

	for (UINT i = 0; i < FrameCount; i++) {
		m_renderTargets[i].Reset();
		m_frameFenceValues[i] = m_frameFenceValues[m_frameIndex];
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	HRESULT hr = m_swapChain->GetDesc(&swapChainDesc);
	hr = m_swapChain->ResizeBuffers(FrameCount, width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	UpdateRenderTargetViews(m_device.Get(), m_swapChain.Get(), m_rtvHeap.Get());

	this->OnResize(width, height);
}

bool DxFrame::CheckTearingSupport()
{
	BOOL allowTearing = FALSE;

	// Rather than create the DXGI 1.5 factory interface directly, we create the
	// DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
	// graphics debugging tools which will not support the 1.5 factory interface 
	// until a future update.
	ComPtr<IDXGIFactory4> factory4;
	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
	{
		ComPtr<IDXGIFactory5> factory5;
		if (SUCCEEDED(factory4.As(&factory5)))
		{
			if (FAILED(factory5->CheckFeatureSupport(
				DXGI_FEATURE_PRESENT_ALLOW_TEARING,
				&allowTearing, sizeof(allowTearing))))
			{
				allowTearing = FALSE;
			}
		}
	}

	return allowTearing == TRUE;
}


// Helper function for resolving the full path of assets.
std::wstring DxFrame::GetAssetFullPath(LPCWSTR assetName)
{
	return m_assetsPath + assetName;
}

// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
_Use_decl_annotations_
void DxFrame::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;
	SIZE_T maxDedicatedVideoMemory = 0;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			if (desc.DedicatedVideoMemory > maxDedicatedVideoMemory)
				break;
			maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
		}
	}

	*ppAdapter = adapter.Detach();
}

// Helper function for setting the window's title text.
void DxFrame::SetCustomWindowText(LPCWSTR text)
{
	std::wstring windowText = m_title + L": " + text;
	SetWindowText(DxApplication::GetHwnd(), windowText.c_str());
}

XMMATRIX DxFrame::GetWVP(const XMMATRIX & world) const {
	return world * m_matView * m_matProjection;
}

// Helper function for parsing any supplied command line args.
_Use_decl_annotations_
void DxFrame::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
	for (int i = 1; i < argc; ++i)
	{
		if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0 ||
			_wcsnicmp(argv[i], L"/warp", wcslen(argv[i])) == 0)
		{
			m_useWarpDevice = true;
			m_title = m_title + L" (WARP)";
		}
	}
}
