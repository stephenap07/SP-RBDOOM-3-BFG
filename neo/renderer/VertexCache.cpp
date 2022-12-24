/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013-2014 Robert Beckebans
Copyright (C) 2016-2017 Dustin Land
Copyright (C) 2022 Stephen Pridham

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

#include "precompiled.h"
#pragma hdrstop

#include "RenderCommon.h"
#include "sys/DeviceManager.h"
#include "StagingBuffer.h"

idVertexCache vertexCache;

idCVar r_showVertexCache( "r_showVertexCache", "0", CVAR_RENDERER | CVAR_BOOL, "Print stats about the vertex cache every frame" );
idCVar r_showVertexCacheTimings( "r_showVertexCacheTimings", "0", CVAR_RENDERER | CVAR_BOOL, "Print stats about the vertex cache every frame" );

extern DeviceManager* deviceManager;

/*
==============
ClearGeoBufferSet
==============
*/
static void ClearGeoBufferSet( geoBufferSet_t& gbs )
{
	gbs.indexMemUsed.SetValue( 0 );
	gbs.vertexMemUsed.SetValue( 0 );
	gbs.allocations = 0;

	if( gbs.staging )
	{
		gbs.staging->Clear();
	}

	if( gbs.instanceBuffer )
	{
		gbs.instanceBuffer->Clear();
	}

	if( gbs.geometryBuffer )
	{
		gbs.geometryBuffer->Clear();
	}

	if( gbs.geometryStaging )
	{
		gbs.geometryStaging->Clear();
	}

	if( gbs.materialStaging )
	{
		gbs.materialStaging->Clear();
	}

	if( gbs.skinnedStaging )
	{
		gbs.skinnedStaging->Clear();
	}

	if( gbs.jointStagingBuffer )
	{
		gbs.jointStagingBuffer->Clear();
	}
}

/*
==============
MapGeoBufferSet
==============
*/
static void MapGeoBufferSet( geoBufferSet_t& gbs )
{
	if( gbs.mappedVertexBase == NULL )
	{
		gbs.mappedVertexBase = ( byte* )gbs.vertexBuffer.MapBuffer( BM_WRITE );
	}

	if( gbs.mappedIndexBase == NULL )
	{
		gbs.mappedIndexBase = ( byte* )gbs.indexBuffer.MapBuffer( BM_WRITE );
	}
}

/*
==============
UnmapGeoBufferSet
==============
*/
static void UnmapGeoBufferSet( geoBufferSet_t& gbs )
{
	if( gbs.mappedVertexBase != NULL )
	{
		gbs.vertexBuffer.UnmapBuffer();
		gbs.mappedVertexBase = NULL;
	}

	if( gbs.mappedIndexBase != NULL )
	{
		gbs.indexBuffer.UnmapBuffer();
		gbs.mappedIndexBase = NULL;
	}
}

static int allocFrameNum = 0;

struct BufferData
{
	int vertexBytes = 0;
	int indexBytes = 0;
	int jointBytes = 0;
	int instanceBytes = 0;
	int stagingBytes = 0;
	int skinnedBytes = 0;
	int materialBytes = 0;
	int geometryBytes = 0;
};

