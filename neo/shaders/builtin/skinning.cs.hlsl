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
#include <vulkan.hlsli>
#include "packing.hlsli"
#include "skinning.cb.hlsli"

ByteAddressBuffer t_VertexBuffer :
register( t0 );
StructuredBuffer<float4> t_JointMatrices :
register( t1 );

RWByteAddressBuffer u_VertexBuffer :
register( u0 );

#ifdef SPIRV

[[vk::push_constant]] ConstantBuffer<SkinningConstants> g_Const;

#else

cbuffer g_Const :
register( b0 )
{
	SkinningConstants g_Const;
}

#endif


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
		normal = Unpack_RGBA8_SNORM( t_VertexBuffer.Load( offset + g_Const.inputNormalOffset ) );
	}

	if( g_Const.flags & SkinningFlag_Tangents )
	{
		tangent = Unpack_RGBA8_SNORM( t_VertexBuffer.Load( offset + g_Const.inputTangentOffset ) );
	}

	if( g_Const.flags & SkinningFlag_TexCoord1 )
	{
		texCoord1 = t_VertexBuffer.Load( offset + g_Const.inputTexCoord1Offset );
	}

	if( g_Const.flags & SkinningFlag_TexCoord2 )
	{
		texCoord2 = t_VertexBuffer.Load( offset + g_Const.inputTexCoord2Offset );
	}

	float divisor = 1.0 / 255.0;

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
		matX += t_JointMatrices[index] * jointWeights[i];
		matY += t_JointMatrices[index + 1] * jointWeights[i];
		matZ += t_JointMatrices[index + 2] * jointWeights[i];
	}

	float4 modelPosition;
	modelPosition.x = dot( matX, float4( position, 1 ) );
	modelPosition.y = dot( matY, float4( position, 1 ) );
	modelPosition.z = dot( matZ, float4( position, 1 ) );

	float4 modelNormal;
	modelNormal.x = dot( matX, normal );
	modelNormal.y = dot( matY, normal );
	modelNormal.z = dot( matZ, normal );
	modelNormal = normalize( modelNormal );

	float4 modelTangent;
	modelTangent.x = dot( matX, tangent );
	modelTangent.y = dot( matY, tangent );
	modelTangent.z = dot( matZ, tangent );
	modelTangent = normalize( modelTangent );

	position = modelPosition.xyz;
	normal.xyz = modelNormal.xyz;
	tangent.xyz = modelTangent.xyz;

	// float3 prevPosition;
	// if (g_Const.flags & SkinningFlag_FirstFrame)
	// 	prevPosition = position;
	// else
	// 	prevPosition = asfloat(u_VertexBuffer.Load3(offset + g_Const.outputPositionOffset));

	//u_VertexBuffer.Store3(i_globalIdx * c_SizeOfPosition + g_Const.outputPrevPositionOffset, asuint(prevPosition));

	u_VertexBuffer.Store3( offset + g_Const.outputPositionOffset, asuint( position ) );

	if( g_Const.flags & SkinningFlag_Normals )
	{
		u_VertexBuffer.Store( offset + g_Const.outputNormalOffset, Pack_RGBA8_SNORM( normal ) );
	}

	if( g_Const.flags & SkinningFlag_Tangents )
	{
		u_VertexBuffer.Store( offset + g_Const.outputTangentOffset, Pack_RGBA8_SNORM( tangent ) );
	}

	if( g_Const.flags & SkinningFlag_TexCoord1 )
	{
		u_VertexBuffer.Store2( offset + g_Const.outputTexCoord1Offset, texCoord1 );
	}

	if( g_Const.flags & SkinningFlag_TexCoord2 )
	{
		u_VertexBuffer.Store2( offset + g_Const.outputTexCoord2Offset, texCoord2 );
	}
}