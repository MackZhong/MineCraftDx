#include "pch.h"
#include "TexturedCube.h"
#include "Application.h"
#include "CommandQueue.h"
#include "CommandList.h"
#include "Helpers.h"
#include "Light.h"
#include "Material.h"
#include "Window.h"

using namespace DirectX;
using namespace Microsoft::WRL;

namespace {
	struct Mat
	{
		XMMATRIX ModelMatrix;
		XMMATRIX ModelViewMatrix;
		XMMATRIX InverseTransposeModelViewMatrix;
		XMMATRIX ModelViewProjectionMatrix;
	};

	struct LightProperties
	{
		uint32_t NumPointLights;
		uint32_t NumSpotLights;
	};

	// An enum for root signature parameters.
	enum RootParameters
	{
		MatricesCB,         // ConstantBuffer<Mat> MatCB : register(b0);
		MaterialCB,         // ConstantBuffer<Material> MaterialCB : register( b0, space1 );
		LightPropertiesCB,  // ConstantBuffer<LightProperties> LightPropertiesCB : register( b1 );
		PointLights,        // StructuredBuffer<PointLight> PointLights : register( t0 );
		SpotLights,         // StructuredBuffer<SpotLight> SpotLights : register( t1 );
		Textures,           // Texture2D DiffuseTexture : register( t2 );
		NumRootParameters
	};

	// Clamp a value between a min and max range.
	template<typename T>
	constexpr const T& clamp(const T& val, const T& min, const T& max)
	{
		return val < min ? min : val > max ? max : val;
	};

	// Builds a look-at (world) matrix from a point, up and direction vectors.
	XMMATRIX XM_CALLCONV LookAtMatrix(FXMVECTOR Position, FXMVECTOR Direction, FXMVECTOR Up)
	{
		assert(!XMVector3Equal(Direction, XMVectorZero()));
		assert(!XMVector3IsInfinite(Direction));
		assert(!XMVector3Equal(Up, XMVectorZero()));
		assert(!XMVector3IsInfinite(Up));

		XMVECTOR R2 = XMVector3Normalize(Direction);

		XMVECTOR R0 = XMVector3Cross(Up, R2);
		R0 = XMVector3Normalize(R0);

		XMVECTOR R1 = XMVector3Cross(R2, R0);

		XMMATRIX M(R0, R1, R2, Position);

		return M;
	};
};

TexturedCube::TexturedCube(const std::wstring& name, int width, int height, bool vSync)
	: super(name, width, height, vSync)
	, m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
	, m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)))
	, m_DepthBuffer(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT,
		width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0),
		D3D12_RESOURCE_STATE_COMMON, L"Depth Buffer")
	, m_Forward(0)
	, m_Backward(0)
	, m_Left(0)
	, m_Right(0)
	, m_Up(0)
	, m_Down(0)
	, m_Pitch(0)
	, m_Yaw(0)
	, m_AnimateLights(false)
	, m_Shift(false)
	, m_Width(800)
	, m_Height(600) {
	XMVECTOR cameraPos = XMVectorSet(3, 8, -15, 1);
	XMVECTOR cameraTarget = XMVectorSet(1, -5, 0, 1);
	XMVECTOR cameraUp = XMVectorSet(0, 1, 0, 0);

	m_Camera.set_LookAt(cameraPos, cameraTarget, cameraUp);

	m_pAlignedCameraData = (CameraData*)_aligned_malloc(sizeof(CameraData), 16);

	m_pAlignedCameraData->m_InitialCamPos = m_Camera.get_Translation();
	m_pAlignedCameraData->m_InitialCamRot = m_Camera.get_Rotation();
}

TexturedCube::~TexturedCube()
{
	_aligned_free(m_pAlignedCameraData);
}

