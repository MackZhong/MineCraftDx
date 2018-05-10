#pragma once

using namespace DirectX;
using namespace Microsoft::WRL;

namespace DirectX {

	HRESULT CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob);

	//// Shader resources
	//enum ConstanBuffer
	//{
	//	CB_Appliation,
	//	CB_Frame,
	//	CB_Object,
	//	NumConstantBuffers
	//};

	//ID3D11Buffer* g_d3dConstantBuffers[NumConstantBuffers];

}