/*
==============
AllocGeoBufferSet
==============
*/
static void AllocGeoBufferSet( geoBufferSet_t& gbs, BufferData bufferData, bufferUsageType_t usage, nvrhi::ICommandList* commandList )
{
	int f = allocFrameNum;

	if( usage == BU_DYNAMIC )
	{
		gbs.vertexBuffer.SetDebugName( va( "Vertex Buffer [Frame %d]", f ) );
	}
	else
	{
		gbs.vertexBuffer.SetDebugName( "Static Vertex Buffer" );
	}
	gbs.vertexBuffer.AllocBufferObject( NULL, bufferData.vertexBytes, usage, commandList );

	if( usage == BU_DYNAMIC )
	{
		gbs.indexBuffer.SetDebugName( va( "Index Buffer [Frame % d]", f ) );
	}
	else
	{
		gbs.indexBuffer.SetDebugName( "Static Index Buffer" );
	}
	gbs.indexBuffer.AllocBufferObject( NULL, bufferData.indexBytes, usage, commandList );

	nvrhi::BufferDesc jointDesc;
	jointDesc.isConstantBuffer = true;
	jointDesc.byteSize = bufferData.jointBytes;
	jointDesc.structStride = sizeof( idJointMat );
	jointDesc.canHaveRawViews = true;
	jointDesc.canHaveTypedViews = true;
	jointDesc.keepInitialState = true;
	jointDesc.initialState = nvrhi::ResourceStates::Common;
	if( usage == BU_DYNAMIC ) {
		jointDesc.debugName = va( "Joint Buffer [Frame %d]", f );
		jointDesc.cpuAccess = nvrhi::CpuAccessMode::Write;
		gbs.jointStagingBuffer = new idStagingBuffer( deviceManager->GetDevice(), jointDesc );
	} else if( bufferData.jointBytes > 0 ) {
		jointDesc.debugName = "Joint Buffer";
		gbs.jointBuffer = new idTrackedBuffer( deviceManager->GetDevice(), jointDesc );
	}

	if( usage == BU_DYNAMIC )
	{
		nvrhi::BufferDesc materialStagingDesc;
		materialStagingDesc.byteSize = bufferData.materialBytes;
		materialStagingDesc.structStride = sizeof( materialData_t );
		materialStagingDesc.cpuAccess = nvrhi::CpuAccessMode::Write;
		materialStagingDesc.canHaveRawViews = true;
		materialStagingDesc.canHaveTypedViews = true;
		materialStagingDesc.keepInitialState = true;
		materialStagingDesc.initialState = nvrhi::ResourceStates::Common;
		gbs.materialStaging = new idStagingBuffer( deviceManager->GetDevice(), materialStagingDesc );
	}
	else if( bufferData.materialBytes > 0 )
	{
		nvrhi::BufferDesc materialBufferDesc;
		materialBufferDesc.isVertexBuffer = true;
		materialBufferDesc.byteSize = bufferData.geometryBytes;
		materialBufferDesc.structStride = sizeof( materialData_t );
		materialBufferDesc.debugName = "Material Buffer";
		materialBufferDesc.canHaveTypedViews = true;
		materialBufferDesc.canHaveRawViews = true;
		materialBufferDesc.canHaveUAVs = true;
		materialBufferDesc.initialState = nvrhi::ResourceStates::Common;
		materialBufferDesc.keepInitialState = true;
		gbs.materialBuffer = new idTrackedBuffer( deviceManager->GetDevice(), materialBufferDesc );
	}

	if( bufferData.stagingBytes > 0 )
	{
		nvrhi::BufferDesc stagingDesc;
		stagingDesc.byteSize = bufferData.stagingBytes;
		stagingDesc.structStride = sizeof( instanceData_t );
		stagingDesc.cpuAccess = nvrhi::CpuAccessMode::Write;
		stagingDesc.canHaveRawViews = true;
		stagingDesc.canHaveTypedViews = true;
		stagingDesc.initialState = nvrhi::ResourceStates::Common;
		stagingDesc.debugName = va( "Instance Buffer [Frame %d]", f );
		gbs.staging = new idStagingBuffer( deviceManager->GetDevice(), stagingDesc );
	}

	if( bufferData.instanceBytes > 0 )
	{
		nvrhi::BufferDesc instanceBufferDesc;
		instanceBufferDesc.isVertexBuffer = true;
		instanceBufferDesc.byteSize = bufferData.instanceBytes;
		instanceBufferDesc.debugName = "Instance Buffer";
		instanceBufferDesc.canHaveTypedViews = true;
		instanceBufferDesc.canHaveRawViews = true;
		instanceBufferDesc.canHaveUAVs = true;
		instanceBufferDesc.keepInitialState = true;
		instanceBufferDesc.structStride = sizeof( instanceData_t );
		instanceBufferDesc.initialState = nvrhi::ResourceStates::Common;
		gbs.instanceBuffer = new idTrackedBuffer( deviceManager->GetDevice(), instanceBufferDesc );
	}

	if( usage == BU_DYNAMIC )
	{
		nvrhi::BufferDesc geometryStagingDesc;
		geometryStagingDesc.byteSize = 2048 * sizeof( geometryData_t );
		geometryStagingDesc.cpuAccess = nvrhi::CpuAccessMode::Write;
		geometryStagingDesc.canHaveRawViews = true;
		geometryStagingDesc.canHaveTypedViews = true;
		geometryStagingDesc.keepInitialState = true;
		geometryStagingDesc.structStride = sizeof( geometryData_t );
		geometryStagingDesc.initialState = nvrhi::ResourceStates::Common;
		geometryStagingDesc.debugName = va( "Geometry Buffer [Frame %d]", f );
		gbs.geometryStaging = new idStagingBuffer( deviceManager->GetDevice(), geometryStagingDesc );
	}
	else if( bufferData.geometryBytes > 0 )
	{
		nvrhi::BufferDesc geometryBufferDesc;
		geometryBufferDesc.isVertexBuffer = true;
		geometryBufferDesc.byteSize = bufferData.geometryBytes;
		geometryBufferDesc.structStride = sizeof( geometryData_t );
		geometryBufferDesc.debugName = "Geometry Buffer";
		geometryBufferDesc.canHaveTypedViews = true;
		geometryBufferDesc.canHaveRawViews = true;
		geometryBufferDesc.canHaveUAVs = true;
		geometryBufferDesc.initialState = nvrhi::ResourceStates::Common;
		geometryBufferDesc.keepInitialState = true;
		gbs.geometryBuffer = new idTrackedBuffer( deviceManager->GetDevice(), geometryBufferDesc );
	}

	nvrhi::BufferDesc skinnedDesc;
	skinnedDesc.byteSize = bufferData.skinnedBytes;
	skinnedDesc.isVertexBuffer = true;
	skinnedDesc.canHaveTypedViews = true;
	skinnedDesc.canHaveRawViews = true;
	skinnedDesc.canHaveUAVs = true;
	skinnedDesc.keepInitialState = true;
	skinnedDesc.initialState = nvrhi::ResourceStates::Common;
	skinnedDesc.structStride = sizeof( idDrawVert );

	if( usage == BU_DYNAMIC )
	{
		//skinnedDesc.debugName = va( "Skinned Buffer [Frame %d]", f );
		//skinnedDesc.canHaveUAVs = false;
		//skinnedDesc.cpuAccess = nvrhi::CpuAccessMode::Write;
		//gbs.skinnedStaging = new idStagingBuffer( deviceManager->GetDevice(), skinnedDesc );
	}
	else if( bufferData.skinnedBytes > 0 )
	{
		skinnedDesc.debugName = "Skinned Buffer";
		gbs.skinnedBuffer = new idTrackedBuffer( deviceManager->GetDevice(), skinnedDesc );
		
		skinnedDesc.debugName = "Static Skinned Buffer";
		gbs.staticSkinnedBuffer = new idTrackedBuffer( deviceManager->GetDevice(), skinnedDesc );
	}

	ClearGeoBufferSet( gbs );

	if( usage == BU_DYNAMIC )
	{
		allocFrameNum++;
	}
}

