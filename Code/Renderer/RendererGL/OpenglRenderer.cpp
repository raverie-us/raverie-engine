// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"
#include "OpenglRenderer.hpp"

#ifdef ZeroGl
#  define ZeroIfGl(X) X
#else
#  define ZeroIfGl(X)
#endif

#ifdef ZeroWebgl
#  define ZeroIfWebgl(X) X
#else
#  define ZeroIfWebgl(X)
#endif

static const size_t cMaxDrawBuffers = 4;

// As Of NVidia Driver 302 exporting this symbol will enable GPU hardware
// accelerated graphics when using Optimus (Laptop NVidia gpu / Intel HD auto
// switching). This is important for two reasons first is performance and second
// is stability since Intel seems to have a fair amount of bugs and crashes in
// their OpenGl drivers
extern "C"
{
  ZeroExport int NvOptimusEnablement = 0x00000001;
}

// temporary to prevent string constructions every frame
// RenderQueue structures should have semantics for setting shader parameters
namespace
{
const Zero::String cFrameTime("FrameData.FrameTime");
const Zero::String cLogicTime("FrameData.LogicTime");
const Zero::String cNearPlane("CameraData.NearPlane");
const Zero::String cFarPlane("CameraData.FarPlane");
const Zero::String cViewportSize("CameraData.ViewportSize");
const Zero::String cInverseViewportSize("CameraData.InverseViewportSize");
const Zero::String cObjectWorldPosition("CameraData.ObjectWorldPosition");

const Zero::String cLocalToWorld("TransformData.LocalToWorld");
const Zero::String cWorldToLocal("TransformData.WorldToLocal");
const Zero::String cWorldToView("TransformData.WorldToView");
const Zero::String cViewToWorld("TransformData.ViewToWorld");
const Zero::String cLocalToView("TransformData.LocalToView");
const Zero::String cViewToLocal("TransformData.ViewToLocal");
const Zero::String cLocalToWorldNormal("TransformData.LocalToWorldNormal");
const Zero::String cWorldToLocalNormal("TransformData.WorldToLocalNormal");
const Zero::String cLocalToViewNormal("TransformData.LocalToViewNormal");
const Zero::String cViewToLocalNormal("TransformData.ViewToLocalNormal");
const Zero::String cLocalToPerspective("TransformData.LocalToPerspective");
const Zero::String cViewToPerspective("TransformData.ViewToPerspective");
const Zero::String cPerspectiveToView("TransformData.PerspectiveToView");
const Zero::String cZeroPerspectiveToApiPerspective("TransformData.ZeroPerspectiveToApiPerspective");

const Zero::String cSpriteSource("SpriteSource_SpriteSourceColor");
const Zero::String cSpriteSourceCubePreview("SpriteSource_TextureCubePreview");
} // namespace

namespace Zero
{

void EmptyUniformFunc(GLint, GLsizei, const void*)
{
}

const bool cTransposeMatrices = !(ColumnBasis == 1);

struct GlTextureEnums
{
  GLint mInternalFormat;
  GLint mFormat;
  GLint mType;
};

GlTextureEnums gTextureEnums[] = {
    {/* internalFormat    , format            , type */},                       // None
    {GL_R8, GL_RED, GL_UNSIGNED_BYTE},                                          // R8
    {GL_RG8, GL_RG, GL_UNSIGNED_BYTE},                                          // RG8
    {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE},                                        // RGB8
    {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE},                                      // RGBA8
    {GL_R16I, GL_RED, GL_UNSIGNED_SHORT},                                        // R16
    {GL_RG16I, GL_RG, GL_UNSIGNED_SHORT},                                        // RG16
    {GL_RGB16I, GL_RGB, GL_UNSIGNED_SHORT},                                      // RGB16
    {GL_RGBA16I, GL_RGBA, GL_UNSIGNED_SHORT},                                    // RGBA16
    {GL_R16F, GL_RED, GL_HALF_FLOAT},                                           // R16f
    {GL_RG16F, GL_RG, GL_HALF_FLOAT},                                           // RG16f
    {GL_RGB16F, GL_RGB, GL_HALF_FLOAT},                                         // RGB16f
    {GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT},                                       // RGBA16f
    {GL_R32F, GL_RED, GL_FLOAT},                                                // R32f
    {GL_RG32F, GL_RG, GL_FLOAT},                                                // RG32f
    {GL_RGB32F, GL_RGB, GL_FLOAT},                                              // RGB32f
    {GL_RGBA32F, GL_RGBA, GL_FLOAT},                                            // RGBA32f
    {GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE},                                       // SRGB8
    {GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE},                               // SRGB8A8
    {GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT},              // Depth16
    {GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT},                // Depth24
    {GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT},                // Depth32
    {GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT},                      // Depth32f
    {GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8},              // Depth24Stencil8
    {GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV} // Depth32fStencil8Pad24
};

void WebglConvertTextureFormat(AddTextureInfo* info)
{
  // 16 integer formats are unsupported, fallback to half floats to preserve
  // data size.
  if (IsShortColorFormat(info->mFormat))
  {
    switch (info->mFormat)
    {
    case TextureFormat::R16:
      info->mFormat = TextureFormat::R16f;
      break;
    case TextureFormat::RG16:
      info->mFormat = TextureFormat::RG16f;
      break;
    case TextureFormat::RGB16:
      info->mFormat = TextureFormat::RGB16f;
      break;
    case TextureFormat::RGBA16:
      info->mFormat = TextureFormat::RGBA16f;
      break;
    default:
      return;
    }

    uint componentCount = GetPixelSize(info->mFormat) / sizeof(u16);

    for (uint i = 0; i < info->mMipCount; ++i)
    {
      MipHeader* mipHeader = info->mMipHeaders + i;
      u16* imageData = (u16*)(info->mImageData + mipHeader->mDataOffset);
      uint pixelCount = mipHeader->mWidth * mipHeader->mHeight;

      for (uint p = 0; p < pixelCount * componentCount; ++p)
      {
        float normalized = imageData[p] / 65535.0f;
        imageData[p] = HalfFloatConverter::ToHalfFloat(normalized);
      }
    }
  }
}

void WebglConvertRenderTargetFormat(AddTextureInfo* info)
{
  // For unsupported target formats, fallback to formats that do not drop any
  // data. Formats are either converting to floats, adding an alpha channel, or
  // both.
  switch (info->mFormat)
  {
  case TextureFormat::R16:
    info->mFormat = TextureFormat::R16f;
    break;
  case TextureFormat::RG16:
    info->mFormat = TextureFormat::RG16f;
    break;
  case TextureFormat::RGB16:
    info->mFormat = TextureFormat::RGBA16f;
    break;
  case TextureFormat::RGBA16:
    info->mFormat = TextureFormat::RGBA16f;
    break;
  case TextureFormat::RGB16f:
    info->mFormat = TextureFormat::RGBA16f;
    break;
  case TextureFormat::RGB32f:
    info->mFormat = TextureFormat::RGBA32f;
    break;
  case TextureFormat::SRGB8:
    info->mFormat = TextureFormat::SRGB8A8;
    break;
  case TextureFormat::Depth32:
    info->mFormat = TextureFormat::Depth32f;
    break;
  default:
    break;
  }
}

GLint GlInternalFormat(TextureCompression::Enum compression)
{
  switch (compression)
  {
  case TextureCompression::BC1:
    return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
  case TextureCompression::BC2:
    return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
  case TextureCompression::BC3:
    return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
  case TextureCompression::BC4:
    return GL_COMPRESSED_RED_RGTC1;
  case TextureCompression::BC5:
    return GL_COMPRESSED_RG_RGTC2;
  case TextureCompression::BC6:
    return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB;
  // case TextureCompression::BC7: return GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
  default:
    return 0;
  }
}

GLuint ToOpenglType(VertexElementType::Enum type)
{
  switch (type)
  {
  case VertexElementType::Byte:
  case VertexElementType::NormByte:
    return GL_UNSIGNED_BYTE;

  case VertexElementType::Short:
  case VertexElementType::NormShort:
    return GL_UNSIGNED_SHORT;

  case VertexElementType::Half:
    return GL_HALF_FLOAT;
  case VertexElementType::Real:
    return GL_FLOAT;

  default:
    return 0;
  }
}

GLuint GlPrimitiveType(PrimitiveType::Enum value)
{
  switch (value)
  {
  case PrimitiveType::Triangles:
    return GL_TRIANGLES;
  case PrimitiveType::Lines:
    return GL_LINES;
  case PrimitiveType::Points:
    return GL_POINTS;
  default:
    return 0;
  }
}

GLuint GlTextureType(TextureType::Enum value)
{
  switch (value)
  {
  case TextureType::Texture2D:
    return GL_TEXTURE_2D;
  case TextureType::TextureCube:
    return GL_TEXTURE_CUBE_MAP;
  default:
    return 0;
  }
}

GLuint GlTextureFace(TextureFace::Enum value)
{
  switch (value)
  {
  case TextureFace::None:
    return GL_TEXTURE_2D;
  case TextureFace::PositiveX:
    return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
  case TextureFace::PositiveY:
    return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
  case TextureFace::PositiveZ:
    return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
  case TextureFace::NegativeX:
    return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
  case TextureFace::NegativeY:
    return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
  case TextureFace::NegativeZ:
    return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
  default:
    return 0;
  }
}

GLuint GlTextureAddressing(TextureAddressing::Enum value)
{
  switch (value)
  {
  case TextureAddressing::Clamp:
    return GL_CLAMP_TO_EDGE;
  case TextureAddressing::Repeat:
    return GL_REPEAT;
  case TextureAddressing::Mirror:
    return GL_MIRRORED_REPEAT;
  default:
    return 0;
  }
}

GLuint GlTextureFilteringMin(TextureFiltering::Enum value)
{
  switch (value)
  {
  case TextureFiltering::Nearest:
    return GL_NEAREST_MIPMAP_NEAREST;
  case TextureFiltering::Bilinear:
    return GL_LINEAR_MIPMAP_NEAREST;
  case TextureFiltering::Trilinear:
    return GL_LINEAR_MIPMAP_LINEAR;
  default:
    return 0;
  }
}

GLuint GlTextureFilteringMag(TextureFiltering::Enum value)
{
  switch (value)
  {
  case TextureFiltering::Nearest:
    return GL_NEAREST;
  case TextureFiltering::Bilinear:
    return GL_LINEAR;
  case TextureFiltering::Trilinear:
    return GL_LINEAR;
  default:
    return 0;
  }
}

GLfloat GlTextureAnisotropy(TextureAnisotropy::Enum value)
{
  switch (value)
  {
  case TextureAnisotropy::x1:
    return 1.0f;
  case TextureAnisotropy::x2:
    return 2.0f;
  case TextureAnisotropy::x4:
    return 4.0f;
  case TextureAnisotropy::x8:
    return 8.0f;
  case TextureAnisotropy::x16:
    return 16.0f;
  default:
    return 1.0f;
  }
}

GLuint GlTextureMipMapping(TextureMipMapping::Enum value)
{
  switch (value)
  {
  case TextureMipMapping::None:
    return 0;
  case TextureMipMapping::PreGenerated:
    return 1000;
  case TextureMipMapping::GpuGenerated:
    return 1000;
  default:
    return 0;
  }
}

void CheckShader(GLuint shader, StringParam shaderCode)
{
#ifdef ZeroDebug
  GLint status = 0;
  ImportGlGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE)
  {
    GLint infoLogLength;
    ImportGlGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    GLchar* strInfoLog = (GLchar*)alloca(infoLogLength + 1);
    ImportGlGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
    ZPrint("Compile Error\n%s\n", strInfoLog);

    static size_t sMaxPrints = 4;
    if (sMaxPrints > 0)
    {
      ZPrint("\n************************************************************\n%"
             "s\n************************************************************\n",
             shaderCode.c_str());
      --sMaxPrints;
    }
  }
#endif
}

