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

#ifndef RENDERER_DESCRIPTORTABLEMANAGER_H_
#define RENDERER_DESCRIPTORTABLEMANAGER_H_

class DescriptorTableManager;
typedef int DescriptorIndex;

// Stores a descriptor index in a descriptor table. Releases the descriptor when destroyed.
class DescriptorHandle
{
public:
	DescriptorHandle();
	DescriptorHandle( DescriptorTableManager* managerPtr, DescriptorIndex index );

	void Free();

	[[nodiscard]] bool IsValid() const
	{
		return index >= 0;
	}
	[[nodiscard]] DescriptorIndex Get() const
	{
		if( index >= 0 )
		{
			assert( manager != nullptr );
		}
		return index;
	}
	void Reset()
	{
		index = -1;
		manager = nullptr;
	}

private:

	DescriptorTableManager*		manager;
	DescriptorIndex				index;

};

class DescriptorTableManager
{
public:

	DescriptorTableManager( nvrhi::IDevice* device, nvrhi::IBindingLayout* layout );
	~DescriptorTableManager();

	nvrhi::IDescriptorTable*        GetDescriptorTable() const
	{
		return descriptorTable;
	}

	DescriptorIndex                 CreateDescriptor( nvrhi::BindingSetItem item );
	DescriptorHandle                CreateDescriptorHandle( nvrhi::BindingSetItem item );
	nvrhi::BindingSetItem           GetDescriptor( DescriptorIndex index );
	void                            ReleaseDescriptor( DescriptorIndex index );

protected:
	size_t BindingSetItemHasher( const nvrhi::BindingSetItem& item ) const
	{
		size_t hash = 0;
		nvrhi::hash_combine( hash, item.resourceHandle );
		nvrhi::hash_combine( hash, item.type );
		nvrhi::hash_combine( hash, item.format );
		nvrhi::hash_combine( hash, item.dimension );
		nvrhi::hash_combine( hash, item.rawData[ 0 ] );
		nvrhi::hash_combine( hash, item.rawData[ 1 ] );
		return hash;
	}

	// Custom equality tester that doesn't look at the binding slot

	bool BindingSetItemsEqual( const nvrhi::BindingSetItem& a, const nvrhi::BindingSetItem& b ) const
	{
		return a.resourceHandle == b.resourceHandle
			   && a.type == b.type
			   && a.format == b.format
			   && a.dimension == b.dimension
			   && a.subresources == b.subresources;
	}

	const DescriptorIndex* FindDescriptorIndex( size_t hash, nvrhi::BindingSetItem item ) const;

	nvrhi::DeviceHandle             device;
	nvrhi::DescriptorTableHandle    descriptorTable;

	idList<nvrhi::BindingSetItem>   descriptors;
	idList<DescriptorIndex>         descriptorIndexes;
	idList<bool>                    allocatedDescriptors;
	idHashIndex					    bindingHash;

	int                             searchStart;
};

#endif