bool TexturedCube::LoadContent()
{
	auto device = Application::Get().GetDevice();
	auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
	auto commandList = commandQueue->GetCommandList();

	// Create a Cube mesh
	m_CubeMesh = Mesh::CreateCube(*commandList);
	m_SphereMesh = Mesh::CreateSphere(*commandList);
	m_ConeMesh = Mesh::CreateCone(*commandList);

	// Load some textures
	commandList->LoadTextureFromFile(m_DefaultTexture, L"Textures/DefaultWhite.bmp");
	m_DefaultTexture.SetName(L"Default Texture");
	//commandList->LoadTextureFromFile(m_MonaLisaTexture, L"Textures/Mona_Lisa.jpg");

	commandList->CreateCubeTexture(L"CubeTextures", m_MonaLisaTexture);

	// Load the vertex shader.
	ComPtr<ID3DBlob> vertexShaderBlob;
	ThrowIfFailed(D3DReadFileToBlob(L"Shaders/TexturedCube_VS.cso", &vertexShaderBlob));

	// Load the pixel shader.
	ComPtr<ID3DBlob> pixelShaderBlob;
	ThrowIfFailed(D3DReadFileToBlob(L"Shaders/TexturedCube_PS.cso", &pixelShaderBlob));

	// Create a root signature.
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	// Allow input layout and deny unnecessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	CD3DX12_DESCRIPTOR_RANGE1 descriptorRage(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);

	CD3DX12_ROOT_PARAMETER1 rootParameters[RootParameters::NumRootParameters];
	rootParameters[RootParameters::MatricesCB].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[RootParameters::MaterialCB].InitAsConstantBufferView(0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootParameters::LightPropertiesCB].InitAsConstants(sizeof(LightProperties) / 4, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootParameters::PointLights].InitAsShaderResourceView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootParameters::SpotLights].InitAsShaderResourceView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootParameters::Textures].InitAsDescriptorTable(1, &descriptorRage, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_STATIC_SAMPLER_DESC linearRepeatSampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
	CD3DX12_STATIC_SAMPLER_DESC anisotropicSampler(0, D3D12_FILTER_ANISOTROPIC);
	CD3DX12_STATIC_SAMPLER_DESC pointSampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
	rootSignatureDescription.Init_1_1(RootParameters::NumRootParameters, rootParameters, 1, &pointSampler, rootSignatureFlags);

	m_RootSignature.SetRootSignatureDesc(rootSignatureDescription.Desc_1_1, featureData.HighestVersion);

	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
	} pipelineStateStream;

	D3D12_RT_FORMAT_ARRAY rtvFormats = {};
	rtvFormats.NumRenderTargets = 1;
	rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	pipelineStateStream.pRootSignature = m_RootSignature.GetRootSignature().Get();
	pipelineStateStream.InputLayout = { CubeVertexPositionNormalTexture::InputElements, CubeVertexPositionNormalTexture::InputElementCount };
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
	pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.RTVFormats = rtvFormats;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
	};

	ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipelineState)));

	auto fenceValue = commandQueue->ExecuteCommandList(commandList);
	commandQueue->WaitForFenceValue(fenceValue);

	return true;
}

void TexturedCube::ResizeDepthBuffer(int width, int height)
{
	// Flush any GPU commands that might be referencing the depth buffer.
	Application::Get().Flush();

	width = std::max(1, width);
	height = std::max(1, height);

	m_DepthBuffer.Resize(width, height);
}

void TexturedCube::OnResize(ResizeEventArgs& e)
{
	super::OnResize(e);

	if (m_Width != e.Width || m_Height != e.Height)
	{
		m_Width = e.Width;
		m_Height = e.Height;

		float aspectRatio = e.Width / (float)e.Height;
		m_Camera.set_Projection(75.0f, aspectRatio, 0.1f, 100.0f);

		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
			static_cast<float>(e.Width), static_cast<float>(e.Height));

		ResizeDepthBuffer(e.Width, e.Height);
	}
}

void TexturedCube::UnloadContent()
{
}

