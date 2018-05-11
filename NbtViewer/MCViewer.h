#pragma once
#include <DirectXMath.h>
#include "DxGame.h"
#include "DxCamera.h"
#include "DxMesh.h"
#include "Blocks.h"
#include "nbt.h"

namespace MineCraft {
	const int MAX_LIGHTS = 8;

	struct _Material
	{
		_Material()
			: Emissive(0.0f, 0.0f, 0.0f, 1.0f)
			, Ambient(0.1f, 0.1f, 0.1f, 1.0f)
			, Diffuse(1.0f, 1.0f, 1.0f, 1.0f)
			, Specular(1.0f, 1.0f, 1.0f, 1.0f)
			, SpecularPower(128.0f)
			, UseTexture(false)
		{}

		DirectX::XMFLOAT4   Emissive;
		//----------------------------------- (16 byte boundary)
		DirectX::XMFLOAT4   Ambient;
		//----------------------------------- (16 byte boundary)
		DirectX::XMFLOAT4   Diffuse;
		//----------------------------------- (16 byte boundary)
		DirectX::XMFLOAT4   Specular;
		//----------------------------------- (16 byte boundary)
		float               SpecularPower;
		// Add some padding complete the 16 byte boundary.
		int                 UseTexture;
		// Add some padding to complete the 16 byte boundary.
		float                 Padding[2];
		//----------------------------------- (16 byte boundary)
	}; // Total:                                80 bytes (5 * 16)

	struct MaterialProperties
	{
		_Material   Material;
	};

	enum LightType
	{
		DirectionalLight = 0,
		PointLight = 1,
		SpotLight = 2
	};

	struct Light
	{
		Light()
			: Position(0.0f, 0.0f, 0.0f, 1.0f)
			, Direction(0.0f, 0.0f, 1.0f, 0.0f)
			, Color(1.0f, 1.0f, 1.0f, 1.0f)
			, SpotAngle(DirectX::XM_PIDIV2)
			, ConstantAttenuation(1.0f)
			, LinearAttenuation(0.0f)
			, QuadraticAttenuation(0.0f)
			, LightType(DirectionalLight)
			, Enabled(0)
		{}

		DirectX::XMFLOAT4    Position;
		//----------------------------------- (16 byte boundary)
		DirectX::XMFLOAT4    Direction;
		//----------------------------------- (16 byte boundary)
		DirectX::XMFLOAT4    Color;
		//----------------------------------- (16 byte boundary)
		float       SpotAngle;
		float       ConstantAttenuation;
		float       LinearAttenuation;
		float       QuadraticAttenuation;
		//----------------------------------- (16 byte boundary)
		int         LightType;
		int         Enabled;
		// Add some padding to make this struct size a multiple of 16 bytes.
		int         Padding[2];
		//----------------------------------- (16 byte boundary)
	};  // Total:                              80 bytes ( 5 * 16 )

	struct LightProperties
	{
		LightProperties()
			: EyePosition(0.0f, 0.0f, 0.0f, 1.0f)
			, GlobalAmbient(0.2f, 0.2f, 0.8f, 1.0f)
		{}

		DirectX::XMFLOAT4   EyePosition;
		//----------------------------------- (16 byte boundary)
		DirectX::XMFLOAT4   GlobalAmbient;
		//----------------------------------- (16 byte boundary)
		Light               Lights[MAX_LIGHTS]; // 80 * 8 bytes
	};  // Total:                                  672 bytes (42 * 16)

	class MCViewer : public DxGame
	{
		using super = DxGame;
		using RegionMap = std::map<std::wstring, CompoundTagPtr>;

		Blocks m_Blocks;
		const wchar_t* m_BasePath;
		RegionMap m_Regions;
		const byte m_Range = 3;

	public:
		MCViewer(DxWindow& window);
		~MCViewer();

		bool LoadChunks(Byte8 ySection, int zChunk, int xChunk);

		// Í¨¹ý DxGame ¼Ì³Ð
		virtual bool LoadContent() override;
		virtual void UnloadContent() override;

	protected:
		MCViewer(const MCViewer&) = delete;

		virtual void OnUpdate(UpdateEventArgs& e);
		virtual void OnRender(RenderEventArgs& e);
		virtual void OnResize(ResizeEventArgs& e);

	private:
		DxCamera m_Camera;

		__declspec(align(16)) struct AlignedData
		{
			DirectX::XMVECTOR m_InitialCameraPos;
			DirectX::XMVECTOR m_InitialCameraRot;
		};
		AlignedData* pData;

		// Per-Frame constant buffer defined in the instanced vertex shader.
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dPerFrameConstantBuffer;

		// Per-Object constant buffer defined in the simple vertex shader.
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dPerObjectConstantBuffer;

		// Material properties defined in the pixel shader
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dMaterialPropertiesConstantBuffer;
		std::vector<MaterialProperties> m_MaterialProperties;

		// Vertex shader for simple (non-instanced) geometry.
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_d3dSimplVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dTexturedLitPixelShader;

		// Light properties defined in the pixel shader
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dLightPropertiesConstantBuffer;
		LightProperties m_LightProperties;

		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_d3dVertexPositionNormalTextureInputLayout;

		std::unique_ptr<DxMesh> m_Cube;
		std::unique_ptr<DxMesh> m_LightSphere;
		std::unique_ptr<DxMesh> m_LightCone;

		// Samplers used in the pixel shader
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_d3dSamplerState;
	};
}