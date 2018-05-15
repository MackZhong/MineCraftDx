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

#pragma once

#include "DirectXHelper.h"
#include "DxApplication.h"

class DxFrame
{
public:
	DxFrame(UINT width, UINT height, std::wstring name);
	virtual ~DxFrame();

	void Init(HWND hwnd);
	void Update();
	void Render();
	void Destroy();
	void Resize(UINT32 width, UINT32 height);

	// Samples override the event handlers to handle specific messages.
	virtual void OnKeyDown(UINT8 /*key*/)   {}
	virtual void OnKeyUp(UINT8 /*key*/)     {}

	// Accessors.
	UINT GetWidth() const           { return m_width; }
	UINT GetHeight() const          { return m_height; }
	const WCHAR* GetTitle() const   { return m_title.c_str(); }
	bool IsInitialized() const { return m_initialized; }

	void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

protected:
	virtual void OnInit(ID3D12Device2* device, ID3D12GraphicsCommandList* commandList) = 0;
	virtual void OnUpdate(long long lTicks) = 0;
	virtual void OnRender(ID3D12GraphicsCommandList* commandList) = 0;
	virtual void OnDestroy() = 0;
	virtual void OnResize(UINT32 width, UINT32 height) = 0;

protected:
	void EnableDebugLayer();
	ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp);
	ComPtr<ID3D12Device2> CreateDevice(IDXGIAdapter4* adapter);
	ComPtr<ID3D12CommandQueue> CreateCommandQueue(ID3D12Device2* device, D3D12_COMMAND_LIST_TYPE type);
	ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hWnd, ID3D12CommandQueue* commandQueue, UINT32 width, UINT32 height, UINT32 bufferCount);
	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ID3D12Device2* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT32 numDescriptors);
	ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(ID3D12Device2* device, D3D12_COMMAND_LIST_TYPE type);
	ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ID3D12Device2* device, ID3D12CommandAllocator* commandAllocator, D3D12_COMMAND_LIST_TYPE type);
	ComPtr<ID3D12Fence> CreateFence(ID3D12Device2* device);
	HANDLE CreateEventHandle();

	void UpdateRenderTargetViews(ID3D12Device2* device, IDXGISwapChain4* swapChain, ID3D12DescriptorHeap* descriptorHeap, ComPtr<ID3D12Resource>* ppTargets, UINT count);
	UINT64 Signal(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, UINT64 value);
	void WaitforFence(ID3D12Fence* fence, UINT64 value, HANDLE event, std::chrono::milliseconds duration = std::chrono::milliseconds::max());
	void Flush(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, UINT64 value, HANDLE event);
	void SetFullscreen(bool fullscreen = true);

	bool CheckTearingSupport();
	void ExecuteCommandLists(UINT numCommandLists, ID3D12CommandList* const* ppCommandLists);

	std::wstring GetAssetFullPath(LPCWSTR assetName);
	void GetHardwareAdapter(_In_ IDXGIFactory2* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);
	void SetCustomWindowText(LPCWSTR text);
	DirectX::XMMATRIX GetWVP(const DirectX::XMMATRIX& world) const;

private:
	static const UINT FrameCount = 2;

	// Pipeline objects.
	ComPtr<ID3D12Device2> m_device;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<IDXGISwapChain4> m_swapChain;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	UINT m_rtvDescriptorSize{ 0 };

	// Synchronization objects.
	UINT m_frameIndex{ 0 };
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue{ 0 };
	UINT64 m_frameFenceValues[FrameCount] = {};
	HANDLE m_fenceEvent{ NULL };

	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	//ComPtr<ID3D12RootSignature> m_rootSignature;
	//ComPtr<ID3D12PipelineState> m_pipelineState;

	DirectX::XMMATRIX m_matProjection;
	DirectX::XMMATRIX m_matView;

	// Viewport dimensions.
	UINT m_width{ 800 };
	UINT m_height{ 600 };
	RECT m_rcWindow;
	float m_aspectRatio{ 1.0f };

	// Adapter info.
	bool m_useWarpDevice{ false };

	// Set to true once the DX12 objects have been initialized.
	bool m_initialized{ false };

	// V-Sync
	bool m_vSync{ false };
	bool m_tearingSupported{ false };
	bool m_fullscreen{ false };

private:
	// Root assets path.
	std::wstring m_assetsPath;

	// Window title.
	std::wstring m_title;
};
