#include "pch.h"
#include "D3D12TexturedCub.h"

D3D12TexturedCub::D3D12TexturedCub(UINT width, UINT height, std::wstring name)
	: DxFrame(width, height, name)
{
}

D3D12TexturedCub::~D3D12TexturedCub()
{
}

void D3D12TexturedCub::OnInit(ID3D12Device2* device, ID3D12GraphicsCommandList* commandList)
{
	//LoadPipeline();
	{
		// Describe and create a shader resource view (SRV) heap for the texture.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
		SetName(m_srvHeap.Get(), L"Shader Resource view");

		// Describe and create a constant buffer view (CBV) descriptor heap.
		// Flags indicate that this descriptor heap can be bound to the pipeline 
		// and that descriptors contained in it can be referenced by a root table.
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = 1;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));
		SetName(m_cbvHeap.Get(), L"Constant buffer view");
	}
	LoadAssets(device, commandList);
}

//void D3D12TexturedCub::LoadPipeline()
//{
////	UINT dxgiFactoryFlags = 0;
////
////#if defined(_DEBUG)
////	// Enable the debug layer (requires the Graphics Tools "optional feature").
////	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
////	{
////		ComPtr<ID3D12Debug> debugController;
////		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
////		{
////			debugController->EnableDebugLayer();
////
////			// Enable additional debug layers.
////			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
////		}
////	}
////#endif
////
////	ComPtr<IDXGIFactory4> factory;
////	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));
////
////	ComPtr<IDXGIAdapter1> dxgiAdapter1;
////	ComPtr<IDXGIAdapter4> dxgiAdapter4;
////	if (m_useWarpDevice)
////	{
////		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
////	}
////	else
////	{
////		GetHardwareAdapter(factory.Get(), &dxgiAdapter1);
////
////		//ThrowIfFailed(D3D12CreateDevice(
////		//	dxgiAdapter1.Get(),
////		//	D3D_FEATURE_LEVEL_11_0,
////		//	IID_PPV_ARGS(&m_device)
////		//));
////	}
////	ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
////
////	ThrowIfFailed(D3D12CreateDevice(
////		dxgiAdapter4.Get(),
////		D3D_FEATURE_LEVEL_11_0,
////		IID_PPV_ARGS(&m_device)
////	));
////
////#if defined(_DEBUG)
////	// Enable DEBUG message in debug mode.
////	ComPtr<ID3D12InfoQueue> pInfoQueue;
////	if (SUCCEEDED(m_device.As(&pInfoQueue))) {
////		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
////		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
////		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
////
////		// Supperess whole categories of messages
////		// D3D12_MESSAGE_CATEGORY messageCategories[] = {}
////
////		// Supperess messages based on their severity level
////		D3D12_MESSAGE_SEVERITY messageSeverities[] = {
////			D3D12_MESSAGE_SEVERITY_INFO
////		};
////
////		// Superess individual messages by their ID
////		D3D12_MESSAGE_ID messageIds[] = {
////			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
////			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
////			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
////		};
////
////		D3D12_INFO_QUEUE_FILTER messageFilter = { 0 };
////		messageFilter.DenyList.NumSeverities = arraysize(messageSeverities);
////		messageFilter.DenyList.pSeverityList = messageSeverities;
////		messageFilter.DenyList.NumIDs = arraysize(messageIds);
////		messageFilter.DenyList.pIDList = messageIds;
////		ThrowIfFailed(pInfoQueue->PushStorageFilter(&messageFilter));
////	}
////#endif
////
////	// Describe and create the command queue.
////	{
////		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
////		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
////		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
////		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
////		queueDesc.NodeMask = 0;
////
////		ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
////	}
////
////	// Describe and create the swap chain.
////	{
////		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
////		swapChainDesc.Width = m_width;
////		swapChainDesc.Height = m_height;
////		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
////		swapChainDesc.Stereo = FALSE;
////		swapChainDesc.SampleDesc = { 1, 0 };
////		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
////		swapChainDesc.BufferCount = FrameCount;
////		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
////		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
////		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
////		swapChainDesc.Flags = m_tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
////
////		ComPtr<IDXGISwapChain1> swapChain;
////		ThrowIfFailed(factory->CreateSwapChainForHwnd(
////			m_commandQueue.Get(),		// Swap chain needs the queue so that it can force a flush on it.
////			DxApplication::GetHwnd(),
////			&swapChainDesc,
////			nullptr,
////			nullptr,
////			&swapChain
////		));
////
////		// This sample does not support fullscreen transitions.
////		ThrowIfFailed(factory->MakeWindowAssociation(DxApplication::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));
////
////		ThrowIfFailed(swapChain.As(&m_swapChain));
////		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
////	}
////
////	// Create descriptor heaps.
////	//
////	//
////	{
////		// Describe and create a render target view (RTV) descriptor heap.
////		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
////		rtvHeapDesc.NumDescriptors = FrameCount;
////		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
////		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
////		ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
////		SetName(m_rtvHeap.Get(), L"Render target view");
////
////		// Describe and create a shader resource view (SRV) heap for the texture.
////		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
////		srvHeapDesc.NumDescriptors = 1;
////		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
////		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
////		ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
////		SetName(m_srvHeap.Get(), L"Shader Resource view");
////
////		// Describe and create a constant buffer view (CBV) descriptor heap.
////		// Flags indicate that this descriptor heap can be bound to the pipeline 
////		// and that descriptors contained in it can be referenced by a root table.
////		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
////		cbvHeapDesc.NumDescriptors = 1;
////		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
////		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
////		ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));
////		SetName(m_cbvHeap.Get(), L"Constant buffer view");
////	}
////
////	// Create frame resources.
////	UpdateRenderTargetViews();
////
////	// Create command allocator
////	{
////		for (UINT n = 0; n < FrameCount; n++)
////		{
////			ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])));
////		}
////
////		// Create the command list.
////		ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
////	}
//}

