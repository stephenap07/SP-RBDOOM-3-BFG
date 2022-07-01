/*
* Copyright 2017 Stanislav Pidhorskyi. All rights reserved.
* Copyright 2022 Stephen Pridham
*
* License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
*/

#pragma pack_matrix(row_major)

struct SkyConstants
{
	float4		sunDirection;
	float4		skyLuminanceXYZ;
	float4		parameters;			// x - sun size, y - sun bloom, z - exposition
	float4		perezCoeff[5];
	float4x4	invViewProj;
};

// *INDENT-OFF*
#ifdef SPIRV
[[vk::push_constant]] ConstantBuffer<SkyConstants> gSkyParams;
#else
cbuffer gSkyParams : register(b0) { SkyConstants gSkyParams; }
#endif

struct VS_IN {
	float4 position : POSITION;
};

struct VS_OUT {
	float4 position			: SV_Position;
	float2 screenPosition	: TEXCOORD0;
	float3 color			: COLOR0;
	float3 viewDir			: COLOR1;
};
// *INDENT-ON*


#define _float2( x )	float2( x, x )
#define _float3( x )	float3( x, x, x )
#define _float4( x )	float4( x, x, x, x )

float3 Perez(float3 A, float3 B, float3 C, float3 D, float3 E, float costeta, float cosgamma)
{
	float _1_costeta = 1.0 / costeta;
	float cos2gamma = cosgamma * cosgamma;
	float gamma = acos(cosgamma);
	float3 f = (float(1.0) + A * exp(B * _1_costeta))
		* (_float3(1.0) + C * exp(D * gamma) + E * cos2gamma);
	return f;
}

float3 convertXYZ2RGB(float3 xyz)
{
	float3 rgb;
	rgb.x = dot(float3(3.2404542, -1.5371385, -0.4985314), xyz);
	rgb.y = dot(float3(-0.9692660, 1.8760108, 0.0415560), xyz);
	rgb.z = dot(float3(0.0556434, -0.2040259, 1.0572252), xyz);
	return rgb;
}

void main (VS_IN vertex, out VS_OUT result )
{
	result.screenPosition = vertex.position.xy;

	//float4x4 flipMat = float4x4(
	//	0.0f, -1.0f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 1.0f, 0.0f,
	//	-1.0f, 0.0f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 0.0f, 1.0f);

	float4 rayStart = mul( gSkyParams.invViewProj, float4( float3( vertex.position.xy, 0.0 ), 1.0 ) );
	float4 rayEnd = mul( gSkyParams.invViewProj, float4( float3( vertex.position.xy, 1.0 ), 1.0 ) );

	rayStart = rayStart / rayStart.w;
	rayEnd = rayEnd / rayEnd.w;

	result.viewDir = normalize(rayEnd.xyz - rayStart.xyz);
	result.viewDir.y = abs(result.viewDir.y);

	result.position = vertex.position;

	float3 lightDir = normalize(gSkyParams.sunDirection.xyz);
	float3 skyDir = float3(0.0, 1.0, 0.0);

	// Perez coefficients.
	float3 A = gSkyParams.perezCoeff[0].xyz;
	float3 B = gSkyParams.perezCoeff[1].xyz;
	float3 C = gSkyParams.perezCoeff[2].xyz;
	float3 D = gSkyParams.perezCoeff[3].xyz;
	float3 E = gSkyParams.perezCoeff[4].xyz;

	float costeta = max( dot( result.viewDir.xyz, skyDir ), 0.001 );
	float cosgamma = clamp( dot( result.viewDir.xyz, lightDir ), -0.9999, 0.9999 );
	float cosgammas = dot( skyDir, lightDir );

	float3 P = Perez(A, B, C, D, E, costeta, cosgamma);
	float3 P0 = Perez(A, B, C, D, E, 1.0, cosgammas);

	float3 skyColorxyY = float3(
		gSkyParams.skyLuminanceXYZ.x / (gSkyParams.skyLuminanceXYZ.x + gSkyParams.skyLuminanceXYZ.y + gSkyParams.skyLuminanceXYZ.z)
		, gSkyParams.skyLuminanceXYZ.y / (gSkyParams.skyLuminanceXYZ.x + gSkyParams.skyLuminanceXYZ.y + gSkyParams.skyLuminanceXYZ.z)
		, gSkyParams.skyLuminanceXYZ.y
	);

	float3 Yp = skyColorxyY * P / P0;

	float3 skyColorXYZ = float3(Yp.x * Yp.z / Yp.y, Yp.z, (1.0 - Yp.x - Yp.y) * Yp.z / Yp.y);

	result.color = convertXYZ2RGB(skyColorXYZ * gSkyParams.parameters.z);
}