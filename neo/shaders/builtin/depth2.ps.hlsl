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
#include <scene_material.hlsli>
#include <vulkan.hlsli>

// ----------------------
// YCoCg Color Conversion
// ----------------------
// Co
#define matrixRGB1toCoCg1YX float4( 0.50,  0.0, -0.50, 0.50196078 )
// Cg
#define matrixRGB1toCoCg1YY float4( -0.25,  0.5, -0.25, 0.50196078 )
// 1.0
#define matrixRGB1toCoCg1YZ float4( 0.0,   0.0,  0.0,  1.0 )
// Y
#define matrixRGB1toCoCg1YW float4( 0.25,  0.5,  0.25, 0.0 )

#define matrixCoCg1YtoRGB1X float4( 1.0, -1.0,  0.0,        1.0 )
// -0.5 * 256.0 / 255.0
#define matrixCoCg1YtoRGB1Y float4( 0.0,  1.0, -0.50196078, 1.0 )
// +1.0 * 256.0 / 255.0
#define matrixCoCg1YtoRGB1Z float4( -1.0, -1.0,  1.00392156, 1.0 )

static float3 ConvertYCoCgToRGB( float4 YCoCg )
{
	float4 rgbColor;

	YCoCg.z = ( YCoCg.z * 31.875 ) + 1.0;			//z = z * 255.0/8.0 + 1.0
	YCoCg.z = 1.0 / YCoCg.z;
	YCoCg.xy *= YCoCg.z;
	rgbColor.x = dot( YCoCg, matrixCoCg1YtoRGB1X );
	rgbColor.y = dot( YCoCg, matrixCoCg1YtoRGB1Y );
	rgbColor.z = dot( YCoCg, matrixCoCg1YtoRGB1Z );
	return rgbColor.xyz;
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
	in float4 i_position : SV_Position,
	in float2 i_uv : TEXCOORD,
	nointerpolation in uint i_material : MATERIAL,
	out float4 o_color : SV_Target0 )
{
	MaterialConstants material = LoadMaterialConstants( t_MaterialConstants, i_material );
	MaterialTextureSample textures = DefaultMaterialTextures();

	for( int i = 0; i < material.numAmbientStages; i++ )
	{
		materialAmbientData_t ambientStage = LoadMaterialAmbientStage( t_MaterialConstants, i_material + SizeOfMaterialConstants + ( i * SizeOfAmbientStage ) );
		Texture2D texture = t_BindlessTextures[ ambientStage.textureId ];
		float4 color = texture.Sample( s_MaterialSampler, i_uv ) * ambientStage.color;
		float test = color.w - ambientStage.alphaTest;
		clip( test );
		if( test < 0 )
		{
			break;
		}
	}
}