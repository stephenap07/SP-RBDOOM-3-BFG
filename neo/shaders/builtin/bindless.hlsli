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

#ifndef BINDLESS_H_
#define BINDLESS_H_

#include "material_cb.h"

struct GeometryData
{
    uint numIndices;
    uint numVertices;
    int indexBufferIndex;
    uint indexOffset;

    int vertexBufferIndex;
    uint positionOffset;
    uint prevPositionOffset;
    uint texCoord1Offset;

    uint texCoord2Offset;
    uint normalOffset;
    uint tangentOffset;
    uint materialIndex;

    uint padding[ 4 ];
};


struct InstanceData
{
    int2 padding;
    uint firstGeometryIndex;
    uint numGeometries;

    float4x4 transform;
    float4x4 prevTransform;
};

#ifndef __cplusplus

static const uint c_SizeOfVertex = 32;
static const uint c_SizeOfTriangleIndices = 2;
static const uint c_SizeOfPosition = 12;
static const uint c_SizeOfTexcoord = 4;
static const uint c_SizeOfNormal = 4;
static const uint c_SizeOfJointIndices = 4;
static const uint c_SizeOfJointWeights = 4;

GeometryData LoadGeometryData(ByteAddressBuffer buffer, uint offset)
{
    uint4 a = buffer.Load4(offset + 16 * 0);
    uint4 b = buffer.Load4(offset + 16 * 1);
    uint4 c = buffer.Load4(offset + 16 * 2);

    GeometryData ret;
    ret.numIndices = a.x;
    ret.numVertices = a.y;
    ret.indexBufferIndex = int(a.z);
    ret.indexOffset = a.w;
    ret.vertexBufferIndex = int(b.x);
    ret.positionOffset = b.y;
    ret.prevPositionOffset = b.z;
    ret.texCoord1Offset = b.w;
    ret.texCoord2Offset = c.x;
    ret.normalOffset = c.y;
    ret.tangentOffset = c.z;
    ret.materialIndex = c.w;
    return ret;
}

InstanceData LoadInstanceData(ByteAddressBuffer buffer, uint offset)
{
    uint4 a = buffer.Load4(offset + 16 * 0);

    uint4 b = buffer.Load4(offset + 16 * 1);
    uint4 c = buffer.Load4(offset + 16 * 2);
    uint4 d = buffer.Load4(offset + 16 * 3);
    uint4 e = buffer.Load4(offset + 16 * 4);

    uint4 f = buffer.Load4(offset + 16 * 5);
    uint4 g = buffer.Load4(offset + 16 * 6);
    uint4 h = buffer.Load4(offset + 16 * 7);
    uint4 i = buffer.Load4(offset + 16 * 8);

    InstanceData ret;
    ret.padding = a.xy;
    ret.firstGeometryIndex = a.z;
    ret.numGeometries = a.w;
    ret.transform = float4x4(asfloat(b), asfloat(c), asfloat(d), asfloat(e));
    ret.prevTransform = float4x4(asfloat(f), asfloat(g), asfloat(h), asfloat(i));
    return ret;
}

MaterialConstants LoadMaterialConstants(ByteAddressBuffer buffer, uint offset)
{
    uint4 a = buffer.Load4(offset + 16 * 0);
    uint4 b = buffer.Load4(offset + 16 * 1);
    uint4 c = buffer.Load4(offset + 16 * 2);
    uint4 d = buffer.Load4(offset + 16 * 3);
    uint4 e = buffer.Load4(offset + 16 * 4);
    uint4 f = buffer.Load4(offset + 16 * 5);
    uint4 g = buffer.Load4(offset + 16 * 6);

    MaterialConstants ret;
    ret.baseOrDiffuseColor = asfloat(a.xyz);
    ret.flags = int(a.w);
    ret.specularColor = asfloat(b.xyz);
    ret.materialID = int(b.w);
    ret.emissiveColor = asfloat(c.xyz);
    ret.domain = int(c.w);
    ret.opacity = asfloat(d.x);
    ret.roughness = asfloat(d.y);
    ret.metalness = asfloat(d.z);
    ret.normalTextureScale = asfloat(d.w);
    ret.occlusionStrength = asfloat(e.x);
    ret.alphaCutoff = asfloat(e.y);
    ret.transmissionFactor = asfloat(e.z);
    ret.baseOrDiffuseTextureIndex = asfloat(e.w);
    ret.metalRoughOrSpecularTextureIndex = asfloat(f.x);
    ret.emissiveTextureIndex = int(f.y);
    ret.normalTextureIndex = int(f.z);
    ret.occlusionTextureIndex = int(f.w);
    ret.transmissionTextureIndex = int(g.x);
    ret.numAmbientStages = int(f.y);
    ret.padding[0] = int(f.z);
    ret.padding[1] = int(f.w);
    return ret;   
}

materialAmbientData_t LoadMaterialAmbientStage( ByteAddressBuffer buffer, int offset )
{
	uint4 a = buffer.Load4(offset + 16 * 0);
    uint4 b = buffer.Load4(offset + 16 * 1);
    uint4 c = buffer.Load4(offset + 16 * 2);
    uint4 d = buffer.Load4(offset + 16 * 3);
    uint4 e = buffer.Load4(offset + 16 * 4);
    uint4 f = buffer.Load4(offset + 16 * 5);
    uint4 g = buffer.Load4(offset + 16 * 6);
    uint4 h = buffer.Load4(offset + 16 * 7);
    uint4 i = buffer.Load4(offset + 16 * 8);

    materialAmbientData_t stage;
    stage.padding[0] = a.x;
    stage.padding[1] = a.y;
	stage.alphaTest = asfloat(a.z);
	stage.textureId = a.w;
	stage.texGen[0] = asfloat(b);
    stage.texGen[1] = asfloat(c);
    stage.texGen[2] = asfloat(d);
	stage.textureMatrix[0] = asfloat(e);
    stage.textureMatrix[1] = asfloat(f);
	stage.color = asfloat(g);
	stage.vertexColorModulate = asfloat(h);
	stage.vertexColorAdd = asfloat(i);

    return stage;
}

// ByteAddressBuffer::Load can only load addresses divisible by 4 bytes.
// Every two index addresses will be the same, so every 2nd index needs
// to be shifted down 2 bytes.
uint LoadIndex(ByteAddressBuffer indexBuffer, uint offset, uint vertexID)
{
    uint index = indexBuffer.Load(offset + 2 * vertexID);
	int i = vertexID % 2;
	return ( index >> ( i * 16 ) ) & 0xFFFF;
}

#endif

#endif // BINDLESS_H_