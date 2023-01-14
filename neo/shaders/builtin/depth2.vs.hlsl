/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

/*
* Copyright (c) 2014-2021, NVIDIA CORPORATION. All rights reserved.
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

// GPU half-float bit patterns
#define HF_MANTISSA(x)	(x&1023)
#define HF_EXP(x)		((x&32767)>>10)
#define HF_SIGN(x)		((x&32768)?-1:1)

float F16toF32( uint x )
{
	int e = HF_EXP( x );
	int m = HF_MANTISSA( x );
	int s = HF_SIGN( x );

	if( 0 < e && e < 31 )
	{
		return s * pow( 2.0f, ( e - 15.0f ) ) * ( 1 + m / 1024.0f );
	}
	else if( m == 0 )
	{
		return s * 0.0f;
	}
	return s * pow( 2.0f, -14.0f ) * ( m / 1024.0f );
}

struct InstanceConstants
{
	uint instance;
	uint geometryInMesh;
};

struct DepthView
{
	float4x4 matWorldToClip;
};

// *INDENT-OFF*
ConstantBuffer<DepthView> g_View : register( b0 );
VK_PUSH_CONSTANT ConstantBuffer<InstanceConstants> g_Instance : register( b1 );
StructuredBuffer<InstanceData> t_InstanceData : register( t0 );
StructuredBuffer<GeometryData> t_GeometryData : register( t1 );
ByteAddressBuffer t_MaterialConstants : register( t2 );
VK_BINDING( 0, 1 ) ByteAddressBuffer t_BindlessBuffers[] : register( t0, space1 );
VK_BINDING( 1, 1 ) Texture2D t_BindlessTextures[] : register( t0, space2 );
SamplerState s_MaterialSampler : register( s0 );
// *INDENT-ON*

void main(
	in uint i_vertexID : SV_VertexID,
	out float4 o_position : SV_Position,
	out float2 o_uv : TEXCOORD,
	out uint o_material : MATERIAL )
{
	InstanceData instance = t_InstanceData[g_Instance.instance];
	GeometryData geometry = t_GeometryData[instance.firstGeometryIndex + g_Instance.geometryInMesh];

	ByteAddressBuffer indexBuffer = t_BindlessBuffers[geometry.indexBufferIndex];
	ByteAddressBuffer vertexBuffer = t_BindlessBuffers[geometry.vertexBufferIndex];

	uint index = LoadIndex( indexBuffer, geometry.indexOffset, i_vertexID );

	float2 texcoord = 0;
	if( geometry.texCoord1Offset != ~0u )
	{
		uint texcoordIn = vertexBuffer.Load( geometry.texCoord1Offset + ( index * c_SizeOfVertex ) );
		texcoord.x = F16toF32( ( texcoordIn >> 0 ) & 0xFFFF );
		texcoord.y = F16toF32( ( texcoordIn >> 16 ) & 0xFFFF );
	}

	float3 pos = asfloat( vertexBuffer.Load3( geometry.positionOffset + ( index * c_SizeOfVertex ) ) );

	o_position = mul( instance.transform, float4( pos, 1.0 ) );
	o_uv = texcoord;
	o_material = geometry.materialIndex;
}