void SetClearData(void* clearData, TextureFormat::Enum format, Vec4 color, float depth)
{
  switch (format)
  {
  case TextureFormat::RGB8:
  case TextureFormat::RGBA8:
    *(ByteColor*)clearData = ToByteColor(color);
    break;

  case TextureFormat::RGB32f:
  case TextureFormat::RGBA32f:
    *(Vec4*)clearData = color;
    break;

  case TextureFormat::Depth16:
    *(u16*)clearData = (u16)((u16)-1 * (double)depth);
    break;

  case TextureFormat::Depth24:
  case TextureFormat::Depth32:
    *(uint*)clearData = (uint)((uint)-1 * (double)depth);
    break;

  case TextureFormat::Depth32f:
    *(float*)clearData = depth;
    break;

  case TextureFormat::Depth24Stencil8:
    // not handled, but this function is not currently being used
    break;
  default:
    break;
  }
}

GLuint GlCullFace(CullMode::Enum value)
{
  switch (value)
  {
  case CullMode::BackFace:
    return GL_BACK;
  case CullMode::FrontFace:
    return GL_FRONT;
  default:
    return 0;
  }
}

GLuint GlBlendFactor(BlendFactor::Enum value)
{
  switch (value)
  {
  case BlendFactor::Zero:
    return GL_ZERO;
  case BlendFactor::One:
    return GL_ONE;
  case BlendFactor::SourceColor:
    return GL_SRC_COLOR;
  case BlendFactor::InvSourceColor:
    return GL_ONE_MINUS_SRC_COLOR;
  case BlendFactor::DestColor:
    return GL_DST_COLOR;
  case BlendFactor::InvDestColor:
    return GL_ONE_MINUS_DST_COLOR;
  case BlendFactor::SourceAlpha:
    return GL_SRC_ALPHA;
  case BlendFactor::InvSourceAlpha:
    return GL_ONE_MINUS_SRC_ALPHA;
  case BlendFactor::DestAlpha:
    return GL_DST_ALPHA;
  case BlendFactor::InvDestAlpha:
    return GL_ONE_MINUS_DST_ALPHA;
  case BlendFactor::SourceAlphaSaturate:
    return GL_SRC_ALPHA_SATURATE;
  default:
    return 0;
  }
}

GLuint GlBlendEquation(BlendEquation::Enum blendEquation)
{
  switch (blendEquation)
  {
  case BlendEquation::Add:
    return GL_FUNC_ADD;
  case BlendEquation::Subtract:
    return GL_FUNC_SUBTRACT;
  case BlendEquation::ReverseSubtract:
    return GL_FUNC_REVERSE_SUBTRACT;
  case BlendEquation::Min:
    return GL_MIN;
  case BlendEquation::Max:
    return GL_MAX;
  default:
    return 0;
  }
}

GLboolean GlDepthMode(DepthMode::Enum value)
{
  switch (value)
  {
  case DepthMode::Read:
    return false;
  case DepthMode::Write:
    return true;
  default:
    return false;
  }
}

GLuint GlStencilOp(StencilOp::Enum value)
{
  switch (value)
  {
  case StencilOp::Zero:
    return GL_ZERO;
  case StencilOp::Keep:
    return GL_KEEP;
  case StencilOp::Replace:
    return GL_REPLACE;
  case StencilOp::Invert:
    return GL_INVERT;
  case StencilOp::Increment:
    return GL_INCR;
  case StencilOp::IncrementWrap:
    return GL_INCR_WRAP;
  case StencilOp::Decrement:
    return GL_DECR;
  case StencilOp::DecrementWrap:
    return GL_DECR_WRAP;
  default:
    return 0;
  }
}

GLuint GlCompareMode(TextureCompareMode::Enum compareMode)
{
  switch (compareMode)
  {
  case TextureCompareMode::Disabled:
    return GL_NONE;
  case TextureCompareMode::Enabled:
    return GL_COMPARE_REF_TO_TEXTURE;
  default:
    return 0;
  }
}

GLuint GlCompareFunc(TextureCompareFunc::Enum value)
{
  switch (value)
  {
  case TextureCompareFunc::Never:
    return GL_NEVER;
  case TextureCompareFunc::Always:
    return GL_ALWAYS;
  case TextureCompareFunc::Less:
    return GL_LESS;
  case TextureCompareFunc::LessEqual:
    return GL_LEQUAL;
  case TextureCompareFunc::Greater:
    return GL_GREATER;
  case TextureCompareFunc::GreaterEqual:
    return GL_GEQUAL;
  case TextureCompareFunc::Equal:
    return GL_EQUAL;
  case TextureCompareFunc::NotEqual:
    return GL_NOTEQUAL;
  default:
    return 0;
  }
}

void SetBlendSettings(const BlendSettings& blendSettings)
{
  switch (blendSettings.mBlendMode)
  {
  case BlendMode::Disabled:
    ImportGlDisable(GL_BLEND);
    break;
  case BlendMode::Enabled:
    ImportGlEnable(GL_BLEND);
    ImportGlBlendEquation(GlBlendEquation(blendSettings.mBlendEquation));
    ImportGlBlendFunc(GlBlendFactor(blendSettings.mSourceFactor), GlBlendFactor(blendSettings.mDestFactor));
    break;
  case BlendMode::Separate:
    ImportGlEnable(GL_BLEND);
    ImportGlBlendEquationSeparate(GlBlendEquation(blendSettings.mBlendEquation),
                            GlBlendEquation(blendSettings.mBlendEquationAlpha));
    ImportGlBlendFuncSeparate(GlBlendFactor(blendSettings.mSourceFactor),
                        GlBlendFactor(blendSettings.mDestFactor),
                        GlBlendFactor(blendSettings.mSourceFactorAlpha),
                        GlBlendFactor(blendSettings.mDestFactorAlpha));
    break;
  }
}

void SetRenderSettings(const RenderSettings& renderSettings, bool drawBuffersBlend)
{
  switch (renderSettings.mCullMode)
  {
  case CullMode::Disabled:
    ImportGlDisable(GL_CULL_FACE);
    break;
  case CullMode::BackFace:
  case CullMode::FrontFace:
    ImportGlEnable(GL_CULL_FACE);
    ImportGlCullFace(GlCullFace(renderSettings.mCullMode));
    break;
  }

  if (renderSettings.mSingleColorTarget || drawBuffersBlend == false)
  {
    SetBlendSettings(renderSettings.mBlendSettings[0]);
  }
  else
  {
    for (uint i = 0; i < cMaxDrawBuffers; ++i)
    {
      const BlendSettings& blendSettings = renderSettings.mBlendSettings[i];
      switch (blendSettings.mBlendMode)
      {
      case BlendMode::Disabled:
        ImportGlDisablei(GL_BLEND, i);
        break;
      case BlendMode::Enabled:
        ImportGlEnablei(GL_BLEND, i);
        ImportGlBlendEquationi(i, GlBlendEquation(blendSettings.mBlendEquation));
        ImportGlBlendFunci(i, GlBlendFactor(blendSettings.mSourceFactor), GlBlendFactor(blendSettings.mDestFactor));
        break;
      case BlendMode::Separate:
        ImportGlEnablei(GL_BLEND, i);
        ImportGlBlendEquationSeparatei(
            i, GlBlendEquation(blendSettings.mBlendEquation), GlBlendEquation(blendSettings.mBlendEquationAlpha));
        ImportGlBlendFuncSeparatei(i,
                             GlBlendFactor(blendSettings.mSourceFactor),
                             GlBlendFactor(blendSettings.mDestFactor),
                             GlBlendFactor(blendSettings.mSourceFactorAlpha),
                             GlBlendFactor(blendSettings.mDestFactorAlpha));
        break;
      }
    }
  }

  const DepthSettings& depthSettings = renderSettings.mDepthSettings;
  switch (depthSettings.mDepthMode)
  {
  case DepthMode::Disabled:
    ImportGlDisable(GL_DEPTH_TEST);
    break;
  case DepthMode::Read:
  case DepthMode::Write:
    ImportGlEnable(GL_DEPTH_TEST);
    ImportGlDepthMask(GlDepthMode(depthSettings.mDepthMode));
    ImportGlDepthFunc(GlCompareFunc(depthSettings.mDepthCompareFunc));
    break;
  }

  switch (depthSettings.mStencilMode)
  {
  case StencilMode::Disabled:
    ImportGlDisable(GL_STENCIL_TEST);
    ImportGlStencilMask(0);
    break;
  case StencilMode::Enabled:
    ImportGlEnable(GL_STENCIL_TEST);
    ImportGlStencilFunc(GlCompareFunc(depthSettings.mStencilCompareFunc),
                  depthSettings.mStencilTestValue,
                  depthSettings.mStencilReadMask);
    ImportGlStencilOp(GlStencilOp(depthSettings.mStencilFailOp),
                GlStencilOp(depthSettings.mDepthFailOp),
                GlStencilOp(depthSettings.mDepthPassOp));
    ImportGlStencilMask(depthSettings.mStencilWriteMask);
    break;
  case StencilMode::Separate:
    ImportGlEnable(GL_STENCIL_TEST);
    ImportGlStencilFuncSeparate(GL_FRONT,
                          GlCompareFunc(depthSettings.mStencilCompareFunc),
                          depthSettings.mStencilTestValue,
                          depthSettings.mStencilReadMask);
    ImportGlStencilOpSeparate(GL_FRONT,
                        GlStencilOp(depthSettings.mStencilFailOp),
                        GlStencilOp(depthSettings.mDepthFailOp),
                        GlStencilOp(depthSettings.mDepthPassOp));
    ImportGlStencilMaskSeparate(GL_FRONT, depthSettings.mStencilWriteMask);
    ImportGlStencilFuncSeparate(GL_BACK,
                          GlCompareFunc(depthSettings.mStencilCompareFuncBackFace),
                          depthSettings.mStencilTestValueBackFace,
                          depthSettings.mStencilReadMaskBackFace);
    ImportGlStencilOpSeparate(GL_BACK,
                        GlStencilOp(depthSettings.mStencilFailOpBackFace),
                        GlStencilOp(depthSettings.mDepthFailOpBackFace),
                        GlStencilOp(depthSettings.mDepthPassOpBackFace));
    ImportGlStencilMaskSeparate(GL_BACK, depthSettings.mStencilWriteMaskBackFace);
    break;
  }

  switch (renderSettings.mScissorMode)
  {
  case ScissorMode::Disabled:
    ImportGlDisable(GL_SCISSOR_TEST);
    break;
  case ScissorMode::Enabled:
    ImportGlEnable(GL_SCISSOR_TEST);
    break;
  }
}

void BindTexture(TextureType::Enum textureType, uint textureSlot, uint textureId, bool samplerObjects)
{
  // Clear anything bound to this texture unit
  ImportGlActiveTexture(GL_TEXTURE0 + textureSlot);
  ImportGlBindTexture(GL_TEXTURE_2D, 0);
  ImportGlBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  if (samplerObjects)
    ImportGlBindSampler(textureSlot, 0);
  // Bind texture
  ImportGlBindTexture(GlTextureType(textureType), textureId);
}

void CheckFramebufferStatus()
{
#ifdef ZeroDebug
  GLenum status = ImportGlCheckFramebufferStatus(GL_FRAMEBUFFER);
  switch (status)
  {
  case GL_FRAMEBUFFER_UNSUPPORTED:
    DebugPrint("Unsupported framebuffer format\n");
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
    DebugPrint("Framebuffer incomplete, missing attachment\n");
    break;
  case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
    DebugPrint("Framebuffer incomplete, incomplete attachment\n");
    break;
  }
  ErrorIf(status != GL_FRAMEBUFFER_COMPLETE, "Framebuffer incomplete");
#endif
}

void DrawBuffer(GLenum buf)
{
  GLenum drawBuffers[8] = {buf, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE};
  ImportGlDrawBuffers(1, drawBuffers);
}

