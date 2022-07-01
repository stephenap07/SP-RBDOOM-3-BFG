/*
* Copyright 2017 Stanislav Pidhorskyi. All rights reserved.
* Copyright 2022 Stephen Pridham
*
* License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
*/

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

struct PS_IN
{
	float4 position			: SV_Position;
	float2 screenPosition	: TEXCOORD0;
	float3 color			: COLOR0;
	float3 viewDir			: COLOR1;
};

// *INDENT-ON*

float toGamma(float _r)
{
	return pow(abs(_r), 1.0 / 2.2);
}

float3 toGamma(float3 _rgb)
{
	return pow(abs(_rgb), float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}

float4 toGamma(float4 _rgba)
{
	return float4(toGamma(_rgba.xyz), _rgba.w);
}

// https://www.shadertoy.com/view/4ssXRX
// http://www.loopit.dk/banding_in_games.pdf
// http://www.dspguide.com/ch2/6.htm

// uniformly distributed, normalized rand, [0, 1)
float nrand(in float2 n)
{
	return frac(sin(dot(n.xy, float2(12.9898, 78.233))) * 43758.5453);
}

float n4rand_ss(in float2 n)
{
	float nrnd0 = nrand(n + 0.07 * frac(gSkyParams.parameters.w));
	float nrnd1 = nrand(n + 0.11 * frac(gSkyParams.parameters.w + 0.573953));
	return 0.23 * sqrt(-log(nrnd0 + 0.00001)) * cos(2.0 * 3.141592 * nrnd1) + 0.5;
}

void main( in PS_IN fragment, out float4 outColor : SV_TARGET)
{
	float size2 = gSkyParams.parameters.x * gSkyParams.parameters.x;

	float3 lightDir = normalize(gSkyParams.sunDirection.xyz);
	float distance = 2.0 * (1.0 - dot(normalize(fragment.viewDir.xyz), lightDir));
	float sun = exp(-distance / gSkyParams.parameters.y / size2) + step(distance, size2);
	float sun2 = min(sun * sun, 1.0);
	float3 color = fragment.color.rgb + sun2;
	//color = toGamma(color);
	//float r = n4rand_ss(fragment.screenPosition.xy);
	//color += float3(r, r, r) / 40.0;

	outColor = float4(color, 1.0);
}