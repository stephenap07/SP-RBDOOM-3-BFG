#ifndef RENDERER_PASSES_GEOMETRYPASSES_H_
#define RENDERER_PASSES_GEOMETRYPASSES_H_

constexpr std::size_t MAX_IMAGE_PARMS = 16;

class IGeometryPass
{
public:
    virtual         ~IGeometryPass( ) = default;

    virtual void    SetupView(nvrhi::ICommandList* commandList, viewDef_t* viewDef ) = 0;
    virtual bool    SetupMaterial(const idMaterial* material, nvrhi::RasterCullMode cullMode, nvrhi::GraphicsState& state) = 0;
    virtual void    SetupInputBuffers(const drawSurf_t* drawSurf, nvrhi::GraphicsState& state) = 0;
    virtual void    SetPushConstants(nvrhi::ICommandList* commandList, nvrhi::GraphicsState& state, nvrhi::DrawArguments& args) = 0;

protected:

    void PrepareStageTexturing( const shaderStage_t* stage, const drawSurf_t* surf );
    void FinishStageTexturing( const shaderStage_t* stage, const drawSurf_t* surf );

protected:

    int										currentImageParm = 0;
    idArray< idImage*, MAX_IMAGE_PARMS >	imageParms;
    uint64                                  glStateBits;
    nvrhi::GraphicsPipelineDesc             pipelineDesc;
    nvrhi::GraphicsPipelineHandle			pipeline;
    BindingCache				        	bindingCache;
    Framebuffer*                            previousFramebuffer;
    Framebuffer*                            currentFramebuffer;
    const viewDef_t*                        viewDef;
    const viewEntity_t*                     currentSpace;
    idScreenRect					        currentViewport;
    idScreenRect					        currentScissor;
    idVec4                                  clearColor;
    float                                   depthClearValue;
    byte                                    stencilClearValue;

    // Updates state to bits in stateBits. Only updates the different bits.
    bool GL_State( uint64 stateBits, bool forceGlState = false );
    void GL_SelectTexture( int textureNum );
    void GL_BindTexture( idImage* img );
    void GL_BindFramebuffer( Framebuffer* framebuffer );
    void GL_BindGraphicsShader( int shader );
    void GL_DepthBoundsTest( const float zmin, const float zmax );
    void GL_PolygonOffset( float scale, float bias );
    void GL_Viewport( int x, int y, int w, int h );
    void GL_Scissor( int x, int y, int w, int h );
    void GL_Color( const idVec4 color );
    void GL_ClearColor( const idVec4 color );
    void GL_ClearDepthStencilValue( float depthValue, byte stencilValue = 0xF );
    void GL_ClearColor( nvrhi::ICommandList* commandList, int attachmentIndex = 0 );
    void GL_ClearDepthStencil( nvrhi::ICommandList* commandList );

    ID_INLINE uint64 GL_GetCurrentState( ) const
    {
        return glStateBits;
    }

    ID_INLINE void	GL_ViewportAndScissor( int x, int y, int w, int h )
    {
        GL_Viewport( x, y, w, h );
        GL_Scissor( x, y, w, h );
    }
};

#endif