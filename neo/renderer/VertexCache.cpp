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
	gbs.jointMemUsed.SetValue( 0 );
	gbs.instanceMemUsed.SetValue( 0 );
	gbs.skinnedMemUsed.SetValue( 0 );
	gbs.materialMemUsed.SetValue( 0 );
	gbs.allocations = 0;
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

	if( gbs.mappedJointBase == NULL && gbs.jointBuffer.GetAllocedSize() != 0 )
	{
		gbs.mappedJointBase = ( byte* )gbs.jointBuffer.MapBuffer( BM_WRITE );
	}

	if( gbs.mappedInstanceBase == NULL && gbs.instanceBuffer.GetAllocedSize() != 0 )
	{
		gbs.mappedInstanceBase = ( byte* )gbs.instanceBuffer.MapBuffer( BM_WRITE );
	}

	if( gbs.mappedSkinnedBase == NULL && gbs.skinnedBuffer.GetAllocedSize() != 0 )
	{
		gbs.mappedSkinnedBase = ( byte* )gbs.skinnedBuffer.MapBuffer( BM_WRITE );
	}

	if( gbs.mappedMaterialBase == NULL && gbs.materialBuffer.GetAllocedSize() != 0 )
	{
		gbs.mappedMaterialBase = ( byte* )gbs.materialBuffer.MapBuffer( BM_WRITE );
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

	if( gbs.mappedJointBase != NULL )
	{
		gbs.jointBuffer.UnmapBuffer();
		gbs.mappedJointBase = NULL;
	}

	if( gbs.mappedInstanceBase != NULL )
	{
		gbs.instanceBuffer.UnmapBuffer();
		gbs.mappedInstanceBase = NULL;
	}

	if( gbs.mappedSkinnedBase != NULL )
	{
		gbs.skinnedBuffer.UnmapBuffer();
		gbs.mappedSkinnedBase = NULL;
	}

	if( gbs.mappedMaterialBase != NULL )
	{
		gbs.materialBuffer.UnmapBuffer();
		gbs.mappedMaterialBase = NULL;
	}
}

static int allocFrameNum = 0;

struct BufferData
{
	int vertexBytes = 0;
	int indexBytes = 0;
	int jointBytes = 0;
	int instanceBytes = 0;
	int skinnedBytes = 0;
	int materialBytes = 0;
};

