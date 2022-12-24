/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
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

#ifndef RENDERER_STAGINGBUFFER_H_
#define RENDERER_STAGINGBUFFER_H_

// A linear allocator upload buffer. Used in the vertex cache
// to provide a thread-safe way of mapping data to the gpu.
// Use ICommandList::copyBuffer to copy this data to another buffer
// when you're done updating.
class idStagingBuffer
{
public:

	constexpr static int MAPPED_FLAG = 1 << ( 4 /* sizeof( int ) */ * 8 - 1 );

	explicit idStagingBuffer( nvrhi::DeviceHandle device, nvrhi::BufferDesc desc );
	~idStagingBuffer();

	vertCacheHandle_t	Alloc( const void* data, size_t numBytes, vertCacheHandle_t inHandle );
	void				Clear();
	void				CopyBuffers( nvrhi::ICommandList* commandList, nvrhi::IBuffer* destBuffer );
	void				Update( vertCacheHandle_t handle, void* data );

	nvrhi::BufferHandle Buffer()
	{
		return buffer;
	}

public:

	idStagingBuffer( const idStagingBuffer& )				= delete;
	idStagingBuffer& operator=( const idStagingBuffer& )	= delete;

private:

	void*					MapBuffer();

	nvrhi::BufferHandle		buffer;
	nvrhi::DeviceHandle		device;
	byte*					mappedBase;
	idSysInterlockedInteger memUsed;
	idSysInterlockedInteger numAllocs;
	vertCacheHandle_t*		allocations;
};

#endif