/*
==============
StageAlloc
==============
*/
static vertCacheHandle_t StageAlloc( idStagingBuffer* stagingBuffer, idTrackedBuffer* buffer, size_t numBytes, int currentFrame, const void* data, bool isStatic = true )
{
	vertCacheHandle_t handle = buffer->Alloc( numBytes, currentFrame, isStatic );
	stagingBuffer->Alloc( data, numBytes, handle );
	return handle;
}

/*
==============
idVertexCache::Init
==============
*/
void idVertexCache::Init( int _uniformBufferOffsetAlignment, nvrhi::ICommandList* commandList )
{
	currentFrame = 0;
	listNum = 0;

	uniformBufferOffsetAlignment = _uniformBufferOffsetAlignment;

	mostUsedVertex = 0;
	mostUsedIndex = 0;
	mostUsedJoint = 0;

	allocFrameNum = 0;

	nvrhi::CommandListParameters parms;
	parms.setQueueType( nvrhi::CommandQueue::Copy );

	BufferData mappedData;
	mappedData.vertexBytes = VERTCACHE_VERTEX_MEMORY_PER_FRAME;
	mappedData.indexBytes = VERTCACHE_INDEX_MEMORY_PER_FRAME;
	mappedData.jointBytes = VERTCACHE_JOINT_MEMORY_PER_FRAME;
	mappedData.skinnedBytes = VERTCACHE_SKINNED_VERTEX_MEMORY_PER_FRAME;

	mappedData.stagingBytes = VERTCACHE_INSTANCE_MEMORY_PER_FRAME;
	mappedData.materialBytes = VERTCACHE_MATERIAL_MEMORY_PER_FRAME;
	mappedData.geometryBytes = 2048 * sizeof( geometryData_t );

	BufferData staticBufferData;
	staticBufferData.vertexBytes = STATIC_VERTEX_MEMORY;
	staticBufferData.indexBytes = STATIC_INDEX_MEMORY;
	staticBufferData.jointBytes = VERTCACHE_JOINT_MEMORY_PER_FRAME;
	staticBufferData.stagingBytes = 0;
	staticBufferData.instanceBytes = VERTCACHE_INSTANCE_MEMORY_PER_FRAME;
	staticBufferData.skinnedBytes = STATIC_SKINNED_VERTEX_MEMORY;
	staticBufferData.materialBytes = 16384 * sizeof( materialData_t );
	staticBufferData.geometryBytes = 2048 * sizeof( geometryData_t );

	for( int i = 0; i < NUM_FRAME_DATA; i++ )
	{
		AllocGeoBufferSet( frameData[i], mappedData, BU_DYNAMIC, commandList );
	}

	AllocGeoBufferSet( staticData, staticBufferData, BU_STATIC, commandList );

	MapGeoBufferSet( frameData[ listNum ] );
}