void TexturedCube::OnUpdate(UpdateEventArgs& e)
{
	static uint64_t frameCount = 0;
	static double totalTime = 0.0;

	super::OnUpdate(e);

	totalTime += e.ElapsedTime;
	frameCount++;

	if (totalTime > 1.0)
	{
		double fps = frameCount / totalTime;

		wchar_t buffer[512];
		swprintf_s(buffer, L"%s - FPS: %f\n", this->m_pWindow->GetWindowName().c_str(), fps);
		this->m_pWindow->SetWindowTitle(buffer);

		frameCount = 0;
		totalTime = 0.0;
	}


	// Update the camera.
	float speedMultipler = (m_Shift ? 16.0f : 4.0f);

	XMVECTOR cameraTranslate = XMVectorSet(m_Right - m_Left, 0.0f, m_Forward - m_Backward, 1.0f) * speedMultipler * static_cast<float>(e.ElapsedTime);
	XMVECTOR cameraPan = XMVectorSet(0.0f, m_Up - m_Down, 0.0f, 1.0f) * speedMultipler * static_cast<float>(e.ElapsedTime);
	m_Camera.Translate(cameraTranslate, Space::Local);
	m_Camera.Translate(cameraPan, Space::Local);

	XMVECTOR cameraRotation = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_Pitch), XMConvertToRadians(m_Yaw), 0.0f);
	m_Camera.set_Rotation(cameraRotation);

	XMMATRIX viewMatrix = m_Camera.get_ViewMatrix();

	const int numPointLights = 4;
	const int numSpotLights = 4;

	static const XMVECTORF32 LightColors[] =
	{
		Colors::White, Colors::Orange, Colors::Yellow, Colors::Green, Colors::Blue, Colors::Indigo, Colors::Violet, Colors::White
	};

	static float lightAnimTime = 0.0f;
	if (m_AnimateLights)
	{
		lightAnimTime += static_cast<float>(e.ElapsedTime) * 0.5f * XM_PI;
	}

	const float radius = 8.0f;
	const float offset = 2.0f * XM_PI / numPointLights;
	const float offset2 = offset + (offset / 2.0f);

	// Update the cube
	static float cubeRotationTime = 0.0f;
	if (m_AnimateLights)
	{
		cubeRotationTime += static_cast<float>(e.ElapsedTime) * 0.5f * XM_PI;
	}
	XMMATRIX rotationMatrix = XMMatrixRotationY(cubeRotationTime)/* * XMMatrixRotationZ(cubeRotationTime)*/;
	XMMATRIX translationMatrix = XMMatrixTranslation(4.0f, 4.0f, 4.0f);
	XMMATRIX scaleMatrix = XMMatrixScaling(4.0f, 4.0f, 4.0f);
	m_CubeMesh->World = rotationMatrix * scaleMatrix * translationMatrix;

	// Setup the light buffers.
	m_PointLights.resize(numPointLights);
	for (int i = 0; i < numPointLights; ++i)
	{
		PointLight& l = m_PointLights[i];

		l.PositionWS = {
			static_cast<float>(std::sin(lightAnimTime + offset * i)) * radius,
			9.0f,
			static_cast<float>(std::cos(lightAnimTime + offset * i)) * radius,
			1.0f
		};
		XMVECTOR positionWS = XMLoadFloat4(&l.PositionWS);
		XMVECTOR positionVS = XMVector3TransformCoord(positionWS, viewMatrix);
		XMStoreFloat4(&l.PositionVS, positionVS);

		l.Color = XMFLOAT4(LightColors[i]);
		l.ConstantAttenuation = 1.0f;
		l.LinearAttenuation = 0.08f;
		l.QuadraticAttenuation = 0.0f;
	}

	m_SpotLights.resize(numSpotLights);
	for (int i = 0; i < numSpotLights; ++i)
	{
		SpotLight& l = m_SpotLights[i];

		l.PositionWS = {
			static_cast<float>(std::sin(lightAnimTime + offset * i + offset2)) * radius,
			9.0f,
			static_cast<float>(std::cos(lightAnimTime + offset * i + offset2)) * radius,
			1.0f
		};
		XMVECTOR positionWS = XMLoadFloat4(&l.PositionWS);
		XMVECTOR positionVS = XMVector3TransformCoord(positionWS, viewMatrix);
		XMStoreFloat4(&l.PositionVS, positionVS);

		XMVECTOR directionWS = XMVector3Normalize(XMVectorSetW(XMVectorNegate(positionWS), 0));
		XMVECTOR directionVS = XMVector3Normalize(XMVector3TransformNormal(directionWS, viewMatrix));
		XMStoreFloat4(&l.DirectionWS, directionWS);
		XMStoreFloat4(&l.DirectionVS, directionVS);

		l.Color = XMFLOAT4(LightColors[numPointLights + i]);
		l.SpotAngle = XMConvertToRadians(45.0f);
		l.ConstantAttenuation = 1.0f;
		l.LinearAttenuation = 0.08f;
		l.QuadraticAttenuation = 0.0f;
	}
}