/*
==============
AllocGeoBufferSet
==============
*/
static void AllocGeoBufferSet( geoBufferSet_t& gbs, BufferData bufferData, bufferUsageType_t usage, nvrhi::ICommandList* commandList )
{
	idStr usageStr = ( usage == BU_STATIC ) ? "Static" : "Mapped";
	idStr vertexBufferName = usageStr;
	vertexBufferName.Append( " Vertex Buffer" );

	uintptr_t f = allocFrameNum;

	if( usage == BU_DYNAMIC )
	{
		vertexBufferName.Append( va( " [Frame % d]", ( int )f ) );
	}
	gbs.vertexBuffer.AllocBufferObject( NULL, bufferData.vertexBytes, usage, commandList );

	idStr indexBufferName = usageStr;
	indexBufferName.Append( " Index Buffer" );
	if( usage == BU_DYNAMIC )
	{
		indexBufferName.Append( va( " [Frame % d]", ( int )f ) );
	}
	gbs.indexBuffer.SetDebugName( indexBufferName.c_str() );
	gbs.indexBuffer.AllocBufferObject( NULL, bufferData.indexBytes, usage, commandList );

	if( bufferData.jointBytes > 0 )
	{
		gbs.jointBuffer.AllocBufferObject( NULL, bufferData.jointBytes, sizeof( idVec4 ), usage, commandList );
	}

	idStr instanceBufferName = usageStr;
	instanceBufferName.Append( " Instance Buffer" );
	if( usage == BU_DYNAMIC )
	{
		instanceBufferName.Append( va( " [Frame % d]", ( int )f ) );
	}
	gbs.instanceBuffer.SetDebugName( instanceBufferName.c_str() );
	if( bufferData.instanceBytes > 0 )
	{
		gbs.instanceBuffer.AllocBufferObject( NULL, bufferData.instanceBytes, usage, commandList );
	}

	// maybe only allocate static for skinned vertices. these should change on a variable basis every 1 or 2 frames.
	idStr skinnedVertexBufferName = usageStr;
	skinnedVertexBufferName.Append( " Skinned Vertex Buffer" );
	if( usage == BU_DYNAMIC )
	{
		skinnedVertexBufferName.Append( va( " [Frame % d]", ( int )f ) );
	}
	nvrhi::BufferDesc skinnedDesc;
	skinnedDesc.isVertexBuffer = true;
	skinnedDesc.debugName = skinnedVertexBufferName.c_str();
	skinnedDesc.canHaveTypedViews = true;
	skinnedDesc.canHaveRawViews = true;
	skinnedDesc.canHaveUAVs = true;
	skinnedDesc.keepInitialState = true;
	skinnedDesc.initialState = nvrhi::ResourceStates::VertexBuffer;
	if( usage == BU_DYNAMIC )
	{
		skinnedDesc.canHaveUAVs = false;
		skinnedDesc.cpuAccess = nvrhi::CpuAccessMode::Write;
	}

	if( bufferData.skinnedBytes > 0 )
	{
		gbs.skinnedBuffer.AllocBufferObject( NULL, bufferData.skinnedBytes, usage, skinnedDesc, commandList );
	}

	// maybe only allocate static for skinned vertices. these should change on a variable basis every 1 or 2 frames.
	idStr materialBufferName = usageStr;
	materialBufferName.Append( " Material Buffer" );
	if( usage == BU_DYNAMIC )
	{
		materialBufferName.Append( va( " [Frame % d]", ( int )f ) );
	}
	gbs.materialBuffer.SetDebugName( materialBufferName.c_str() );
	if( bufferData.materialBytes > 0 )
	{
		gbs.materialBuffer.AllocBufferObject( NULL, bufferData.materialBytes, 0, usage, commandList );
	}

	ClearGeoBufferSet( gbs );

	if( usage == BU_DYNAMIC )
	{
		allocFrameNum++;
	}
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
	mappedData.instanceBytes = VERTCACHE_INSTANCE_MEMORY_PER_FRAME;
	mappedData.skinnedBytes = VERTCACHE_SKINNED_VERTEX_MEMORY_PER_FRAME;
	mappedData.materialBytes = VERTCACHE_MATERIAL_MEMORY_PER_FRAME;

	BufferData staticBufferData;
	staticBufferData.vertexBytes = STATIC_VERTEX_MEMORY;
	staticBufferData.indexBytes = STATIC_INDEX_MEMORY;
	staticBufferData.jointBytes = 0;
	staticBufferData.instanceBytes = 0;
	staticBufferData.skinnedBytes = STATIC_SKINNED_VERTEX_MEMORY;
	staticBufferData.materialBytes = 0;

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
		frameData[i].jointBuffer.FreeBufferObject();
		frameData[i].instanceBuffer.FreeBufferObject();
		frameData[i].skinnedBuffer.FreeBufferObject();
		frameData[i].materialBuffer.FreeBufferObject();
	}

	// SRS - free static buffers to avoid Vulkan validation layer errors on shutdown
	staticData.vertexBuffer.FreeBufferObject();
	staticData.indexBuffer.FreeBufferObject();
	staticData.jointBuffer.FreeBufferObject();
	staticData.instanceBuffer.FreeBufferObject();
	staticData.skinnedBuffer.FreeBufferObject();
	staticData.materialBuffer.FreeBufferObject();
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
		case CACHE_JOINT:
		{
			endPos = vcs.jointMemUsed.Add( bytes );
			if( endPos > vcs.jointBuffer.GetAllocedSize() )
			{
				idLib::Error( "Out of joint buffer cache" );
			}

			offset = endPos - bytes;

			if( data != NULL )
			{
				if( vcs.jointBuffer.GetUsage() == BU_DYNAMIC )
				{
					MapGeoBufferSet( vcs );
				}
				vcs.jointBuffer.Update( data, bytes, offset, false, commandList );
			}

			break;
		}
		case CACHE_INSTANCE:
		{
			endPos = vcs.instanceMemUsed.Add( bytes );
			if( endPos > vcs.instanceBuffer.GetAllocedSize() )
			{
				idLib::Error( "Out of instance cache" );
			}

			offset = endPos - bytes;

			if( data != NULL )
			{
				if( vcs.instanceBuffer.GetUsage() == BU_DYNAMIC )
				{
					MapGeoBufferSet( vcs );
				}
				vcs.instanceBuffer.Update( data, bytes, offset, false, commandList );
			}
			break;
		}
		case CACHE_SKINNED_VERTEX:
		{
			endPos = vcs.skinnedMemUsed.Add( bytes );
			if( endPos > vcs.skinnedBuffer.GetAllocedSize() )
			{
				idLib::Error( "Out of skinned vertex cache" );
			}

			offset = endPos - bytes;

			if( data != NULL )
			{
				if( vcs.skinnedBuffer.GetUsage() == BU_DYNAMIC )
				{
					MapGeoBufferSet( vcs );
				}
				vcs.skinnedBuffer.Update( data, bytes, offset, false, commandList );
			}
			break;
		}
		case CACHE_MATERIAL:
		{
			endPos = vcs.materialMemUsed.Add( bytes );
			if( endPos > vcs.materialBuffer.GetAllocedSize() )
			{
				idLib::Error( "Out of material cache" );
			}

			offset = endPos - bytes;

			if( data != NULL )
			{
				if( vcs.materialBuffer.GetUsage() == BU_DYNAMIC )
				{
					MapGeoBufferSet( vcs );
				}
				vcs.materialBuffer.Update( data, bytes, offset, false, commandList );
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
vertCacheHandle_t idVertexCache::AllocJoint( const void* data, int num, size_t size /*= sizeof( idJointMat ) */, nvrhi::ICommandList* commandList )
{
	return ActuallyAlloc( frameData[ listNum ], data, ALIGN( num * size, uniformBufferOffsetAlignment ), CACHE_JOINT, commandList );
}

/*
==============
idVertexCache::AllocInstance
==============
*/
vertCacheHandle_t idVertexCache::AllocInstance( const void* data, int num, size_t size, nvrhi::ICommandList* commandList )
{
	return ActuallyAlloc( frameData[ listNum ], data, ALIGN( num * size, VERTEX_CACHE_ALIGN ), CACHE_INSTANCE, commandList );
}

/*
==============
idVertexCache::AllocSkinnedVertex
==============
*/
vertCacheHandle_t idVertexCache::AllocSkinnedVertex( const void* data, int num, size_t size, nvrhi::ICommandList* commandList )
{
	return ActuallyAlloc( frameData[ listNum ], data, ALIGN( num * size, VERTEX_CACHE_ALIGN ), CACHE_SKINNED_VERTEX, commandList );
}

/*
==============
idVertexCache::AllocMaterial
==============
*/
vertCacheHandle_t idVertexCache::AllocMaterial( const void* data, int num, size_t size, nvrhi::ICommandList* commandList )
{
	return ActuallyAlloc( frameData[ listNum ], data, ALIGN( num * size, MATERIAL_CACHE_ALIGN ), CACHE_MATERIAL, commandList );
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
vertCacheHandle_t idVertexCache::AllocStaticInstance( const void* data, int bytes, nvrhi::ICommandList* commandList )
{
	if( staticData.instanceMemUsed.GetValue() + bytes > STATIC_VERTEX_MEMORY )
	{
		idLib::FatalError( "AllocStaticInstance failed, increase STATIC_VERTEX_MEMORY" );
	}
	return ActuallyAlloc( staticData, data, bytes, CACHE_INSTANCE, commandList );
}

/*
==============
idVertexCache::AllocStaticSkinnedVertex
==============
*/
vertCacheHandle_t idVertexCache::AllocStaticSkinnedVertex( const void* data, int bytes, nvrhi::ICommandList* commandList )
{
	if( staticData.skinnedMemUsed.GetValue() + bytes > STATIC_VERTEX_MEMORY )
	{
		idLib::FatalError( "AllocStaticSkinnedVertex failed, increase STATIC_VERTEX_MEMORY" );
	}
	return ActuallyAlloc( staticData, data, bytes, CACHE_SKINNED_VERTEX, commandList );
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
bool idVertexCache::GetJointBuffer( vertCacheHandle_t handle, idUniformBuffer* jb )
{
	const int isStatic = handle & VERTCACHE_STATIC;
	const uint64 numBytes = ( int )( handle >> VERTCACHE_SIZE_SHIFT ) & VERTCACHE_SIZE_MASK;
	const uint64 jointOffset = ( int )( handle >> VERTCACHE_OFFSET_SHIFT ) & VERTCACHE_OFFSET_MASK;
	const uint64 frameNum = ( int )( handle >> VERTCACHE_FRAME_SHIFT ) & VERTCACHE_FRAME_MASK;
	if( isStatic )
	{
		jb->Reference( staticData.jointBuffer, jointOffset, numBytes );
		return true;
	}
	if( frameNum != ( ( currentFrame - 1 ) & VERTCACHE_FRAME_MASK ) )
	{
		return false;
	}
	jb->Reference( frameData[ drawListNum ].jointBuffer, jointOffset, numBytes );
	return true;
}

bool idVertexCache::GetInstanceBuffer( vertCacheHandle_t handle, idVertexBuffer* vb )
{
	const int isStatic = handle & VERTCACHE_STATIC;
	const uint64 numBytes = ( int )( handle >> VERTCACHE_SIZE_SHIFT ) & VERTCACHE_SIZE_MASK;
	const uint64 offset = ( int )( handle >> VERTCACHE_OFFSET_SHIFT ) & VERTCACHE_OFFSET_MASK;
	const uint64 frameNum = ( int )( handle >> VERTCACHE_FRAME_SHIFT ) & VERTCACHE_FRAME_MASK;
	if( isStatic )
	{
		vb->Reference( staticData.instanceBuffer, offset, numBytes );
		return true;
	}
	if( frameNum != ( ( currentFrame - 1 ) & VERTCACHE_FRAME_MASK ) )
	{
		return false;
	}
	vb->Reference( frameData[drawListNum].instanceBuffer, offset, numBytes );
	return true;
}

/*
==============
idVertexCache::GetSkinnedVertexBuffer
==============
*/
bool idVertexCache::GetSkinnedVertexBuffer( vertCacheHandle_t handle, idVertexBuffer* vb )
{
	const int isStatic = handle & VERTCACHE_STATIC;
	const uint64 numBytes = ( int )( handle >> VERTCACHE_SIZE_SHIFT ) & VERTCACHE_SIZE_MASK;
	const uint64 offset = ( int )( handle >> VERTCACHE_OFFSET_SHIFT ) & VERTCACHE_OFFSET_MASK;
	const uint64 frameNum = ( int )( handle >> VERTCACHE_FRAME_SHIFT ) & VERTCACHE_FRAME_MASK;
	if( isStatic )
	{
		vb->Reference( staticData.skinnedBuffer, offset, numBytes );
		return true;
	}
	if( frameNum != ( ( currentFrame - 1 ) & VERTCACHE_FRAME_MASK ) )
	{
		return false;
	}
	vb->Reference( frameData[drawListNum].skinnedBuffer, offset, numBytes );
	return true;
}

/*
==============
idVertexCache::GetMaterialBuffer
==============
*/
bool idVertexCache::GetMaterialBuffer( vertCacheHandle_t handle, idUniformBuffer* mb )
{
	const int isStatic = handle & VERTCACHE_STATIC;
	const uint64 numBytes = ( int )( handle >> VERTCACHE_SIZE_SHIFT ) & VERTCACHE_SIZE_MASK;
	const uint64 offset = ( int )( handle >> VERTCACHE_OFFSET_SHIFT ) & VERTCACHE_OFFSET_MASK;
	const uint64 frameNum = ( int )( handle >> VERTCACHE_FRAME_SHIFT ) & VERTCACHE_FRAME_MASK;
	if( isStatic )
	{
		mb->Reference( staticData.materialBuffer, offset, numBytes );
		return true;
	}
	if( frameNum != ( ( currentFrame - 1 ) & VERTCACHE_FRAME_MASK ) )
	{
		return false;
	}
	mb->Reference( frameData[ drawListNum ].materialBuffer, offset, numBytes );
	return true;
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
	mostUsedJoint = Max( mostUsedJoint, frameData[ listNum ].jointMemUsed.GetValue() );
	mostUsedInstance = Max( mostUsedInstance, frameData[listNum].instanceMemUsed.GetValue() );

	if( r_showVertexCache.GetBool() )
	{
		idLib::Printf( "%08d: %d allocations, %dkB vertex, %dkB index, %ikB joint, instance %ikB: %dkB vertex, %dkB index, %ikB joint, %ikB instance\n",
					   currentFrame, frameData[ listNum ].allocations,
					   frameData[ listNum ].vertexMemUsed.GetValue() / 1024,
					   frameData[ listNum ].indexMemUsed.GetValue() / 1024,
					   frameData[ listNum ].jointMemUsed.GetValue() / 1024,
					   frameData[ listNum ].instanceMemUsed.GetValue() / 1024,
					   mostUsedVertex / 1024,
					   mostUsedIndex / 1024,
					   mostUsedJoint / 1024,
					   mostUsedInstance / 1024 );
	}

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

	ClearGeoBufferSet( frameData[ listNum ] );
}

