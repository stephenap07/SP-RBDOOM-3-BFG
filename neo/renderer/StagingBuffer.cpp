#include "precompiled.h"
#pragma hdrstop

#include "VertexCache.h"
#include "StagingBuffer.h"

idStagingBuffer::idStagingBuffer( nvrhi::DeviceHandle device, nvrhi::BufferDesc desc )
	: buffer( nullptr )
	, device( device )
	, mappedBase( nullptr )
	, memUsed()
	, numAllocs()
	, allocations( 0 )
{
	release_assert( desc.byteSize > 0 );
	release_assert( !desc.canHaveUAVs );
	release_assert( desc.cpuAccess == nvrhi::CpuAccessMode::Write );

	buffer = device->createBuffer( desc );

	MapBuffer();

	size_t maxAllocs = 16 << 12;
	if( desc.structStride > 0 )
	{
		maxAllocs = desc.byteSize / desc.structStride;
	}

	// we can make upto 2^16 allocations/updates per frame or
	// however many structs fit in the allocated memory.
	allocations = new vertCacheHandle_t[maxAllocs];
}

idStagingBuffer::~idStagingBuffer()
{
	buffer.Reset();
	delete allocations;
}

vertCacheHandle_t idStagingBuffer::Alloc( const void* data, size_t numBytes, vertCacheHandle_t inHandle )
{
	if( numBytes == 0 )
	{
		return ( vertCacheHandle_t )0;
	}

	assert_16_byte_aligned( data );
	assert( ( numBytes & 15 ) == 0 );

	int endPos = 0;
	int offset = 0;
	int allocPos = 0;

	endPos = memUsed.Add( numBytes );
	allocPos = numAllocs.Add( 1 );

	if( endPos > buffer->getDesc().byteSize )
	{
		idLib::Error( "Out of staging cache" );
	}

	offset = endPos - numBytes;

	if( data != nullptr )
	{
		CopyBuffer( mappedBase + offset, ( const byte* )data, numBytes );
	}

	allocations[allocPos - 1] = inHandle;

	return inHandle;
}

void idStagingBuffer::Clear()
{
	memUsed.SetValue( 0 );
	numAllocs.SetValue( 0 );
}

void idStagingBuffer::CopyBuffers( nvrhi::ICommandList* commandList, nvrhi::IBuffer* destBuffer )
{
	// TODO: Is it better to make smaller copies or larger ones? I'm only hesitent because
	// a pair of barriers are inserted for each copy. Theoretically, you could protect
	// resources for asynchronous use with these barriers but at the same time, could it
	// just be wasting time synchronizing data?
	uint64 srcOffset = 0;
	for( int i = 0; i < numAllocs.GetValue(); i++ )
	{
		vertCacheHandle_t handle = allocations[ i ];
		const uint64 size = ( int )( handle >> VERTCACHE_SIZE_SHIFT ) & VERTCACHE_SIZE_MASK;
		const uint64 offset = ( int )( handle >> VERTCACHE_OFFSET_SHIFT ) & VERTCACHE_OFFSET_MASK;
		commandList->copyBuffer( destBuffer, offset, buffer, srcOffset, size );
		srcOffset += size;
	}
}

void idStagingBuffer::Update( vertCacheHandle_t handle, void* data )
{
	const uint64 size = ( int )( handle >> VERTCACHE_SIZE_SHIFT ) & VERTCACHE_SIZE_MASK;
	const uint64 offset = ( int )( handle >> VERTCACHE_OFFSET_SHIFT ) & VERTCACHE_OFFSET_MASK;

	assert( size < buffer->getDesc().byteSize );

	if( data != nullptr )
	{
		CopyBuffer( mappedBase + offset, ( const byte* )data, size );
	}
}

void* idStagingBuffer::MapBuffer()
{
	assert( buffer );
	assert( !mappedBase );

	mappedBase = ( byte* )device->mapBuffer( buffer, nvrhi::CpuAccessMode::Write );

	if( mappedBase == nullptr )
	{
		idLib::FatalError( "idStagingBuffer::MapBuffer: failed" );
	}

	return buffer;
}
