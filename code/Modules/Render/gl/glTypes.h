#pragma once
//------------------------------------------------------------------------------
/**
    @class Oryol::Render::glTypes
    @brief OpenGL type conversion helpers
*/
#include "Core/Types.h"
#include "Render/Core/Enums.h"
#include "Render/gl/gl_decl.h"

namespace Oryol {
namespace Render {
    
class glTypes {
public:
    /// convert Oryol pixel format to glTexImage format
    static GLenum AsGLTexImageFormat(PixelFormat::Code c);
    /// convert Oryol pixel format to glTexImage format, this is == TexImageFormat on GLES but may be different on desktop GL
    static GLenum AsGLTexImageInternalFormat(PixelFormat::Code c);
    /// convert Oryol pixel format to glTexImage type
    static GLenum AsGLTexImageType(PixelFormat::Code c);
    /// convert Oryol pixel format to renderbuffer format
    static GLenum AsGLRenderbufferFormat(PixelFormat::Code c);
};
    
} // namespace Render
} // namespace Oryol