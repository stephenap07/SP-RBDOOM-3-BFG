/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013 Robert Beckebans
Copyright (C) 2016-2017 Dustin Land

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
#include "../RenderCommon.h"

#include "sys/DeviceManager.h"

#include <stddef.h>

extern idCVar r_showBuffers;

//static const GLenum bufferUsage = GL_STATIC_DRAW;
static const GLenum bufferUsage = GL_DYNAMIC_DRAW;

extern DeviceManager* deviceManager;
extern nvrhi::CommandListHandle vcCommandList;

/*
================================================================================================

	Buffer Objects

================================================================================================
*/

/*
========================
UnbindBufferObjects
========================
*/
void UnbindBufferObjects()
{
	//glBindBuffer( GL_ARRAY_BUFFER, 0 );
	//glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}



/*
================================================================================================

idVertexBuffer

================================================================================================
*/

/*
========================
idVertexBuffer::idVertexBuffer
========================
*/
idVertexBuffer::idVertexBuffer()
{
	size = 0;
	offsetInOtherBuffer = OWNS_BUFFER_FLAG;
	bufferHandle.Reset( );
	SetUnmapped();
}

/*
========================
idVertexBuffer::AllocBufferObject
========================
*/
bool idVertexBuffer::AllocBufferObject( const void* data, int allocSize, bufferUsageType_t _usage )
{
	assert( !bufferHandle );
	assert_16_byte_aligned( data );

	if( allocSize <= 0 )
	{
		idLib::Error( "idVertexBuffer::AllocBufferObject: allocSize = %i", allocSize );
	}

	size = allocSize;
	usage = _usage;

	bool allocationFailed = false;

	int numBytes = GetAllocedSize();

	nvrhi::VertexAttributeDesc attributes[] = {
	nvrhi::VertexAttributeDesc( )
		.setName( "POSITION" )
		.setFormat( nvrhi::Format::RGB32_FLOAT )
		.setOffset( offsetof( idDrawVert, xyz ) )
		.setElementStride( sizeof( idDrawVert ) ),
	nvrhi::VertexAttributeDesc( )
		.setName( "NORMAL" )
		.setFormat( nvrhi::Format::RGBA8_UINT )
		.setOffset( offsetof( idDrawVert, st ) )
		.setElementStride( sizeof( idDrawVert ) ),
	nvrhi::VertexAttributeDesc( )
		.setName( "COLOR" )
		.setFormat( nvrhi::Format::RGBA8_UINT )
		.setOffset( offsetof( idDrawVert, st ) )
		.setElementStride( sizeof( idDrawVert ) ),
	nvrhi::VertexAttributeDesc( )
		.setName( "COLOR2" )
		.setFormat( nvrhi::Format::RGBA8_UINT )
		.setOffset( offsetof( idDrawVert, st ) )
		.setElementStride( sizeof( idDrawVert ) ),
	nvrhi::VertexAttributeDesc( )
		.setName( "ST" )
		.setFormat( nvrhi::Format::RG16_UINT )
		.setOffset( offsetof( idDrawVert, st ) )
		.setElementStride( sizeof( idDrawVert ) ),
	nvrhi::VertexAttributeDesc( )
		.setName( "TANGENT" )
		.setFormat( nvrhi::Format::RGBA8_UINT )
		.setOffset( offsetof( idDrawVert, st ) )
		.setElementStride( sizeof( idDrawVert ) ),
	};

	nvrhi::BufferDesc vertexBufferDesc;
	vertexBufferDesc.byteSize = numBytes;
	vertexBufferDesc.isVertexBuffer = true;
	vertexBufferDesc.debugName = "VertexBuffer";
	vertexBufferDesc.initialState = nvrhi::ResourceStates::CopyDest;
	vertexBufferDesc.cpuAccess = nvrhi::CpuAccessMode::Write;
	bufferHandle = deviceManager->GetDevice( )->createBuffer( vertexBufferDesc );

	if( r_showBuffers.GetBool( ) )
	{
		idLib::Printf( "vertex buffer alloc %p, api %p (%i bytes)\n", this, bufferHandle.Get(), GetSize( ) );
	}

	// copy the data
	if( data != NULL )
	{
		Update( data, allocSize );
	}

	return !allocationFailed;
}

/*
========================
idVertexBuffer::FreeBufferObject
========================
*/
void idVertexBuffer::FreeBufferObject()
{
	if( IsMapped() )
	{
		UnmapBuffer();
	}

	// if this is a sub-allocation inside a larger buffer, don't actually free anything.
	if( OwnsBuffer() == false )
	{
		ClearWithoutFreeing();
		return;
	}

	if( !bufferHandle )
	{
		return;
	}

	if( r_showBuffers.GetBool() )
	{
		idLib::Printf( "vertex buffer free %p, api %p (%i bytes)\n", this, bufferHandle.Get(), GetSize( ) );
	}

	bufferHandle.Reset( );

	ClearWithoutFreeing();
}

/*
========================
idVertexBuffer::Update
========================
*/
void idVertexBuffer::Update( const void* data, int updateSize, int offset ) const
{
	assert( bufferHandle );
	assert_16_byte_aligned( data );
	assert( ( GetOffset() & 15 ) == 0 );

	if( updateSize > GetSize() )
	{
		idLib::FatalError( "idVertexBuffer::Update: size overrun, %i > %i\n", updateSize, GetSize() );
	}

	int numBytes = ( updateSize + 15 ) & ~15;

	if( usage == BU_DYNAMIC )
	{
		CopyBuffer( ( byte* )buffer + offset, ( const byte* )data, numBytes );
	}
	else
	{
		vcCommandList->beginTrackingBufferState( bufferHandle, nvrhi::ResourceStates::CopyDest );
		vcCommandList->writeBuffer( bufferHandle, data, numBytes, GetOffset() + offset );
		vcCommandList->setPermanentBufferState( bufferHandle, nvrhi::ResourceStates::VertexBuffer );
	}
}

/*
========================
idVertexBuffer::MapBuffer
========================
*/
void* idVertexBuffer::MapBuffer( bufferMapType_t mapType )
{
	assert( bufferHandle );
	assert( IsMapped( ) == false );

	nvrhi::CpuAccessMode accessMode = nvrhi::CpuAccessMode::Write;
	if( mapType == bufferMapType_t::BM_READ )
	{
		accessMode = nvrhi::CpuAccessMode::Read;
	}

	buffer = deviceManager->GetDevice( )->mapBuffer( bufferHandle, accessMode );

	SetMapped( );

	if( buffer == NULL )
	{
		idLib::FatalError( "idVertexBuffer::MapBuffer: failed" );
	}

	return buffer;
}

/*
========================
idVertexBuffer::UnmapBuffer
========================
*/
void idVertexBuffer::UnmapBuffer()
{
	assert( bufferHandle );
	assert( IsMapped( ) );

	deviceManager->GetDevice( )->unmapBuffer( bufferHandle );

	SetUnmapped( );
}

/*
========================
idVertexBuffer::ClearWithoutFreeing
========================
*/
void idVertexBuffer::ClearWithoutFreeing()
{
	size = 0;
	offsetInOtherBuffer = OWNS_BUFFER_FLAG;
	bufferHandle.Reset( );
}

/*
================================================================================================

idIndexBuffer

================================================================================================
*/

/*
========================
idIndexBuffer::idIndexBuffer
========================
*/
idIndexBuffer::idIndexBuffer()
{
	size = 0;
	offsetInOtherBuffer = OWNS_BUFFER_FLAG;
	bufferHandle.Reset();
	SetUnmapped( );
}

/*
========================
idIndexBuffer::AllocBufferObject
========================
*/
bool idIndexBuffer::AllocBufferObject( const void* data, int allocSize, bufferUsageType_t _usage )
{
	assert( !bufferHandle );
	assert_16_byte_aligned( data );

	if( allocSize <= 0 )
	{
		idLib::Error( "idIndexBuffer::AllocBufferObject: allocSize = %i", allocSize );
	}

	size = allocSize;
	usage = _usage;

	bool allocationFailed = false;

	int numBytes = GetAllocedSize( );

	nvrhi::BufferDesc indexBufferDesc;
	indexBufferDesc.byteSize = numBytes;
	indexBufferDesc.isIndexBuffer = true;
	indexBufferDesc.debugName = "IndexBuffer";
	indexBufferDesc.initialState = nvrhi::ResourceStates::CopyDest;
	indexBufferDesc.cpuAccess = nvrhi::CpuAccessMode::Write;

	bufferHandle  = deviceManager->GetDevice( )->createBuffer( indexBufferDesc );

	// copy the data
	if( data != NULL )
	{
		Update( data, allocSize );
	}

	return !allocationFailed;
}

/*
========================
idIndexBuffer::FreeBufferObject
========================
*/
void idIndexBuffer::FreeBufferObject()
{
	if( IsMapped( ) )
	{
		UnmapBuffer( );
	}

	// if this is a sub-allocation inside a larger buffer, don't actually free anything.
	if( OwnsBuffer( ) == false )
	{
		ClearWithoutFreeing( );
		return;
	}

	if( !bufferHandle )
	{
		return;
	}

	if( r_showBuffers.GetBool( ) )
	{
		idLib::Printf( "index buffer free %p, api %p (%i bytes)\n", this, bufferHandle.Get(), GetSize( ) );
	}

	bufferHandle.Reset( );

	ClearWithoutFreeing( );
}

/*
========================
idIndexBuffer::Update
========================
*/
void idIndexBuffer::Update( const void* data, int updateSize, int offset ) const
{
	assert( bufferHandle );
	assert_16_byte_aligned( data );
	assert( ( GetOffset( ) & 15 ) == 0 );

	if( updateSize > GetSize( ) )
	{
		idLib::FatalError( "idIndexBuffer::Update: size overrun, %i > %i\n", updateSize, GetSize( ) );
	}

	int numBytes = ( updateSize + 15 ) & ~15;

	if( usage == BU_DYNAMIC )
	{
		void* buffer = deviceManager->GetDevice( )->mapBuffer( bufferHandle, nvrhi::CpuAccessMode::Write );
		CopyBuffer( ( byte* )buffer + offset, ( const byte* )data, numBytes );
	}
	else
	{
		vcCommandList->beginTrackingBufferState( bufferHandle, nvrhi::ResourceStates::CopyDest );
		vcCommandList->writeBuffer( bufferHandle, data, numBytes, GetOffset( ) + offset );
		vcCommandList->setPermanentBufferState( bufferHandle, nvrhi::ResourceStates::IndexBuffer );
	}
}

/*
========================
idIndexBuffer::MapBuffer
========================
*/
void* idIndexBuffer::MapBuffer( bufferMapType_t mapType )
{
	assert( bufferHandle );
	assert( IsMapped( ) == false );

	nvrhi::CpuAccessMode accessMode = nvrhi::CpuAccessMode::Write;
	if( mapType == bufferMapType_t::BM_READ )
	{
		accessMode = nvrhi::CpuAccessMode::Read;
	}

	buffer = deviceManager->GetDevice( )->mapBuffer( bufferHandle, accessMode );

	SetMapped( );

	if( buffer == NULL )
	{
		idLib::FatalError( "idVertexBuffer::MapBuffer: failed" );
	}

	return buffer;
}

/*
========================
idIndexBuffer::UnmapBuffer
========================
*/
void idIndexBuffer::UnmapBuffer()
{
	assert( bufferHandle );
	assert( IsMapped( ) );

	deviceManager->GetDevice( )->unmapBuffer( bufferHandle );

	SetUnmapped( );
}

/*
========================
idIndexBuffer::ClearWithoutFreeing
========================
*/
void idIndexBuffer::ClearWithoutFreeing()
{
	size = 0;
	offsetInOtherBuffer = OWNS_BUFFER_FLAG;
	bufferHandle.Reset( );
}

/*
================================================================================================

idUniformBuffer

================================================================================================
*/

/*
========================
idUniformBuffer::idUniformBuffer
========================
*/
idUniformBuffer::idUniformBuffer()
{
	size = 0;
	offsetInOtherBuffer = OWNS_BUFFER_FLAG;
	bufferHandle.Reset();
	SetUnmapped();
}

/*
========================
idUniformBuffer::AllocBufferObject
========================
*/
bool idUniformBuffer::AllocBufferObject( const void* data, int allocSize, bufferUsageType_t _usage )
{
	assert( !bufferHandle );
	assert_16_byte_aligned( data );

	if( allocSize <= 0 )
	{
		idLib::Error( "idIndexBuffer::AllocBufferObject: allocSize = %i", allocSize );
	}

	size = allocSize;
	usage = _usage;

	bool allocationFailed = false;

	int numBytes = GetAllocedSize( );

	nvrhi::BufferDesc indexBufferDesc;
	indexBufferDesc.byteSize = numBytes;
	indexBufferDesc.isConstantBuffer = true;
	indexBufferDesc.debugName = "ConstantBuffer";
	indexBufferDesc.initialState = nvrhi::ResourceStates::CopyDest;
	indexBufferDesc.cpuAccess = nvrhi::CpuAccessMode::Write;

	bufferHandle = deviceManager->GetDevice( )->createBuffer( indexBufferDesc );

	// copy the data
	if( data != NULL )
	{
		Update( data, allocSize );
	}

	return !allocationFailed;
}

/*
========================
idUniformBuffer::FreeBufferObject
========================
*/
void idUniformBuffer::FreeBufferObject()
{
}

/*
========================
idUniformBuffer::Update
========================
*/
void idUniformBuffer::Update( const void* data, int updateSize, int offset ) const
{
	assert( bufferHandle );
	assert_16_byte_aligned( data );
	assert( ( GetOffset( ) & 15 ) == 0 );

	if( updateSize > GetSize( ) )
	{
		idLib::FatalError( "idIndexBuffer::Update: size overrun, %i > %i\n", updateSize, GetSize( ) );
	}

	int numBytes = ( updateSize + 15 ) & ~15;

	if( usage == BU_DYNAMIC )
	{
		void* buffer = deviceManager->GetDevice( )->mapBuffer( bufferHandle, nvrhi::CpuAccessMode::Write );
		CopyBuffer( ( byte* )buffer + offset, ( const byte* )data, numBytes );
	}
	else
	{
		vcCommandList->beginTrackingBufferState( bufferHandle, nvrhi::ResourceStates::CopyDest );
		vcCommandList->writeBuffer( bufferHandle, data, numBytes, GetOffset( ) + offset );
		vcCommandList->setPermanentBufferState( bufferHandle, nvrhi::ResourceStates::ConstantBuffer );
	}
}

/*
========================
idUniformBuffer::MapBuffer
========================
*/
void* idUniformBuffer::MapBuffer( bufferMapType_t mapType )
{
	assert( bufferHandle );
	assert( IsMapped( ) == false );

	nvrhi::CpuAccessMode accessMode = nvrhi::CpuAccessMode::Write;
	if( mapType == bufferMapType_t::BM_READ )
	{
		accessMode = nvrhi::CpuAccessMode::Read;
	}

	buffer = deviceManager->GetDevice( )->mapBuffer( bufferHandle, accessMode );

	SetMapped( );

	if( buffer == NULL )
	{
		idLib::FatalError( "idUniformBuffer::MapBuffer: failed" );
	}

	return buffer;
}

/*
========================
idUniformBuffer::UnmapBuffer
========================
*/
void idUniformBuffer::UnmapBuffer()
{
	assert( bufferHandle );
	assert( IsMapped( ) );

	deviceManager->GetDevice( )->unmapBuffer( bufferHandle );

	SetUnmapped( );
}

/*
========================
idUniformBuffer::ClearWithoutFreeing
========================
*/
void idUniformBuffer::ClearWithoutFreeing()
{
	size = 0;
	offsetInOtherBuffer = OWNS_BUFFER_FLAG;
	bufferHandle.Reset( );
}