// Transition a resource
void TexturedCube::TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
	Microsoft::WRL::ComPtr<ID3D12Resource> resource,
	D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		resource.Get(),
		beforeState, afterState);

	commandList->ResourceBarrier(1, &barrier);
}

// Clear a render target.
void TexturedCube::ClearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
	D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor)
{
	commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void TexturedCube::ClearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
	D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth)
{
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

void XM_CALLCONV ComputeMatrices(FXMMATRIX model, CXMMATRIX view, CXMMATRIX viewProjection, Mat& mat)
{
	mat.ModelMatrix = model;
	mat.ModelViewMatrix = model * view;
	mat.InverseTransposeModelViewMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, mat.ModelViewMatrix));
	mat.ModelViewProjectionMatrix = model * viewProjection;
}

void TexturedCube::OnRender(RenderEventArgs& e)
{
	super::OnRender(e);

	auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto commandList = commandQueue->GetCommandList();

	UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
	auto& renderTarget = m_pWindow->GetCurrentRenderTarget();

	// Clear the render targets.
	{
		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

		commandList->ClearTexture(renderTarget, clearColor);
		commandList->ClearDepthStencilTexture(m_DepthBuffer, D3D12_CLEAR_FLAG_DEPTH);
	}

	commandList->SetPipelineState(m_PipelineState);
	commandList->SetGraphicsRootSignature(m_RootSignature);

	// Upload lights
	LightProperties lightProps;
	lightProps.NumPointLights = static_cast<uint32_t>(m_PointLights.size());
	lightProps.NumSpotLights = static_cast<uint32_t>(m_SpotLights.size());

	commandList->SetGraphics32BitConstants(RootParameters::LightPropertiesCB, lightProps);
	commandList->SetGraphicsDynamicStructuredBuffer(RootParameters::PointLights, m_PointLights);
	commandList->SetGraphicsDynamicStructuredBuffer(RootParameters::SpotLights, m_SpotLights);

	commandList->SetViewport(m_Viewport);
	commandList->SetScissorRect(m_ScissorRect);

	commandList->SetRenderTarget(&renderTarget, &m_DepthBuffer);

	// Draw the earth sphere
	//XMMATRIX translationMatrix = XMMatrixTranslation(-4.0f, 2.0f, -4.0f);
	//XMMATRIX rotationMatrix = XMMatrixIdentity();
	//XMMATRIX scaleMatrix = XMMatrixScaling(4.0f, 4.0f, 4.0f);
	//XMMATRIX worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;
	//XMMATRIX viewMatrix = m_Camera.get_ViewMatrix();
	//XMMATRIX viewProjectionMatrix = viewMatrix * m_Camera.get_ProjectionMatrix();

	//Mat matrices;
	//ComputeMatrices(worldMatrix, viewMatrix, viewProjectionMatrix, matrices);

	//commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
	//commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, Material::White);
	////commandList->SetShaderResourceView(RootParameters::Textures, 0, m_EarthTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	//m_SphereMesh->Draw(*commandList);

	// Draw a cube
	XMMATRIX viewMatrix = m_Camera.get_ViewMatrix();
	XMMATRIX viewProjectionMatrix = viewMatrix * m_Camera.get_ProjectionMatrix();

	Mat matrices;
	ComputeMatrices(m_CubeMesh->World, viewMatrix, viewProjectionMatrix, matrices);

	commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
	commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, Material::White);
	commandList->SetShaderResourceView(RootParameters::Textures, 0, m_MonaLisaTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	m_CubeMesh->Draw(*commandList);

	//// Draw a torus
	//translationMatrix = XMMatrixTranslation(4.0f, 0.6f, -4.0f);
	//rotationMatrix = XMMatrixRotationY(XMConvertToRadians(45.0f));
	//scaleMatrix = XMMatrixScaling(4.0f, 4.0f, 4.0f);
	//worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	//ComputeMatrices(worldMatrix, viewMatrix, viewProjectionMatrix, matrices);

	//commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
	//commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, Material::Ruby);
	//commandList->SetShaderResourceView(RootParameters::Textures, 0, m_DefaultTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	//m_TorusMesh->Draw(*commandList);

	//// Floor plane.
	//float scalePlane = 20.0f;
	//float translateOffset = scalePlane / 2.0f;

	//translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	//rotationMatrix = XMMatrixIdentity();
	//scaleMatrix = XMMatrixScaling(scalePlane, 1.0f, scalePlane);
	//worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	//ComputeMatrices(worldMatrix, viewMatrix, viewProjectionMatrix, matrices);

	//commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
	//commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, Material::White);
	//commandList->SetShaderResourceView(RootParameters::Textures, 0, m_DirectXTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	//m_PlaneMesh->Draw(*commandList);

	//// Back wall
	//translationMatrix = XMMatrixTranslation(0, translateOffset, translateOffset);
	//rotationMatrix = XMMatrixRotationX(XMConvertToRadians(-90));
	//worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	//ComputeMatrices(worldMatrix, viewMatrix, viewProjectionMatrix, matrices);

	//commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);

	//m_PlaneMesh->Draw(*commandList);

	//// Ceiling plane
	//translationMatrix = XMMatrixTranslation(0, translateOffset * 2.0f, 0);
	//rotationMatrix = XMMatrixRotationX(XMConvertToRadians(180));
	//worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	//ComputeMatrices(worldMatrix, viewMatrix, viewProjectionMatrix, matrices);

	//commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);

	//m_PlaneMesh->Draw(*commandList);

	//// Front wall
	//translationMatrix = XMMatrixTranslation(0, translateOffset, -translateOffset);
	//rotationMatrix = XMMatrixRotationX(XMConvertToRadians(90));
	//worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	//ComputeMatrices(worldMatrix, viewMatrix, viewProjectionMatrix, matrices);

	//commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);

	//m_PlaneMesh->Draw(*commandList);

	//// Left wall
	//translationMatrix = XMMatrixTranslation(-translateOffset, translateOffset, 0);
	//rotationMatrix = XMMatrixRotationX(XMConvertToRadians(-90)) * XMMatrixRotationY(XMConvertToRadians(-90));
	//worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	//ComputeMatrices(worldMatrix, viewMatrix, viewProjectionMatrix, matrices);

	//commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
	//commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, Material::Red);
	commandList->SetShaderResourceView(RootParameters::Textures, 0, m_DefaultTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	//m_PlaneMesh->Draw(*commandList);

	//// Right wall
	//translationMatrix = XMMatrixTranslation(translateOffset, translateOffset, 0);
	//rotationMatrix = XMMatrixRotationX(XMConvertToRadians(-90)) * XMMatrixRotationY(XMConvertToRadians(90));
	//worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	//ComputeMatrices(worldMatrix, viewMatrix, viewProjectionMatrix, matrices);

	//commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
	//commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, Material::Blue);
	//m_PlaneMesh->Draw(*commandList);

	// Draw shapes to visualize the position of the lights in the scene.
	Material lightMaterial;
	// No specular
	lightMaterial.Specular = { 0, 0, 0, 1 };
	for (const auto& l : m_PointLights)
	{
		lightMaterial.Emissive = l.Color;
		XMVECTOR lightPos = XMLoadFloat4(&l.PositionWS);
		XMMATRIX worldMatrix = XMMatrixTranslationFromVector(lightPos);
		ComputeMatrices(worldMatrix, viewMatrix, viewProjectionMatrix, matrices);

		commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
		commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, lightMaterial);

		m_SphereMesh->Draw(*commandList);
	}

	for (const auto& l : m_SpotLights)
	{
		lightMaterial.Emissive = l.Color;
		XMVECTOR lightPos = XMLoadFloat4(&l.PositionWS);
		XMVECTOR lightDir = XMLoadFloat4(&l.DirectionWS);
		XMVECTOR up = XMVectorSet(0, 1, 0, 0);

		// Rotate the cone so it is facing the Z axis.
		XMMATRIX rotationMatrix = XMMatrixRotationX(XMConvertToRadians(-90.0f));
		XMMATRIX worldMatrix = rotationMatrix * LookAtMatrix(lightPos, lightDir, up);

		ComputeMatrices(worldMatrix, viewMatrix, viewProjectionMatrix, matrices);

		commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, matrices);
		commandList->SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, lightMaterial);

		m_ConeMesh->Draw(*commandList);
	}

	commandQueue->ExecuteCommandList(commandList);

	// Present
	m_pWindow->Present();
}

