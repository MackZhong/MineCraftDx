#pragma once
#include <d3d11_4.h>
#include <DirectXMath.h>
#include "SimpleMath.h"
#include "GeometricPrimitive.h"

namespace MC {
	class Block
	{
	public:
		Block(Block const&) = delete;
		Block& operator=(Block const&) = delete;
		Block(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float x, float y, float z) {
			m_gpBox = DirectX::GeometricPrimitive::CreateBox(deviceContext, DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
			m_world = DirectX::SimpleMath::Matrix::CreateWorld(DirectX::SimpleMath::Vector3::Vector3(x, y, z),
				DirectX::SimpleMath::Vector3::Backward,
				DirectX::SimpleMath::Vector3::Up);
			DX::ThrowIfFailed(
				CreateWICTextureFromFile(device, L"E:/Games/MineCraft/assets/minecraft/textures/blocks/grass_top.png", nullptr,
					m_texture.ReleaseAndGetAddressOf()));
		};
		void XM_CALLCONV Draw(
			ID3D11DeviceContext* deviceContext, 
			DirectX::CXMMATRIX view,
			DirectX::CXMMATRIX projection) const {
			m_gpBox->Draw(m_world, view, projection, Colors::LightGreen, m_texture.Get());
		};
		void Reset() {
			m_texture.Reset();
		}

	private:
		std::unique_ptr<DirectX::GeometricPrimitive> m_gpBox;
		DirectX::SimpleMath::Matrix m_world;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
		//DirectX::XMMATRIX
	};

}