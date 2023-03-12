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

#include "precompiled.h"
#pragma hdrstop


DescriptorHandle::DescriptorHandle()
	: manager( nullptr )
	, index( -1 )
{	
}

DescriptorHandle::DescriptorHandle( DescriptorTableManager* managerPtr, DescriptorIndex index )
	: manager( managerPtr )
	, index( index )
{
}

void DescriptorHandle::Free()
{
	if( index >= 0 )
	{
		assert( manager != nullptr );
		manager->ReleaseDescriptor( index );
		Reset();
	}
}

DescriptorTableManager::DescriptorTableManager( nvrhi::IDevice* device, nvrhi::IBindingLayout* layout )
	: device( device )
	, searchStart( 0 )
{
	descriptorTable = device->createDescriptorTable( layout );

	size_t capacity = descriptorTable->getCapacity();
	allocatedDescriptors.SetNum( capacity );
	memset( allocatedDescriptors.Ptr(), 0, sizeof( bool ) * capacity );
	descriptors.SetNum( capacity );
	memset( descriptors.Ptr(), 0, sizeof( nvrhi::BindingSetItem ) * capacity );
}

DescriptorTableManager::~DescriptorTableManager()
{
	for( auto& descriptor : descriptors )
	{
		if( descriptor.resourceHandle )
		{
			descriptor.resourceHandle->Release();
			descriptor.resourceHandle = nullptr;
		}
	}

	descriptors.Clear();
}

DescriptorIndex DescriptorTableManager::CreateDescriptor( nvrhi::BindingSetItem item )
{
	auto hash = BindingSetItemHasher( item );
	for( int i = bindingHash.First( hash ); i != -1; i = bindingHash.Next( i ) )
	{
		const int index = descriptorIndexes[i];
		if( descriptors[index] == item )
		{
			return index;
		}
	}

	uint32_t capacity = descriptorTable->getCapacity();
	bool foundFreeSlot = false;
	uint32_t index = 0;
	for( index = searchStart; index < capacity; index++ )
	{
		if( !allocatedDescriptors[index] )
		{
			foundFreeSlot = true;
			break;
		}
	}

	if( !foundFreeSlot )
	{
		uint32_t newCapacity = Max( 64u, capacity * 2 ); // handle the initial case when capacity == 0
		device->resizeDescriptorTable( descriptorTable, newCapacity );
		allocatedDescriptors.SetNum( newCapacity );
		memset( &allocatedDescriptors[capacity], 0, sizeof( bool ) * ( newCapacity - capacity ) );

		descriptors.SetNum( newCapacity );
		memset( &descriptors[ capacity ], 0, sizeof( nvrhi::BindingSetItem ) * ( newCapacity - capacity ) );

		index = capacity;
		capacity = newCapacity;
	}

	item.slot = index;
	searchStart = index + 1;
	allocatedDescriptors[index] = true;
	descriptors[index] = item;
	bindingHash.Add( hash, descriptorIndexes.Append( index ) );
	device->writeDescriptorTable( descriptorTable, item );

	if( item.resourceHandle )
	{
		item.resourceHandle->AddRef();
	}

	return index;
}

DescriptorHandle DescriptorTableManager::CreateDescriptorHandle( nvrhi::BindingSetItem item )
{
	DescriptorIndex index = CreateDescriptor( item );
	return DescriptorHandle( this, index );
}

nvrhi::BindingSetItem DescriptorTableManager::GetDescriptor( DescriptorIndex index )
{
	if( size_t( index ) >= descriptors.Size() )
	{
		return nvrhi::BindingSetItem::None( 0 );
	}
	return descriptors[index];
}

void DescriptorTableManager::ReleaseDescriptor( DescriptorIndex index )
{
	nvrhi::BindingSetItem& descriptor = descriptors[index];
	if( descriptor.resourceHandle )
	{
		descriptor.resourceHandle->Release();
		descriptor.resourceHandle = nullptr;
	}

	bindingHash.Remove( BindingSetItemHasher( descriptor ), index );

	descriptor = nvrhi::BindingSetItem::None( index );

	device->writeDescriptorTable( descriptorTable, descriptor );

	allocatedDescriptors[index] = false;
	searchStart = Min( searchStart, index );
}