void TexturedCube::OnKeyPressed(KeyEventArgs& e)
{
	super::OnKeyPressed(e);

	switch (e.Key)
	{
	case KeyCode::Escape:
		Application::Get().Quit(0);
		break;
	case KeyCode::Enter:
		if (e.Alt)
		{
	case KeyCode::F11:
		m_pWindow->ToggleFullscreen();
		break;
		}
	case KeyCode::V:
		m_pWindow->ToggleVSync();
		break;
	case KeyCode::R:
		// Reset camera transform
		m_Camera.set_Translation(m_pAlignedCameraData->m_InitialCamPos);
		m_Camera.set_Rotation(m_pAlignedCameraData->m_InitialCamRot);
		m_Pitch = 0.0f;
		m_Yaw = 0.0f;
		break;
	case KeyCode::Up:
	case KeyCode::W:
		m_Forward = 1.0f;
		break;
	case KeyCode::Left:
	case KeyCode::A:
		m_Left = 1.0f;
		break;
	case KeyCode::Down:
	case KeyCode::S:
		m_Backward = 1.0f;
		break;
	case KeyCode::Right:
	case KeyCode::D:
		m_Right = 1.0f;
		break;
	case KeyCode::Q:
		m_Down = 1.0f;
		break;
	case KeyCode::E:
		m_Up = 1.0f;
		break;
	case KeyCode::Space:
		m_AnimateLights = !m_AnimateLights;
		break;
	case KeyCode::ShiftKey:
		m_Shift = true;
		break;

	}
}

