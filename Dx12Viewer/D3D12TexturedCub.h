#pragma once
#include "DxFrame.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class D3D12TexturedCub : public DxFrame
{
private:
	static const UINT FrameCount = 2;
	static const UINT TextureWidth = 16;
	static const UINT TextureHeight = 16;
	static const UINT TexturePixelSize = 4;

	struct BlockVertex {
		XMFLOAT3 position;
		XMFLOAT2 uv;
	};

	struct SceneConstantBuffer
	{
		XMMATRIX wvp;
	};

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
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;

	// App resources.
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	ComPtr<ID3D12Resource> m_indexBuffer;
	UINT m_IndexCount{ 0 };
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	ComPtr<ID3D12Resource> m_texture;
	ComPtr<ID3D12Resource> m_constantBuffer;
	SceneConstantBuffer m_constantData{};
	UINT8* m_pCbvDataBegin{ nullptr };
	XMMATRIX m_matProjection;
	XMMATRIX m_matView;

	void LoadPipeline();
	void LoadAssets();
	std::vector<UINT8> GenerateTextureData();
	void PopulateCommandList();
	void WaitForPreviousFrame();

public:
	D3D12TexturedCub(UINT width, UINT height, std::wstring name);
	~D3D12TexturedCub();

	// Í¨¹ý DXSample ¼Ì³Ð
	virtual void OnInit() override;
	virtual void OnUpdate() override;
	virtual void OnRender() override;
	virtual void OnDestroy() override;
};

