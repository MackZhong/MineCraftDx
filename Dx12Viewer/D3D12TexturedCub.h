#pragma once
#include "DxFrame.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class D3D12TexturedCub : public DxFrame
{
private:
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

	// App resources.
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	UINT m_IndexCount{ 0 };
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ComPtr<ID3D12Resource> m_texture;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	ComPtr<ID3D12Resource> m_constantBuffer;
	SceneConstantBuffer m_constantData{};
	UINT8* m_pCbvDataBegin{ nullptr };

	//void LoadPipeline();
	void LoadAssets(ID3D12Device2* device, ID3D12GraphicsCommandList* commandList);
	std::vector<UINT8> GenerateTextureData();
	//void PopulateCommandList();
	//void WaitForPreviousFrame();

protected:

public:
	D3D12TexturedCub(UINT width, UINT height, std::wstring name);
	~D3D12TexturedCub();

	// Í¨¹ý DXSample ¼Ì³Ð
	virtual void OnInit(ID3D12Device2* device, ID3D12GraphicsCommandList* commandList) override;
	virtual void OnUpdate(long long lTicks) override;
	virtual void OnRender(ID3D12GraphicsCommandList* commandList) override;
	virtual void OnDestroy() override;
	virtual void OnResize(UINT32 width, UINT32 height) override;
};