/*
==============
idVertexCache::Shutdown
==============
*/
void idVertexCache::Shutdown()
{
	for( int i = 0; i < NUM_FRAME_DATA; i++ )
	{
		frameData[i].vertexBuffer.FreeBufferObject();
		frameData[i].indexBuffer.FreeBufferObject();
		delete frameData[i].staging;
		frameData[i].staging = nullptr;
		delete frameData[i].geometryStaging;
		frameData[i].geometryStaging = nullptr;
		delete frameData[i].materialStaging;
		frameData[i].materialStaging = nullptr;
		delete frameData[i].materialBuffer;
		frameData[i].materialBuffer = nullptr;
		delete frameData[i].skinnedStaging;
		frameData[i].skinnedStaging = nullptr;
		delete frameData[i].jointStagingBuffer;
		frameData[i].jointStagingBuffer = nullptr;
	}

	// SRS - free static buffers to avoid Vulkan validation layer errors on shutdown
	staticData.vertexBuffer.FreeBufferObject();
	staticData.indexBuffer.FreeBufferObject();

	delete staticData.staging;
	staticData.staging = nullptr;
	delete staticData.instanceBuffer;
	staticData.instanceBuffer = nullptr;
	delete staticData.geometryBuffer;
	staticData.geometryBuffer = nullptr;
	delete staticData.geometryStaging;
	staticData.geometryStaging = nullptr;
	delete staticData.materialBuffer;
	staticData.materialBuffer = nullptr;
	delete staticData.materialStaging;
	staticData.materialStaging = nullptr;
	delete staticData.skinnedBuffer;
	staticData.skinnedBuffer = nullptr;
	delete staticData.jointBuffer;
	staticData.jointStagingBuffer = nullptr;
}

/*
==============
idVertexCache::PurgeAll
==============
*/
void idVertexCache::PurgeAll( nvrhi::ICommandList* commandList )
{
	Shutdown();
	Init( uniformBufferOffsetAlignment, commandList );
}

/*
==============
idVertexCache::FreeStaticData

call on loading a new map
==============
*/
void idVertexCache::FreeStaticData()
{
	ClearGeoBufferSet( staticData );
	mostUsedVertex = 0;
	mostUsedIndex = 0;
	mostUsedJoint = 0;
}

/*
==============
idVertexCache::ActuallyAlloc
==============
*/
vertCacheHandle_t idVertexCache::ActuallyAlloc( geoBufferSet_t& vcs, const void* data, int bytes, cacheType_t type, nvrhi::ICommandList* commandList )
{
	if( bytes == 0 )
	{
		return ( vertCacheHandle_t )0;
	}

	// RB: changed UINT_PTR to uintptr_t
	assert( ( ( ( uintptr_t )( data ) ) & 15 ) == 0 );
	// RB end

	assert( ( bytes & 15 ) == 0 );

	int	endPos = 0;
	int offset = 0;

	switch( type )
	{
		case CACHE_INDEX:
		{
			endPos = vcs.indexMemUsed.Add( bytes );
			if( endPos > vcs.indexBuffer.GetAllocedSize() )
			{
				idLib::Error( "Out of index cache" );
			}

			offset = endPos - bytes;

			if( data != NULL )
			{
				if( vcs.indexBuffer.GetUsage() == BU_DYNAMIC )
				{
					MapGeoBufferSet( vcs );
				}
				vcs.indexBuffer.Update( data, bytes, offset, false, commandList );
			}

			break;
		}
		case CACHE_VERTEX:
		{
			endPos = vcs.vertexMemUsed.Add( bytes );
			if( endPos > vcs.vertexBuffer.GetAllocedSize() )
			{
				idLib::Error( "Out of vertex cache" );
			}

			offset = endPos - bytes;

			if( data != NULL )
			{
				if( vcs.vertexBuffer.GetUsage() == BU_DYNAMIC )
				{
					MapGeoBufferSet( vcs );
				}
				vcs.vertexBuffer.Update( data, bytes, offset, false, commandList );
			}

			break;
		}
		default:
			assert( false );
	}

	vcs.allocations++;

	vertCacheHandle_t handle =	( ( uint64 )( currentFrame & VERTCACHE_FRAME_MASK ) << VERTCACHE_FRAME_SHIFT ) |
								( ( uint64 )( offset & VERTCACHE_OFFSET_MASK ) << VERTCACHE_OFFSET_SHIFT ) |
								( ( uint64 )( bytes & VERTCACHE_SIZE_MASK ) << VERTCACHE_SIZE_SHIFT );
	if( &vcs == &staticData )
	{
		handle |= VERTCACHE_STATIC;
	}
	return handle;
}

