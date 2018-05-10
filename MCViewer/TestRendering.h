#pragma once
#include "Engine/Game_DR.h"
#include <wrl.h>
#include "Block.h"
#include "Commonstates.h"

class TestRendering : public Game_DR
{
	struct cbParameters {
		XMFLOAT4 Diffuse;
		//XMFLOAT3 Emissive;
		//XMFLOAT3 SpecularColor;

		XMMATRIX WorldViewProj;
	};

public:
	TestRendering();
	~TestRendering();

	// ͨ�� Game_DR �̳�
	virtual LPCWSTR GetClass() const override { return L"TestClass"; };
	virtual LPCWSTR GetTitle() const override {
		return L"TestRendering";
	};

	virtual void OnUpdate(DX::StepTimer const& timer) override;
	virtual void OnRender(ID3D11DeviceContext1 * context) override;
	virtual void OnDeviceLost() override;

private:
	virtual void OnDeviceDependentResources(ID3D11Device* device);

	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	//std::unique_ptr<DirectX::BasicEffect> m_effect;
	std::unique_ptr<DirectX::CommonStates> m_states;
	std::unique_ptr<MC::Block> m_Grass;
	cbParameters m_Parameters;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_CbParametersBuffer{ nullptr };
};

