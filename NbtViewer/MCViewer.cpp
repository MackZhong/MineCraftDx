#include "stdafx.h"
#include "MCViewer.h"
#include "NbtReader.h"
#include "DxWindow.h"
#include "DxHelper.h"

using namespace DirectX;
using namespace Microsoft::WRL;

namespace MineCraft {
	// A structure to hold the data for a per-object constant buffer
// defined in the vertex shader.
	struct PerFrameConstantBufferData
	{
		DirectX::XMMATRIX ViewProjectionMatrix;
	};

	// This structure is used in the simple vertex shader.
	struct PerObjectConstantBufferData
	{
		DirectX::XMMATRIX WorldMatrix;
		DirectX::XMMATRIX InverseTransposeWorldMatrix;
		DirectX::XMMATRIX WorldViewProjectionMatrix;
	};

	MCViewer::MCViewer(DxWindow& window)
		: super(window)
		, m_BasePath(L"E:/Games/MineCraft/.minecraft/versions/1.12.2/saves/新的世界")
	{
		pData = (AlignedData*)_aligned_malloc(sizeof(AlignedData), 16);

		DirectX::XMVECTOR cameraPos = DirectX::XMVectorSet(0, 5, -20, 1);
		DirectX::XMVECTOR cameraTarget = DirectX::XMVectorSet(0, 5, 0, 1);
		DirectX::XMVECTOR cameraUp = DirectX::XMVectorSet(0, 1, 0, 0);

		m_Camera.set_LookAt(cameraPos, cameraTarget, cameraUp);

		pData->m_InitialCameraPos = m_Camera.get_Translation();
		pData->m_InitialCameraRot = m_Camera.get_Rotation();
	}

	MCViewer::~MCViewer()
	{
		// Make sure the content is unloaded.
		UnloadContent();
		_aligned_free(pData);
	}