/*
==============
idVertexCache::AllocVertex
==============
*/
vertCacheHandle_t idVertexCache::AllocVertex( const void* data, int num, size_t size /*= sizeof( idDrawVert ) */, nvrhi::ICommandList* commandList )
{
	return ActuallyAlloc( frameData[ listNum ], data, ALIGN( num * size, VERTEX_CACHE_ALIGN ), CACHE_VERTEX, commandList );
}

/*
==============
idVertexCache::AllocIndex
==============
*/
vertCacheHandle_t idVertexCache::AllocIndex( const void* data, int num, size_t size /*= sizeof( triIndex_t ) */, nvrhi::ICommandList* commandList )
{
	return ActuallyAlloc( frameData[ listNum ], data, ALIGN( num * size, INDEX_CACHE_ALIGN ), CACHE_INDEX, commandList );
}

/*
==============
idVertexCache::AllocJoint
==============
*/
vertCacheHandle_t idVertexCache::AllocJoint( const void* data, int num, size_t size /*= sizeof( idJointMat ) */ )
{
	return StageAlloc( frameData[listNum].jointStagingBuffer, staticData.jointBuffer, num * size, currentFrame, data, false );
}

/*
==============
idVertexCache::AllocMaterial
==============
*/
vertCacheHandle_t idVertexCache::AllocMaterial( const void* data, int numBytes )
{
	vertCacheHandle_t handle = staticData.materialBuffer->Alloc( numBytes, currentFrame );
	frameData[listNum].materialStaging->Alloc( data, numBytes, handle );
	return handle;
}

/*
==============
idVertexCache::AllocStaticVertex
==============
*/
vertCacheHandle_t idVertexCache::AllocStaticVertex( const void* data, int bytes, nvrhi::ICommandList* commandList )
{
	if( staticData.vertexMemUsed.GetValue() + bytes > STATIC_VERTEX_MEMORY )
	{
		idLib::FatalError( "AllocStaticVertex failed, increase STATIC_VERTEX_MEMORY" );
	}
	return ActuallyAlloc( staticData, data, bytes, CACHE_VERTEX, commandList );
}

/*
==============
idVertexCache::AllocStaticIndex
==============
*/
vertCacheHandle_t idVertexCache::AllocStaticIndex( const void* data, int bytes, nvrhi::ICommandList* commandList )
{
	if( staticData.indexMemUsed.GetValue() + bytes > STATIC_INDEX_MEMORY )
	{
		idLib::FatalError( "AllocStaticIndex failed, increase STATIC_INDEX_MEMORY" );
	}
	return ActuallyAlloc( staticData, data, bytes, CACHE_INDEX, commandList );
}

/*
==============
idVertexCache::AllocStaticInstance
==============
*/
vertCacheHandle_t idVertexCache::AllocStaticInstance( const void* data, int bytes )
{
	return StageAlloc( frameData[listNum].staging, staticData.instanceBuffer, bytes, currentFrame, data, false );
}

/*
==============
idVertexCache::AllocGeometryData
==============
*/
vertCacheHandle_t idVertexCache::AllocGeometryData( const void* data, int bytes )
{
	return StageAlloc( frameData[listNum].geometryStaging, staticData.geometryBuffer, bytes, currentFrame, data, false );
}

/*
==============
idVertexCache::AllocStaticSkinnedVertex

Allocates static skinned vertices.
==============
*/
vertCacheHandle_t idVertexCache::AllocStaticSkinnedVertex( int bytes )
{
	return staticData.staticSkinnedBuffer->Alloc( bytes, currentFrame, true );
}

