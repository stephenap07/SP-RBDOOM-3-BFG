#ifndef RENDERER_PASSES_MIPMAPGENPASS_H_
#define RENDERER_PASSES_MIPMAPGENPASS_H_

class CommonRenderPasses;

class MipMapGenPass {

public:

    enum Mode : uint8_t {
        MODE_COLOR = 0,  // bilinear reduction of RGB channels
        MODE_MIN = 1,    // min() reduction of R channel
        MODE_MAX = 2,    // max() reduction of R channel
        MODE_MINMAX = 3, // min() and max() reductions of R channel into RG channels
    };

    // note : 'texture' must have been allocated with some mip levels
    MipMapGenPass(
        nvrhi::IDevice* device,
        nvrhi::TextureHandle texture,
        Mode mode = Mode::MODE_MAX);

    // Dispatches reduction kernel : reads LOD 0 and populates
    // LOD 1 and up
    void Dispatch(nvrhi::ICommandList* commandList, int maxLOD=-1);

    // debug : blits mip-map levels in spiral pattern to 'target'
    // (assumes 'target' texture resolution is high enough...)
    void Display(
        CommonRenderPasses& commonPasses,
        nvrhi::ICommandList* commandList, 
        nvrhi::IFramebuffer* target);

private:

    nvrhi::DeviceHandle m_Device;
    nvrhi::ShaderHandle m_Shader;
    nvrhi::TextureHandle m_Texture;
    nvrhi::BufferHandle m_ConstantBuffer;
    nvrhi::BindingLayoutHandle m_BindingLayout;
    idList<nvrhi::BindingSetHandle> m_BindingSets;
    nvrhi::ComputePipelineHandle m_Pso;

    // Set of unique dummy textures - see details in class implementation
    struct NullTextures;
    std::shared_ptr<NullTextures> m_NullTextures;

    BindingCache m_BindingCache;
};

#endif