	bool MCViewer::LoadChunks(Byte8 ySection, int zChunk, int xChunk) {
		int regionX = xChunk >> 5;
		int regionZ = zChunk >> 5;
		wchar_t pathName[MAX_PATH];
		wsprintfW(pathName, L"%s/region/r.%i.%i.mca", m_BasePath, regionX, regionZ);

		CompoundTagPtr regions = nullptr;
		auto mca = m_Regions.find(pathName);
		if (m_Regions.end() != mca)
			regions = mca->second;
		else {
			regions = NbtReader::LoadRegionFile(pathName);
			if (nullptr != regions) {
				m_Regions.try_emplace(pathName, regions);
				//std::wofstream ofs("regions.dat", std::ios::binary);
				//ofs << pathName << std::endl;
				//ofs << *regions;
				//ofs.close();
			}
		}

		if (nullptr == regions) {
			return false;
		}

		wsprintfW(pathName, L"%d,%d", xChunk & 31, zChunk & 31);
		CompoundTagPtr chunk = regions->GetByName<CompoundTag>(pathName);
		if (nullptr != chunk) {
			CompoundTagPtr _Level = chunk->GetByName<CompoundTag>(L"Level");
			IntTag* _DataVersion = chunk->GetByName<IntTag>(L"DataVersion");
			IntTag* _LastChange = chunk->GetByName<IntTag>(L"LastChange");

			if (nullptr == _DataVersion || nullptr == _Level) {
				throw "Error chunk format";
			}

			Int32 datVersion;
			_DataVersion->GetValue(&datVersion);
			assert(datVersion == 1343);

			auto _xPos = _Level->GetByName<IntTag>(L"xPos");
			Int32 xPos;
			_xPos->GetValue(&xPos);
			auto _zPos = _Level->GetByName<IntTag>(L"zPos");
			Int32 zPos;
			_zPos->GetValue(&zPos);

			auto sectionBlocks = std::make_unique<Block[]>(4096);
			ListTagPtr _Sections = _Level->GetByName<ListTag>(L"Sections");
			for (int s = 0; s < _Sections->Size(); s++) {
				CompoundTagPtr section = _Sections->GetByIndex<CompoundTag>(s);
				if (nullptr == section) {
					continue;
				}
				auto _Y = section->GetByName<ByteTag>(L"Y");
				Byte8 y;
				_Y->GetValue(&y);
				if (y < ySection - m_Range && y > ySection + m_Range) {
					continue;
				}
				auto _Data = section->GetByName<ByteArrayTag>(L"Data");
				Byte8* datas = (Byte8*)_Data->Value();
				auto _BLockLight = section->GetByName<ByteArrayTag>(L"BLockLight");
				Byte8* blockLights = (Byte8*)_BLockLight->Value();
				auto _SkyLight = section->GetByName<ByteArrayTag>(L"SkyLight");
				Byte8* skyLights = (Byte8*)_SkyLight->Value();
				auto _Blocks = section->GetByName<ByteArrayTag>(L"Blocks");
				Byte8* blocks = (Byte8*)_Blocks->Value();
				assert(_Blocks->Size() == 4096);
				for (int b = 0; b < 4096; b++) {
					sectionBlocks[b].Id = blocks[b];
					int i = b / 2;
					if (0 == b % 2)
					{
						sectionBlocks[b].Data = datas[i] & 0x0F;
						sectionBlocks[b].BlockLight = blockLights[i] & 0x0F;
						sectionBlocks[b].SkyLight = skyLights[i] & 0x0F;
					}
					else {
						sectionBlocks[b].Data = (datas[i] & 0xF0) >> 4;
						sectionBlocks[b].BlockLight = (blockLights[i] & 0xF0) >> 4;
						sectionBlocks[b].SkyLight = (skyLights[i] & 0xF0) >> 4;
					}
				}
				auto _Add = section->GetByName<ByteArrayTag>(L"Add");
				if (nullptr != _Add) {
					Byte8* adds = (Byte8*)_Add->Value();
					for (int i = 0; i < 2048; i++) {
						int b = i << 1;
						sectionBlocks[b].Id += (adds[i] & 0x0F) << 8;
						sectionBlocks[b + 1].Id += ((adds[i] & 0xF0) >> 4) << 8;
					}
				}
				int posX = 0;
				int posZ = 0;
				int posY = 0;
				float startX = xPos * 16.0f;
				float startZ = zPos * 16.0f;
				for (int b = 0; b < 4096; b++, posX++) {
					if (posX > 15) {
						posZ++;
						posX = 0;
					}
					if (posZ > 15) {
						posY++;
						posZ = 0;
						posX = 0;
					}
					assert(posY < 16);
					sectionBlocks[b].pos = { posX + startX, posY * 16.0f, posZ + startZ };
					m_Blocks.Add(sectionBlocks[b]);
				}
			}	// sections
		}	// chunk

		return true;
	}