void D3D12TexturedCub::LoadAssets(ID3D12Device2* device, ID3D12GraphicsCommandList* commandList)
{
	HRESULT hr = S_OK;

	// Create the vertex buffer.
	{
		// Define the geometry for a triangle.
		BlockVertex rectangleVertices[] =
		{
			{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
			{ { -0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f } },
			{ { 0.5f, 0.5f, -0.5f }, { 1.0f, 0.0f } },
			{ { 0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f } },
			{ { -0.5f, -0.5f, 0.5f }, { 0.0f, 1.0f } },
			{ { -0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f } },
			{ { 0.5f, 0.5f, 0.5f }, { 1.0f, 0.0f } },
			{ { 0.5f, -0.5f, 0.5f }, { 1.0f, 1.0f } },
		};
		const UINT vertexBufferSize = sizeof(rectangleVertices);

		short indices[] = { 4, 5, 1, 4, 1, 0, 0, 1, 2, 0, 2, 3, 3, 2, 6, 3, 6, 7,
		1, 5, 6, 1, 6, 2, 5, 4, 7, 5, 7, 6, 4, 0, 3, 4, 3, 7 };
		const UINT indexBufferSize = sizeof(indices);
		m_IndexCount = sizeof(indices) / sizeof(short);

		// Note: using upload heaps to transfer static data like vert buffers is not 
		// recommended. Every time the GPU needs it, the upload heap will be marshalled 
		// over. Please read up on Default Heap usage. An upload heap is used here for 
		// code simplicity and because there are very few verts to actually transfer.
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)));

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, rectangleVertices, sizeof(rectangleVertices));
		m_vertexBuffer->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(BlockVertex);
		m_vertexBufferView.SizeInBytes = vertexBufferSize;

		// index buffer
		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_indexBuffer));
		readRange.Begin = readRange.End = 0;
		hr = m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
		memcpy(pVertexDataBegin, indices, indexBufferSize);
		m_indexBuffer->Unmap(0, nullptr);

		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
		m_indexBufferView.SizeInBytes = indexBufferSize;
	}

	// Note: ComPtr's are CPU objects but this resource needs to stay in scope until
	// the command list that references it has finished executing on the GPU.
	// We will flush the GPU at the end of this method to ensure the resource is not
	// prematurely destroyed.
	ComPtr<ID3D12Resource> textureUploadHeap;

	// Create the texture.
	{
		// Describe and create a Texture2D.
		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.Width = TextureWidth;
		textureDesc.Height = TextureHeight;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_texture)));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);

		// Create the GPU upload buffer.
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&textureUploadHeap)));

		// Copy data to the intermediate upload heap and then schedule a copy 
		// from the upload heap to the Texture2D.
		std::vector<UINT8> texture = GenerateTextureData();

		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = &texture[0];
		textureData.RowPitch = TextureWidth * TexturePixelSize;
		textureData.SlicePitch = textureData.RowPitch * TextureHeight;

		UpdateSubresources(commandList, m_texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		// Describe and create a SRV for the texture.
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	// Create the constant buffer.
	{
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBuffer)));

		// Describe and create a constant buffer view.
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = (sizeof(SceneConstantBuffer) + 255) & ~255;	// CB size is required to be 256-byte aligned.
		device->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

		// Map and initialize the constant buffer. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
		m_constantData.wvp = XMMatrixIdentity();
		memcpy(m_pCbvDataBegin, &m_constantData, sizeof(m_constantData));
	}

	//// Create synchronization objects and wait until assets have been uploaded to the GPU.
	//{
	//	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	//	m_fenceValue = 1;

	//	// Create an event handle to use for frame synchronization.
	//	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	//	if (m_fenceEvent == nullptr)
	//	{
	//		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	//	}

	//	// Wait for the command list to execute; we are reusing the same command 
	//	// list in our main loop but for now, we just want to wait for setup to 
	//	// complete before continuing.
	//	//WaitForPreviousFrame();
	//}
}

