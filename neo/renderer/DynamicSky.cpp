#include "precompiled.h"
#pragma hdrstop

#include "DynamicSky.h"

#include "nvrhi/utils.h"
#include "Passes/CommonPasses.h"

DynamicSky::DynamicSky()
	: tri( nullptr ), paramsUpdated( false ), device( nullptr )
{
}

DynamicSky::~DynamicSky()
{
	Mem_Free( tri );
}

/**
 * Generates a screen - space grid of triangles.
 * Because of performance reasons, and because sky color is smooth, sky color is computed in vertex shader.
 * 32x32 is a reasonable size for the grid to have smooth enough colors.
 */ 
void DynamicSky::Init(nvrhi::IDevice* newDevice, const int verticalCount, const int horizontalCount )
{
	device = newDevice;

	if( tri )
	{
		Mem_Free( tri );
	}

	tri = static_cast<srfTriangles_t*>( Mem_ClearedAlloc( sizeof(*tri), TAG_RENDER_ENTITY ) );

	tri->numIndexes = (verticalCount - 1) * (horizontalCount - 1) * 6;
	tri->numVerts = verticalCount * horizontalCount;

	const int indexSize = tri->numIndexes * sizeof(tri->indexes[0]);
	const int allocatedIndexBytes = ALIGN( indexSize, 16 );
	tri->indexes = static_cast<triIndex_t*>( Mem_Alloc( allocatedIndexBytes, TAG_RENDER_ENTITY ) );

	const int vertexSize = tri->numVerts * sizeof(tri->verts[0]);
	const int allocatedVertexBytes = ALIGN( vertexSize, 16 );
	tri->verts = static_cast<idDrawVert*>( Mem_ClearedAlloc( allocatedVertexBytes, TAG_RENDER_ENTITY ) );

	for( int i = 0; i < verticalCount; i++ )
	{
		for( int j = 0; j < horizontalCount; j++ )
		{
			idDrawVert& v = tri->verts[i * verticalCount + j];
			idVec3 pos((float)(j) / (horizontalCount - 1) * 2.0f - 1.0f,
					   (float)(i) / (verticalCount - 1) * 2.0f - 1.0f,
						1.0f);
			v.xyz = pos;
		}
	}

	int k = 0;
	for( int i = 0; i < verticalCount - 1; i++ )
	{
		for( int j = 0; j < horizontalCount - 1; j++ )
		{
			tri->indexes[k++] = static_cast<triIndex_t>( j + 0 + horizontalCount * (i + 0) );
			tri->indexes[k++] = static_cast<triIndex_t>( j + 1 + horizontalCount * (i + 0) );
			tri->indexes[k++] = static_cast<triIndex_t>( j + 0 + horizontalCount * (i + 1) );

			tri->indexes[k++] = static_cast<triIndex_t>( j + 1 + horizontalCount * (i + 0) );
			tri->indexes[k++] = static_cast<triIndex_t>( j + 1 + horizontalCount * (i + 1) );
			tri->indexes[k++] = static_cast<triIndex_t>( j + 0 + horizontalCount * (i + 1) );
		}
	}

	paramBuffer = device->createBuffer(
		nvrhi::utils::CreateVolatileConstantBufferDesc( sizeof( SkyConstants ),
														"SkyParams",
														c_MaxRenderPassConstantBufferVersions ) );
}

void DynamicSky::SetInvProjMatrix( const idRenderMatrix& mat )
{
	params.invViewProj = mat;
	paramsUpdated = true;
}

void DynamicSky::UpdateParams( const SkyConstants newParams )
{
	if( newParams == params )
	{
		return;
	}

	params = newParams;
	paramsUpdated = true;
}

void DynamicSky::WriteParams( nvrhi::ICommandList* commandList )
{
	if( paramsUpdated )
	{
		constexpr auto paramSize = sizeof(params);
		commandList->writeBuffer( paramBuffer, &params, paramSize);
		paramsUpdated = false;
	}
}