void SetSingleRenderTargets(GLuint fboId, TextureRenderData** colorTargets, TextureRenderData* depthTarget)
{
  ImportGlBindFramebuffer(GL_FRAMEBUFFER, fboId);

  ImportGlFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
  DrawBuffer(GL_NONE);

  ImportGlFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
  ImportGlFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

  GlTextureRenderData* colorRenderData = (GlTextureRenderData*)colorTargets[0];
  if (colorRenderData != nullptr)
  {
    ImportGlFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorRenderData->mId, 0);
    DrawBuffer(GL_COLOR_ATTACHMENT0);
  }

  GlTextureRenderData* depthRenderData = (GlTextureRenderData*)depthTarget;
  if (depthRenderData != nullptr)
  {
    if (IsDepthStencilFormat(depthRenderData->mFormat))
      ImportGlFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthRenderData->mId, 0);
    else
      ImportGlFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthRenderData->mId, 0);
  }

  CheckFramebufferStatus();
}

void SetMultiRenderTargets(GLuint fboId, TextureRenderData** colorTargets, TextureRenderData* depthTarget)
{
  ImportGlBindFramebuffer(GL_FRAMEBUFFER, fboId);

  ImportGlFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
  ImportGlFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

  GLenum drawBuffers[cMaxDrawBuffers];

  for (uint i = 0; i < cMaxDrawBuffers; ++i)
  {
    GlTextureRenderData* colorRenderData = (GlTextureRenderData*)colorTargets[i];
    if (colorRenderData != nullptr)
    {
      ImportGlFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorRenderData->mId, 0);
      drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }
    else
    {
      ImportGlFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);
      drawBuffers[i] = GL_NONE;
    }
  }

  // Set active buffers, some drivers do not work correctly if all are always
  // active
  ImportGlDrawBuffers(cMaxDrawBuffers, drawBuffers);

  GlTextureRenderData* depthRenderData = (GlTextureRenderData*)depthTarget;
  if (depthRenderData != nullptr)
  {
    if (IsDepthStencilFormat(depthRenderData->mFormat))
      ImportGlFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthRenderData->mId, 0);
    else
      ImportGlFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthRenderData->mId, 0);
  }

  CheckFramebufferStatus();
}

void StreamedVertexBuffer::Initialize()
{
  mBufferSize = 1 << 18; // 256Kb, 1213 sprites at 216 bytes per sprite
  mCurrentBufferOffset = 0;

  mVertexArray = ImportGlGenVertexArray();
  ImportGlBindVertexArray(mVertexArray);

  mVertexBuffer = ImportGlGenBuffer();
  ImportGlBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
  ImportGlBufferData(GL_ARRAY_BUFFER, mBufferSize, nullptr, GL_STREAM_DRAW);

  ImportGlEnableVertexAttribArray(VertexSemantic::Position);
  ImportGlVertexAttribPointer(VertexSemantic::Position,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(StreamedVertex),
                        (void*)ZeroOffsetOf(StreamedVertex, mPosition));
  ImportGlEnableVertexAttribArray(VertexSemantic::Uv);
  ImportGlVertexAttribPointer(
      VertexSemantic::Uv, 2, GL_FLOAT, GL_FALSE, sizeof(StreamedVertex), (void*)ZeroOffsetOf(StreamedVertex, mUv));
  ImportGlEnableVertexAttribArray(VertexSemantic::Color);
  ImportGlVertexAttribPointer(VertexSemantic::Color,
                        4,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(StreamedVertex),
                        (void*)ZeroOffsetOf(StreamedVertex, mColor));
  ImportGlEnableVertexAttribArray(VertexSemantic::UvAux);
  ImportGlVertexAttribPointer(VertexSemantic::UvAux,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(StreamedVertex),
                        (void*)ZeroOffsetOf(StreamedVertex, mUvAux));

  ImportGlBindVertexArray(0);

  mPrimitiveType = PrimitiveType::Triangles;
  mActive = false;
}

void StreamedVertexBuffer::Destroy()
{
  ImportGlDeleteBuffer(mVertexBuffer);
  ImportGlDeleteVertexArray(mVertexArray);
}

void StreamedVertexBuffer::AddVertices(StreamedVertex* vertices, uint count, PrimitiveType::Enum primitiveType)
{
  if (!mActive)
  {
    ImportGlBindVertexArray(mVertexArray);
    ImportGlBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    mActive = true;
  }

  if (primitiveType != mPrimitiveType)
  {
    FlushBuffer(false);
    mPrimitiveType = primitiveType;
  }

  uint uploadSize = sizeof(StreamedVertex) * count;
  if (mCurrentBufferOffset + uploadSize > mBufferSize)
  {
    FlushBuffer(false);
    // If upload size is larger than the entire buffer then break it into
    // multiple draws
    while (uploadSize > mBufferSize)
    {
      uint verticesPerPrimitive = primitiveType + 1;
      uint primitiveSize = sizeof(StreamedVertex) * verticesPerPrimitive;
      uint maxPrimitiveCount = mBufferSize / primitiveSize;
      uint maxByteCount = maxPrimitiveCount * primitiveSize;

      mCurrentBufferOffset = maxByteCount;
      ImportGlBufferSubData(GL_ARRAY_BUFFER, 0, mCurrentBufferOffset, vertices);
      FlushBuffer(false);
      // Move pointer forward by byte count, below condition will grab this new
      // value
      vertices = (StreamedVertex*)((char*)vertices + maxByteCount);
      uploadSize -= maxByteCount;
    }
  }

  ImportGlBufferSubData(GL_ARRAY_BUFFER, mCurrentBufferOffset, uploadSize, vertices);
  mCurrentBufferOffset += uploadSize;
}

void StreamedVertexBuffer::AddVertices(StreamedVertexArray& vertices,
                                       uint start,
                                       uint count,
                                       PrimitiveType::Enum primitiveType)
{
  while (count > 0)
  {
    uint verticesPerPrimitive = primitiveType + 1;

    // Get the maximum number of contiguous whole primitives.
    uint indexInBucket = start & StreamedVertexArray::BucketMask;
    uint contiguousCount = StreamedVertexArray::BucketSize - indexInBucket;
    uint uploadCount = Math::Min(contiguousCount, count);

    uint remainder = uploadCount % verticesPerPrimitive;
    uploadCount -= remainder;

    AddVertices(&vertices[start], uploadCount, primitiveType);
    start += uploadCount;
    count -= uploadCount;

    if (remainder != 0)
    {
      ErrorIf(count < verticesPerPrimitive, "Bad count if it does not have a whole number of primitives.");
      // Manually populate one primitive over the block array boundary.
      StreamedVertex primitive[3];
      for (uint i = 0; i < verticesPerPrimitive; ++i)
        primitive[i] = vertices[start + i];

      AddVertices(primitive, verticesPerPrimitive, primitiveType);
      start += verticesPerPrimitive;
      count -= verticesPerPrimitive;
    }
  }
}

void StreamedVertexBuffer::FlushBuffer(bool deactivate)
{
  if (mCurrentBufferOffset > 0)
  {
    if (mPrimitiveType == PrimitiveType::Triangles)
      ImportGlDrawArrays(GL_TRIANGLES, 0, mCurrentBufferOffset / sizeof(StreamedVertex));
    else if (mPrimitiveType == PrimitiveType::Lines)
      ImportGlDrawArrays(GL_LINES, 0, mCurrentBufferOffset / sizeof(StreamedVertex));
    else if (mPrimitiveType == PrimitiveType::Points)
      ImportGlDrawArrays(GL_POINTS, 0, mCurrentBufferOffset / sizeof(StreamedVertex));
    mCurrentBufferOffset = 0;
  }

  if (deactivate && mActive)
  {
    ImportGlBindVertexArray(0);
    ImportGlLineWidth(1.0f);
    mActive = false;
  }
}

OpenglRenderer::OpenglRenderer() {
  // TODO(trevor): Support texture compression via extensions
  mDriverSupport.mTextureCompression = false;
  // TODO(trevor): Check webgl for multi target blend support
  mDriverSupport.mMultiTargetBlend = false;
  // WebGL supports this
  mDriverSupport.mSamplerObjects = true;
}

void OpenglRenderer::Initialize()
{
  // V-Sync off by default
  zglSetSwapInterval(this, 0);

  // No padding
  ImportGlPixelStorei(GL_PACK_ALIGNMENT, 1);
  ImportGlPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#if !defined(ZeroWebgl)
  ImportGlEnable(GL_TEXTURE_2D);
  ImportGlEnable(GL_TEXTURE_CUBE_MAP);
  if (ImportGlewIsSupported("GL_ARB_seamless_cube_map"))
    ImportGlEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif

  // buffer for fullscreen triangle
  StreamedVertex triangleVertices[] = {
      {Vec3(-1, 3, 0), Vec2(0, -1), Vec4()},
      {Vec3(-1, -1, 0), Vec2(0, 1), Vec4()},
      {Vec3(3, -1, 0), Vec2(2, 1), Vec4()},
  };

  uint triangleIndices[] = {0, 1, 2};

  mTriangleArray = ImportGlGenVertexArray();
  ImportGlBindVertexArray(mTriangleArray);

  mTriangleVertex = ImportGlGenBuffer();
  ImportGlBindBuffer(GL_ARRAY_BUFFER, mTriangleVertex);
  ImportGlBufferData(GL_ARRAY_BUFFER, sizeof(StreamedVertex) * 3, triangleVertices, GL_STATIC_DRAW);

  ImportGlEnableVertexAttribArray(VertexSemantic::Position);
  ImportGlVertexAttribPointer(VertexSemantic::Position,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(StreamedVertex),
                        (void*)ZeroOffsetOf(StreamedVertex, mPosition));
  ImportGlEnableVertexAttribArray(VertexSemantic::Uv);
  ImportGlVertexAttribPointer(
      VertexSemantic::Uv, 2, GL_FLOAT, GL_FALSE, sizeof(StreamedVertex), (void*)ZeroOffsetOf(StreamedVertex, mUv));

  mTriangleIndex = ImportGlGenBuffer();
  ImportGlBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mTriangleIndex);
  ImportGlBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * 3, triangleIndices, GL_STATIC_DRAW);

  ImportGlBindVertexArray(0);

  // frame buffers
  mSingleTargetFbo = ImportGlGenFramebuffer();
  mMultiTargetFbo = ImportGlGenFramebuffer();

  // Function invocations will fail if calling convention is not correct,
  // currently using GLAPIENTRY
  mUniformFunctions[ShaderInputType::Invalid] = EmptyUniformFunc;
  mUniformFunctions[ShaderInputType::Bool] = (UniformFunction)ImportGlUniform1iv;
  mUniformFunctions[ShaderInputType::Int] = (UniformFunction)ImportGlUniform1iv;
  mUniformFunctions[ShaderInputType::IntVec2] = (UniformFunction)ImportGlUniform2iv;
  mUniformFunctions[ShaderInputType::IntVec3] = (UniformFunction)ImportGlUniform3iv;
  mUniformFunctions[ShaderInputType::IntVec4] = (UniformFunction)ImportGlUniform4iv;
  mUniformFunctions[ShaderInputType::Float] = (UniformFunction)ImportGlUniform1fv;
  mUniformFunctions[ShaderInputType::Vec2] = (UniformFunction)ImportGlUniform2fv;
  mUniformFunctions[ShaderInputType::Vec3] = (UniformFunction)ImportGlUniform3fv;
  mUniformFunctions[ShaderInputType::Vec4] = (UniformFunction)ImportGlUniform4fv;
  mUniformFunctions[ShaderInputType::Mat3] = EmptyUniformFunc;
  mUniformFunctions[ShaderInputType::Mat4] = EmptyUniformFunc;
  mUniformFunctions[ShaderInputType::Texture] = (UniformFunction)ImportGlUniform1iv;

  mStreamedVertexBuffer.Initialize();