void TexturedCube::OnKeyReleased(KeyEventArgs& e)
{
	super::OnKeyReleased(e);

	switch (e.Key)
	{
	case KeyCode::Up:
	case KeyCode::W:
		m_Forward = 0.0f;
		break;
	case KeyCode::Left:
	case KeyCode::A:
		m_Left = 0.0f;
		break;
	case KeyCode::Down:
	case KeyCode::S:
		m_Backward = 0.0f;
		break;
	case KeyCode::Right:
	case KeyCode::D:
		m_Right = 0.0f;
		break;
	case KeyCode::Q:
		m_Down = 0.0f;
		break;
	case KeyCode::E:
		m_Up = 0.0f;
		break;
	case KeyCode::ShiftKey:
		m_Shift = false;
		break;
	}
}

void TexturedCube::OnMouseMoved(MouseMotionEventArgs& e)
{
	super::OnMouseMoved(e);

	const float mouseSpeed = 0.1f;

	if (e.LeftButton)
	{
		m_Pitch -= e.RelY * mouseSpeed;

		m_Pitch = clamp(m_Pitch, -90.0f, 90.0f);

		m_Yaw -= e.RelX * mouseSpeed;
	}
}

void TexturedCube::OnMouseWheel(MouseWheelEventArgs& e)
{
	auto fov = m_Camera.get_FoV();

	fov -= e.WheelDelta;
	fov = clamp(fov, 12.0f, 90.0f);

	m_Camera.set_FoV(fov);

	char buffer[256];
	sprintf_s(buffer, "FoV: %f\n", fov);
	OutputDebugStringA(buffer);
}

