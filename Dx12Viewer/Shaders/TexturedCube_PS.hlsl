struct PixelShaderInput
{
    float4 PositionVS : POSITION;
    float3 NormalVS   : NORMAL;
    float2 TexCoord   : TEXCOORD;
};

struct Material
{
    float4 Emissive;
    //----------------------------------- (16 byte boundary)
    float4 Ambient;
    //----------------------------------- (16 byte boundary)
    float4 Diffuse;
    //----------------------------------- (16 byte boundary)
    float4 Specular;
    //----------------------------------- (16 byte boundary)
    float  SpecularPower;
    float3 Padding;
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 5 = 80 bytes
};

struct PointLight
{
    float4 PositionWS; // Light position in world space.
    //----------------------------------- (16 byte boundary)
    float4 PositionVS; // Light position in view space.
    //----------------------------------- (16 byte boundary)
    float4 Color;
    //----------------------------------- (16 byte boundary)
    float       ConstantAttenuation;
    float       LinearAttenuation;
    float       QuadraticAttenuation;
    float       Padding;
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 4 = 64 bytes
};

struct SpotLight
{
    float4 PositionWS; // Light position in world space.
    //----------------------------------- (16 byte boundary)
    float4 PositionVS; // Light position in view space.
    //----------------------------------- (16 byte boundary)
    float4 DirectionWS; // Light direction in world space.
    //----------------------------------- (16 byte boundary)
    float4 DirectionVS; // Light direction in view space.
    //----------------------------------- (16 byte boundary)
    float4 Color;
    //----------------------------------- (16 byte boundary)
    float       SpotAngle;
    float       ConstantAttenuation;
    float       LinearAttenuation;
    float       QuadraticAttenuation;
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 6 = 96 bytes
};

struct LightProperties
{
    uint NumPointLights;
    uint NumSpotLights;
};

struct LightResult
{
    float4 Diffuse;
    float4 Specular;
};

ConstantBuffer<Material> MaterialCB : register( b0, space1 );
ConstantBuffer<LightProperties> LightPropertiesCB : register( b1 );

StructuredBuffer<PointLight> PointLights : register( t0 );
StructuredBuffer<SpotLight> SpotLights : register( t1 );
//Texture2D DiffuseTexture            : register( t2 );
//SamplerState LinearRepeatSampler    : register(s0);

float DoDiffuse( float3 N, float3 L )
{
    return max( 0, dot( N, L ) );
}

float DoSpecular( float3 V, float3 N, float3 L )
{
    float3 R = normalize( reflect( -L, N ) );
    float RdotV = max( 0, dot( R, V ) );

    return pow( RdotV, MaterialCB.SpecularPower );
}

float DoAttenuation( float c, float l, float q, float d )
{
    return 1.0f / ( c + l * d + q * d * d );
}

float DoSpotCone( float3 spotDir, float3 L, float spotAngle )
{
    float minCos = cos( spotAngle );
    float maxCos = ( minCos + 1.0f ) / 2.0f;
    float cosAngle = dot( spotDir, -L );
    return smoothstep( minCos, maxCos, cosAngle );
}

LightResult DoPointLight( PointLight light, float3 V, float3 P, float3 N )
{
    LightResult result;
    float3 L = ( light.PositionVS.xyz - P );
    float d = length( L );
    L = L / d;

    float attenuation = DoAttenuation( light.ConstantAttenuation,
                                       light.LinearAttenuation,
                                       light.QuadraticAttenuation,
                                       d );

    result.Diffuse = DoDiffuse( N, L ) * attenuation * light.Color;
    result.Specular = DoSpecular( V, N, L ) * attenuation * light.Color;

    return result;
}

LightResult DoSpotLight( SpotLight light, float3 V, float3 P, float3 N )
{
    LightResult result;
    float3 L = ( light.PositionVS.xyz - P );
    float d = length( L );
    L = L / d;

    float attenuation = DoAttenuation( light.ConstantAttenuation,
                                       light.LinearAttenuation,
                                       light.QuadraticAttenuation,
                                       d );

    float spotIntensity = DoSpotCone( light.DirectionVS.xyz, L, light.SpotAngle );

    result.Diffuse = DoDiffuse( N, L ) * attenuation * spotIntensity * light.Color;
    result.Specular = DoSpecular( V, N, L ) * attenuation * spotIntensity * light.Color;

    return result;
}

LightResult DoLighting( float3 P, float3 N )
{
    uint i;

    // Lighting is performed in view space.
    float3 V = normalize( -P );

    LightResult totalResult = (LightResult)0;

    for ( i = 0; i < LightPropertiesCB.NumPointLights; ++i )
    {
        LightResult result = DoPointLight( PointLights[i], V, P, N );

        totalResult.Diffuse += result.Diffuse;
        totalResult.Specular += result.Specular;
    }

    for ( i = 0; i < LightPropertiesCB.NumSpotLights; ++i )
    {
        LightResult result = DoSpotLight( SpotLights[i], V, P, N );

        totalResult.Diffuse += result.Diffuse;
        totalResult.Specular += result.Specular;
    }

    totalResult.Diffuse = saturate( totalResult.Diffuse );
    totalResult.Specular = saturate( totalResult.Specular );

    return totalResult;
}
//
//float4 main( PixelShaderInput IN ) : SV_Target
//{
//    LightResult lit = DoLighting( IN.PositionVS.xyz, normalize( IN.NormalVS ) );
//
//    float4 emissive = MaterialCB.Emissive;
//    float4 ambient = MaterialCB.Ambient;
//    float4 diffuse = MaterialCB.Diffuse * lit.Diffuse;
//    float4 specular = MaterialCB.Specular * lit.Specular;
//    float4 texColor = DiffuseTexture.Sample( LinearRepeatSampler, IN.TexCoord );
//
//    return ( emissive + ambient + diffuse + specular ) * texColor;
//}

struct CubeShaderInput
{
    float4 PositionVS : POSITION;
    float3 NormalVS   : NORMAL;
    float3 TexCoord   : TEXCOORD;
};

Texture2DArray CubeTexture			: register(t2);
SamplerState CubeSampler			: register(s0);

float4 CubePS( CubeShaderInput IN ) : SV_Target
{
    LightResult lit = DoLighting( IN.PositionVS.xyz, normalize( IN.NormalVS ) );

    float4 emissive = MaterialCB.Emissive;
    float4 ambient = MaterialCB.Ambient;
    float4 diffuse = MaterialCB.Diffuse * lit.Diffuse;
    float4 specular = MaterialCB.Specular * lit.Specular;
    float4 texColor = CubeTexture.Sample( CubeSampler, IN.TexCoord );

    return ( emissive + ambient + diffuse + specular ) * texColor;
}