
cbuffer Parameters : register(b0)
{
	float4 DiffuseColor             : packoffset(c0);
	//float3 EmissiveColor            : packoffset(c1);
	//float3 SpecularColor            : packoffset(c2);
	//float  SpecularPower : packoffset(c2.w);

	//float3 LightDirection[3]        : packoffset(c3);
	//float3 LightDiffuseColor[3]     : packoffset(c6);
	//float3 LightSpecularColor[3]    : packoffset(c9);

	//float3 EyePosition              : packoffset(c12);

	//float3 FogColor                 : packoffset(c13);
	//float4 FogVector                : packoffset(c14);

	//float4x4 World                  : packoffset(c15);
	//float3x3 WorldInverseTranspose  : packoffset(c19);
	float4x4 WorldViewProj          : packoffset(c1);// (c22);
};

struct VSInputNmTx
{
	float4 Position : SV_Position;
	float3 Normal   : NORMAL;
	float2 TexCoord : TEXCOORD0;
};

struct VSOutputTx
{
	float4 Diffuse    : COLOR0;
	float4 Specular   : COLOR1;
	float2 TexCoord   : TEXCOORD0;
	float4 PositionPS : SV_Position;
};

struct CommonVSOutput
{
	float4 Pos_ps;
	float4 Diffuse;
	float3 Specular;
	float  FogFactor;
};

float ComputeFogFactor(float4 position)
{
	return 1;// saturate(dot(position, FogVector));
}

CommonVSOutput ComputeCommonVSOutput(float4 position)
{
	CommonVSOutput vout;

	vout.Pos_ps = mul(position, WorldViewProj);
	vout.Diffuse = DiffuseColor;
	vout.Specular = 0;
	vout.FogFactor = ComputeFogFactor(position);

	return vout;
}

VSOutputTx main(VSInputNmTx vin)
{
	VSOutputTx vout;

	CommonVSOutput cout = ComputeCommonVSOutput(vin.Position);
	vout.PositionPS = cout.Pos_ps;
	vout.Diffuse = cout.Diffuse;
	vout.Specular = float4(cout.Specular, cout.FogFactor);
;

	vout.TexCoord = vin.TexCoord;

	return vout;
}