	bool MCViewer::LoadContent()
	{
		CompoundTagPtr root = NbtReader::LoadFromFile(L"E:/Games/MineCraft/.minecraft/versions/1.12.2/saves/新的世界/level.dat");
		{
			CompoundTagPtr data = root->GetByName<CompoundTag>(L"data");
			CompoundTagPtr _Player = data->GetByName<CompoundTag>(L"Player");

			//std::wofstream ofs("player.dat", std::ios::binary);
			//ofs << *_Player;
			//ofs.close();

			ListTagPtr _Pos = _Player->GetByName<ListTag>(L"Pos");
			//Double64 x = _Pos->Get<Double64>(0);
			XMFLOAT3 pos((float)_Pos->GetInternalValue<Double64>(0),
				(float)_Pos->GetInternalValue<Double64>(1),
				(float)_Pos->GetInternalValue<Double64>(2));
			ListTagPtr _Rotation = _Player->GetByName<ListTag>(L"Rotation");
			float yaw = _Rotation->GetInternalValue<Float32>(0);
			float pitch = _Rotation->GetInternalValue<Float32>(1);

			int xChunk = (int)(pos.x / 16.0f);
			int zChunk = (int)(pos.z / 16.0f);
			Byte8 ySection = (Byte8)(pos.y / 16.0f);

			for (int x = xChunk - m_Range; x < xChunk + m_Range; x++) {
				for (int z = zChunk - m_Range; z < zChunk + m_Range; z++) {
					LoadChunks(ySection, z, x);
				}
			}

			//if (nullptr != regions) {
			//	//std::wofstream ofs("regions.dat", std::ios::binary);
			//	//ofs << *regions;
			//	//ofs.close();

			//	for (int c = 0; c < regions->Size(); c++) {
			//		CompoundTagPtr chunk = regions->GetByIndex<CompoundTag>(c);

			//	}
			//}

		}

		// Create a sampler state for texture sampling in the pixel shader
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = 1.0f;
		samplerDesc.BorderColor[1] = 1.0f;
		samplerDesc.BorderColor[2] = 1.0f;
		samplerDesc.BorderColor[3] = 1.0f;
		samplerDesc.MinLOD = -FLT_MAX;
		samplerDesc.MaxLOD = FLT_MAX;

		HRESULT hr = m_d3dDevice->CreateSamplerState(&samplerDesc, m_d3dSamplerState.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			MessageBoxW(m_Window.get_WindowHandle(), L"Failed to create texture sampler.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		// Create some materials
		MaterialProperties defaultMaterial;
		m_MaterialProperties.push_back(defaultMaterial);

		MaterialProperties greenMaterial;
		greenMaterial.Material.Ambient = DirectX::XMFLOAT4(0.07568f, 0.61424f, 0.07568f, 1.0f);
		greenMaterial.Material.Diffuse = DirectX::XMFLOAT4(0.07568f, 0.61424f, 0.07568f, 1.0f);
		greenMaterial.Material.Specular = DirectX::XMFLOAT4(0.07568f, 0.61424f, 0.07568f, 1.0f);
		greenMaterial.Material.SpecularPower = 76.8f;
		m_MaterialProperties.push_back(greenMaterial);

		MaterialProperties redPlasticMaterial;
		redPlasticMaterial.Material.Diffuse = DirectX::XMFLOAT4(0.6f, 0.1f, 0.1f, 1.0f);
		redPlasticMaterial.Material.Specular = DirectX::XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
		redPlasticMaterial.Material.SpecularPower = 32.0f;
		m_MaterialProperties.push_back(redPlasticMaterial);

		MaterialProperties pearlMaterial;
		pearlMaterial.Material.Ambient = DirectX::XMFLOAT4(0.25f, 0.20725f, 0.20725f, 1.0f);
		pearlMaterial.Material.Diffuse = DirectX::XMFLOAT4(1.0f, 0.829f, 0.829f, 1.0f);
		pearlMaterial.Material.Specular = DirectX::XMFLOAT4(0.296648f, 0.296648f, 0.296648f, 1.0f);
		pearlMaterial.Material.SpecularPower = 11.264f;
		m_MaterialProperties.push_back(pearlMaterial);

		Microsoft::WRL::ComPtr<ID3DBlob> shaderCode;
		hr = CompileShader(L"Shaders/PixelShader.hlsl", "TexturedLitPixelShader", "ps_5_0", shaderCode.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			MessageBoxW(m_Window.get_WindowHandle(), L"Failed to compile the pixel shader.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		hr = m_d3dDevice->CreatePixelShader(shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(),
			nullptr, m_d3dTexturedLitPixelShader.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			MessageBoxW(m_Window.get_WindowHandle(), L"Failed to create the pixel shader.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		// Create a constant buffer for the per-object data required by teh simple vertex shader.
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = 0;
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		constantBufferDesc.ByteWidth = sizeof(PerObjectConstantBufferData);
		hr = m_d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, m_d3dPerObjectConstantBuffer.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			MessageBoxW(m_Window.get_WindowHandle(), L"Failed to create constant buffer for per-object data.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		// Create a constant buffer for the material properties required by the pixel shader.
		constantBufferDesc.ByteWidth = sizeof(MaterialProperties);
		hr = m_d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, m_d3dMaterialPropertiesConstantBuffer.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			MessageBoxW(m_Window.get_WindowHandle(), L"Failed to create constant buffer for material properties.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		// Create a constant buffer for the light properties required by the pixel shader.
		constantBufferDesc.ByteWidth = sizeof(LightProperties);
		hr = m_d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, m_d3dLightPropertiesConstantBuffer.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			MessageBoxW(m_Window.get_WindowHandle(), L"Failed to create constant buffer for light properties.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		// Create a constant buffer for the per-frame data required by the instanced vertex shader.
		constantBufferDesc.ByteWidth = sizeof(PerFrameConstantBufferData);
		hr = m_d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, m_d3dPerFrameConstantBuffer.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			MessageBoxW(m_Window.get_WindowHandle(), L"Failed to create constant buffer for per-frame data.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		// Global ambient
		m_LightProperties.GlobalAmbient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

		m_Cube = DxMesh::CreateCube(m_d3dDeviceContext.Get());
		m_LightSphere = DxMesh::CreateSphere(m_d3dDeviceContext.Get(), 1.0f, 16, false);
		m_LightCone = DxMesh::CreateCone(m_d3dDeviceContext.Get(), 1.0f, 1.0f, 32, false);

		// Load a simple vertex shader that will be used to render the shapes.
		hr = CompileShader(L"Shaders/VertexShader.hlsl", "SimpleVertexShader", "vs_5_0", shaderCode.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			MessageBoxW(m_Window.get_WindowHandle(), L"Failed to compile the simple vertex shader.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		hr = m_d3dDevice->CreateVertexShader(shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(),
			nullptr, m_d3dSimplVertexShader.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			MessageBoxW(m_Window.get_WindowHandle(), L"Failed to create the simple vertex shader.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		// Create the input layout for the simple shapes.
		hr = m_d3dDevice->CreateInputLayout(VertexPositionNormalTexture::InputElements,
			VertexPositionNormalTexture::InputElementCount,
			shaderCode->GetBufferPointer(), shaderCode->GetBufferSize(),
			m_d3dVertexPositionNormalTextureInputLayout.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			MessageBoxW(m_Window.get_WindowHandle(), L"Failed to create the input layout for the simple vertex shader.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		// Force a resize event so the camera's projection matrix gets initialized.
		ResizeEventArgs resizeEventArgs(m_Window.get_ClientWidth(), m_Window.get_ClientHeight());
		OnResize(resizeEventArgs);

		return true;
	}

	void MCViewer::UnloadContent()
	{
	}

	void MCViewer::OnUpdate(UpdateEventArgs & e)
	{
		// Camera

		// Update the light properties
		XMStoreFloat4(&m_LightProperties.EyePosition, m_Camera.get_Translation());

		static const DirectX::XMVECTORF32 LightColors[MAX_LIGHTS] = {
			DirectX::Colors::White, DirectX::Colors::Orange, DirectX::Colors::Yellow, DirectX::Colors::Green, DirectX::Colors::Blue, DirectX::Colors::Indigo, DirectX::Colors::Violet, DirectX::Colors::White
		};

		static const LightType LightTypes[MAX_LIGHTS] = {
			SpotLight, SpotLight, SpotLight, PointLight, SpotLight, SpotLight, SpotLight, PointLight
		};

		static const bool LightEnabled[MAX_LIGHTS] = {
			true, true, true, true, true, true, true, true
		};

		const int numLights = MAX_LIGHTS;
		float radius = 8.0f;
		float offset = 2.0f * DirectX::XM_PI / numLights;
		for (int i = 0; i < numLights; ++i)
		{
			Light light;
			light.Enabled = static_cast<int>(LightEnabled[i]);
			light.LightType = LightTypes[i];
			light.Color = DirectX::XMFLOAT4(LightColors[i]);
			light.SpotAngle = DirectX::XMConvertToRadians(45.0f);
			light.ConstantAttenuation = 1.0f;
			light.LinearAttenuation = 0.08f;
			light.QuadraticAttenuation = 0.0f;
			DirectX::XMFLOAT4 LightPosition = DirectX::XMFLOAT4(std::sin(offset * i) * radius, 9.0f, std::cos(offset * i) * radius, 1.0f);
			light.Position = LightPosition;
			DirectX::XMVECTOR LightDirection = DirectX::XMVectorSet(-LightPosition.x, -LightPosition.y, -LightPosition.z, 0.0f);
			LightDirection = DirectX::XMVector3Normalize(LightDirection);
			XMStoreFloat4(&light.Direction, LightDirection);

			m_LightProperties.Lights[i] = light;
		}

		// Update the light properties
		m_d3dDeviceContext->UpdateSubresource(m_d3dLightPropertiesConstantBuffer.Get(), 0, nullptr, &m_LightProperties, 0, 0);

	}

	// Builds a look-at (world) matrix from a point, up and direction vectors.
	DirectX::XMMATRIX XM_CALLCONV LookAtMatrix(DirectX::FXMVECTOR Position, DirectX::FXMVECTOR Direction, DirectX::FXMVECTOR Up)
	{
		assert(!DirectX::XMVector3Equal(Direction, DirectX::XMVectorZero()));
		assert(!DirectX::XMVector3IsInfinite(Direction));
		assert(!DirectX::XMVector3Equal(Up, DirectX::XMVectorZero()));
		assert(!DirectX::XMVector3IsInfinite(Up));

		DirectX::XMVECTOR R2 = DirectX::XMVector3Normalize(Direction);

		DirectX::XMVECTOR R0 = DirectX::XMVector3Cross(Up, R2);
		R0 = DirectX::XMVector3Normalize(R0);

		DirectX::XMVECTOR R1 = DirectX::XMVector3Cross(R2, R0);

		DirectX::XMMATRIX M(R0, R1, R2, Position);

		return M;
	}

	void MCViewer::OnRender(RenderEventArgs & e)
	{
		float aspectRatio = m_Window.get_ClientWidth() / (float)m_Window.get_ClientHeight();

		DirectX::XMMATRIX viewMatrix = m_Camera.get_ViewMatrix();
		DirectX::XMMATRIX projectionMatrix = m_Camera.get_ProjectionMatrix();
		DirectX::XMMATRIX viewProjectionMatrix = viewMatrix * projectionMatrix;

		PerFrameConstantBufferData constantBufferData;
		constantBufferData.ViewProjectionMatrix = viewProjectionMatrix;

		m_d3dDeviceContext->UpdateSubresource(m_d3dPerFrameConstantBuffer.Get(), 0, nullptr, &constantBufferData, 0, 0);

		m_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_d3dDeviceContext->RSSetState(m_d3dRasterizerState.Get());
		D3D11_VIEWPORT viewport = m_Camera.get_Viewport();
		m_d3dDeviceContext->RSSetViewports(1, &viewport);

		m_d3dDeviceContext->VSSetConstantBuffers(0, 1, m_d3dPerFrameConstantBuffer.GetAddressOf());

		m_d3dDeviceContext->PSSetShader(m_d3dTexturedLitPixelShader.Get(), nullptr, 0);

		ID3D11Buffer* pixelShaderConstantBuffers[2] = {
			m_d3dMaterialPropertiesConstantBuffer.Get(),
			m_d3dLightPropertiesConstantBuffer.Get() };
		m_d3dDeviceContext->PSSetConstantBuffers(0, 2, pixelShaderConstantBuffers);

		m_d3dDeviceContext->PSSetSamplers(0, 1, m_d3dSamplerState.GetAddressOf());

		m_d3dDeviceContext->OMSetRenderTargets(1, m_d3dRenderTargetView.GetAddressOf(), m_d3dDepthStencilView.Get());
		m_d3dDeviceContext->OMSetDepthStencilState(m_d3dDepthStencilState.Get(), 0);

		// Draw a cube
		DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(0.0f, 8.0f, 0.0f);
		DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(45.0f));
		rotationMatrix *= DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(45.0f));
		DirectX::XMMATRIX worldMatrix = rotationMatrix * translationMatrix;

		PerObjectConstantBufferData perObjectConstantBufferData;
		perObjectConstantBufferData.WorldMatrix = worldMatrix;
		perObjectConstantBufferData.InverseTransposeWorldMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));
		perObjectConstantBufferData.WorldViewProjectionMatrix = worldMatrix * viewProjectionMatrix;

		m_d3dDeviceContext->UpdateSubresource(m_d3dPerObjectConstantBuffer.Get(), 0, nullptr, &perObjectConstantBufferData, 0, 0);

		m_d3dDeviceContext->UpdateSubresource(m_d3dMaterialPropertiesConstantBuffer.Get(), 0, nullptr, &m_MaterialProperties[2], 0, 0);

		m_d3dDeviceContext->VSSetShader(m_d3dSimplVertexShader.Get(), nullptr, 0);
		m_d3dDeviceContext->VSSetConstantBuffers(0, 1, m_d3dPerObjectConstantBuffer.GetAddressOf());

		m_d3dDeviceContext->IASetInputLayout(m_d3dVertexPositionNormalTextureInputLayout.Get());

		m_Cube->Draw(m_d3dDeviceContext.Get());

		// Draw geometry at the position of the active lights in the scene.
		MaterialProperties lightMaterial = m_MaterialProperties[0];
		for (int i = 0; i < MAX_LIGHTS; ++i)
		{
			Light* pLight = &(m_LightProperties.Lights[i]);
			if (!pLight->Enabled) continue;

			DirectX::XMVECTOR lightPos = XMLoadFloat4(&(pLight->Position));
			DirectX::XMVECTOR lightDir = XMLoadFloat4(&(pLight->Direction));
			DirectX::XMVECTOR UpDirection = DirectX::XMVectorSet(0, 1, 0, 0);

			rotationMatrix = DirectX::XMMatrixRotationX(-90.0f);
			worldMatrix = rotationMatrix * LookAtMatrix(lightPos, lightDir, UpDirection);

			perObjectConstantBufferData.WorldMatrix = worldMatrix;
			perObjectConstantBufferData.InverseTransposeWorldMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));
			perObjectConstantBufferData.WorldViewProjectionMatrix = worldMatrix * viewProjectionMatrix;

			lightMaterial.Material.Emissive = pLight->Color;

			m_d3dDeviceContext->UpdateSubresource(m_d3dPerObjectConstantBuffer.Get(), 0, nullptr, &perObjectConstantBufferData, 0, 0);
			m_d3dDeviceContext->UpdateSubresource(m_d3dMaterialPropertiesConstantBuffer.Get(), 0, nullptr, &lightMaterial, 0, 0);
			switch (pLight->LightType)
			{
			case PointLight:
			{
				m_LightSphere->Draw(m_d3dDeviceContext.Get());
			}
			break;
			case DirectionalLight:
			case SpotLight:
			{
				m_LightCone->Draw(m_d3dDeviceContext.Get());
			}
			break;
			}
		}
	}

	void MCViewer::OnResize(ResizeEventArgs & e)
	{
		// Don't forget to call the base class's resize method.
		// The base class handles resizing of the swap chain.
		super::OnResize(e);

		if (e.Height < 1)
		{
			e.Height = 1;
		}

		float aspectRatio = e.Width / (float)e.Height;

		m_Camera.set_Projection(45.0f, aspectRatio, 0.1f, 100.0f);

		// Setup the viewports for the camera.
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = static_cast<FLOAT>(m_Window.get_ClientWidth());
		viewport.Height = static_cast<FLOAT>(m_Window.get_ClientHeight());
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		m_Camera.set_Viewport(viewport);

		m_d3dDeviceContext->RSSetViewports(1, &viewport);
	}
}