std::vector<UINT8> D3D12TexturedCub::GenerateTextureData()
{
	const UINT rowPitch = TextureWidth * TexturePixelSize;
	const UINT cellPitch = rowPitch >> 3;		// The width of a cell in the checkboard texture.
	const UINT cellHeight = TextureWidth >> 3;	// The height of a cell in the checkerboard texture.
	const UINT textureSize = rowPitch * TextureHeight;

	std::vector<UINT8> data(textureSize);
	UINT8* pData = &data[0];

	for (UINT n = 0; n < textureSize; n += TexturePixelSize)
	{
		UINT x = n % rowPitch;
		UINT y = n / rowPitch;
		UINT i = x / cellPitch;
		UINT j = y / cellHeight;

		if (i % 2 == j % 2)
		{
			pData[n] = 0x00;		// R
			pData[n + 1] = 0x00;	// G
			pData[n + 2] = 0x00;	// B
			pData[n + 3] = 0xff;	// A
		}
		else
		{
			pData[n] = 0xff;		// R
			pData[n + 1] = 0xff;	// G
			pData[n + 2] = 0xff;	// B
			pData[n + 3] = 0xff;	// A
		}
	}

	return data;
}
//
//void D3D12TexturedCub::PopulateCommandList()
//{
//}
//
//void D3D12TexturedCub::WaitForPreviousFrame()
//{
//	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
//	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
//	// sample illustrates how to use fences for efficient resource usage and to
//	// maximize GPU utilization.
//
//	// Signal and increment the fence value.
//	const UINT64 fence = m_fenceValue++;
//	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
//
//	// Wait until the previous frame is finished.
//	if (m_fence->GetCompletedValue() < fence)
//	{
//		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
//		WaitForSingleObject(m_fenceEvent, INFINITE);
//	}
//
//	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
//}

void D3D12TexturedCub::OnUpdate(long long lTicks)
{
	m_constantData.wvp = GetWVP(XMMatrixIdentity());

	memcpy(m_pCbvDataBegin, &m_constantData, sizeof(m_constantData));
}

void D3D12TexturedCub::OnRender(ID3D12GraphicsCommandList* commandList)
{
	//commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	////m_commandList->DrawInstanced(3, 1, 0, 0);
	//commandList->IASetIndexBuffer(&m_indexBufferView);

	//ID3D12DescriptorHeap* ppHeaps[] = { m_cbvHeap.Get() };
	//commandList->SetDescriptorHeaps(arraysize(ppHeaps), ppHeaps);
	//commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	//ppHeaps[0] = { m_srvHeap.Get() };
	//commandList->SetDescriptorHeaps(arraysize(ppHeaps), ppHeaps);
	//commandList->SetGraphicsRootDescriptorTable(1, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
	//commandList->DrawIndexedInstanced(m_IndexCount, 1, 0, 0, 0);
}

void D3D12TexturedCub::OnDestroy() {
}

void D3D12TexturedCub::OnResize(UINT32 width, UINT32 height) {
}
