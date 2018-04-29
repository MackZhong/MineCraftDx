#pragma once
#include "Game_DR.h"
#include "mc.h"
#include "nbttag.h"
#include "Block.h"
#include <wrl.h>

namespace MC
{
	class McGame :
		public Game_DR
	{
		std::wstring m_VersionPath;
		std::wstring m_WorldName;

	public:
		McGame();
		~McGame();

	private:
		virtual LPCWSTR GetClass() const override { return L"McWindowClass"; }

		virtual LPCWSTR GetTitle() const override { return L"MineCraftDirectX11Reload"; }

		virtual void OnRender(ID3D11DeviceContext1 * context) override;
		//std::unique_ptr<Model> m_Entities;

		virtual void OnDeviceDependentResources(ID3D11Device * device);

		virtual void OnWindowSizeDependentResources(int width, int height) override;

		virtual void OnDeviceLost();

		virtual void OnUpdate(DX::StepTimer const& timer) override;


	private:
		std::unique_ptr<CompoundTag> m_Root;
		std::vector<std::shared_ptr<MC::Block>> m_Blocks;
		std::unique_ptr<DirectX::BasicEffect> m_effect;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	};
}

