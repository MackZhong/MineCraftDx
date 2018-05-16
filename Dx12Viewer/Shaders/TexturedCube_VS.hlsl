struct Mat
{
	matrix ModelMatrix;
	matrix ModelViewMatrix;
	matrix InverseTransposeModelViewMatrix;
	matrix ModelViewProjectionMatrix;
};

ConstantBuffer<Mat> MatCB : register(b0);

struct VertexPositionNormalTexture
{
	float3 Position : POSITION;
	float3 Normal   : NORMAL;
	float2 TexCoord : TEXCOORD;
};

struct VertexShaderOutput
{
	float4 PositionVS : POSITION;
	float3 NormalVS   : NORMAL;
	float2 TexCoord   : TEXCOORD;
	float4 Position   : SV_Position;
};

VertexShaderOutput main(VertexPositionNormalTexture IN)
{
	VertexShaderOutput OUT;

	OUT.Position = mul(MatCB.ModelViewProjectionMatrix, float4(IN.Position, 1.0f));
	OUT.PositionVS = mul(MatCB.ModelViewMatrix, float4(IN.Position, 1.0f));
	OUT.NormalVS = mul((float3x3)MatCB.InverseTransposeModelViewMatrix, IN.Normal);
	OUT.TexCoord = IN.TexCoord;

	return OUT;
}

struct CubeVertexPositionNormalTexture
{
	float3 Position : POSITION;
	float3 Normal   : NORMAL;
	float3 TexCoord : TEXCOORD;
};

struct CubeVertexShaderOutput
{
	float4 PositionVS : POSITION;
	float3 NormalVS   : NORMAL;
	float3 TexCoord   : TEXCOORD;
	float4 Position   : SV_Position;
};

CubeVertexShaderOutput CubeVS(CubeVertexPositionNormalTexture IN)
{
	CubeVertexShaderOutput OUT;

	OUT.Position = mul(MatCB.ModelViewProjectionMatrix, float4(IN.Position, 1.0f));
	OUT.PositionVS = mul(MatCB.ModelViewMatrix, float4(IN.Position, 1.0f));
	OUT.NormalVS = mul((float3x3)MatCB.InverseTransposeModelViewMatrix, IN.Normal);
	//float texIndex = 0;
	//if (IN.Position.y == 0.5) {
	//	texIndex = 0;
	//}
	//else if (IN.Position.z == -0.5) {
	//	texIndex = 1;
	//}
	//else if (IN.Position.y == -0.5) {
	//	texIndex = 2;
	//}
	//else if (IN.Position.z == 0.5) {
	//	texIndex = 3;
	//}
	//else if (IN.Position.x == -0.5) {
	//	texIndex = 4;
	//}
	//else {
	//	texIndex = 5;
	//}
	//OUT.TexCoord.x = IN.TexCoord.x;
	//OUT.TexCoord.y = IN.TexCoord.y;
	//OUT.TexCoord.z = texIndex;
	OUT.TexCoord = IN.TexCoord;

	return OUT;
}