/*
==============
idVertexCache::AllocFrameSkinnedVertex

Allocates frame temporary skinned vertices.
==============
*/
vertCacheHandle_t idVertexCache::AllocFrameSkinnedVertex( int bytes )
{
	return staticData.skinnedBuffer->Alloc( bytes, currentFrame, false );
}

/*
==============
idVertexCache::MappedVertexBuffer
==============
*/
byte* idVertexCache::MappedVertexBuffer( vertCacheHandle_t handle )
{
	release_assert( !CacheIsStatic( handle ) );
	const uint64 offset = ( int )( handle >> VERTCACHE_OFFSET_SHIFT ) & VERTCACHE_OFFSET_MASK;
	const uint64 frameNum = ( int )( handle >> VERTCACHE_FRAME_SHIFT ) & VERTCACHE_FRAME_MASK;
	release_assert( frameNum == ( currentFrame & VERTCACHE_FRAME_MASK ) );
	return frameData[ listNum ].mappedVertexBase + offset;
}

/*
==============
idVertexCache::MappedIndexBuffer
==============
*/
byte* idVertexCache::MappedIndexBuffer( vertCacheHandle_t handle )
{
	release_assert( !CacheIsStatic( handle ) );
	const uint64 offset = ( int )( handle >> VERTCACHE_OFFSET_SHIFT ) & VERTCACHE_OFFSET_MASK;
	const uint64 frameNum = ( int )( handle >> VERTCACHE_FRAME_SHIFT ) & VERTCACHE_FRAME_MASK;
	release_assert( frameNum == ( currentFrame & VERTCACHE_FRAME_MASK ) );
	return frameData[ listNum ].mappedIndexBase + offset;
}

/*
==============
idVertexCache::MappedSkinnedVertexBuffer
==============
*/
byte* idVertexCache::MappedSkinnedVertexBuffer( vertCacheHandle_t handle )
{
	return nullptr;
}

/*
==============
idVertexCache::CacheIsCurrent
==============
*/
bool idVertexCache::CacheIsCurrent( const vertCacheHandle_t handle )
{
	const int isStatic = handle & VERTCACHE_STATIC;
	if( isStatic )
	{
		return true;
	}
	const uint64 frameNum = ( int )( handle >> VERTCACHE_FRAME_SHIFT ) & VERTCACHE_FRAME_MASK;
	if( frameNum != ( currentFrame & VERTCACHE_FRAME_MASK ) )
	{
		return false;
	}
	return true;
}

/*
==============
idVertexCache::GetVertexBuffer
==============
*/
bool idVertexCache::GetVertexBuffer( vertCacheHandle_t handle, idVertexBuffer* vb )
{
	const int isStatic = handle & VERTCACHE_STATIC;
	const uint64 size = ( int )( handle >> VERTCACHE_SIZE_SHIFT ) & VERTCACHE_SIZE_MASK;
	const uint64 offset = ( int )( handle >> VERTCACHE_OFFSET_SHIFT ) & VERTCACHE_OFFSET_MASK;
	const uint64 frameNum = ( int )( handle >> VERTCACHE_FRAME_SHIFT ) & VERTCACHE_FRAME_MASK;
	if( isStatic )
	{
		vb->Reference( staticData.vertexBuffer, offset, size );
		return true;
	}
	if( frameNum != ( ( currentFrame - 1 ) & VERTCACHE_FRAME_MASK ) )
	{
		return false;
	}
	vb->Reference( frameData[ drawListNum ].vertexBuffer, offset, size );
	return true;
}

/*
==============
idVertexCache::GetIndexBuffer
==============
*/
bool idVertexCache::GetIndexBuffer( vertCacheHandle_t handle, idIndexBuffer* ib )
{
	const int isStatic = handle & VERTCACHE_STATIC;
	const uint64 size = ( int )( handle >> VERTCACHE_SIZE_SHIFT ) & VERTCACHE_SIZE_MASK;
	const uint64 offset = ( int )( handle >> VERTCACHE_OFFSET_SHIFT ) & VERTCACHE_OFFSET_MASK;
	const uint64 frameNum = ( int )( handle >> VERTCACHE_FRAME_SHIFT ) & VERTCACHE_FRAME_MASK;
	if( isStatic )
	{
		ib->Reference( staticData.indexBuffer, offset, size );
		return true;
	}
	if( frameNum != ( ( currentFrame - 1 ) & VERTCACHE_FRAME_MASK ) )
	{
		return false;
	}
	ib->Reference( frameData[ drawListNum ].indexBuffer, offset, size );
	return true;
}

