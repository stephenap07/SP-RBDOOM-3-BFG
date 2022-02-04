#include "precompiled.h"
#pragma hdrstop

#include "BindingCache.h"

void BindingCache::Init( nvrhi::IDevice* _device )
{
    device = _device;
}

nvrhi::BindingSetHandle BindingCache::GetCachedBindingSet(const nvrhi::BindingSetDesc& desc, nvrhi::IBindingLayout* layout)
{
    size_t hash = 0;
    nvrhi::hash_combine(hash, desc);
    nvrhi::hash_combine(hash, layout);

    mutex.Lock( );

    nvrhi::BindingSetHandle result = nullptr;
    for( int i = bindingHash.First( hash ); i != -1; i = bindingHash.Next( i ) )
    {
        nvrhi::BindingSetHandle bindingSet = bindingSets[i];
        if( *bindingSet->getDesc() == desc)
        {
            result = bindingSet;
            break;
        }
    }

    mutex.Unlock( );

    if (result)
    {
        assert(result->getDesc() && *result->getDesc( ) == desc );
    }

    return result;
}

nvrhi::BindingSetHandle BindingCache::GetOrCreateBindingSet(const nvrhi::BindingSetDesc& desc, nvrhi::IBindingLayout* layout)
{
    size_t hash = 0;
    nvrhi::hash_combine(hash, desc);
    nvrhi::hash_combine(hash, layout);

    mutex.Lock( );

    nvrhi::BindingSetHandle result = nullptr;
    for( int i = bindingHash.First( hash ); i != -1; i = bindingHash.Next( i ) )
    {
        nvrhi::BindingSetHandle bindingSet = bindingSets[i];
        if( *bindingSet->getDesc( ) == desc )
        {
            result = bindingSet;
            break;
        }
    }
    
    mutex.Unlock( );

    if (!result)
    {
        mutex.Lock( );

        int entryIndex = bindingSets.Append( result );
        bindingHash.Add( hash, entryIndex );

        nvrhi::BindingSetHandle& entry = bindingSets[entryIndex];

        if( !entry )
        {
            result = device->createBindingSet(desc, layout);
            entry = result;
        }
        else
        {
            result = entry;
        }

        mutex.Unlock( );
    }

    if (result)
    {
        assert(result->getDesc() && *result->getDesc( ) == desc );
    }

    return result;
}

void BindingCache::Clear()
{
    mutex.Lock( );
    bindingSets.Clear( );
    bindingHash.Clear( );
    mutex.Unlock( );
}