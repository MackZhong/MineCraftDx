#pragma once
#include "DXSample.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class D3D12TexturedCub : public DXSample
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
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	UINT m_rtvDescriptorSize{ 0 };

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

	// Synchronization objects.
	UINT m_frameIndex{ 0 };
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

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

