//------------------------------------------------------------------------------
//  debugTextRenderer.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "debugTextRenderer.h"
#include "Gfx/Gfx.h"
#include "IO/Stream/MemoryStream.h"
#include "Dbg/shaders/DebugShaders.h"

namespace Oryol {
namespace _priv {

extern const char *kc85_4_Font;

//------------------------------------------------------------------------------
debugTextRenderer::debugTextRenderer() :
textScale(1.0f, 1.0f),
valid(false) {
    // NOTE: text rendering will be setup lazily when the text rendering
    // method is called first
    this->stringBuilder.Reserve(MaxNumChars * 2);
}

//------------------------------------------------------------------------------
debugTextRenderer::~debugTextRenderer() {
    if (this->valid) {
        this->discard();
    }
}

//------------------------------------------------------------------------------
void
debugTextRenderer::setTextScale(const glm::vec2& s) {
    this->textScale = s;
}

//------------------------------------------------------------------------------
const glm::vec2&
debugTextRenderer::getTextScale() const {
    return this->textScale;
}

//------------------------------------------------------------------------------
void
debugTextRenderer::setup() {
    o_assert(!this->valid);
    this->setupFontTexture();
    this->setupTextMesh();
    this->setupTextDrawState();
    this->valid = true;
}

//------------------------------------------------------------------------------
void
debugTextRenderer::discard() {
    o_assert(this->valid);
    this->valid = false;
    this->textMesh.Release();
    this->textDrawState.Release();
    this->fontTexture.Release();
}

//------------------------------------------------------------------------------
bool
debugTextRenderer::isValid() const {
    return this->valid;
}

//------------------------------------------------------------------------------
void
debugTextRenderer::print(const char* txt) {
    this->rwLock.LockWrite();
    this->stringBuilder.Append(txt);
    this->rwLock.UnlockWrite();
}

//------------------------------------------------------------------------------
void
debugTextRenderer::printf(const char* text, std::va_list args) {
    this->rwLock.LockWrite();
    this->stringBuilder.AppendFormat(1024, text, args);
    this->rwLock.UnlockWrite();
}

//------------------------------------------------------------------------------
void
debugTextRenderer::cursorPos(uint8 x, uint8 y) {
    this->rwLock.LockWrite();
    this->stringBuilder.Append((char) 0x1B);   // start ESC control sequence
    this->stringBuilder.Append((char) 0x01);   // set cursor
    this->stringBuilder.Append((char)x);
    this->stringBuilder.Append((char)y);
    this->rwLock.UnlockWrite();
}

//------------------------------------------------------------------------------
void
debugTextRenderer::textColor(const glm::vec4& color) {
    this->rwLock.LockWrite();
    this->stringBuilder.Append(0x1B);   // start ESC control sequence
    this->stringBuilder.Append(0x02);   // set color
    this->stringBuilder.Append((char) (color.x * 255.0f));
    this->stringBuilder.Append((char) (color.y * 255.0f));
    this->stringBuilder.Append((char) (color.z * 255.0f));
    this->stringBuilder.Append((char) (color.w * 255.0f));
    this->rwLock.UnlockWrite();
}

//------------------------------------------------------------------------------
void
debugTextRenderer::drawTextBuffer() {
    
    // one-time setup
    if (!this->valid) {
        this->setup();
    }
    
    // get the currently accumulated string
    this->rwLock.LockWrite();
    String str = this->stringBuilder.GetString();
    this->stringBuilder.Clear();
    this->rwLock.UnlockWrite();
    
    // convert string into vertices
    int32 numVertices = this->convertStringToVertices(str);

    // draw the vertices
    if (numVertices > 0) {
        // compute the size factor for one 8x8 glyph on screen
        // FIXME: this would be wrong if rendering to a render target which
        // isn't the same size as the back buffer, there's no method yet
        // to query the current render target width/height
        const float w = 8.0f / Gfx::RenderTargetAttrs().FramebufferWidth;   // glyph is 8 pixels wide
        const float h = 8.0f / Gfx::RenderTargetAttrs().FramebufferHeight;  // glyph is 8 pixel tall
        const glm::vec2 glyphSize = glm::vec2(w * 2.0f, h * 2.0f) * this->textScale;
    
        Gfx::UpdateVertices(this->textMesh, numVertices * this->vertexLayout.ByteSize(), this->vertexData);
        Gfx::ApplyDrawState(this->textDrawState);
        Gfx::ApplyVariable(DebugShaders::TextShader::GlyphSize, glyphSize);
        Gfx::ApplyVariable(DebugShaders::TextShader::Texture, this->fontTexture);
        Gfx::Draw(PrimitiveGroup(PrimitiveType::Triangles, 0, numVertices));
    }
}

//------------------------------------------------------------------------------
void
debugTextRenderer::setupFontTexture() {
    o_assert(!this->fontTexture.IsValid());
    
    // convert the KC85/4 font into 8bpp image data
    const int32 numChars = 128;
    const int32 charWidth = 8;
    const int32 charHeight = 8;
    const int32 imgWidth = numChars * charWidth;
    const int32 imgHeight = charHeight;
    const int32 bytesPerChar = charWidth * charHeight;
    const int32 imgDataSize = numChars * bytesPerChar;
    o_assert((imgWidth * imgHeight) == imgDataSize);
    
    // setup a memory buffer and write font image data to it
    auto data = MemoryStream::Create();
    data->Open(OpenMode::WriteOnly);
    uint8* dstPtr = data->MapWrite(imgDataSize);
    const char* srcPtr = kc85_4_Font;
    for (int32 charIndex = 0; charIndex < numChars; charIndex++) {
        int32 xOffset = charIndex * charWidth;
        for (int32 y = 0; y < charHeight; y++) {
            int32 yOffset = y * imgWidth;
            for (int32 x = 0; x < charWidth; x++) {
                char c = *srcPtr++;
                o_assert_dbg(c != 0);
                dstPtr[x + xOffset + yOffset] = (c == '-') ? 0 : 255;
            }
        }
    }
    data->UnmapWrite();
    data->Close();
    
    // setup texture, pixel format is 8bpp uncompressed
    TextureSetup setup = TextureSetup::FromPixelData(imgWidth, imgHeight, false, PixelFormat::L8);
    setup.MinFilter = TextureFilterMode::Nearest;
    setup.MagFilter = TextureFilterMode::Nearest;
    setup.WrapU = TextureWrapMode::ClampToEdge;
    setup.WrapV = TextureWrapMode::ClampToEdge;
    this->fontTexture = Gfx::CreateResource(setup, data);
    o_assert(this->fontTexture.IsValid());
    o_assert(Gfx::QueryResourceState(this->fontTexture) == ResourceState::Valid);
}

//------------------------------------------------------------------------------
void
debugTextRenderer::setupTextMesh() {
    o_assert(!this->textMesh.IsValid());
    o_assert(this->vertexLayout.Empty());
    
    // setup an empty mesh, only vertices
    int32 maxNumVerts = MaxNumChars * 6;
    this->vertexLayout
        .Add(VertexAttr::Position, VertexFormat::UByte4)
        .Add(VertexAttr::Color0, VertexFormat::UByte4N);
    o_assert(sizeof(this->vertexData) == maxNumVerts * this->vertexLayout.ByteSize());
    MeshSetup setup = MeshSetup::Empty(maxNumVerts, Usage::Stream);
    setup.Layout = this->vertexLayout;
    this->textMesh = Gfx::CreateResource(setup);
    o_assert(this->textMesh.IsValid());
    o_assert(Gfx::QueryResourceState(this->textMesh) == ResourceState::Valid);
}

//------------------------------------------------------------------------------
void
debugTextRenderer::setupTextDrawState() {
    o_assert(!this->textDrawState.IsValid());
    o_assert(this->textMesh.IsValid());

    // shader
    GfxId prog = Gfx::CreateResource(DebugShaders::TextShader::CreateSetup());
    
    // finally create draw state
    auto dss = DrawStateSetup::FromMeshAndProg(this->textMesh, prog, 0);
    dss.DepthStencilState.DepthWriteEnabled = false;
    dss.DepthStencilState.DepthCmpFunc = CompareFunc::Always;
    dss.BlendState.BlendEnabled = true;
    dss.BlendState.SrcFactorRGB = BlendFactor::SrcAlpha;
    dss.BlendState.DstFactorRGB = BlendFactor::OneMinusSrcAlpha;
    this->textDrawState = Gfx::CreateResource(dss);
}

//------------------------------------------------------------------------------
int32
debugTextRenderer::writeVertex(int32 index, uint8 x, uint8 y, uint8 u, uint8 v, uint32 rgba) {
    this->vertexData[index][0] = (v << 24) | (u << 16) | (y << 8) | x;
    this->vertexData[index][1] = rgba;
    return index + 1;
}

//------------------------------------------------------------------------------
int32
debugTextRenderer::convertStringToVertices(const String& str) {

    int32 cursorX = 0;
    int32 cursorY = 0;
    const int32 cursorMaxX = MaxNumColumns - 1;
    const int32 cursorMaxY = MaxNumLines - 1;
    int32 vIndex = 0;
    uint32 rgba = 0xFF00FFFF;
    
    const int32 numChars = str.Length() > MaxNumChars ? MaxNumChars : str.Length();
    const char* ptr = str.AsCStr();
    for (int32 charIndex = 0; charIndex < numChars; charIndex++) {
        uchar c = (uchar) ptr[charIndex];
        
        // control character?
        if (c < 0x20) {
            switch (c) {
                case 0x08: // cursor left
                    cursorX = cursorX > 0 ? cursorX - 1 : 0; break;
                case 0x09: // cursor right
                    cursorX = cursorX < cursorMaxX ? cursorX + 1 : cursorMaxX; break;
                case 0x0A: // cursor down
                    cursorY = cursorY < cursorMaxY ? cursorY + 1 : cursorMaxY; break;
                case 0x0B: // cursor up
                    cursorY = cursorY > 0 ? cursorY - 1 : 0; break;
                case 0x0D: // line feed
                    cursorX = 0; break;
                case 0x10: // home
                    cursorX = 0; cursorY = 0; break;
                case 0x1B: // handle escape sequence (position cursor or change text color)
                    {
                        o_assert((charIndex + 1) < numChars);
                        char escCode = ptr[charIndex + 1];
                        if (escCode == 1) {
                            // reposition cursor
                            o_assert((charIndex + 3) < numChars);
                            cursorX = ptr[charIndex + 2];
                            cursorY = ptr[charIndex + 3];
                            charIndex += 3;
                        }
                        else if (escCode == 2) {
                            // change color
                            o_assert((charIndex + 5) < numChars);
                            uchar r = (uchar) ptr[charIndex + 2];
                            uchar g = (uchar) ptr[charIndex + 3];
                            uchar b = (uchar) ptr[charIndex + 4];
                            uchar a = (uchar) ptr[charIndex + 5];
                            charIndex += 5;
                            rgba = (a<<24) | (b << 16) | (g << 8) | (r);
                        }
                        else {
                            o_error("Invalid escape code '%d'\n", escCode);
                        }
                    }
                default: break;
            }
        }
        else {
            // still space in vertex buffer?
            if ((vIndex < (MaxNumVertices - 6)) && (cursorX <= cursorMaxX)) {
                // renderable character, only consider 7 bit (codes > 127 can be
                // used to render control-code characters)
                c &= 0x7F;
                
                // write 6 vertices
                vIndex = this->writeVertex(vIndex, cursorX, cursorY, c, 0, rgba);
                vIndex = this->writeVertex(vIndex, cursorX+1, cursorY, c+1, 0, rgba);
                vIndex = this->writeVertex(vIndex, cursorX+1, cursorY+1, c+1, 1, rgba);
                vIndex = this->writeVertex(vIndex, cursorX, cursorY, c, 0, rgba);
                vIndex = this->writeVertex(vIndex, cursorX+1, cursorY+1, c+1, 1, rgba);
                vIndex = this->writeVertex(vIndex, cursorX, cursorY+1, c, 1, rgba);
                
                // advance horizontal cursor position
                cursorX++;
            }
            else {
                // vertex buffer overflow
                break;
            }
        }
    }
    return vIndex;
}

} // namespace _priv
} // namespace Oryol
