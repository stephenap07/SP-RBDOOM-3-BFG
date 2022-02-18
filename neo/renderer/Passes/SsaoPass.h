#ifndef RENDERER_PASSES_SSAOPASS_H_
#define RENDERER_PASSES_SSAOPASS_H_

struct SsaoParameters
{
    float amount = 2.f;
    float backgroundViewDepth = 100.f;
    float radiusWorld = 0.5f;
    float surfaceBias = 0.1f;
    float powerExponent = 2.f;
    bool enableBlur = true;
    float blurSharpness = 16.f;
};

class SsaoPass
{
private:
    struct SubPass
    {
        nvrhi::ShaderHandle Shader;
        nvrhi::BindingLayoutHandle BindingLayout;
        std::vector<nvrhi::BindingSetHandle> BindingSets;
        nvrhi::ComputePipelineHandle Pipeline;
    };

    SubPass m_Deinterleave;
    SubPass m_Compute;
    SubPass m_Blur;

    nvrhi::DeviceHandle m_Device;
    nvrhi::BufferHandle m_ConstantBuffer;
    CommonRenderPasses* commonRenderPasses;

    nvrhi::TextureHandle m_DeinterleavedDepth;
    nvrhi::TextureHandle m_DeinterleavedOcclusion;
    idVec2 m_QuantizedGbufferTextureSize;

public:
    struct CreateParameters
    {
        idVec2 dimensions;
        bool inputLinearDepth = false;
        bool octEncodedNormals = false;
        bool directionalOcclusion = false;
        int numBindingSets = 1;
    };

    SsaoPass(
        nvrhi::IDevice* device,
        const CreateParameters& params,
        CommonRenderPasses* commonRenderPasses);

    SsaoPass(
        nvrhi::IDevice* device,
        CommonRenderPasses* commonPasses,
        nvrhi::ITexture* gbufferDepth,
        nvrhi::ITexture* gbufferNormals,
        nvrhi::ITexture* destinationTexture );

    void CreateBindingSet(
        nvrhi::ITexture* gbufferDepth,
        nvrhi::ITexture* gbufferNormals,
        nvrhi::ITexture* destinationTexture,
        int bindingSetIndex = 0);

    void Render(
        nvrhi::ICommandList* commandList,
        const SsaoParameters& params,
        viewDef_t* viewDef,
        int bindingSetIndex = 0);
};

#endif