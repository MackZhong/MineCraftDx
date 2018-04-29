#include "EnginePCH.h"
#include "TestRendering.h"
#include "VertexTypes.h"

TestRendering::TestRendering()
{
}


TestRendering::~TestRendering()
{
}

void TestRendering::OnRender(ID3D11DeviceContext1 * context)
{
	context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	context->RSSetState(m_states->CullClockwise());

	m_Grass->Draw(context, m_effect.get(), m_view, m_proj);
}

void TestRendering::OnDeviceLost()
{
	m_Grass.reset();
	m_effect.reset();
	m_states.reset();
}

void TestRendering::OnDeviceDependentResources(ID3D11Device * device)
{

	m_states = std::make_unique<CommonStates>(device);
	m_effect = std::make_unique<BasicEffect>(device);

	m_Grass = std::make_unique<MC::Block>();
	m_Grass->Initialize(device, m_effect.get());

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureResourceView;
	DX::ThrowIfFailed(
		CreateWICTextureFromFile(device, L"E:/Games/MineCraft/assets/minecraft/textures/blocks/dirt.png", nullptr,
			&textureResourceView));
	m_Grass->AddTexture(MC::Block::down, textureResourceView.Get());

	DX::ThrowIfFailed(
		CreateWICTextureFromFile(device, L"E:/Games/MineCraft/assets/minecraft/textures/blocks/grass_top.png", nullptr,
			&textureResourceView));
	m_Grass->AddTexture(MC::Block::up, textureResourceView.Get());

	DX::ThrowIfFailed(
		CreateWICTextureFromFile(device, L"E:/Games/MineCraft/assets/minecraft/textures/blocks/grass_side.png", nullptr,
			&textureResourceView));
	m_Grass->AddTexture(MC::Block::north, textureResourceView.Get());
	m_Grass->AddTexture(MC::Block::south, textureResourceView.Get());
	m_Grass->AddTexture(MC::Block::west, textureResourceView.Get());
	m_Grass->AddTexture(MC::Block::east, textureResourceView.Get());

	m_yaw = 0.0f;
	m_pitch = -5.f * XM_2PI / 360.0F;
	m_cameraPos.x = 0.0f;
	m_cameraPos.y = 2.0f;
	m_cameraPos.z = 10.f;
}
