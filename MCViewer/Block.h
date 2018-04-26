#pragma once
#include <d3d11_4.h>

namespace MC {
	using VertexType = DirectX::VertexPositionNormalTexture;
	using VertexCollection = std::vector<VertexType>;
	using IndexCollection = std::vector<uint16_t>;

	class Block
	{
	public:
		Block(Block const&) = delete;
		Block& operator=(Block const&) = delete;
		Block(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float x, float y, float z);
		~Block();
		void XM_CALLCONV Draw(
			ID3D11DeviceContext* deviceContext, 
			FXMMATRIX world,
			CXMMATRIX view,
			CXMMATRIX projection,
			FXMVECTOR color = Colors::White,
			_In_opt_ ID3D11ShaderResourceView* texture = nullptr,
			bool wireframe = false,
			_In_opt_ std::function<void __cdecl()> setCustomState = nullptr) const;

	private:
		void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const VertexCollection& vertices, const IndexCollection& indices);
		void Draw(
			ID3D11DeviceContext* deviceContext, 
			IEffect* effect,
			ID3D11InputLayout* inputLayout,
			bool alpha,
			bool wireframe,
			std::function<void()>& setCustomState) const;

	private:
		ComPtr<ID3D11Buffer> mVertexBuffer;
		ComPtr<ID3D11Buffer> mIndexBuffer;
		UINT mIndexCount;

		// Only one of these helpers is allocated per D3D device context, even if there are multiple GeometricPrimitive instances.
		class SharedResources
		{
		public:
			SharedResources(_In_ ID3D11DeviceContext* deviceContext);

			void PrepareForRendering(bool alpha, bool wireframe) const;

			std::unique_ptr<BasicEffect> effect;

			ComPtr<ID3D11InputLayout> inputLayoutTextured;
			ComPtr<ID3D11InputLayout> inputLayoutUntextured;

			std::unique_ptr<CommonStates> stateObjects;
		};


		// Per-device-context data.
		std::shared_ptr<SharedResources> mResources;

		static DirectX::SharedResourcePool<ID3D11DeviceContext*, SharedResources> sharedResourcesPool;
	};

}