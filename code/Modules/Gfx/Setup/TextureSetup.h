#pragma once
//------------------------------------------------------------------------------
/**
    @class Oryol::TextureSetup
    @ingroup Gfx
    @brief setup object for textures and render targets
    @todo setup from file, setup from stream, multiple rendertarget support, mipmap generation mode...?
*/
#include "Resource/Locator.h"
#include "Resource/Id.h"
#include "Gfx/Core/Enums.h"
#include "Gfx/Core/GfxId.h"

namespace Oryol {
    
class TextureSetup {
public:
    /// setup a texture from an image file URL
    static TextureSetup FromFile(const Locator& loc, int32 ioLane=0, TextureSetup blueprint=TextureSetup());
    /// setup a texture from a image file data in stream
    static TextureSetup FromImageFileData(TextureSetup blueprint=TextureSetup());
    /// setup texture from raw pixel data
    static TextureSetup FromPixelData(int32 w, int32 h, bool hasMipMaps, PixelFormat::Code fmt);
    /// setup as absolute-size render target
    static TextureSetup RenderTarget(int32 w, int32 h);
    /// setup as render target with size relative to current display size
    static TextureSetup RelSizeRenderTarget(float32 relWidth, float32 relHeight);
    /// create render target with shared depth buffer
    static TextureSetup SharedDepthRenderTarget(const GfxId& depthRenderTarget);

    /// default constructor
    TextureSetup();
    /// return true if texture should be setup from a file
    bool ShouldSetupFromFile() const;
    /// return true if texture should be setup from image file data in stream
    bool ShouldSetupFromImageFileData() const;
    /// return true if texture should be setup from raw pixel data
    bool ShouldSetupFromPixelData() const;
    /// return true if texture should be setup as render target
    bool ShouldSetupAsRenderTarget() const;
    /// return true if rel-size render target
    bool IsRelSizeRenderTarget() const;
    /// return true if render target has depth (shared or non-shared)
    bool HasDepth() const;
    /// return true if render target with shared depth buffer
    bool HasSharedDepth() const;
    /// return true if texture should be generated with mipmaps (only when created from raw pixel data)
    bool HasMipMaps() const;
        
    /// the resource locator
    class Locator Locator;
    /// IOLane index
    int32 IOLane;
    /// the width in pixels (only if absolute-size render target)
    int32 Width;
    /// the height in pixels (only if absolute-size render target)
    int32 Height;
    /// display-relative width (only if screen render target)
    float32 RelWidth;
    /// display-relative height (only if screen render target)
    float32 RelHeight;
    /// the color pixel format (only if render target)
    PixelFormat::Code ColorFormat;
    /// the depth pixel format (only if render target, InvalidPixelFormat if render target should not have depth buffer)
    PixelFormat::Code DepthFormat;
    /// resource id of render target which owns the depth buffer (only if render target with shared depth buffer)
    GfxId DepthRenderTarget;
    
    /// texture wrap mode for u dimension (default is Repeat for textures, and ClampToEdge for render targets)
    TextureWrapMode::Code WrapU;
    /// texture wrap mode for v dimension
    TextureWrapMode::Code WrapV;
    /// texture wrap mode for w dimension
    TextureWrapMode::Code WrapW;
    
    /// magnification sample filter
    TextureFilterMode::Code MagFilter;
    /// minification sample filter
    TextureFilterMode::Code MinFilter;
    
private:
    bool shouldSetupFromFile : 1;
    bool shouldSetupFromImageFileData : 1;
    bool shouldSetupFromPixelData : 1;
    bool shouldSetupAsRenderTarget : 1;
    bool isRelSizeRenderTarget : 1;
    bool hasSharedDepth : 1;
    bool hasMipMaps : 1;
};
    
} // namespace Oryol
