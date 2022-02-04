#ifndef RENDERER_BINDING_CACHE_H_
#define RENDERER_BINDING_CACHE_H_

#include <nvrhi/nvrhi.h>

class BindingCache
{
public:
    BindingCache( ) {}

    void                    Init( nvrhi::IDevice* _device );
    void                    Clear( );

    nvrhi::BindingSetHandle GetCachedBindingSet(const nvrhi::BindingSetDesc& desc, nvrhi::IBindingLayout* layout);
    nvrhi::BindingSetHandle GetOrCreateBindingSet(const nvrhi::BindingSetDesc& desc, nvrhi::IBindingLayout* layout);

private:
    nvrhi::IDevice*                 device;
    idList<nvrhi::BindingSetHandle> bindingSets;
    idHashIndex						bindingHash;
    idSysMutex                      mutex;
};

#endif