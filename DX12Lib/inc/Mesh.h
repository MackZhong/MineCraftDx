/**
 * A mesh class encapsulates the index and vertex buffers for a geometric primitive.
 */
#pragma once

#include <CommandList.h>
#include <VertexBuffer.h>
#include <IndexBuffer.h>

#include <DirectXMath.h>
#include <d3d12.h>

#include <wrl.h>

#include <memory> // For std::unique_ptr
#include <vector>
//
// // Vertex struct holding position, normal vector, and texture mapping information.
//struct MeshVertexPositionNormalTexture
//{
//	MeshVertexPositionNormalTexture()
//	{ }
//
//	MeshVertexPositionNormalTexture(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT2& textureCoordinate)
//		: m_Position(position),
//		m_Normal(normal),
//		m_TexCoord(textureCoordinate)
//	{ }
//
//	MeshVertexPositionNormalTexture(DirectX::FXMVECTOR position, DirectX::FXMVECTOR normal, DirectX::FXMVECTOR textureCoordinate)
//	{
//		XMStoreFloat3(&this->m_Position, position);
//		XMStoreFloat3(&this->m_Normal, normal);
//		XMStoreFloat2(&this->m_TexCoord, textureCoordinate);
//	}
//
//	DirectX::XMFLOAT3 m_Position;
//	DirectX::XMFLOAT3 m_Normal;
//	DirectX::XMFLOAT2 m_TexCoord;
//
//	static const int InputElementCount = 3;
//	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
//};
//
//using VertexCollection = std::vector<MeshVertexPositionNormalTexture>;
using IndexCollection = std::vector<uint16_t>;

// Vertex struct holding position, normal vector, and texture mapping information.
struct CubeVertexPositionNormalTexture
{
	CubeVertexPositionNormalTexture()
	{ }

	CubeVertexPositionNormalTexture(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT2& textureCoordinate, int textureIndex = 0)
		: m_Position(position)
		, m_Normal(normal)
		, m_TexCoord(textureCoordinate.x, textureCoordinate.y, (float)textureIndex)
	{ }

	CubeVertexPositionNormalTexture(DirectX::FXMVECTOR position, DirectX::FXMVECTOR normal, DirectX::FXMVECTOR textureCoordinate)
	{
		XMStoreFloat3(&this->m_Position, position);
		XMStoreFloat3(&this->m_Normal, normal);
		XMStoreFloat3(&this->m_TexCoord, textureCoordinate);
	}

	DirectX::XMFLOAT3 m_Position;
	DirectX::XMFLOAT3 m_Normal;
	DirectX::XMFLOAT3 m_TexCoord;

	static const int InputElementCount = 3;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

using CubeVertexCollection = std::vector<CubeVertexPositionNormalTexture>;

class Mesh
{
public:
	DirectX::XMMATRIX World;
	void Draw(CommandList& commandList);

	static std::unique_ptr<Mesh> CreateCube(CommandList& commandList, float size = 1, bool rhcoords = false);
	static std::unique_ptr<Mesh> CreateSphere(CommandList& commandList, float diameter = 1, size_t tessellation = 16, bool rhcoords = false);
	static std::unique_ptr<Mesh> CreateCone(CommandList& commandList, float diameter = 1, float height = 1, size_t tessellation = 32, bool rhcoords = false);
	static std::unique_ptr<Mesh> CreateTorus(CommandList& commandList, float diameter = 1, float thickness = 0.333f, size_t tessellation = 32, bool rhcoords = false);
	static std::unique_ptr<Mesh> CreatePlane(CommandList& commandList, float width = 1, float height = 1, bool rhcoords = false);

protected:

private:
	friend struct std::default_delete<Mesh>;

	Mesh();
	Mesh(const Mesh& copy) = delete;
	virtual ~Mesh();

	template<typename T>
	void Initialize(CommandList& commandList, std::vector<T>& vertices, IndexCollection& indices, bool rhcoords);

	VertexBuffer m_VertexBuffer;
	IndexBuffer m_IndexBuffer;

	UINT m_IndexCount;
};