#define ZeroGlVertexIn ZeroIfGl("in") ZeroIfWebgl("attribute")
#define ZeroGlVertexOut ZeroIfGl("out") ZeroIfWebgl("varying")
#define ZeroGlPixelIn ZeroIfGl("in") ZeroIfWebgl("varying")

  // @Nate: This will most likley have to change to use uniform buffers
  String loadingShaderVertex = ZeroIfGl("#version 150\n") ZeroIfWebgl("#version 100\n")
      ZeroIfWebgl("precision mediump float;\n") "uniform mat4 Transform;\n"
                                                "uniform mat3 "
                                                "UvTransform;\n" ZeroGlVertexIn " vec3 LocalPosition;\n" ZeroGlVertexIn
                                                " vec2 Uv;\n" ZeroGlVertexOut " vec2 psInUv;\n"
                                                "void main(void)\n"
                                                "{\n"
                                                "  psInUv = (vec3(Uv, 1.0) * "
                                                "UvTransform).xy;\n"
                                                "  gl_Position = vec4(LocalPosition, "
                                                "1.0) * Transform;\n"
                                                "}";

  String loadingShaderPixel = ZeroIfGl("#version 150\n") ZeroIfWebgl("#version 100\n")
      ZeroIfWebgl("precision mediump float;\n") "uniform sampler2D Texture;\n"
                                                "uniform float Alpha;\n" ZeroGlPixelIn " vec2 psInUv;\n"
                                                "void main(void)\n"
                                                "{\n"
                                                "  vec2 uv = vec2(psInUv.x, 1.0 - "
                                                "psInUv.y);\n"
                                                "  gl_FragColor = texture2D(Texture, "
                                                "uv);\n"
                                                "  gl_FragColor.xyz *= Alpha;\n"
                                                "}";

  CreateShader(loadingShaderVertex, String(), loadingShaderPixel, mLoadingShader);
}

void OpenglRenderer::Shutdown()
{
  ErrorIf(mGlShaders.Empty() == false, "Not all shaders were deleted.");
  ErrorIf(mShaderEntries.Empty() == false, "Not all shaders were deleted.");

  DelayedRenderDataDestruction();

  ImportGlDeleteFramebuffer(mSingleTargetFbo);
  ImportGlDeleteFramebuffer(mMultiTargetFbo);

  ImportGlDeleteVertexArray(mTriangleArray);
  ImportGlDeleteBuffer(mTriangleVertex);
  ImportGlDeleteBuffer(mTriangleIndex);

  ImportGlDeleteProgram(mLoadingShader);

  mStreamedVertexBuffer.Destroy();

  forRange (GLuint sampler, mSamplers.Values())
    ImportGlDeleteSamplers(1, &sampler);
  mSamplers.Clear();
}

void OpenglRenderer::BuildOrthographicTransform(
    Mat4Ref matrix, float size, float aspect, float nearPlane, float farPlane)
{
  BuildOrthographicTransformGl(matrix, size, aspect, nearPlane, farPlane);
}

void OpenglRenderer::BuildPerspectiveTransform(Mat4Ref matrix, float fov, float aspect, float nearPlane, float farPlane)
{
  BuildPerspectiveTransformGl(matrix, fov, aspect, nearPlane, farPlane);
}

bool OpenglRenderer::YInvertImageData(TextureType::Enum type)
{
  // OpenGL convention for cubemaps is not Y-inverted
  return (type != TextureType::TextureCube);
}

GlMaterialRenderData* OpenglRenderer::CreateMaterialRenderData()
{
  GlMaterialRenderData* renderData = new GlMaterialRenderData();
  renderData->mResourceId = 0;
  return renderData;
}

GlMeshRenderData* OpenglRenderer::CreateMeshRenderData()
{
  GlMeshRenderData* renderData = new GlMeshRenderData();
  renderData->mVertexBuffer = 0;
  renderData->mIndexBuffer = 0;
  renderData->mVertexArray = 0;
  renderData->mIndexCount = 0;
  return renderData;
}

GlTextureRenderData* OpenglRenderer::CreateTextureRenderData()
{
  GlTextureRenderData* renderData = new GlTextureRenderData();
  renderData->mId = 0;
  renderData->mFormat = TextureFormat::None;
  return renderData;
}

void OpenglRenderer::AddMaterial(AddMaterialInfo* info)
{
  GlMaterialRenderData* renderData = (GlMaterialRenderData*)info->mRenderData;

  renderData->mCompositeName = info->mCompositeName;
  renderData->mResourceId = info->mMaterialId;
}

void OpenglRenderer::AddMesh(AddMeshInfo* info)
{
  GlMeshRenderData* renderData = (GlMeshRenderData*)info->mRenderData;
  if (renderData->mVertexArray != 0)
  {
    ImportGlDeleteVertexArray(renderData->mVertexArray);
    ImportGlDeleteBuffer(renderData->mVertexBuffer);
    ImportGlDeleteBuffer(renderData->mIndexBuffer);
  }

  if (info->mVertexData == nullptr)
  {
    renderData->mVertexBuffer = 0;
    renderData->mIndexBuffer = 0;
    renderData->mVertexArray = 0;
    renderData->mIndexCount = info->mIndexCount;
    renderData->mPrimitiveType = info->mPrimitiveType;
    return;
  }

  GLuint vertexArray = ImportGlGenVertexArray();
  ImportGlBindVertexArray(vertexArray);

  GLuint vertexBuffer = ImportGlGenBuffer();
  ImportGlBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  ImportGlBufferData(GL_ARRAY_BUFFER, info->mVertexCount * info->mVertexSize, info->mVertexData, GL_STATIC_DRAW);

  forRange (VertexAttribute& element, info->mVertexAttributes.All())
  {
    bool normalized = element.mType >= VertexElementType::NormByte;
    ImportGlEnableVertexAttribArray(element.mSemantic);
    if (element.mType == VertexElementType::Byte || element.mType == VertexElementType::Short)
      ImportGlVertexAttribIPointer(element.mSemantic,
                             element.mCount,
                             ToOpenglType(element.mType),
                             info->mVertexSize,
                             (void*)(uintptr_t)element.mOffset);
    else
      ImportGlVertexAttribPointer(element.mSemantic,
                            element.mCount,
                            ToOpenglType(element.mType),
                            normalized,
                            info->mVertexSize,
                            (void*)(uintptr_t)element.mOffset);
  }

  GLuint indexBuffer = 0;
  if (info->mIndexData != nullptr)
  {
    indexBuffer = ImportGlGenBuffer();
    ImportGlBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    ImportGlBufferData(GL_ELEMENT_ARRAY_BUFFER, info->mIndexCount * info->mIndexSize, info->mIndexData, GL_STATIC_DRAW);
  }

  ImportGlBindVertexArray(0);

  renderData->mVertexBuffer = vertexBuffer;
  renderData->mIndexBuffer = indexBuffer;
  renderData->mVertexArray = vertexArray;
  renderData->mIndexCount = info->mIndexCount;
  renderData->mPrimitiveType = info->mPrimitiveType;

  delete[] info->mVertexData;
  delete[] info->mIndexData;

  renderData->mBones.Assign(info->mBones.All());
}

void OpenglRenderer::AddTexture(AddTextureInfo* info)
{
  GlTextureRenderData* renderData = (GlTextureRenderData*)info->mRenderData;

  if (info->mFormat == TextureFormat::None)
  {
    if (renderData->mId != 0)
    {
      ImportGlDeleteTexture(renderData->mId);
      renderData->mId = 0;
    }
  }
  else
  {
    if (info->mType != renderData->mType && renderData->mId != 0)
    {
      ImportGlDeleteTexture(renderData->mId);
      renderData->mId = 0;
    }

    if (renderData->mId == 0) {
      renderData->mId = ImportGlGenTexture();
    }

    BindTexture(info->mType, 0, renderData->mId, mDriverSupport.mSamplerObjects);

    // RenderTarget upload if data size is 0 (not calculated).
    // A texture resource with uploaded data will never set data size to 0.
    if (info->mTotalDataSize == 0)
    {
      ZeroIfWebgl(WebglConvertRenderTargetFormat(info));
      GlTextureEnums texEnums = gTextureEnums[info->mFormat];

      // Rendering to cubemap is not implemented.
      ImportGlTexImage2D(GL_TEXTURE_2D,
                   0,
                   texEnums.mInternalFormat,
                   info->mWidth,
                   info->mHeight,
                   texEnums.mFormat,
                   texEnums.mType,
                   nullptr);
    }
    // Do not try to reallocate texture data if no new data is given.
    else if (info->mImageData != nullptr)
    {
      ZeroIfWebgl(WebglConvertTextureFormat(info));
      GlTextureEnums texEnums = gTextureEnums[info->mFormat];

      for (uint i = 0; i < info->mMipCount; ++i)
      {
        MipHeader* mipHeader = info->mMipHeaders + i;
        byte* mipData = info->mImageData + mipHeader->mDataOffset;

        if (info->mSubImage)
        {
          ErrorIf(mipHeader->mLevel != 0, "Sub-image uploading to lower mip levels is not supported.");
          uint xOffset = info->mXOffset;
          uint yOffset = info->mHeight - (mipHeader->mHeight + info->mYOffset);
          ImportGlTexSubImage2D(GlTextureFace((TextureFace::Enum)mipHeader->mFace),
                          mipHeader->mLevel,
                          xOffset,
                          yOffset,
                          mipHeader->mWidth,
                          mipHeader->mHeight,
                          texEnums.mFormat,
                          texEnums.mType,
                          mipData);
        }
        else
        {
          if (info->mCompression != TextureCompression::None)
            ImportGlCompressedTexImage2D(GlTextureFace((TextureFace::Enum)mipHeader->mFace),
                                   mipHeader->mLevel,
                                   GlInternalFormat(info->mCompression),
                                   mipHeader->mWidth,
                                   mipHeader->mHeight,
                                   mipHeader->mDataSize,
                                   mipData);
          else
            ImportGlTexImage2D(GlTextureFace((TextureFace::Enum)mipHeader->mFace),
                         mipHeader->mLevel,
                         texEnums.mInternalFormat,
                         mipHeader->mWidth,
                         mipHeader->mHeight,
                         texEnums.mFormat,
                         texEnums.mType,
                         mipData);
        }
      }
    }

    ImportGlTexParameteri(GlTextureType(info->mType), GL_TEXTURE_WRAP_S, GlTextureAddressing(info->mAddressingX));
    ImportGlTexParameteri(GlTextureType(info->mType), GL_TEXTURE_WRAP_T, GlTextureAddressing(info->mAddressingY));
    ImportGlTexParameteri(GlTextureType(info->mType), GL_TEXTURE_MIN_FILTER, GlTextureFilteringMin(info->mFiltering));
    ImportGlTexParameteri(GlTextureType(info->mType), GL_TEXTURE_MAG_FILTER, GlTextureFilteringMag(info->mFiltering));
    ImportGlTexParameteri(GlTextureType(info->mType), GL_TEXTURE_COMPARE_MODE, GlCompareMode(info->mCompareMode));
    ImportGlTexParameteri(GlTextureType(info->mType), GL_TEXTURE_COMPARE_FUNC, GlCompareFunc(info->mCompareFunc));
    ImportGlTexParameterf(GlTextureType(info->mType), GL_TEXTURE_MAX_ANISOTROPY, GlTextureAnisotropy(info->mAnisotropy));
    ImportGlTexParameteri(GlTextureType(info->mType), GL_TEXTURE_MAX_LEVEL, GlTextureMipMapping(info->mMipMapping));

    if (info->mMipMapping == TextureMipMapping::GpuGenerated)
    {
      if (info->mMaxMipOverride > 0)
        ImportGlTexParameteri(GlTextureType(info->mType), GL_TEXTURE_MAX_LEVEL, info->mMaxMipOverride);
      ImportGlGenerateMipmap(GlTextureType(info->mType));
    }

    ImportGlBindTexture(GlTextureType(info->mType), 0);
  }

  renderData->mType = info->mType;
  renderData->mFormat = info->mFormat;
  renderData->mWidth = info->mWidth;
  renderData->mHeight = info->mHeight;

  renderData->mSamplerSettings = 0;
  renderData->mSamplerSettings |= SamplerSettings::AddressingX(info->mAddressingX);
  renderData->mSamplerSettings |= SamplerSettings::AddressingY(info->mAddressingY);
  renderData->mSamplerSettings |= SamplerSettings::Filtering(info->mFiltering);
  renderData->mSamplerSettings |= SamplerSettings::CompareMode(info->mCompareMode);
  renderData->mSamplerSettings |= SamplerSettings::CompareFunc(info->mCompareFunc);

  delete[] info->mImageData;
  delete[] info->mMipHeaders;
}

