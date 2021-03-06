#pragma once
//------------------------------------------------------------------------------
/**
    @class Oryol::ShaderSetup
    @ingroup Gfx
    @brief setup class for vertex- or fragment shaders
 */
#include "Core/Containers/Map.h"
#include "Core/String/String.h"
#include "Gfx/Core/Enums.h"
#include "Resource/Locator.h"

namespace Oryol {
    
class ShaderSetup {
public:
    /// default constructor
    ShaderSetup();
    /// construct with locator and shader type
    ShaderSetup(const Locator& loc, ShaderType::Code type);
    /// resource locator
    class Locator Locator;
    /// shader type
    ShaderType::Code Type;
    /// add a source code of a shader language syntax
    void AddSource(ShaderLang::Code slang, const String& source);
    /// get the shader sources by shader language version (may return empty string)
    const String& Source(ShaderLang::Code slang) const;
    
private:
    String sources[ShaderLang::NumShaderLangs];
};
    
} // namespace Oryol
