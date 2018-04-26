#include "EnginePCH.h"
#include "Block.h"

using namespace DirectX;

namespace MC {
	SharedResourcePool<ID3D11DeviceContext*, Block::SharedResources> Block::sharedResourcesPool;

	const float SQRT2 = 1.41421356237309504880f;
	const float SQRT3 = 1.73205080756887729352f;
	const float SQRT6 = 2.44948974278317809820f;
	// Helper for creating a D3D vertex or index buffer.
	template<typename T>
	void CreateBuffer(_In_ ID3D11Device* device, T const& data, D3D11_BIND_FLAG bindFlags, _Outptr_ ID3D11Buffer** pBuffer)
	{
		assert(pBuffer != 0);

		D3D11_BUFFER_DESC bufferDesc = {};

		bufferDesc.ByteWidth = (UINT)data.size() * sizeof(typename T::value_type);
		bufferDesc.BindFlags = bindFlags;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA dataDesc = {};

		dataDesc.pSysMem = data.data();

		ThrowIfFailed(
			device->CreateBuffer(&bufferDesc, &dataDesc, pBuffer)
		);

		_Analysis_assume_(*pBuffer != 0);

		SetDebugObjectName(*pBuffer, "DirectXTK:GeometricPrimitive");
	}


	// Helper for creating a D3D input layout.
	void CreateInputLayout(_In_ ID3D11Device* device, IEffect* effect, _Outptr_ ID3D11InputLayout** pInputLayout)
	{
		assert(pInputLayout != 0);

		void const* shaderByteCode;
		size_t byteCodeLength;

		effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

		DX::ThrowIfFailed(
			device->CreateInputLayout(
				MC::VertexType::InputElements,
				MC::VertexType::InputElementCount,
				shaderByteCode, byteCodeLength,
				pInputLayout)
		);

		_Analysis_assume_(*pInputLayout != 0);

		SetDebugObjectName(*pInputLayout, "DirectXTK:GeometricPrimitive");
	}

	inline void CheckIndexOverflow(size_t value)
	{
		// Use >=, not > comparison, because some D3D level 9_x hardware does not support 0xFFFF index values.
		if (value >= USHRT_MAX)
			throw std::exception("Index value out of range: cannot tesselate primitive so finely");
	}

	// Collection types used when generating the geometry.
	inline void index_push_back(IndexCollection& indices, size_t value)
	{
		CheckIndexOverflow(value);
		indices.push_back((uint16_t)value);
	}

	// Helper for flipping winding of geometric primitives for LH vs. RH coords
	inline void ReverseWinding(IndexCollection& indices, VertexCollection& vertices)
	{
		assert((indices.size() % 3) == 0);
		for (auto it = indices.begin(); it != indices.end(); it += 3)
		{
			std::swap(*it, *(it + 2));
		}

		for (auto it = vertices.begin(); it != vertices.end(); ++it)
		{
			it->textureCoordinate.x = (1.f - it->textureCoordinate.x);
		}
	}


	// Helper for inverting normals of geometric primitives for 'inside' vs. 'outside' viewing
	inline void InvertNormals(VertexCollection& vertices)
	{
		for (auto it = vertices.begin(); it != vertices.end(); ++it)
		{
			it->normal.x = -it->normal.x;
			it->normal.y = -it->normal.y;
			it->normal.z = -it->normal.z;
		}
	}

	void ComputeBox(VertexCollection& vertices, IndexCollection& indices,
		const XMFLOAT3& position = { 0.0f,0.0f,0.0f },
		const XMFLOAT3& size = { 1.0f, 1.0f,1.0f },
		bool rhcoords = true, bool invertn = false)
	{
		vertices.clear();
		indices.clear();

		// A box has six faces, each one pointing in a different direction.
		const int FaceCount = 6;

		static const XMVECTORF32 faceNormals[FaceCount] =
		{
			{ { { 0,  0,  1, 0 } } },
		{ { { 0,  0, -1, 0 } } },
		{ { { 1,  0,  0, 0 } } },
		{ { { -1,  0,  0, 0 } } },
		{ { { 0,  1,  0, 0 } } },
		{ { { 0, -1,  0, 0 } } },
		};

		static const XMVECTORF32 textureCoordinates[4] =
		{
			{ { { 1, 0, 0, 0 } } },
		{ { { 1, 1, 0, 0 } } },
		{ { { 0, 1, 0, 0 } } },
		{ { { 0, 0, 0, 0 } } },
		};

		XMVECTOR tsize = XMLoadFloat3(&size);
		tsize = XMVectorDivide(tsize, g_XMTwo);

		// Create each face in turn.
		for (int i = 0; i < FaceCount; i++)
		{
			XMVECTOR normal = faceNormals[i];

			// Get two vectors perpendicular both to the face normal and to each other.
			XMVECTOR basis = (i >= 4) ? g_XMIdentityR2 : g_XMIdentityR1;

			XMVECTOR side1 = XMVector3Cross(normal, basis);
			XMVECTOR side2 = XMVector3Cross(normal, side1);

			// Six indices (two triangles) per face.
			size_t vbase = vertices.size();
			index_push_back(indices, vbase + 0);
			index_push_back(indices, vbase + 1);
			index_push_back(indices, vbase + 2);

			index_push_back(indices, vbase + 0);
			index_push_back(indices, vbase + 2);
			index_push_back(indices, vbase + 3);

			// Four vertices per face.
			vertices.push_back(VertexPositionNormalTexture((normal - side1 - side2) * tsize, normal, textureCoordinates[0]));
			vertices.push_back(VertexPositionNormalTexture((normal - side1 + side2) * tsize, normal, textureCoordinates[1]));
			vertices.push_back(VertexPositionNormalTexture((normal + side1 + side2) * tsize, normal, textureCoordinates[2]));
			vertices.push_back(VertexPositionNormalTexture((normal + side1 - side2) * tsize, normal, textureCoordinates[3]));
		}

		// Build RH above
		if (!rhcoords)
			ReverseWinding(indices, vertices);

		if (invertn)
			InvertNormals(vertices);
	}