void OpenglRenderer::RemoveMaterial(MaterialRenderData* data)
{
  mMaterialRenderDataToDestroy.PushBack((GlMaterialRenderData*)data);
}

void OpenglRenderer::RemoveMesh(MeshRenderData* data)
{
  mMeshRenderDataToDestroy.PushBack((GlMeshRenderData*)data);
}

void OpenglRenderer::RemoveTexture(TextureRenderData* data)
{
  mTextureRenderDataToDestroy.PushBack((GlTextureRenderData*)data);
}

bool OpenglRenderer::GetLazyShaderCompilation()
{
  return mLazyShaderCompilation;
}

void OpenglRenderer::SetLazyShaderCompilation(bool isLazy)
{
  mLazyShaderCompilation = isLazy;
}

void OpenglRenderer::AddShaders(Array<ShaderEntry>& entries, uint forceCompileBatchCount)
{
  if (mLazyShaderCompilation && forceCompileBatchCount == 0)
  {
    forRange (ShaderEntry& entry, entries.All())
    {
      ShaderKey shaderKey(entry.mComposite, StringPair(entry.mCoreVertex, entry.mRenderPass));
      mShaderEntries.Insert(shaderKey, entry);
    }
    entries.Clear();
  }
  else
  {
    uint processCount = Math::Min(forceCompileBatchCount, (uint)entries.Size());
    if (processCount == 0)
      processCount = entries.Size();

    for (uint i = 0; i < processCount; ++i)
    {
      ShaderEntry& entry = entries[i];
      ShaderKey shaderKey(entry.mComposite, StringPair(entry.mCoreVertex, entry.mRenderPass));
      mShaderEntries.Erase(shaderKey);
      CreateShader(entry);
    }

    entries.Erase(entries.SubRange(0, processCount));
  }
}

void OpenglRenderer::RemoveShaders(Array<ShaderEntry>& entries)
{
  forRange (ShaderEntry& entry, entries)
  {
    ShaderKey shaderKey(entry.mComposite, StringPair(entry.mCoreVertex, entry.mRenderPass));

    if (mGlShaders.ContainsKey(shaderKey))
      ImportGlDeleteProgram(mGlShaders[shaderKey].mId);

    mGlShaders.Erase(shaderKey);
    mShaderEntries.Erase(shaderKey);
  }
}

void OpenglRenderer::SetVSync(bool vsync)
{
  int swapInterval = vsync ? 1 : 0;
  zglSetSwapInterval(this, swapInterval);
  mVsync = vsync;
}

void OpenglRenderer::GetTextureData(GetTextureDataInfo* info)
{
  GlTextureRenderData* renderData = (GlTextureRenderData*)info->mRenderData;
  info->mImage = nullptr;
  if (!IsColorFormat(renderData->mFormat))
    return;

  info->mWidth = renderData->mWidth;
  info->mHeight = renderData->mHeight;
  if (info->mWidth == 0 || info->mHeight == 0)
    return;

  if (IsFloatColorFormat(renderData->mFormat))
    info->mFormat = TextureFormat::RGB32f;
  else if (IsShortColorFormat(renderData->mFormat))
    info->mFormat = TextureFormat::RGBA16;
  else
    info->mFormat = TextureFormat::RGBA8;

  SetSingleRenderTargets(mSingleTargetFbo, &info->mRenderData, nullptr);

  uint imageSize = info->mWidth * info->mHeight * GetPixelSize(info->mFormat);
  info->mImage = new byte[imageSize];

  GlTextureEnums textureEnums = gTextureEnums[info->mFormat];
  ImportGlReadPixels(0, 0, info->mWidth, info->mHeight, textureEnums.mFormat, textureEnums.mType, info->mImage);

  YInvertNonCompressed(info->mImage, info->mWidth, info->mHeight, GetPixelSize(info->mFormat));

  ImportGlBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenglRenderer::ShowProgress(ShowProgressInfo* info)
{
  // Get data off info (we don't technically need to copy this anymore or lock)
  GlTextureRenderData* loadingTexture = (GlTextureRenderData*)info->mLoadingTexture;
  GlTextureRenderData* logoTexture = (GlTextureRenderData*)info->mLogoTexture;
  GlTextureRenderData* whiteTexture = (GlTextureRenderData*)info->mWhiteTexture;
  GlTextureRenderData* splashTexture = (GlTextureRenderData*)info->mSplashTexture;
  GlTextureRenderData* fontTexture = (GlTextureRenderData*)info->mFontTexture;
  uint logoFrameSize = info->mLogoFrameSize;
  Array<StreamedVertex> progressText = info->mProgressText;
  int progressWidth = info->mProgressWidth;
  float currentPercent = info->mCurrentPercent;
  double time = info->mTimer.Time();
  bool splashMode = info->mSplashMode;
  float alpha = splashMode ? info->mSplashFade : 1.0f;

  IntVec2 size = zglGetWindowRenderableSize(this);

  Mat4 viewportToNdc;
  viewportToNdc.BuildTransform(Vec3(-1.0f, 1.0f, 0.0f), Mat3::cIdentity, Vec3(2.0f / size.x, -2.0f / size.y, 1.0f));

  // Logo uv transform for animating
  uint xFrames = logoTexture->mWidth / logoFrameSize;
  uint yFrames = logoTexture->mHeight / logoFrameSize;
  double framesPerSecond = 30.0;
  uint frame = (uint)(time * framesPerSecond) % (xFrames * yFrames);
  Vec2 logoUvScale = Vec2(1.0f / xFrames, 1.0f / yFrames);
  Vec2 logoUvTranslate = Vec2((float)(frame % xFrames), (float)(frame / xFrames)) * logoUvScale;
  Mat3 logoUvTransform;
  logoUvTransform.BuildTransform(logoUvTranslate, 0.0f, logoUvScale);

  // Object transforms
  Vec3 logoScale = Vec3((float)logoFrameSize, (float)logoFrameSize, 1.0f);
  Vec3 logoTranslation = Vec3((size.x - loadingTexture->mWidth + logoScale.x) * 0.5f, size.y * 0.5f, 0.0f);
  Mat4 logoTransform;
  logoTransform.BuildTransform(logoTranslation, Mat3::cIdentity, logoScale);
  logoTransform = viewportToNdc * logoTransform;

  Vec3 loadingScale = Vec3((float)loadingTexture->mWidth, (float)loadingTexture->mHeight, 1.0f);
  Vec3 loadingTranslation = Vec3(size.x * 0.5f, size.y * 0.5f, 0.0f);
  Mat4 loadingTransform;
  loadingTransform.BuildTransform(loadingTranslation, Mat3::cIdentity, loadingScale);
  loadingTransform = viewportToNdc * loadingTransform;

  Vec3 progressScale = Vec3(progressWidth * currentPercent, 20.0f, 1.0f);
  Vec3 progressTranslation =
      Vec3((size.x - loadingScale.x + progressScale.x) * 0.5f, (size.y + loadingScale.y) * 0.5f + 40.0f, 0.0f);
  Mat4 progressTransform;
  progressTransform.BuildTransform(progressTranslation, Mat3::cIdentity, progressScale);
  progressTransform = viewportToNdc * progressTransform;

  Vec3 textScale = Vec3(1.0f);
  Vec3 textTranslation = Vec3((size.x - loadingScale.x) * 0.5f, (size.y + loadingScale.y) * 0.5f + 5.0f, 0.0f);
  Mat4 textTransform;
  textTransform.BuildTransform(textTranslation, Mat3::cIdentity, textScale);
  textTransform = viewportToNdc * textTransform;

  Vec3 splashScale = Vec3((float)splashTexture->mWidth, (float)splashTexture->mHeight, 1.0f);
  if (size.x < splashScale.x)
    splashScale *= size.x / splashScale.x;
  if (size.y < splashScale.y)
    splashScale *= size.y / splashScale.y;
  Vec3 splashTranslation = Vec3(size.x * 0.5f, size.y * 0.5f, 0.0f);
  Mat4 splashTransform;
  splashTransform.BuildTransform(splashTranslation, Mat3::cIdentity, splashScale);
  splashTransform = viewportToNdc * splashTransform;

  StreamedVertex quadVertices[] = {
      {Vec3(-0.5f, -0.5f, 0.0f), Vec2(0.0f, 0.0f), Vec4(1.0f)},
      {Vec3(-0.5f, 0.5f, 0.0f), Vec2(0.0f, 1.0f), Vec4(1.0f)},
      {Vec3(0.5f, 0.5f, 0.0f), Vec2(1.0f, 1.0f), Vec4(1.0f)},
      {Vec3(0.5f, 0.5f, 0.0f), Vec2(1.0f, 1.0f), Vec4(1.0f)},
      {Vec3(0.5f, -0.5f, 0.0f), Vec2(1.0f, 0.0f), Vec4(1.0f)},
      {Vec3(-0.5f, -0.5f, 0.0f), Vec2(0.0f, 0.0f), Vec4(1.0f)},
  };

  ImportGlClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  ImportGlClear(GL_COLOR_BUFFER_BIT);

  ImportGlUseProgram(mLoadingShader);
  ImportGlEnable(GL_BLEND);
  ImportGlBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLint textureLoc = ImportGlGetUniformLocation(mLoadingShader, "Texture");
  GLint transformLoc = ImportGlGetUniformLocation(mLoadingShader, "Transform");
  GLint uvTransformLoc = ImportGlGetUniformLocation(mLoadingShader, "UvTransform");

  GLint alphaLoc = ImportGlGetUniformLocation(mLoadingShader, "Alpha");
  ImportGlUniform1fv(alphaLoc, 1, &alpha);

  GLint textureSlot = 0;
  ImportGlActiveTexture(GL_TEXTURE0 + textureSlot);
  ImportGlUniform1iv(textureLoc, 1, &textureSlot);

  ImportGlUniformMatrix3fv(uvTransformLoc, 1, cTransposeMatrices, Mat3::cIdentity.array);

  if (!splashMode)
  {
    // Loading
    ImportGlUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, loadingTransform.array);
    ImportGlBindTexture(GL_TEXTURE_2D, loadingTexture->mId);
    mStreamedVertexBuffer.AddVertices(quadVertices, 6, PrimitiveType::Triangles);
    mStreamedVertexBuffer.FlushBuffer(true);

    // Progress bar
    ImportGlUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, progressTransform.array);
    ImportGlBindTexture(GL_TEXTURE_2D, whiteTexture->mId);
    mStreamedVertexBuffer.AddVertices(quadVertices, 6, PrimitiveType::Triangles);
    mStreamedVertexBuffer.FlushBuffer(true);

    // Progress text
    if (progressText.Size() > 0)
    {
      ImportGlUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, textTransform.array);
      ImportGlBindTexture(GL_TEXTURE_2D, fontTexture->mId);
      mStreamedVertexBuffer.AddVertices(&progressText[0], progressText.Size(), PrimitiveType::Triangles);
      mStreamedVertexBuffer.FlushBuffer(true);
    }

    // Logo
    ImportGlUniformMatrix3fv(uvTransformLoc, 1, cTransposeMatrices, logoUvTransform.array);
    ImportGlUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, logoTransform.array);
    ImportGlBindTexture(GL_TEXTURE_2D, logoTexture->mId);
    mStreamedVertexBuffer.AddVertices(quadVertices, 6, PrimitiveType::Triangles);
    mStreamedVertexBuffer.FlushBuffer(true);
  }
  else
  {
    // Splash
    ImportGlUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, splashTransform.array);
    ImportGlBindTexture(GL_TEXTURE_2D, splashTexture->mId);
    mStreamedVertexBuffer.AddVertices(quadVertices, 6, PrimitiveType::Triangles);
    mStreamedVertexBuffer.FlushBuffer(true);
  }

  ImportGlDisable(GL_BLEND);
  ImportGlUseProgram(0);

  // Disable v-sync so we don't wait on frames (mostly for single threaded mode)
  // This could cause tearing, but it's the loading screen.
  zglSetSwapInterval(this, 0);

  zglSwapBuffers(this);

  int swapInterval = mVsync ? 1 : 0;
  zglSetSwapInterval(this, swapInterval);
}

