/*
* Copyright (c) 2021, NVIDIA CORPORATION. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

#pragma pack_matrix(row_major)

#include "bindless.hlsli"
#include "packing.hlsli"
#include "skinning.cb.hlsli"

#include <vulkan.hlsli>

// *INDENT-OFF*
ByteAddressBuffer t_VertexBuffer : register( t0 );
ByteAddressBuffer t_JointMatrices : register( t1 );
RWByteAddressBuffer u_VertexBuffer : register( u0 );

#ifdef SPIRV
[[vk::push_constant]] ConstantBuffer<SkinningConstants> g_Const;
#else
cbuffer g_Const : register( b0 )
{
	SkinningConstants g_Const;
}
#endif
// *INDENT-ON*

// GPU half-float bit patterns
#define HF_MANTISSA(x)	(x&1023)
#define HF_EXP(x)		((x&32767)>>10)
#define HF_SIGN(x)		((x&32768)?-1:1)

[numthreads( 256, 1, 1 )]
void main( in uint i_globalIdx : SV_DispatchThreadID )
{
	if( i_globalIdx >= g_Const.numVertices )
	{
		return;
	}

	// offset into the idDrawVert
	int offset = i_globalIdx * c_SizeOfVertex;

	float3 position = asfloat( t_VertexBuffer.Load3( offset + g_Const.inputPositionOffset ) );
	float4 normal = 0;
	float4 tangent = 0;
	uint texCoord1 = 0;
	uint texCoord2 = 0;

	if( g_Const.flags & SkinningFlag_Normals )
	{
		normal = UIntToFloat4( t_VertexBuffer.Load( offset + g_Const.inputNormalOffset ) );
	}

	if( g_Const.flags & SkinningFlag_Tangents )
	{
		tangent = UIntToFloat4( t_VertexBuffer.Load( offset + g_Const.inputTangentOffset ) );
	}

	if( g_Const.flags & SkinningFlag_TexCoord1 )
	{
		// two half floats
		texCoord1 = t_VertexBuffer.Load( offset + g_Const.inputTexCoord1Offset );
	}

	if( g_Const.flags & SkinningFlag_TexCoord2 )
	{
		// two half floats
		texCoord2 = t_VertexBuffer.Load( offset + g_Const.inputTexCoord2Offset );
	}

	const float divisor = 1.0 / 255.0;

	uint jp = t_VertexBuffer.Load( offset + g_Const.inputJointIndexOffset );
	float4 jointIndices = float4( ( ( jp >> 0 ) & 0xff ),
								  ( ( jp >> 8 ) & 0xff ),
								  ( ( jp >> 16 ) & 0xff ),
								  ( ( jp >> 24 ) & 0xff ) ) * divisor;

	uint jwp = t_VertexBuffer.Load( offset + g_Const.inputJointWeightOffset );
	float4 jointWeights = float4( ( ( jwp >> 0 ) & 0xff ),
								  ( ( jwp >> 8 ) & 0xff ),
								  ( ( jwp >> 16 ) & 0xff ),
								  ( ( jwp >> 24 ) & 0xff ) ) * divisor;

	float4 matX = 0, matY = 0, matZ = 0;
	[unroll]
	for( int i = 0; i < 4; i++ )
	{
		int index = int( jointIndices[i] * 255.1 * 3.0 );
		matX += asfloat( t_JointMatrices.Load4(g_Const.inputJointMatOffset + ( index + 0 ) * 16 ) ) * jointWeights[i];
		matY += asfloat( t_JointMatrices.Load4(g_Const.inputJointMatOffset + ( index + 1 ) * 16 ) ) * jointWeights[i];
		matZ += asfloat( t_JointMatrices.Load4(g_Const.inputJointMatOffset + ( index + 2 ) * 16 ) ) * jointWeights[i];
	}

	float3 modelPosition;
	modelPosition.x = dot( matX, float4( position, 1 ) );
	modelPosition.y = dot( matY, float4( position, 1 ) );
	modelPosition.z = dot( matZ, float4( position, 1 ) );

	float3 modelNormal;
	modelNormal.x = dot( matX.xyz, normal.xyz );
	modelNormal.y = dot( matY.xyz, normal.xyz );
	modelNormal.z = dot( matZ.xyz, normal.xyz );
	modelNormal = normalize( modelNormal );

	float3 modelTangent;
	modelTangent.x = dot( matX.xyz, tangent.xyz );
	modelTangent.y = dot( matY.xyz, tangent.xyz );
	modelTangent.z = dot( matZ.xyz, tangent.xyz );
	modelTangent = normalize( modelTangent );

	position = modelPosition;
	normal.xyz = modelNormal;
	tangent.xyz = modelTangent;

	// float3 prevPosition;
	// if (g_Const.flags & SkinningFlag_FirstFrame)
	// 	prevPosition = position;
	// else
	// 	prevPosition = asfloat(u_VertexBuffer.Load3(offset + g_Const.outputPositionOffset));

	//u_VertexBuffer.Store3(i_globalIdx * c_SizeOfPosition + g_Const.outputPrevPositionOffset, asuint(prevPosition));

	u_VertexBuffer.Store3( offset + g_Const.outputPositionOffset, asuint( position ) );

	if( g_Const.flags & SkinningFlag_Normals )
	{
		u_VertexBuffer.Store( offset + g_Const.outputNormalOffset, Float4ToUInt( normal ) );
	}

	if( g_Const.flags & SkinningFlag_Tangents )
	{
		u_VertexBuffer.Store( offset + g_Const.outputTangentOffset, Float4ToUInt( tangent ) );
	}

	if( g_Const.flags & SkinningFlag_TexCoord1 )
	{
		u_VertexBuffer.Store( offset + g_Const.outputTexCoord1Offset, texCoord1 );
	}

	if( g_Const.flags & SkinningFlag_TexCoord2 )
	{
		u_VertexBuffer.Store( offset + g_Const.outputTexCoord2Offset, texCoord2 );
	}
}