	Block::Block(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float x, float y, float z)
	{
		VertexCollection vertices;
		IndexCollection indices;
		ComputeBox(vertices, indices, XMFLOAT3(x, y, z));

		// Create the primitive object.
		//std::unique_ptr<GeometricPrimitive> primitive(new GeometricPrimitive());

		Initialize(device, deviceContext, vertices, indices);
	}


	Block::~Block()
	{
	}

	void XM_CALLCONV Block::Draw(ID3D11DeviceContext* deviceContext,
		FXMMATRIX world,
		CXMMATRIX view,
		CXMMATRIX projection,
		FXMVECTOR color,
		ID3D11ShaderResourceView * texture,
		bool wireframe,
		std::function<void()> setCustomState) const
	{
		assert(mResources != 0);
		auto effect = mResources->effect.get();
		assert(effect != 0);

		ID3D11InputLayout *inputLayout;
		if (texture)
		{
			effect->SetTextureEnabled(true);
			effect->SetTexture(texture);

			inputLayout = mResources->inputLayoutTextured.Get();
		}
		else
		{
			effect->SetTextureEnabled(false);

			inputLayout = mResources->inputLayoutUntextured.Get();
		}

		// Set effect parameters.
		effect->SetMatrices(world, view, projection);

		effect->SetColorAndAlpha(color);

		float alpha = XMVectorGetW(color);
		Draw(deviceContext, effect, inputLayout, (alpha < 1.f), wireframe, setCustomState);
	}

	void Block::Initialize(ID3D11Device * device, ID3D11DeviceContext * deviceContext, const VertexCollection & vertices, const IndexCollection & indices)
	{
		if (vertices.size() >= USHRT_MAX)
			throw std::exception("Too many vertices for 16-bit index buffer");

		mResources = sharedResourcesPool.DemandCreate(deviceContext);

		CreateBuffer(device, vertices, D3D11_BIND_VERTEX_BUFFER, &mVertexBuffer);
		CreateBuffer(device, indices, D3D11_BIND_INDEX_BUFFER, &mIndexBuffer);

		mIndexCount = static_cast<UINT>(indices.size());
	}

	void Block::Draw(ID3D11DeviceContext* deviceContext, IEffect * effect, ID3D11InputLayout * inputLayout, bool alpha, bool wireframe, std::function<void()> setCustomState) const
	{
		assert(mResources != 0);
		assert(deviceContext != 0);

		// Set state objects.
		mResources->PrepareForRendering(alpha, wireframe);

		// Set input layout.
		assert(inputLayout != 0);
		deviceContext->IASetInputLayout(inputLayout);

		// Activate our shaders, constant buffers, texture, etc.
		assert(effect != 0);
		effect->Apply(deviceContext);

		// Set the vertex and index buffer.
		auto vertexBuffer = mVertexBuffer.Get();
		UINT vertexStride = sizeof(VertexType);
		UINT vertexOffset = 0;

		deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &vertexOffset);

		deviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		// Hook lets the caller replace our shaders or state settings with whatever else they see fit.
		if (setCustomState)
		{
			setCustomState();
		}

		// Draw the primitive.
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		deviceContext->DrawIndexed(mIndexCount, 0, 0);
	}

	Block::SharedResources::SharedResources(_In_ ID3D11DeviceContext* deviceContext)
	{
		ComPtr<ID3D11Device> device;
		deviceContext->GetDevice(&device);

		// Create the BasicEffect.
		effect = std::make_unique<BasicEffect>(device.Get());

		effect->EnableDefaultLighting();

		// Create state objects.
		stateObjects = std::make_unique<CommonStates>(device.Get());

		// Create input layouts.
		effect->SetTextureEnabled(true);
		CreateInputLayout(device.Get(), effect.get(), &inputLayoutTextured);

		effect->SetTextureEnabled(false);
		CreateInputLayout(device.Get(), effect.get(), &inputLayoutUntextured);
	}
}