GlShader* OpenglRenderer::GetShader(ShaderKey& shaderKey)
{
  // Check if new shader is pending
  if (mShaderEntries.ContainsKey(shaderKey))
  {
    ShaderEntry& entry = mShaderEntries[shaderKey];

    CreateShader(entry);
    mShaderEntries.Erase(shaderKey);
  }

  return mGlShaders.FindPointer(shaderKey);
}

void OpenglRenderer::DoRenderTasks(RenderTasks* renderTasks, RenderQueues* renderQueues)
{
  mRenderTasks = renderTasks;
  mRenderQueues = renderQueues;

  forRange (RenderTaskRange& taskRange, mRenderTasks->mRenderTaskRanges.All())
    DoRenderTaskRange(taskRange);

  zglSwapBuffers(this);

  DelayedRenderDataDestruction();

  DestroyUnusedSamplers();
}

void OpenglRenderer::DoRenderTaskRange(RenderTaskRange& taskRange)
{
  mFrameBlock = &mRenderQueues->mFrameBlocks[taskRange.mFrameBlockIndex];
  mViewBlock = &mRenderQueues->mViewBlocks[taskRange.mViewBlockIndex];

  uint taskIndex = taskRange.mTaskIndex;
  for (uint i = 0; i < taskRange.mTaskCount; ++i)
  {
    ErrorIf(taskIndex >= mRenderTasks->mRenderTaskBuffer.mCurrentIndex, "Render task data is not valid.");
    RenderTask* task = (RenderTask*)&mRenderTasks->mRenderTaskBuffer.mRenderTaskData[taskIndex];

    switch (task->mId)
    {
    case RenderTaskType::ClearTarget:
      DoRenderTaskClearTarget((RenderTaskClearTarget*)task);
      taskIndex += sizeof(RenderTaskClearTarget);
      break;

    case RenderTaskType::RenderPass:
    {
      RenderTaskRenderPass* renderPass = (RenderTaskRenderPass*)task;
      DoRenderTaskRenderPass(renderPass);
      // RenderPass tasks can have multiple following task entries for sub
      // RenderGroup settings. Have to index past all sub tasks.
      taskIndex += sizeof(RenderTaskRenderPass) * (renderPass->mSubRenderGroupCount + 1);
      i += renderPass->mSubRenderGroupCount;
    }
    break;

    case RenderTaskType::PostProcess:
      DoRenderTaskPostProcess((RenderTaskPostProcess*)task);
      taskIndex += sizeof(RenderTaskPostProcess);
      break;

    case RenderTaskType::BackBufferBlit:
      DoRenderTaskBackBufferBlit((RenderTaskBackBufferBlit*)task);
      taskIndex += sizeof(RenderTaskBackBufferBlit);
      break;

    case RenderTaskType::TextureUpdate:
      DoRenderTaskTextureUpdate((RenderTaskTextureUpdate*)task);
      taskIndex += sizeof(RenderTaskTextureUpdate);
      break;

    default:
      Error("Render task not implemented.");
      break;
    }
  }
}