/*
==============
idVertexCache::GetJointBuffer
==============
*/
bool idVertexCache::GetJointBuffer( vertCacheHandle_t handle, bufferView_t* bv )
{
	return staticData.jointBuffer->GetBufferView( handle, bv );
}

/*
==============
idVertexCache::GetInstanceBuffer
==============
*/
bool idVertexCache::GetInstanceBuffer( vertCacheHandle_t handle, bufferView_t* bv )
{
	return staticData.instanceBuffer->GetBufferView( handle, bv );
}

/*
==============
idVertexCache::GetGeometryData
==============
*/
bool idVertexCache::GetGeometryData( vertCacheHandle_t handle, bufferView_t* bv )
{
	return staticData.geometryBuffer->GetBufferView( handle, bv );
}

/*
==============
idVertexCache::GetSkinnedVertexBuffer
==============
*/
bool idVertexCache::GetSkinnedVertexBuffer( vertCacheHandle_t handle, bufferView_t* bv )
{
	if( CacheIsStatic( handle ) )
	{
		return staticData.staticSkinnedBuffer->GetBufferView( handle, bv );
	}
	return staticData.skinnedBuffer->GetBufferView( handle, bv );
}

/*
==============
idVertexCache::GetMaterialBuffer
==============
*/
bool idVertexCache::GetMaterialBuffer( vertCacheHandle_t handle, bufferView_t* bv )
{
	return staticData.materialBuffer->GetBufferView( handle, bv );
}

/*
==============
idVertexCache::UpdateInstanceBuffer
==============
*/
void idVertexCache::UpdateInstanceBuffer( vertCacheHandle_t handle, void* data )
{
	const uint64 numBytes = ( int )( handle >> VERTCACHE_SIZE_SHIFT ) & VERTCACHE_SIZE_MASK;
	frameData[listNum].staging->Alloc( data, numBytes, handle );
}

/*
==============
idVertexCache::UpdateMaterialBuffer
==============
*/
void idVertexCache::UpdateMaterialBuffer( vertCacheHandle_t handle, void* data )
{
	const uint64 numBytes = ( int )( handle >> VERTCACHE_SIZE_SHIFT ) & VERTCACHE_SIZE_MASK;
	frameData[listNum].materialStaging->Alloc( data, numBytes, handle );
}

/*
==============
idVertexCache::BeginBackEnd
==============
*/
void idVertexCache::BeginBackEnd()
{
	mostUsedVertex = Max( mostUsedVertex, frameData[ listNum ].vertexMemUsed.GetValue() );
	mostUsedIndex = Max( mostUsedIndex, frameData[ listNum ].indexMemUsed.GetValue() );
	//mostUsedJoint = Max( mostUsedJoint, frameData[ listNum ].jointBuffer.GetValue() );

	if( r_showVertexCache.GetBool() )
	{
		idLib::Printf( "%08d: %d allocations, %dkB vertex, %dkB index, %ikB joint, instance %ikB: %dkB vertex, %dkB index, joint, %ikB instance\n",
					   currentFrame, frameData[ listNum ].allocations,
					   frameData[ listNum ].vertexMemUsed.GetValue() / 1024,
					   frameData[ listNum ].indexMemUsed.GetValue() / 1024,
					   //frameData[ listNum ].jointMemUsed.GetValue() / 1024,
					   mostUsedVertex / 1024,
					   mostUsedIndex / 1024,
					   mostUsedJoint / 1024,
					   mostUsedInstance / 1024 );
	}

	nvrhi::ICommandList* commandList = tr.CommandList();

	// Copy buffers from the upload CPU accessible buffers into vmram for
	// faster reads on the gpu. For UMA architectures this wouldn't be very productive
	// since there is no performance penalty for using upload buffers directly.
	commandList->beginMarker( "Copy buffers" );
	frameData[listNum].staging->CopyBuffers( commandList, staticData.instanceBuffer->GetBuffer() );
	frameData[listNum].geometryStaging->CopyBuffers( commandList, staticData.geometryBuffer->GetBuffer() );
	frameData[listNum].materialStaging->CopyBuffers( commandList, staticData.materialBuffer->GetBuffer() );
	//frameData[listNum].skinnedStaging->CopyBuffers( commandList, staticData.skinnedBuffer->GetBuffer() );
	frameData[listNum].jointStagingBuffer->CopyBuffers( commandList, staticData.jointBuffer->GetBuffer() );
	commandList->endMarker();

	// unmap the current frame so the GPU can read it
	const int startUnmap = Sys_Milliseconds();
	UnmapGeoBufferSet( frameData[ listNum ] );
	UnmapGeoBufferSet( staticData );
	const int endUnmap = Sys_Milliseconds();
	if( endUnmap - startUnmap > 1 )
	{
		idLib::PrintfIf( r_showVertexCacheTimings.GetBool(), "idVertexCache::unmap took %i msec\n", endUnmap - startUnmap );
	}
	drawListNum = listNum;

	// prepare the next frame for writing to by the CPU
	currentFrame++;

	listNum = currentFrame % NUM_FRAME_DATA;
	const int startMap = Sys_Milliseconds();
	MapGeoBufferSet( frameData[ listNum ] );

	const int endMap = Sys_Milliseconds();
	if( endMap - startMap > 1 )
	{
		idLib::PrintfIf( r_showVertexCacheTimings.GetBool(), "idVertexCache::map took %i msec\n", endMap - startMap );
	}

	// "Clear" address space of these buffers, but doesn't actually clear the data for use by the backend on the gpu.
	staticData.instanceBuffer->Clear();
	staticData.geometryBuffer->Clear();
	staticData.materialBuffer->Clear();
	staticData.skinnedBuffer->Clear();
	staticData.jointBuffer->Clear();

	ClearGeoBufferSet( frameData[listNum] );
}