void OpenglRenderer::DoRenderTaskClearTarget(RenderTaskClearTarget* task)
{
  SetRenderTargets(task->mRenderSettings);

  ImportGlStencilMask(task->mStencilWriteMask);
  ImportGlDepthMask(true);

  ImportGlClearColor(task->mColor.x, task->mColor.y, task->mColor.z, task->mColor.w);
  ImportGlClearDepth(task->mDepth);
  ImportGlClearStencil(0);
  ImportGlClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  ImportGlStencilMask(0);
  ImportGlDepthMask(false);

  ImportGlBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenglRenderer::DoRenderTaskRenderPass(RenderTaskRenderPass* task)
{
  // Create a map of RenderGroup id to task memory index for every sub group
  // entry.
  HashMap<int, size_t> taskIndexMap;
  while (taskIndexMap.Size() < task->mSubRenderGroupCount)
  {
    size_t index = taskIndexMap.Size() + 1;
    RenderTaskRenderPass* subTask = task + index;
    taskIndexMap.InsertOrError(subTask->mRenderGroupIndex, index);
  }

  // Initialize to invalid index so state is set for the first object.
  int currentTaskIndex = -1;

  // All ViewNodes under the base RenderGroup.
  IndexRange viewNodeRange = mViewBlock->mRenderGroupRanges[task->mRenderGroupIndex];

  for (uint i = viewNodeRange.start; i < viewNodeRange.end; ++i)
  {
    ViewNode& viewNode = mViewBlock->mViewNodes[i];
    FrameNode& frameNode = mFrameBlock->mFrameNodes[viewNode.mFrameNodeIndex];

    // Get the index for this object's RenderGroup settings. Always default to
    // the base task entry.
    size_t index = taskIndexMap.FindValue(viewNode.mRenderGroupId, 0);

    // Sub RenderGroups have unique render settings when a different task index
    // is encountered. Or this is just the first set.
    if (index != currentTaskIndex)
    {
      // Offsets to sub RenderGroup settings or just the base task.
      RenderTaskRenderPass* subTask = task + index;

      // Different RenderPass tasks are also made to denote RenderGroups to not
      // render. Don't change state or render the object.
      if (subTask->mRender == false)
        continue;

      currentTaskIndex = index;

      // Flush potential pending draw call before changing state.
      mStreamedVertexBuffer.FlushBuffer(true);

      mViewportSize = IntVec2(subTask->mRenderSettings.mTargetsWidth, subTask->mRenderSettings.mTargetsHeight);

      mShaderInputsId = subTask->mShaderInputsId;
      mRenderPassName = subTask->mRenderPassName;

      SetRenderSettings(subTask->mRenderSettings, mDriverSupport.mMultiTargetBlend);
      mClipMode = subTask->mRenderSettings.mScissorMode == ScissorMode::Enabled;

      // For easily resetting blend settings after overriding.
      mCurrentBlendSettings = subTask->mRenderSettings.mBlendSettings[0];

      SetRenderTargets(subTask->mRenderSettings);

      ImportGlViewport(0, 0, mViewportSize.x, mViewportSize.y);
    }

    // Render the object.
    switch (frameNode.mRenderingType)
    {
    case RenderingType::Static:
      mStreamedVertexBuffer.FlushBuffer(true);
      DrawStatic(viewNode, frameNode);
      break;

    case RenderingType::Streamed:
      DrawStreamed(viewNode, frameNode);
      break;
    }
  }

  mStreamedVertexBuffer.FlushBuffer(true);
  mActiveTexture = 0;
  mActiveMaterial = 0;
  mClipMode = false;
  mCurrentClip = Vec4(0, 0, 0, 0);

  SetShader(0);
  SetRenderSettings(RenderSettings(), mDriverSupport.mMultiTargetBlend);
  ImportGlBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenglRenderer::DoRenderTaskPostProcess(RenderTaskPostProcess* task)
{
  mViewportSize = IntVec2(task->mRenderSettings.mTargetsWidth, task->mRenderSettings.mTargetsHeight);
  if (mViewportSize.x == 0 || mViewportSize.y == 0)
    return;

  GlMaterialRenderData* materialData = (GlMaterialRenderData*)task->mMaterialRenderData;
  if (materialData == nullptr && task->mPostProcessName.Empty() == true)
    return;

  String compositeName = materialData ? materialData->mCompositeName : task->mPostProcessName;
  u64 resourceId = materialData ? (u64)materialData->mResourceId : cFragmentShaderInputsId;

  ShaderKey shaderKey(compositeName, StringPair(cPostVertex, String()));
  GlShader* shader = GetShader(shaderKey);
  if (shader == nullptr)
    return;

  SetRenderTargets(task->mRenderSettings);
  SetRenderSettings(task->mRenderSettings, mDriverSupport.mMultiTargetBlend);

  ImportGlViewport(0, 0, mViewportSize.x, mViewportSize.y);

  SetShader(shader->mId);

  SetShaderParameters(mFrameBlock, mViewBlock);

  // Set Material or PostProcess fragment parameters
  mNextTextureSlot = 0;
  SetShaderParameters(resourceId, task->mShaderInputsId, mNextTextureSlot);
  SetShaderParameters(cGlobalShaderInputsId, task->mShaderInputsId, mNextTextureSlot);

  // draw fullscreen triangle
  ImportGlBindVertexArray(mTriangleArray);
  ImportGlDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)0);
  ImportGlBindVertexArray(0);

  SetShader(0);
  SetRenderSettings(RenderSettings(), mDriverSupport.mMultiTargetBlend);
  ImportGlBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenglRenderer::DoRenderTaskBackBufferBlit(RenderTaskBackBufferBlit* task)
{
  GlTextureRenderData* renderData = (GlTextureRenderData*)task->mColorTarget;
  ScreenViewport viewport = task->mViewport;

  ImportGlBindFramebuffer(GL_READ_FRAMEBUFFER, mSingleTargetFbo);
  ImportGlFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderData->mId, 0);
  ImportGlFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
  ImportGlFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
  CheckFramebufferStatus();

  mThreadLock.Lock();
  if (mBackBufferSafe)
    ImportGlBlitFramebuffer(0,
                      0,
                      renderData->mWidth,
                      renderData->mHeight,
                      viewport.x,
                      viewport.y,
                      viewport.x + viewport.width,
                      viewport.y + viewport.height,
                      GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
  mThreadLock.Unlock();

  ImportGlBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void OpenglRenderer::DoRenderTaskTextureUpdate(RenderTaskTextureUpdate* task)
{
  AddTextureInfo info;
  info.mRenderData = task->mRenderData;
  info.mWidth = task->mWidth;
  info.mHeight = task->mHeight;
  info.mType = task->mType;
  info.mFormat = task->mFormat;
  info.mAddressingX = task->mAddressingX;
  info.mAddressingY = task->mAddressingY;
  info.mFiltering = task->mFiltering;
  info.mCompareMode = task->mCompareMode;
  info.mCompareFunc = task->mCompareFunc;
  info.mAnisotropy = task->mAnisotropy;
  info.mMipMapping = task->mMipMapping;

  info.mMipCount = 0;
  info.mTotalDataSize = 0;
  info.mImageData = nullptr;
  info.mMipHeaders = nullptr;

  AddTexture(&info);
}

void OpenglRenderer::SetRenderTargets(RenderSettings& renderSettings)
{
  if (renderSettings.mSingleColorTarget)
    SetSingleRenderTargets(mSingleTargetFbo, renderSettings.mColorTargets, renderSettings.mDepthTarget);
  else
    SetMultiRenderTargets(mMultiTargetFbo, renderSettings.mColorTargets, renderSettings.mDepthTarget);
}

void OpenglRenderer::DrawStatic(ViewNode& viewNode, FrameNode& frameNode)
{
  GlMeshRenderData* meshData = (GlMeshRenderData*)frameNode.mMeshRenderData;
  GlMaterialRenderData* materialData = (GlMaterialRenderData*)frameNode.mMaterialRenderData;
  if (meshData == nullptr || materialData == nullptr)
    return;

  // Shader permutation lookup for vertex type and render pass
  ShaderKey shaderKey(materialData->mCompositeName,
                      StringPair(GetCoreVertexFragmentName(frameNode.mCoreVertexType), mRenderPassName));
  GlShader* shader = GetShader(shaderKey);
  if (shader == nullptr)
    return;

  if (shader->mId != mActiveShader)
  {
    SetShader(shader->mId);
    // Set non-object built-in inputs once per active shader
    SetShaderParameters(mFrameBlock, mViewBlock);
    mActiveMaterial = 0;
  }

  // Per object built-in inputs
  SetShaderParameters(&frameNode, &viewNode);

  // Set RenderPass inputs once on new shader or if a reset is triggered
  if (mActiveMaterial == 0)
  {
    mNextTextureSlot = 0;
    SetShaderParameters(cFragmentShaderInputsId, mShaderInputsId, mNextTextureSlot);
  }

  // On change of materials, material inputs followed by ImportGlobal inputs have to
  // be reset
  if (materialData->mResourceId != mActiveMaterial)
  {
    mNextTextureSlotMaterial = mNextTextureSlot;
    SetShaderParameters((u64)materialData->mResourceId, mShaderInputsId, mNextTextureSlotMaterial);
    SetShaderParameters(cGlobalShaderInputsId, mShaderInputsId, mNextTextureSlotMaterial);

    mActiveMaterial = (u64)materialData->mResourceId;
  }

  // Don't need to use a permanent texture slot
  uint textureSlot = mNextTextureSlotMaterial;

  // Per object shader inputs
  if (frameNode.mShaderInputRange.Count() != 0)
  {
    SetShaderParameters(frameNode.mShaderInputRange, textureSlot);
    // Since object input overrides could apply to other fragments that aren't
    // from the material (i.e. RenderPass) have to trigger a reset of all
    // inputs, done by resetting active material, but shader does not have to be
    // reset
    mActiveMaterial = 0;
  }

  GlTextureRenderData* textureData = (GlTextureRenderData*)frameNode.mTextureRenderData;
  if (textureData != nullptr)
  {
    BindTexture(TextureType::Texture2D, textureSlot, textureData->mId, mDriverSupport.mSamplerObjects);
    SetShaderParameter(ShaderInputType::Texture, "HeightMapWeights_HeightMapTextureWeights", &textureSlot);
  }

  ImportGlBindVertexArray(meshData->mVertexArray);
  if (meshData->mIndexBuffer == 0)
    // If nothing is bound, ImportGlDrawArrays will invoke the shader pipeline the
    // given number of times
    ImportGlDrawArrays(GlPrimitiveType(meshData->mPrimitiveType), 0, meshData->mIndexCount);
  else
    ImportGlDrawElements(GlPrimitiveType(meshData->mPrimitiveType), meshData->mIndexCount, GL_UNSIGNED_INT, (void*)0);
  ImportGlBindVertexArray(0);
}

void OpenglRenderer::DrawStreamed(ViewNode& viewNode, FrameNode& frameNode)
{
  GlMaterialRenderData* materialData = (GlMaterialRenderData*)frameNode.mMaterialRenderData;
  if (materialData == nullptr)
    return;

  // Shader permutation lookup for vertex type and render pass
  ShaderKey shaderKey(materialData->mCompositeName,
                      StringPair(GetCoreVertexFragmentName(frameNode.mCoreVertexType), mRenderPassName));
  GlShader* shader = GetShader(shaderKey);
  if (shader == nullptr)
    return;

  if (viewNode.mStreamedVertexCount == 0)
    return;

  u64 materialId = materialData->mResourceId;
  GLuint shaderId = shader->mId;
  GLuint textureId = 0;

  GlTextureRenderData* textureData = (GlTextureRenderData*)frameNode.mTextureRenderData;
  if (textureData != nullptr)
    textureId = textureData->mId;

  if (mCurrentLineWidth != frameNode.mBorderThickness)
  {
    mCurrentLineWidth = frameNode.mBorderThickness;
    mStreamedVertexBuffer.FlushBuffer(false);
    ImportGlLineWidth(frameNode.mBorderThickness);
  }

  if (mClipMode && frameNode.mClip != mCurrentClip)
  {
    mStreamedVertexBuffer.FlushBuffer(false);
    mCurrentClip = frameNode.mClip;
    ImportGlScissor((int)mCurrentClip.x,
              mViewportSize.y - (int)mCurrentClip.y - (int)mCurrentClip.w,
              (int)mCurrentClip.z,
              (int)mCurrentClip.w);
  }

  // Check for any state change
  if (shaderId != mActiveShader || textureId != mActiveTexture || materialId != mActiveMaterial)
  {
    mStreamedVertexBuffer.FlushBuffer(false);

    SetShader(shaderId);
    mActiveTexture = textureId;
    mActiveMaterial = materialId;

    // Set non-object data once per active shader
    SetShaderParameters(mFrameBlock, mViewBlock);
    // Set RenderPass fragment parameters
    mNextTextureSlot = 0;
    SetShaderParameters(cFragmentShaderInputsId, mShaderInputsId, mNextTextureSlot);

    SetShaderParameters((u64)materialData->mResourceId, mShaderInputsId, mNextTextureSlot);
    SetShaderParameters(cGlobalShaderInputsId, mShaderInputsId, mNextTextureSlot);

    if (textureId != 0)
    {
      BindTexture(textureData->mType, mNextTextureSlot, textureId, mDriverSupport.mSamplerObjects);
      if (textureData->mType == TextureType::TextureCube)
        SetShaderParameter(ShaderInputType::Texture, cSpriteSourceCubePreview, &mNextTextureSlot);
      else
        SetShaderParameter(ShaderInputType::Texture, cSpriteSource, &mNextTextureSlot);
      ++mNextTextureSlot;
    }
  }

  // Have to force an independent draw call if object has individual shader
  // inputs
  if (frameNode.mShaderInputRange.Count() != 0)
  {
    mStreamedVertexBuffer.FlushBuffer(false);
    SetShaderParameters(frameNode.mShaderInputRange, mNextTextureSlot);
    mActiveMaterial = 0;
  }

  if (frameNode.mBlendSettingsOverride)
  {
    // Only overrides for target 0, temporary functionality for viewports
    mStreamedVertexBuffer.FlushBuffer(false);
    SetBlendSettings(mRenderQueues->mBlendSettingsOverrides[frameNode.mBlendSettingsIndex]);
  }

  uint vertexStart = viewNode.mStreamedVertexStart;
  uint vertexCount = viewNode.mStreamedVertexCount;
  mStreamedVertexBuffer.AddVertices(
      mRenderQueues->mStreamedVertices, vertexStart, vertexCount, viewNode.mStreamedVertexType);

  if (frameNode.mBlendSettingsOverride)
  {
    mStreamedVertexBuffer.FlushBuffer(false);
    SetBlendSettings(mCurrentBlendSettings);
  }
}

void OpenglRenderer::SetShaderParameter(ShaderInputType::Enum uniformType, StringParam name, void* data)
{
  GLint location = ImportGlGetUniformLocation(mActiveShader, name.c_str());
  if (location == -1)
    return;
  mUniformFunctions[uniformType](location, 1, data);
}

void OpenglRenderer::SetShaderParameterMatrix(StringParam name, Mat3& transform)
{
  GLint location = ImportGlGetUniformLocation(mActiveShader, name.c_str());
  if (location == -1)
    return;
  ImportGlUniformMatrix3fv(location, 1, cTransposeMatrices, transform.array);
}

void OpenglRenderer::SetShaderParameterMatrix(StringParam name, Mat4& transform)
{
  GLint location = ImportGlGetUniformLocation(mActiveShader, name.c_str());
  if (location == -1)
    return;
  ImportGlUniformMatrix4fv(location, 1, cTransposeMatrices, transform.array);
}

void OpenglRenderer::SetShaderParameterMatrixInv(StringParam name, Mat3& transform)
{
  GLint location = ImportGlGetUniformLocation(mActiveShader, name.c_str());
  if (location == -1)
    return;
  Mat3 inverse = transform.Inverted();
  ImportGlUniformMatrix3fv(location, 1, cTransposeMatrices, inverse.array);
}

void OpenglRenderer::SetShaderParameterMatrixInv(StringParam name, Mat4& transform)
{
  GLint location = ImportGlGetUniformLocation(mActiveShader, name.c_str());
  if (location == -1)
    return;

  Mat4 inverse = transform.Inverted();
  ImportGlUniformMatrix4fv(location, 1, cTransposeMatrices, inverse.array);
}

void OpenglRenderer::SetShaderParameters(FrameBlock* frameBlock, ViewBlock* viewBlock)
{
  SetShaderParameter(ShaderInputType::Float, cFrameTime, &frameBlock->mFrameTime);
  SetShaderParameter(ShaderInputType::Float, cLogicTime, &frameBlock->mLogicTime);

  SetShaderParameterMatrix(cWorldToView, viewBlock->mWorldToView);
  SetShaderParameterMatrix(cViewToPerspective, viewBlock->mViewToPerspective);
  SetShaderParameterMatrix(cZeroPerspectiveToApiPerspective, viewBlock->mZeroPerspectiveToApiPerspective);
  SetShaderParameterMatrixInv(cViewToWorld, viewBlock->mWorldToView);
  SetShaderParameterMatrixInv(cPerspectiveToView, viewBlock->mViewToPerspective);

  SetShaderParameter(ShaderInputType::Float, cNearPlane, &viewBlock->mNearPlane);
  SetShaderParameter(ShaderInputType::Float, cFarPlane, &viewBlock->mFarPlane);
  SetShaderParameter(ShaderInputType::Vec2, cViewportSize, viewBlock->mViewportSize.array);
  SetShaderParameter(ShaderInputType::Vec2, cInverseViewportSize, viewBlock->mInverseViewportSize.array);
}

void OpenglRenderer::SetShaderParameters(FrameNode* frameNode, ViewNode* viewNode)
{
  SetShaderParameterMatrix(cLocalToWorld, frameNode->mLocalToWorld);
  SetShaderParameterMatrix(cLocalToWorldNormal, frameNode->mLocalToWorldNormal);
  SetShaderParameterMatrixInv(cWorldToLocal, frameNode->mLocalToWorld);
  SetShaderParameterMatrixInv(cWorldToLocalNormal, frameNode->mLocalToWorldNormal);

  SetShaderParameter(ShaderInputType::Vec3, cObjectWorldPosition, frameNode->mObjectWorldPosition.array);

  uint boneCount = frameNode->mBoneMatrixRange.Count();
  uint remapCount = frameNode->mIndexRemapRange.Count();
  if (boneCount > 0 && remapCount > 0)
  {
    GlMeshRenderData* meshData = (GlMeshRenderData*)frameNode->mMeshRenderData;

    Array<Mat4> remappedBoneTransforms;
    for (uint i = frameNode->mIndexRemapRange.start; i < frameNode->mIndexRemapRange.end; ++i)
    {
      uint meshIndex = i - frameNode->mIndexRemapRange.start;
      uint bufferIndex = mRenderQueues->mIndexRemapBuffer[i] + frameNode->mBoneMatrixRange.start;

      remappedBoneTransforms.PushBack(mRenderQueues->mSkinningBuffer[bufferIndex] *
                                      meshData->mBones[meshIndex].mBindTransform);
    }

    GLint location = ImportGlGetUniformLocation(mActiveShader, "MiscData.BoneTransforms");
    if (location != -1)
    {
      ImportGlUniformMatrix4fv(location, remappedBoneTransforms.Size(), cTransposeMatrices, remappedBoneTransforms[0].array);
    }
  }

  SetShaderParameterMatrix(cLocalToView, viewNode->mLocalToView);
  SetShaderParameterMatrix(cLocalToViewNormal, viewNode->mLocalToViewNormal);
  SetShaderParameterMatrix(cLocalToPerspective, viewNode->mLocalToPerspective);
  SetShaderParameterMatrixInv(cViewToLocal, viewNode->mLocalToView);
  SetShaderParameterMatrixInv(cViewToLocalNormal, viewNode->mLocalToViewNormal);
}

void OpenglRenderer::SetShaderParameters(IndexRange inputRange, uint& nextTextureSlot)
{
  Array<ShaderInput>::range shaderInputs = mRenderTasks->mShaderInputs.SubRange(inputRange.start, inputRange.Count());

  forRange (ShaderInput& input, shaderInputs)
  {
    if (input.mShaderInputType == ShaderInputType::Texture)
    {
      GlTextureRenderData* textureData = *(GlTextureRenderData**)input.mValue;
      BindTexture(textureData->mType, nextTextureSlot, textureData->mId, mDriverSupport.mSamplerObjects);

      // Check for custom sampler settings for this input
      // If any sampler attributes were set on the shader input then
      // mSamplerSettings will be non-zero If driver does not have sampler
      // object support then this feature does nothing
      if (input.mSamplerSettings != 0 && mDriverSupport.mSamplerObjects)
      {
        u32 samplerSettings = input.mSamplerSettings;
        // Use texture settings as defaults so that only attributes specified on
        // shader input differ from the texture
        SamplerSettings::FillDefaults(samplerSettings, textureData->mSamplerSettings);
        GLuint sampler = GetSampler(samplerSettings);
        ImportGlBindSampler(nextTextureSlot, sampler);
      }

      SetShaderParameter(input.mShaderInputType, input.mTranslatedInputName, &nextTextureSlot);
      ++nextTextureSlot;
    }
    else if (input.mShaderInputType == ShaderInputType::Bool)
    {
      int value = *(bool*)input.mValue;
      SetShaderParameter(input.mShaderInputType, input.mTranslatedInputName, &value);
    }
    else if (input.mShaderInputType == ShaderInputType::Mat3)
    {
      SetShaderParameterMatrix(input.mTranslatedInputName, *(Mat3*)input.mValue);
    }
    else if (input.mShaderInputType == ShaderInputType::Mat4)
    {
      SetShaderParameterMatrix(input.mTranslatedInputName, *(Mat4*)input.mValue);
    }
    else
    {
      SetShaderParameter(input.mShaderInputType, input.mTranslatedInputName, input.mValue);
    }
  }
}

void OpenglRenderer::SetShaderParameters(u64 objectId, uint shaderInputsId, uint& nextTextureSlot)
{
  Pair<u64, uint> pair(objectId, shaderInputsId);
  IndexRange inputRange = mRenderTasks->mShaderInputRanges.FindValue(pair, IndexRange(0, 0));
  SetShaderParameters(inputRange, nextTextureSlot);
}

void OpenglRenderer::CreateShader(ShaderEntry& entry)
{
#ifdef ZeroDebug
  ZPrint(
      "Compiling shader: %s %s %s\n", entry.mCoreVertex.c_str(), entry.mComposite.c_str(), entry.mRenderPass.c_str());
#endif

  ShaderKey shaderKey(entry.mComposite, StringPair(entry.mCoreVertex, entry.mRenderPass));

  GLuint shaderId = 0;
  CreateShader(entry.mVertexShader, entry.mGeometryShader, entry.mPixelShader, shaderId);

  // Shouldn't fail at this point. Not currently handling gl errors.
  ErrorIf(shaderId == 0, "Failed to compile or link shader.");

  GlShader shader;
  shader.mId = shaderId;

  // Must delete old shader after new one is created or something is getting
  // incorrectly cached/generated
  if (mGlShaders.ContainsKey(shaderKey))
    ImportGlDeleteProgram(mGlShaders[shaderKey].mId);

  mGlShaders.Insert(shaderKey, shader);
}

void OpenglRenderer::CreateShader(StringParam vertexSource,
                                  StringParam geometrySource,
                                  StringParam pixelSource,
                                  GLuint& shader)
{
#ifdef ZeroDebug
  Timer compileTimer;
#endif

  GLuint program = ImportGlCreateProgram();

  const GLchar* vertexSourceData = vertexSource.Data();
  GLint vertexSourceSize = vertexSource.SizeInBytes();
  GLuint vertexShader = ImportGlCreateShader(GL_VERTEX_SHADER);
  ImportGlAttachShader(program, vertexShader);
  ImportGlShaderSource(vertexShader, vertexSourceData, vertexSourceSize);
  ImportGlCompileShader(vertexShader);
  CheckShader(vertexShader, vertexSource);

  GLuint geometryShader = 0;
  if (!geometrySource.Empty())
  {
    const GLchar* geometrySourceData = geometrySource.Data();
    GLint geometrySourceSize = geometrySource.SizeInBytes();
    geometryShader = ImportGlCreateShader(GL_GEOMETRY_SHADER);
    ImportGlAttachShader(program, geometryShader);
    ImportGlShaderSource(geometryShader, geometrySourceData, geometrySourceSize);
    ImportGlCompileShader(geometryShader);
    CheckShader(geometryShader, geometrySource);
  }

  const GLchar* pixelSourceData = pixelSource.Data();
  GLint pixelSourceSize = pixelSource.SizeInBytes();
  GLuint pixelShader = ImportGlCreateShader(GL_FRAGMENT_SHADER);
  ImportGlAttachShader(program, pixelShader);
  ImportGlShaderSource(pixelShader, pixelSourceData, pixelSourceSize);
  ImportGlCompileShader(pixelShader);
  CheckShader(pixelShader, pixelSource);

  ImportGlBindAttribLocation(program, VertexSemantic::Position, "LocalPosition");
  ImportGlBindAttribLocation(program, VertexSemantic::Normal, "LocalNormal");
  ImportGlBindAttribLocation(program, VertexSemantic::Tangent, "LocalTangent");
  ImportGlBindAttribLocation(program, VertexSemantic::Bitangent, "LocalBitangent");
  ImportGlBindAttribLocation(program, VertexSemantic::Uv, "Uv");
  ImportGlBindAttribLocation(program, VertexSemantic::UvAux, "UvAux");
  ImportGlBindAttribLocation(program, VertexSemantic::Color, "Color");
  ImportGlBindAttribLocation(program, VertexSemantic::ColorAux, "ColorAux");
  ImportGlBindAttribLocation(program, VertexSemantic::BoneIndices, "BoneIndices");
  ImportGlBindAttribLocation(program, VertexSemantic::BoneWeights, "BoneWeights");
  // Not implemented by geometry processor
  ImportGlBindAttribLocation(program, 10, "Aux0");
  ImportGlBindAttribLocation(program, 11, "Aux1");
  ImportGlBindAttribLocation(program, 12, "Aux2");
  ImportGlBindAttribLocation(program, 13, "Aux3");
  ImportGlBindAttribLocation(program, 14, "Aux4");
  ImportGlBindAttribLocation(program, 15, "Aux5");

#ifdef ZeroDebug
  double compileSeconds = compileTimer.UpdateAndGetTime();
  ZPrint("Compiled shader in %f seconds\n", compileSeconds);
#endif

#ifdef ZeroDebug
  Timer linkTimer;
#endif

  ImportGlLinkProgram(program);

#ifdef ZeroDebug
  double linkSeconds = linkTimer.UpdateAndGetTime();
  ZPrint("Linked shader in %f seconds\n", linkSeconds);
#endif

#ifdef ZeroDebug
  GLint status;
  ImportGlGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE)
  {
    GLint infoLogLength;
    ImportGlGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    GLchar* strInfoLog = (GLchar*)alloca(infoLogLength + 1);
    ImportGlGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
    ZPrint("Link Error\n%s\n", strInfoLog);

    static size_t sMaxPrints = 4;
    if (sMaxPrints > 0)
    {
      ZPrint("\n************************************************************"
             "VERTEX\n%s"
             "\n************************************************************"
             "GEOMETRY\n%s"
             "\n************************************************************"
             "PIXEL\n%s"
             "\n************************************************************\n",
             vertexSource.c_str(),
             geometrySource.c_str(),
             pixelSource.c_str());
      --sMaxPrints;
    }
  }
#endif

  // For now we always output the shader assuming that all of them compile and
  // link. This is because requesting the link status is a blocking operation.
  shader = program;

  ImportGlDetachShader(program, vertexShader);
  ImportGlDetachShader(program, pixelShader);
  ImportGlDeleteShader(vertexShader);
  ImportGlDeleteShader(pixelShader);

  if (geometryShader != 0)
  {
    ImportGlDetachShader(program, geometryShader);
    ImportGlDeleteShader(geometryShader);
  }

  // We don't currently do this because we don't want to check the status of the
  // shader (blocking).
  // if (status == GL_FALSE)
  //  ImportGlDeleteProgram(program);
}

void OpenglRenderer::SetShader(GLuint shader)
{
  mActiveShader = shader;
  ImportGlUseProgram(mActiveShader);
}

void OpenglRenderer::DelayedRenderDataDestruction()
{
  forRange (GlMaterialRenderData* renderData, mMaterialRenderDataToDestroy.All())
    DestroyRenderData(renderData);
  forRange (GlMeshRenderData* renderData, mMeshRenderDataToDestroy.All())
    DestroyRenderData(renderData);
  forRange (GlTextureRenderData* renderData, mTextureRenderDataToDestroy.All())
    DestroyRenderData(renderData);

  mMaterialRenderDataToDestroy.Clear();
  mMeshRenderDataToDestroy.Clear();
  mTextureRenderDataToDestroy.Clear();
}

void OpenglRenderer::DestroyRenderData(GlMaterialRenderData* renderData)
{
  delete renderData;
}

void OpenglRenderer::DestroyRenderData(GlMeshRenderData* renderData)
{
  ImportGlDeleteVertexArray(renderData->mVertexArray);
  ImportGlDeleteBuffer(renderData->mVertexBuffer);
  ImportGlDeleteBuffer(renderData->mIndexBuffer);

  delete renderData;
}

void OpenglRenderer::DestroyRenderData(GlTextureRenderData* renderData)
{
  ImportGlDeleteTexture(renderData->mId);
  delete renderData;
}

GLuint OpenglRenderer::GetSampler(u32 samplerSettings)
{
  // Sampler objects can be reused for any number of texture units
  if (mSamplers.ContainsKey(samplerSettings))
  {
    mUnusedSamplers.Erase(samplerSettings);
    return mSamplers[samplerSettings];
  }

  GLuint newSampler;
  ImportGlGenSamplers(1, &newSampler);

  TextureAddressing::Enum addressingX = SamplerSettings::AddressingX(samplerSettings);
  TextureAddressing::Enum addressingY = SamplerSettings::AddressingY(samplerSettings);
  TextureFiltering::Enum filtering = SamplerSettings::Filtering(samplerSettings);
  TextureCompareMode::Enum compareMode = SamplerSettings::CompareMode(samplerSettings);
  TextureCompareFunc::Enum compareFunc = SamplerSettings::CompareFunc(samplerSettings);

  ImportGlSamplerParameteri(newSampler, GL_TEXTURE_WRAP_S, GlTextureAddressing(addressingX));
  ImportGlSamplerParameteri(newSampler, GL_TEXTURE_WRAP_T, GlTextureAddressing(addressingY));
  ImportGlSamplerParameteri(newSampler, GL_TEXTURE_MIN_FILTER, GlTextureFilteringMin(filtering));
  ImportGlSamplerParameteri(newSampler, GL_TEXTURE_MAG_FILTER, GlTextureFilteringMag(filtering));
  ImportGlSamplerParameteri(newSampler, GL_TEXTURE_COMPARE_MODE, GlCompareMode(compareMode));
  ImportGlSamplerParameteri(newSampler, GL_TEXTURE_COMPARE_FUNC, GlCompareFunc(compareFunc));

  mSamplers.Insert(samplerSettings, newSampler);

  return newSampler;
}

void OpenglRenderer::DestroyUnusedSamplers()
{
  forRange (u32 id, mUnusedSamplers.All())
  {
    GLuint sampler = mSamplers[id];
    ImportGlDeleteSamplers(1, &sampler);
    mSamplers.Erase(id);
  }
  mUnusedSamplers.Clear();
  mUnusedSamplers.Append(mSamplers.Keys());
}

} // namespace Zero