/*
========================
idTrackedBuffer::idTrackedBuffer
========================
*/
idTrackedBuffer::idTrackedBuffer( nvrhi::DeviceHandle device, nvrhi::BufferDesc desc )
	: memUsed()
	, device( device )
{
	assert( !buffer );

	if( desc.byteSize <= 0 )
	{
		idLib::Error( "idTrackedBuffer::idTrackedBuffer: allocSize = %i", desc.byteSize );
	}

	buffer = deviceManager->GetDevice()->createBuffer( desc );

	assert( tr.backend.descriptorTableManager );
	bindlessHandle = tr.backend.descriptorTableManager->CreateDescriptorHandle(
		nvrhi::BindingSetItem::RawBuffer_SRV( 0, buffer ) );
}

/*
========================
idTrackedBuffer::~idTrackedBuffer
========================
*/
idTrackedBuffer::~idTrackedBuffer()
{
	buffer.Reset();
}

/*
========================
idTrackedBuffer::Alloc
========================
*/
vertCacheHandle_t idTrackedBuffer::Alloc( size_t numBytes, int currentFrame, bool isStatic )
{
	if( numBytes == 0 )
	{
		return ( vertCacheHandle_t )0;
	}

	assert( buffer );
	assert( ( numBytes & 15 ) == 0 );

	uint64 endPos = memUsed.Add( numBytes );
	if( endPos > buffer->getDesc().byteSize )
	{
		idLib::Error( "Out of buffer cache" );
	}

	uint64 offset = endPos - numBytes;

	vertCacheHandle_t handle = ( ( uint64 )( currentFrame & VERTCACHE_FRAME_MASK ) << VERTCACHE_FRAME_SHIFT ) |
							   ( ( uint64 )( offset & VERTCACHE_OFFSET_MASK ) << VERTCACHE_OFFSET_SHIFT ) |
							   ( ( uint64 )( numBytes & VERTCACHE_SIZE_MASK ) << VERTCACHE_SIZE_SHIFT );

	if( isStatic )
	{
		handle |= VERTCACHE_STATIC;
	}

	return handle;
}

void idTrackedBuffer::Clear()
{
	memUsed.SetValue( 0 );
}

bool idTrackedBuffer::GetBufferView( vertCacheHandle_t handle, bufferView_t* bv )
{
	const uint64 numBytes = ( int )( handle >> VERTCACHE_SIZE_SHIFT ) & VERTCACHE_SIZE_MASK;
	const uint64 offset = ( int )( handle >> VERTCACHE_OFFSET_SHIFT ) & VERTCACHE_OFFSET_MASK;
	const uint64 frameNum = ( int )( handle >> VERTCACHE_FRAME_SHIFT ) & VERTCACHE_FRAME_MASK;
	assert( numBytes + offset < buffer->getDesc().byteSize );
	bv->buffer = ( void* )buffer.Get();
	bv->offset = offset;
	bv->size = numBytes;
	bv->bindlessIndex = bindlessHandle.Get();

	return true;
}
