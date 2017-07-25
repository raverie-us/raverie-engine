#include "Precompiled.hpp"

// As Of NVidia Driver 302 exporting this symbol will enable GPU hardware accelerated 
// graphics when using Optimus (Laptop NVidia gpu / Intel HD auto switching). 
// This is important for two reasons first is performance and second is stability
// since Intel seems to have a fair amount of bugs and crashes in their OpenGl drivers
extern "C" 
{ 
  _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

const bool cLazyShaderCompilation = true;

// temporary to prevent string constructions every frame
// RenderQueue structures should have semantics for setting shader parameters
namespace
{
  const Zero::String cFrameTime("FrameTime");
  const Zero::String cLogicTime("LogicTime");
  const Zero::String cNearPlane("NearPlane");
  const Zero::String cFarPlane("FarPlane");
  const Zero::String cViewportSize("ViewportSize");
  const Zero::String cInverseViewportSize("InverseViewportSize");
  const Zero::String cObjectWorldPosition("ObjectWorldPosition");

  const Zero::String cLocalToWorld("LocalToWorld");
  const Zero::String cWorldToLocal("WorldToLocal");
  const Zero::String cWorldToView("WorldToView");
  const Zero::String cViewToWorld("ViewToWorld");
  const Zero::String cLocalToView("LocalToView");
  const Zero::String cViewToLocal("ViewToLocal");
  const Zero::String cLocalToWorldNormal("LocalToWorldNormal");
  const Zero::String cWorldToLocalNormal("WorldToLocalNormal");
  const Zero::String cLocalToViewNormal("LocalToViewNormal");
  const Zero::String cViewToLocalNormal("ViewToLocalNormal");
  const Zero::String cLocalToPerspective("LocalToPerspective");
  const Zero::String cViewToPerspective("ViewToPerspective");
  const Zero::String cPerspectiveToView("PerspectiveToView");
  const Zero::String cZeroPerspectiveToApiPerspective("ZeroPerspectiveToApiPerspective");
}

namespace Zero
{

void GLAPIENTRY EmptyUniformFunc(GLint, GLsizei, const void*) {}

const bool cTransposeMatrices = !(ColumnBasis == 1);

OpenglRenderer* CreateOpenglRenderer(OsHandle windowHandle, String& error)
{
  HWND window = (HWND)windowHandle;
  HDC deviceContext = GetDC(window);

  PIXELFORMATDESCRIPTOR pfd =
  {
    sizeof(PIXELFORMATDESCRIPTOR), // size of this pfd
    1,                             // version number
    PFD_DRAW_TO_WINDOW |           // support window
    PFD_SUPPORT_OPENGL |           // support OpenGL
    PFD_DOUBLEBUFFER,              // double buffered
    PFD_TYPE_RGBA,                 // RGBA type
    32,                            // 32-bit color depth
    0, 0, 0, 0, 0, 0,              // color bits ignored
    0,                             // no alpha buffer
    0,                             // shift bit ignored
    0,                             // no accumulation buffer
    0, 0, 0, 0,                    // accum bits ignored
    0,                             // no z-buffer
    0,                             // no stencil buffer
    0,                             // no auxiliary buffer
    PFD_MAIN_PLANE,                // main layer
    0,                             // reserved
    0, 0, 0                        // layer masks ignored
  };

  int pixelFormat = ChoosePixelFormat(deviceContext, &pfd);
  BOOL success = SetPixelFormat(deviceContext, pixelFormat, &pfd);
  ErrorIf(!success, "Failed to set pixel format.");

  HGLRC renderContext = wglCreateContext(deviceContext);
  wglMakeCurrent(deviceContext, renderContext);

  // Read the OpenGL version support
  const char* gl_version = (const char*)glGetString(GL_VERSION);
  if (gl_version == NULL)
  {
    error = "Unable to query OpenGL version. "
      "Please update your computer's graphics drivers or verify that your graphics card supports OpenGL 2.0.";
    return nullptr;
  }

  const char* gl_sl_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
  const char* gl_vendor = (const char*)glGetString(GL_VENDOR);
  const char* gl_renderer = (const char*)glGetString(GL_RENDERER);
  const char* gl_extensions = (const char*)glGetString(GL_EXTENSIONS);

  ZPrint("OpenGL Version          : %s\n", gl_version ? gl_version : "(no data)");
  ZPrint("OpenGL Shading Language : %s\n", gl_sl_version ? gl_sl_version : "(no data)");
  ZPrint("OpenGL Vendor           : %s\n", gl_vendor ? gl_vendor : "(no data)");
  ZPrint("OpenGL Renderer         : %s\n", gl_renderer ? gl_renderer : "(no data)");

  // Intel integrated graphics currently does not run Zero correctly.
  String vendorString = gl_vendor;
  if (vendorString.Contains("Intel"))
  {
    error = "Intel graphics drivers are unsupported. Please run with dedicated graphics hardware.";
    return nullptr;
  }

  // Initialize glew
  GLenum glewInitStatus = glewInit();
  if (glewInitStatus != GLEW_OK)
  {
    error = String::Format("GLEW failed to initialize with error: %d", glewInitStatus);
    return nullptr;
  }

  bool version_2_0 = glewIsSupported("GL_VERSION_2_0");
  bool framebuffer_object = glewIsSupported("GL_ARB_framebuffer_object");
  bool texture_compression = glewIsSupported("GL_ARB_texture_compression");
  bool draw_buffers_blend = glewIsSupported("GL_ARB_draw_buffers_blend");
  bool sampler_objects = glewIsSupported("GL_ARB_sampler_objects");

  ZPrint("OpenGL *Required Extensions\n");
  ZPrint("OpenGL *(GL_VERSION_2_0) Shader Program support                 : %s\n", version_2_0 ? "True" : "False");
  ZPrint("OpenGL *(GL_ARB_framebuffer_object) Deferred Rendering support  : %s\n", framebuffer_object ? "True" : "False");

  ZPrint("OpenGL (GL_ARB_texture_compression) Texture Compression support : %s\n", texture_compression ? "True" : "False");
  ZPrint("OpenGL (GL_ARB_draw_buffers_blend) Multi Target Blend support   : %s\n", draw_buffers_blend ? "True" : "False");
  ZPrint("OpenGL (GL_ARB_sampler_objects) Sampler Object support          : %s\n", sampler_objects ? "True" : "False");

  ZPrint("OpenGL All Extensions : %s\n", gl_extensions ? gl_extensions : "(no data)");

  // Required OpenGL extensions
  if (!version_2_0 || !framebuffer_object)
  {
    String failedExtensions = BuildString(version_2_0 ? "" : "GL_VERSION_2_0, ",
                                          framebuffer_object ? "" : "GL_ARB_framebuffer_object, ");
    error = String::Format("Required OpenGL extensions: %s are unsupported by the active driver. "
      "Please update your computer's graphics drivers or verify that your graphics card supports the listed features.", failedExtensions.c_str());
    return nullptr;
  }

  OpenglRenderer* renderer = new OpenglRenderer();
  renderer->mWindow = windowHandle;
  renderer->mDeviceContext = deviceContext;
  renderer->mRenderContext = renderContext;

  renderer->mDriverSupport.mTextureCompression = texture_compression;
  renderer->mDriverSupport.mMultiTargetBlend = draw_buffers_blend;
  renderer->mDriverSupport.mSamplerObjects = sampler_objects;

  return renderer;
}

void DestroyOpenglRenderer()
{
  OpenglRenderer* renderer = (OpenglRenderer*)Z::gRenderer;
  HGLRC renderContext = (HGLRC)renderer->mRenderContext;

  delete renderer;

  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(renderContext);
}

void CreateRenderer(OsHandle windowHandle, String& error)
{
  Z::gRenderer = CreateOpenglRenderer(windowHandle, error);
}

void DestroyRenderer()
{
  DestroyOpenglRenderer();
}

struct GlTextureEnums
{
  GLint mInternalFormat;
  GLint mFormat;
  GLint mType;
};

GlTextureEnums gTextureEnums[] =
{
  {/* internalFormat    , format            , type */                          }, // None
  {GL_R8                , GL_RED            , GL_UNSIGNED_BYTE                 }, // R8
  {GL_RG8               , GL_RG             , GL_UNSIGNED_BYTE                 }, // RG8
  {GL_RGB8              , GL_RGB            , GL_UNSIGNED_BYTE                 }, // RGB8
  {GL_RGBA8             , GL_RGBA           , GL_UNSIGNED_BYTE                 }, // RGBA8
  {GL_R16               , GL_RED            , GL_UNSIGNED_SHORT                }, // R16
  {GL_RG16              , GL_RG             , GL_UNSIGNED_SHORT                }, // RG16
  {GL_RGB16             , GL_RGB            , GL_UNSIGNED_SHORT                }, // RGB16
  {GL_RGBA16            , GL_RGBA           , GL_UNSIGNED_SHORT                }, // RGBA16
  {GL_R16F              , GL_RED            , GL_HALF_FLOAT                    }, // R16f
  {GL_RG16F             , GL_RG             , GL_HALF_FLOAT                    }, // RG16f
  {GL_RGB16F            , GL_RGB            , GL_HALF_FLOAT                    }, // RGB16f
  {GL_RGBA16F           , GL_RGBA           , GL_HALF_FLOAT                    }, // RGBA16f
  {GL_R32F              , GL_RED            , GL_FLOAT                         }, // R32f
  {GL_RG32F             , GL_RG             , GL_FLOAT                         }, // RG32f
  {GL_RGB32F            , GL_RGB            , GL_FLOAT                         }, // RGB32f
  {GL_RGBA32F           , GL_RGBA           , GL_FLOAT                         }, // RGBA32f
  {GL_SRGB8             , GL_RGB            , GL_UNSIGNED_BYTE                 }, // SRGB8
  {GL_SRGB8_ALPHA8      , GL_RGBA           , GL_UNSIGNED_BYTE                 }, // SRGB8A8
  {GL_DEPTH_COMPONENT16 , GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT                }, // Depth16
  {GL_DEPTH_COMPONENT24 , GL_DEPTH_COMPONENT, GL_UNSIGNED_INT                  }, // Depth24
  {GL_DEPTH_COMPONENT32 , GL_DEPTH_COMPONENT, GL_UNSIGNED_INT                  }, // Depth32
  {GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT                         }, // Depth32f
  {GL_DEPTH24_STENCIL8  , GL_DEPTH_STENCIL  , GL_UNSIGNED_INT_24_8             }, // Depth24Stencil8
  {GL_DEPTH32F_STENCIL8 , GL_DEPTH_STENCIL  , GL_FLOAT_32_UNSIGNED_INT_24_8_REV}  // Depth32fStencil8Pad24
};

GLint GlInternalFormat(TextureCompression::Enum compression)
{
  switch (compression)
  {
    case TextureCompression::BC1: return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    case TextureCompression::BC2: return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    case TextureCompression::BC3: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    case TextureCompression::BC4: return GL_COMPRESSED_RED_RGTC1;
    case TextureCompression::BC5: return GL_COMPRESSED_RG_RGTC2;
    case TextureCompression::BC6: return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB;
    //case TextureCompression::BC7: return GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
    default: return 0;
  }
}

GLuint ToOpenglType(VertexElementType::Enum type)
{
  switch(type)
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
    case PrimitiveType::Triangles: return GL_TRIANGLES;
    case PrimitiveType::Lines:     return GL_LINES;
    case PrimitiveType::Points:    return GL_POINTS;
    default: return 0;
  }
}

GLuint GlTextureType(TextureType::Enum value)
{
  switch (value)
  {
    case TextureType::Texture2D:   return GL_TEXTURE_2D;
    case TextureType::TextureCube: return GL_TEXTURE_CUBE_MAP;
    default: return 0;
  }
}

GLuint GlTextureFace(TextureFace::Enum value)
{
  switch (value)
  {
    case TextureFace::None:      return GL_TEXTURE_2D;
    case TextureFace::PositiveX: return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
    case TextureFace::PositiveY: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
    case TextureFace::PositiveZ: return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
    case TextureFace::NegativeX: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
    case TextureFace::NegativeY: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
    case TextureFace::NegativeZ: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
    default: return 0;
  }
}

GLuint GlTextureAddressing(TextureAddressing::Enum value)
{
  switch (value)
  {
    case TextureAddressing::Clamp:  return GL_CLAMP_TO_EDGE;
    case TextureAddressing::Repeat: return GL_REPEAT;
    case TextureAddressing::Mirror: return GL_MIRRORED_REPEAT;
    default: return 0;
  }
}

GLuint GlTextureFilteringMin(TextureFiltering::Enum value)
{
  switch (value)
  {
    case TextureFiltering::Nearest:   return GL_NEAREST_MIPMAP_NEAREST;
    case TextureFiltering::Bilinear:  return GL_LINEAR_MIPMAP_NEAREST;
    case TextureFiltering::Trilinear: return GL_LINEAR_MIPMAP_LINEAR;
    default: return 0;
  }
}

GLuint GlTextureFilteringMag(TextureFiltering::Enum value)
{
  switch (value)
  {
    case TextureFiltering::Nearest:   return GL_NEAREST;
    case TextureFiltering::Bilinear:  return GL_LINEAR;
    case TextureFiltering::Trilinear: return GL_LINEAR;
    default: return 0;
  }
}

GLfloat GlTextureAnisotropy(TextureAnisotropy::Enum value)
{
  switch (value)
  {
    case TextureAnisotropy::x1:  return 1.0f;
    case TextureAnisotropy::x2:  return 2.0f;
    case TextureAnisotropy::x4:  return 4.0f;
    case TextureAnisotropy::x8:  return 8.0f;
    case TextureAnisotropy::x16: return 16.0f;
    default: return 1.0f;
  }
}

GLuint GlTextureMipMapping(TextureMipMapping::Enum value)
{
  switch (value)
  {
    case TextureMipMapping::None: return 0;
    case TextureMipMapping::PreGenerated: return 1000;
    case TextureMipMapping::GpuGenerated: return 1000;
    default: return 0;
  }
}

void CheckShader(GLuint shader)
{
  GLint status = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE)
  {
    GLint infoLogLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    GLchar* strInfoLog = (GLchar*)alloca(infoLogLength + 1);
    glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
    ZPrint("Compile Error\n%s\n", strInfoLog);
  }
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
  }
}

GLuint GlCullFace(CullMode::Enum value)
{
  switch (value)
  {
    case CullMode::BackFace:  return GL_BACK;
    case CullMode::FrontFace: return GL_FRONT;
    default: return 0;
  }
}

GLuint GlBlendFactor(BlendFactor::Enum value)
{
  switch (value)
  {
    case BlendFactor::Zero:                return GL_ZERO;
    case BlendFactor::One:                 return GL_ONE;
    case BlendFactor::SourceColor:         return GL_SRC_COLOR;
    case BlendFactor::InvSourceColor:      return GL_ONE_MINUS_SRC_COLOR;
    case BlendFactor::DestColor:           return GL_DST_COLOR;
    case BlendFactor::InvDestColor:        return GL_ONE_MINUS_DST_COLOR;
    case BlendFactor::SourceAlpha:         return GL_SRC_ALPHA;
    case BlendFactor::InvSourceAlpha:      return GL_ONE_MINUS_SRC_ALPHA;
    case BlendFactor::DestAlpha:           return GL_DST_ALPHA;
    case BlendFactor::InvDestAlpha:        return GL_ONE_MINUS_DST_ALPHA;
    case BlendFactor::SourceAlphaSaturate: return GL_SRC_ALPHA_SATURATE;
    default: return 0;
  }
}

GLuint GlBlendEquation(BlendEquation::Enum blendEquation)
{
  switch (blendEquation)
  {
    case BlendEquation::Add:             return GL_FUNC_ADD;
    case BlendEquation::Subtract:        return GL_FUNC_SUBTRACT;
    case BlendEquation::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
    case BlendEquation::Min:             return GL_MIN;
    case BlendEquation::Max:             return GL_MAX;
    default: return 0;
  }
}

GLboolean GlDepthMode(DepthMode::Enum value)
{
  switch (value)
  {
    case DepthMode::Read: return false;
    case DepthMode::Write: return true;
    default: return false;
  }
}

GLuint GlStencilOp(StencilOp::Enum value)
{
  switch (value)
  {
    case StencilOp::Zero:          return GL_ZERO;
    case StencilOp::Keep:          return GL_KEEP;
    case StencilOp::Replace:       return GL_REPLACE;
    case StencilOp::Invert:        return GL_INVERT;
    case StencilOp::Increment:     return GL_INCR;
    case StencilOp::IncrementWrap: return GL_INCR_WRAP;
    case StencilOp::Decrement:     return GL_DECR;
    case StencilOp::DecrementWrap: return GL_DECR_WRAP;
    default: return 0;
  }
}

GLuint GlCompareMode(TextureCompareMode::Enum compareMode)
{
  switch (compareMode)
  {
    case TextureCompareMode::Disabled: return GL_NONE;
    case TextureCompareMode::Enabled:  return GL_COMPARE_R_TO_TEXTURE;
    default: return 0;
  }
}

GLuint GlCompareFunc(TextureCompareFunc::Enum value)
{
  switch (value)
  {
    case TextureCompareFunc::Never:        return GL_NEVER;
    case TextureCompareFunc::Always:       return GL_ALWAYS;
    case TextureCompareFunc::Less:         return GL_LESS;
    case TextureCompareFunc::LessEqual:    return GL_LEQUAL;
    case TextureCompareFunc::Greater:      return GL_GREATER;
    case TextureCompareFunc::GreaterEqual: return GL_GEQUAL;
    case TextureCompareFunc::Equal:        return GL_EQUAL;
    case TextureCompareFunc::NotEqual:     return GL_NOTEQUAL;
    default: return 0;
  }
}

void SetRenderSettings(const RenderSettings& renderSettings, bool drawBuffersBlend)
{
  switch (renderSettings.mCullMode)
  {
    case CullMode::Disabled:
      glDisable(GL_CULL_FACE);
    break;
    case CullMode::BackFace:
    case CullMode::FrontFace:
      glEnable(GL_CULL_FACE);
      glCullFace(GlCullFace(renderSettings.mCullMode));
    break;
  }

  if (renderSettings.mSingleColorTarget || drawBuffersBlend == false)
  {
    const BlendSettings& blendSettings = renderSettings.mBlendSettings[0];
    switch (blendSettings.mBlendMode)
    {
      case BlendMode::Disabled:
        glDisable(GL_BLEND);
      break;
      case BlendMode::Enabled:
        glEnable(GL_BLEND);
        glBlendEquation(GlBlendEquation(blendSettings.mBlendEquation));
        glBlendFunc(GlBlendFactor(blendSettings.mSourceFactor), GlBlendFactor(blendSettings.mDestFactor));
      break;
      case BlendMode::Separate:
        glEnable(GL_BLEND);
        glBlendEquationSeparate(GlBlendEquation(blendSettings.mBlendEquation), GlBlendEquation(blendSettings.mBlendEquationAlpha));
        glBlendFuncSeparate(GlBlendFactor(blendSettings.mSourceFactor), GlBlendFactor(blendSettings.mDestFactor), GlBlendFactor(blendSettings.mSourceFactorAlpha), GlBlendFactor(blendSettings.mDestFactorAlpha));
      break;
    }
  }
  else
  {
    for (uint i = 0; i < 8; ++i)
    {
      const BlendSettings& blendSettings = renderSettings.mBlendSettings[i];
      switch (blendSettings.mBlendMode)
      {
        case BlendMode::Disabled:
          glDisablei(GL_BLEND, i);
        break;
        case BlendMode::Enabled:
          glEnablei(GL_BLEND, i);
          glBlendEquationi(i, GlBlendEquation(blendSettings.mBlendEquation));
          glBlendFunci(i, GlBlendFactor(blendSettings.mSourceFactor), GlBlendFactor(blendSettings.mDestFactor));
        break;
        case BlendMode::Separate:
          glEnablei(GL_BLEND, i);
          glBlendEquationSeparatei(i, GlBlendEquation(blendSettings.mBlendEquation), GlBlendEquation(blendSettings.mBlendEquationAlpha));
          glBlendFuncSeparatei(i, GlBlendFactor(blendSettings.mSourceFactor), GlBlendFactor(blendSettings.mDestFactor), GlBlendFactor(blendSettings.mSourceFactorAlpha), GlBlendFactor(blendSettings.mDestFactorAlpha));
        break;
      }
    }
  }

  const DepthSettings& depthSettings = renderSettings.mDepthSettings;
  switch (depthSettings.mDepthMode)
  {
    case DepthMode::Disabled:
      glDisable(GL_DEPTH_TEST);
    break;
    case DepthMode::Read:
    case DepthMode::Write:
      glEnable(GL_DEPTH_TEST);
      glDepthMask(GlDepthMode(depthSettings.mDepthMode));
      glDepthFunc(GlCompareFunc(depthSettings.mDepthCompareFunc));
    break;
  }

  switch (depthSettings.mStencilMode)
  {
    case StencilMode::Disabled:
      glDisable(GL_STENCIL_TEST);
      glStencilMask(0);
    break;
    case StencilMode::Enabled:
      glEnable(GL_STENCIL_TEST);
      glStencilFunc(GlCompareFunc(depthSettings.mStencilCompareFunc), depthSettings.mStencilTestValue, depthSettings.mStencilReadMask);
      glStencilOp(GlStencilOp(depthSettings.mStencilFailOp), GlStencilOp(depthSettings.mDepthFailOp), GlStencilOp(depthSettings.mDepthPassOp));
      glStencilMask(depthSettings.mStencilWriteMask);
    break;
    case StencilMode::Separate:
      glEnable(GL_STENCIL_TEST);
      glStencilFuncSeparate(GL_FRONT, GlCompareFunc(depthSettings.mStencilCompareFunc), depthSettings.mStencilTestValue, depthSettings.mStencilReadMask);
      glStencilOpSeparate(GL_FRONT, GlStencilOp(depthSettings.mStencilFailOp), GlStencilOp(depthSettings.mDepthFailOp), GlStencilOp(depthSettings.mDepthPassOp));
      glStencilMaskSeparate(GL_FRONT, depthSettings.mStencilWriteMask);
      glStencilFuncSeparate(GL_BACK, GlCompareFunc(depthSettings.mStencilCompareFuncBackFace), depthSettings.mStencilTestValueBackFace, depthSettings.mStencilReadMaskBackFace);
      glStencilOpSeparate(GL_BACK, GlStencilOp(depthSettings.mStencilFailOpBackFace), GlStencilOp(depthSettings.mDepthFailOpBackFace), GlStencilOp(depthSettings.mDepthPassOpBackFace));
      glStencilMaskSeparate(GL_BACK, depthSettings.mStencilWriteMaskBackFace);
    break;
  }

  switch (renderSettings.mScissorMode)
  {
    case ScissorMode::Disabled:
      glDisable(GL_SCISSOR_TEST);
    break;
    case ScissorMode::Enabled:
      glEnable(GL_SCISSOR_TEST);
    break;
  }
}

void BindTexture(TextureType::Enum textureType, uint textureSlot, uint textureId, bool samplerObjects)
{
  // Clear anything bound to this texture unit
  glActiveTexture(GL_TEXTURE0 + textureSlot);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  if (samplerObjects)
    glBindSampler(textureSlot, 0);
  // Bind texture
  glBindTexture(GlTextureType(textureType), textureId);
}

void CheckFramebufferStatus()
{
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
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
}

void SetSingleRenderTargets(GLuint fboId, TextureRenderData** colorTargets, TextureRenderData* depthTarget)
{
  glBindFramebuffer(GL_FRAMEBUFFER, fboId);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
  glDrawBuffer(GL_NONE);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

  GlTextureRenderData* colorRenderData = (GlTextureRenderData*)colorTargets[0];
  if (colorRenderData != nullptr)
  {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorRenderData->mId, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
  }

  GlTextureRenderData* depthRenderData = (GlTextureRenderData*)depthTarget;
  if (depthRenderData != nullptr)
  {
    if (IsDepthStencilFormat(depthRenderData->mFormat))
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthRenderData->mId, 0);
    else
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthRenderData->mId, 0);
  }

  CheckFramebufferStatus();
}

void SetMultiRenderTargets(GLuint fboId, TextureRenderData** colorTargets, TextureRenderData* depthTarget)
{
  glBindFramebuffer(GL_FRAMEBUFFER, fboId);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0, 0);

  GLenum drawBuffers[8];

  for (uint i = 0; i < 8; ++i)
  {
    GlTextureRenderData* colorRenderData = (GlTextureRenderData*)colorTargets[i];
    if (colorRenderData != nullptr)
    {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorRenderData->mId, 0);
      drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }
    else
    {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);
      drawBuffers[i] = GL_NONE;
    }
  }

  // Set active buffers, some drivers do not work correctly if all are always active
  glDrawBuffers(8, drawBuffers);

  GlTextureRenderData* depthRenderData = (GlTextureRenderData*)depthTarget;
  if (depthRenderData != nullptr)
  {
    if (IsDepthStencilFormat(depthRenderData->mFormat))
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthRenderData->mId, 0);
    else
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthRenderData->mId, 0);
  }

  CheckFramebufferStatus();
}

//--------------------------------------------------------------- OpenglRenderer
OpenglRenderer::OpenglRenderer()
{
  // V-Sync off by default
  wglSwapIntervalEXT(0);

  // No padding
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_TEXTURE_CUBE_MAP);

  if (glewIsSupported("GL_ARB_seamless_cube_map"))
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  // GLint maxAttributes;
  // glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttributes); // 16
  // for (int i = 0; i < maxAttributes; ++i)
  //   glEnableVertexAttribArray(i);

  //GLint maxAttach;
  //glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);

  //GLint maxTexUnits;
  //glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTexUnits);

  //GLfloat maxAnisotropy;
  //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);

  mActiveShader = 0;
  mActiveMaterial = 0;
  mActiveTexture = 0;

  mCurrentLineWidth = 1.0f;
  mClipMode = false;
  mCurrentClip = Vec4(0, 0, 0, 0);

  // buffer for fullscreen triangle
  StreamedVertex triangleVertices[] =
  {
    {Vec3(-1, 3, 0), Vec2(0, -1), Vec4()},
    {Vec3(-1, -1, 0), Vec2(0, 1), Vec4()},
    {Vec3(3, -1, 0), Vec2(2, 1), Vec4()},
  };

  uint triangleIndices[] = {0, 1, 2};

  glGenVertexArrays(1, &mTriangleArray);
  glBindVertexArray(mTriangleArray);

  glGenBuffers(1, &mTriangleVertex);
  glBindBuffer(GL_ARRAY_BUFFER, mTriangleVertex);
  glBufferData(GL_ARRAY_BUFFER, sizeof(StreamedVertex) * 3, triangleVertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(VertexSemantic::Position);
  glVertexAttribPointer(VertexSemantic::Position, 3, GL_FLOAT, GL_FALSE, sizeof(StreamedVertex), (void*)offsetof(StreamedVertex, mPosition));
  glEnableVertexAttribArray(VertexSemantic::Uv);
  glVertexAttribPointer(VertexSemantic::Uv, 2, GL_FLOAT, GL_FALSE, sizeof(StreamedVertex), (void*)offsetof(StreamedVertex, mUv));

  glGenBuffers(1, &mTriangleIndex);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mTriangleIndex);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * 3, triangleIndices, GL_STATIC_DRAW);

  glBindVertexArray(0);

  // frame buffers
  glGenFramebuffers(1, &mSingleTargetFbo);
  glGenFramebuffers(1, &mMultiTargetFbo);

  // Function invocations will fail if calling convention is not correct, currently using GLAPIENTRY
  mUniformFunctions[ShaderInputType::Invalid] = EmptyUniformFunc;
  mUniformFunctions[ShaderInputType::Bool]    = (UniformFunction)glUniform1iv;
  mUniformFunctions[ShaderInputType::Int]     = (UniformFunction)glUniform1iv;
  mUniformFunctions[ShaderInputType::IntVec2] = (UniformFunction)glUniform2iv;
  mUniformFunctions[ShaderInputType::IntVec3] = (UniformFunction)glUniform3iv;
  mUniformFunctions[ShaderInputType::IntVec4] = (UniformFunction)glUniform4iv;
  mUniformFunctions[ShaderInputType::Float]   = (UniformFunction)glUniform1fv;
  mUniformFunctions[ShaderInputType::Vec2]    = (UniformFunction)glUniform2fv;
  mUniformFunctions[ShaderInputType::Vec3]    = (UniformFunction)glUniform3fv;
  mUniformFunctions[ShaderInputType::Vec4]    = (UniformFunction)glUniform4fv;
  mUniformFunctions[ShaderInputType::Mat3]    = EmptyUniformFunc;
  mUniformFunctions[ShaderInputType::Mat4]    = EmptyUniformFunc;
  mUniformFunctions[ShaderInputType::Texture] = (UniformFunction)glUniform1iv;

  // should be set by graphics
  mCoreVertexTypeNames[CoreVertexType::Mesh] = "MeshVertex";
  mCoreVertexTypeNames[CoreVertexType::SkinnedMesh] = "SkinnedMeshVertex";
  mCoreVertexTypeNames[CoreVertexType::Streamed] = "StreamedVertex";

  mStreamedVertexBuffer.Initialize();

  #include "LoadingShader.inl"
  CreateShader(LoadingShaderVertex, String(), LoadingShaderPixel, mLoadingShader);
}

OpenglRenderer::~OpenglRenderer()
{
  ErrorIf(mGlShaders.Empty() == false, "Not all shaders were deleted.");
  ErrorIf(mShaderEntries.Empty() == false, "Not all shaders were deleted.");

  DelayedRenderDataDestruction();

  glDeleteFramebuffers(1, &mSingleTargetFbo);
  glDeleteFramebuffers(1, &mMultiTargetFbo);

  glDeleteVertexArrays(1, &mTriangleArray);
  glDeleteBuffers(1, &mTriangleVertex);
  glDeleteBuffers(1, &mTriangleIndex);

  glDeleteProgram(mLoadingShader);

  mStreamedVertexBuffer.Destroy();

  forRange (GLuint sampler, mSamplers.Values())
    glDeleteSamplers(1, &sampler);
  mSamplers.Clear();
}

void OpenglRenderer::BuildOrthographicTransform(Mat4Ref matrix, float size, float aspect, float near, float far)
{
  BuildOrthographicTransformGl(matrix, size, aspect, near, far);
}

void OpenglRenderer::BuildPerspectiveTransform(Mat4Ref matrix, float fov, float aspect, float near, float far)
{
  BuildPerspectiveTransformGl(matrix, fov, aspect, near, far);
}

bool OpenglRenderer::YInvertImageData(TextureType::Enum type)
{
  // OpenGL convention for cubemaps is not Y-inverted
  return (type != TextureType::TextureCube);
}

void OpenglRenderer::CreateRenderData(Material* material)
{
  GlMaterialRenderData* renderData = (GlMaterialRenderData*)material->mRenderData;
  if (renderData != nullptr)
    return;

  renderData = new GlMaterialRenderData();
  renderData->mResourceId = 0;
  material->mRenderData = renderData;
}

void OpenglRenderer::CreateRenderData(Mesh* mesh)
{
  GlMeshRenderData* renderData = (GlMeshRenderData*)mesh->mRenderData;
  if (renderData != nullptr)
    return;

  renderData = new GlMeshRenderData();
  renderData->mVertexBuffer = 0;
  renderData->mIndexBuffer = 0;
  renderData->mVertexArray = 0;
  renderData->mIndexCount = 0;
  mesh->mRenderData = renderData;
}

void OpenglRenderer::CreateRenderData(Texture* texture)
{
  GlTextureRenderData* renderData = (GlTextureRenderData*)texture->mRenderData;
  if (renderData != nullptr)
    return;

  renderData = new GlTextureRenderData();
  renderData->mId = 0;
  renderData->mFormat = TextureFormat::None;
  texture->mRenderData = renderData;
}

void OpenglRenderer::AddMaterial(AddMaterialJob* job)
{
  GlMaterialRenderData* renderData = (GlMaterialRenderData*)job->mRenderData;

  renderData->mCompositeName = job->mCompositeName;
  renderData->mResourceId = job->mMaterialId;
}

void OpenglRenderer::AddMesh(AddMeshJob* job)
{
  GlMeshRenderData* renderData = (GlMeshRenderData*)job->mRenderData;
  if (renderData->mVertexArray != 0)
  {
    glDeleteVertexArrays(1, &renderData->mVertexArray);
    glDeleteBuffers(1, &renderData->mVertexBuffer);
    glDeleteBuffers(1, &renderData->mIndexBuffer);
  }

  if (job->mVertexData == nullptr)
  {
    renderData->mVertexBuffer = 0;
    renderData->mIndexBuffer = 0;
    renderData->mVertexArray = 0;
    renderData->mIndexCount = job->mIndexCount;
    renderData->mPrimitiveType = job->mPrimitiveType;
    return;
  }

  GLuint vertexArray;
  glGenVertexArrays(1, &vertexArray);
  glBindVertexArray(vertexArray);

  GLuint vertexBuffer;
  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, job->mVertexCount * job->mVertexSize, job->mVertexData, GL_STATIC_DRAW);

  forRange (VertexAttribute& element, job->mVertexAttributes.All())
  {
    bool normalized = element.mType >= VertexElementType::NormByte;
    glEnableVertexAttribArray(element.mSemantic);
    if (element.mSemantic == VertexSemantic::BoneIndices)
      glVertexAttribIPointer(element.mSemantic, element.mCount, ToOpenglType(element.mType), job->mVertexSize, (void*)element.mOffset);
    else
      glVertexAttribPointer(element.mSemantic, element.mCount, ToOpenglType(element.mType), normalized, job->mVertexSize, (void*)element.mOffset);
  }

  GLuint indexBuffer = 0;
  if (job->mIndexData != nullptr)
  {
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, job->mIndexCount * job->mIndexSize, job->mIndexData, GL_STATIC_DRAW);
  }

  glBindVertexArray(0);

  renderData->mVertexBuffer = vertexBuffer;
  renderData->mIndexBuffer = indexBuffer;
  renderData->mVertexArray = vertexArray;
  renderData->mIndexCount = job->mIndexCount;
  renderData->mPrimitiveType = job->mPrimitiveType;

  delete[] job->mVertexData;
  delete[] job->mIndexData;

  renderData->mBones.Assign(job->mBones.All());
}

void OpenglRenderer::AddTexture(AddTextureJob* job)
{
  GlTextureRenderData* renderData = (GlTextureRenderData*)job->mRenderData;
  GlTextureEnums glEnums = gTextureEnums[job->mFormat];

  if (job->mFormat == TextureFormat::None)
  {
    if (renderData->mId != 0)
    {
      glDeleteTextures(1, &renderData->mId);
      renderData->mId = 0;
    }
  }
  else
  {
    if (job->mType != renderData->mType && renderData->mId != 0)
    {
      glDeleteTextures(1, &renderData->mId);
      renderData->mId = 0;
    }

    if (renderData->mId == 0)
      glGenTextures(1, &renderData->mId);

    BindTexture(job->mType, 0, renderData->mId, mDriverSupport.mSamplerObjects);

    if (job->mImageData == nullptr)
    {
      // RenderTarget upload if no data, rendering to cubemap is not implemented
      glTexImage2D(GL_TEXTURE_2D, 0, glEnums.mInternalFormat, job->mWidth, job->mHeight, 0, glEnums.mFormat, glEnums.mType, nullptr);
    }
    else
    {
      for (uint i = 0; i < job->mMipCount; ++i)
      {
        MipHeader* mipHeader = job->mMipHeaders + i;
        byte* mipData = job->mImageData + mipHeader->mDataOffset;

        if (job->mSubImage)
        {
          ErrorIf(mipHeader->mLevel != 0, "Sub-image uploading to lower mip levels is not supported.");
          uint xOffset = job->mXOffset;
          uint yOffset = job->mHeight - (mipHeader->mHeight + job->mYOffset);
          glTexSubImage2D(GlTextureFace((TextureFace::Enum)mipHeader->mFace), mipHeader->mLevel, xOffset, yOffset, mipHeader->mWidth, mipHeader->mHeight, glEnums.mFormat, glEnums.mType, mipData);
        }
        else
        {
          if (job->mCompression != TextureCompression::None)
            glCompressedTexImage2D(GlTextureFace((TextureFace::Enum)mipHeader->mFace), mipHeader->mLevel, GlInternalFormat(job->mCompression), mipHeader->mWidth, mipHeader->mHeight, 0, mipHeader->mDataSize, mipData);
          else
            glTexImage2D(GlTextureFace((TextureFace::Enum)mipHeader->mFace), mipHeader->mLevel, glEnums.mInternalFormat, mipHeader->mWidth, mipHeader->mHeight, 0, glEnums.mFormat, glEnums.mType, mipData);
        }
      }
    }

    glTexParameteri(GlTextureType(job->mType), GL_TEXTURE_WRAP_S, GlTextureAddressing(job->mAddressingX));
    glTexParameteri(GlTextureType(job->mType), GL_TEXTURE_WRAP_T, GlTextureAddressing(job->mAddressingY));
    glTexParameteri(GlTextureType(job->mType), GL_TEXTURE_MIN_FILTER, GlTextureFilteringMin(job->mFiltering));
    glTexParameteri(GlTextureType(job->mType), GL_TEXTURE_MAG_FILTER, GlTextureFilteringMag(job->mFiltering));
    glTexParameteri(GlTextureType(job->mType), GL_TEXTURE_COMPARE_MODE, GlCompareMode(job->mCompareMode));
    glTexParameteri(GlTextureType(job->mType), GL_TEXTURE_COMPARE_FUNC, GlCompareFunc(job->mCompareFunc));
    glTexParameterf(GlTextureType(job->mType), GL_TEXTURE_MAX_ANISOTROPY_EXT, GlTextureAnisotropy(job->mAnisotropy));
    glTexParameteri(GlTextureType(job->mType), GL_TEXTURE_MAX_LEVEL, GlTextureMipMapping(job->mMipMapping));

    if (job->mMipMapping == TextureMipMapping::GpuGenerated)
      glGenerateMipmap(GlTextureType(job->mType));

    glBindTexture(GlTextureType(job->mType), 0);
  }

  renderData->mType = job->mType;
  renderData->mFormat = job->mFormat;
  renderData->mWidth = job->mWidth;
  renderData->mHeight = job->mHeight;

  renderData->mSamplerSettings = 0;
  renderData->mSamplerSettings |= SamplerSettings::AddressingX(job->mAddressingX);
  renderData->mSamplerSettings |= SamplerSettings::AddressingY(job->mAddressingY);
  renderData->mSamplerSettings |= SamplerSettings::Filtering(job->mFiltering);
  renderData->mSamplerSettings |= SamplerSettings::CompareMode(job->mCompareMode);
  renderData->mSamplerSettings |= SamplerSettings::CompareFunc(job->mCompareFunc);

  delete[] job->mImageData;
  delete[] job->mMipHeaders;
}

void OpenglRenderer::RemoveMaterial(RemoveMaterialJob* job)
{
  mMaterialRenderDataToDestroy.PushBack((GlMaterialRenderData*)job->mRenderData);
}

void OpenglRenderer::RemoveMesh(RemoveMeshJob* job)
{
  mMeshRenderDataToDestroy.PushBack((GlMeshRenderData*)job->mRenderData);
}

void OpenglRenderer::RemoveTexture(RemoveTextureJob* job)
{
  mTextureRenderDataToDestroy.PushBack((GlTextureRenderData*)job->mRenderData);
}

void OpenglRenderer::AddShaders(AddShadersJob* job)
{
  if (cLazyShaderCompilation && !job->mForceCompile)
  {
    forRange (ShaderEntry& entry, job->mShaders.All())
    {
      ShaderKey shaderKey(entry.mComposite, StringPair(entry.mCoreVertex, entry.mRenderPass));
      mShaderEntries.Insert(shaderKey, entry);
    }
  }
  else
  {
    forRange (ShaderEntry& entry, job->mShaders.All())
    {
      ShaderKey shaderKey(entry.mComposite, StringPair(entry.mCoreVertex, entry.mRenderPass));
      mShaderEntries.Erase(shaderKey);
      CreateShader(entry);
    }
  }
}

void OpenglRenderer::RemoveShaders(RemoveShadersJob* job)
{
  forRange (ShaderEntry& entry, job->mShaders.All())
  {
    ShaderKey shaderKey(entry.mComposite, StringPair(entry.mCoreVertex, entry.mRenderPass));

    if (mGlShaders.ContainsKey(shaderKey))
      glDeleteProgram(mGlShaders[shaderKey].mId);

    mGlShaders.Erase(shaderKey);
    mShaderEntries.Erase(shaderKey);
  }
}

void OpenglRenderer::SetVSync(SetVSyncJob* job)
{
  int swapInterval = job->mVSync ? 1 : 0;
  wglSwapIntervalEXT(swapInterval);
}

void OpenglRenderer::GetTextureData(GetTextureDataJob* job)
{
  GlTextureRenderData* renderData = (GlTextureRenderData*)job->mRenderData;
  job->mImage = nullptr;
  if (!IsColorFormat(renderData->mFormat))
    return;

  job->mWidth = renderData->mWidth;
  job->mHeight = renderData->mHeight;
  if (job->mWidth == 0 || job->mHeight == 0)
    return;

  if (IsFloatColorFormat(renderData->mFormat))
    job->mFormat = TextureFormat::RGB32f;
  else if (IsShortColorFormat(renderData->mFormat))
    job->mFormat = TextureFormat::RGBA16;
  else
    job->mFormat = TextureFormat::RGBA8;

  SetSingleRenderTargets(mSingleTargetFbo, &job->mRenderData, nullptr);

  uint imageSize = job->mWidth * job->mHeight * GetPixelSize(job->mFormat);
  job->mImage = new byte[imageSize];

  GlTextureEnums textureEnums = gTextureEnums[job->mFormat];
  glReadPixels(0, 0, job->mWidth, job->mHeight, textureEnums.mFormat, textureEnums.mType, job->mImage);

  YInvertNonCompressed(job->mImage, job->mWidth, job->mHeight, GetPixelSize(job->mFormat));

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenglRenderer::ShowProgress(ShowProgressJob* job)
{
  // Get data off job
  job->Lock();
  GlTextureRenderData* loadingTexture = (GlTextureRenderData*)job->mLoadingTexture;
  GlTextureRenderData* logoTexture = (GlTextureRenderData*)job->mLogoTexture;
  GlTextureRenderData* whiteTexture = (GlTextureRenderData*)job->mWhiteTexture;
  GlTextureRenderData* splashTexture = (GlTextureRenderData*)job->mSplashTexture;
  GlTextureRenderData* fontTexture = (GlTextureRenderData*)job->mFontTexture;
  uint logoFrameSize = job->mLogoFrameSize;
  Array<StreamedVertex> progressText = job->mProgressText;
  int progressWidth = job->mProgressWidth;
  float currentPercent = job->mCurrentPercent;
  double time = job->mTimer.Time();
  bool splashMode = job->mSplashMode;
  float alpha = splashMode ? job->mSplashFade : 1.0f;
  job->Unlock();

  RECT rect;
  GetClientRect((HWND)mWindow, &rect);
  IntVec2 size = IntVec2(rect.right - rect.left, rect.bottom - rect.top);
  glViewport(0, 0, size.x, size.y);

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
  Vec3 progressTranslation = Vec3((size.x - loadingScale.x + progressScale.x) * 0.5f, (size.y + loadingScale.y) * 0.5f + 40.0f, 0.0f);
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

  StreamedVertex quadVertices[] =
  {
    {Vec3(-0.5f,-0.5f, 0.0f), Vec2(0.0f, 0.0f), Vec4(1.0f)},
    {Vec3(-0.5f, 0.5f, 0.0f), Vec2(0.0f, 1.0f), Vec4(1.0f)},
    {Vec3( 0.5f, 0.5f, 0.0f), Vec2(1.0f, 1.0f), Vec4(1.0f)},
    {Vec3( 0.5f, 0.5f, 0.0f), Vec2(1.0f, 1.0f), Vec4(1.0f)},
    {Vec3( 0.5f,-0.5f, 0.0f), Vec2(1.0f, 0.0f), Vec4(1.0f)},
    {Vec3(-0.5f,-0.5f, 0.0f), Vec2(0.0f, 0.0f), Vec4(1.0f)},
  };

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(mLoadingShader);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLint textureLoc = glGetUniformLocation(mLoadingShader, "Texture");
  GLint transformLoc = glGetUniformLocation(mLoadingShader, "Transform");
  GLint uvTransformLoc = glGetUniformLocation(mLoadingShader, "UvTransform");

  GLint alphaLoc = glGetUniformLocation(mLoadingShader, "Alpha");
  glUniform1fv(alphaLoc, 1, &alpha);

  GLint textureSlot = 0;
  glActiveTexture(GL_TEXTURE0 + textureSlot);
  glUniform1iv(textureLoc, 1, &textureSlot);

  glUniformMatrix3fv(uvTransformLoc, 1, cTransposeMatrices, Mat3::cIdentity.array);

  if (!splashMode)
  {
    // Loading
    glUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, loadingTransform.array);
    glBindTexture(GL_TEXTURE_2D, loadingTexture->mId);
    mStreamedVertexBuffer.AddVertices(quadVertices, 6, PrimitiveType::Triangles);
    mStreamedVertexBuffer.FlushBuffer(true);

    // Progress bar
    glUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, progressTransform.array);
    glBindTexture(GL_TEXTURE_2D, whiteTexture->mId);
    mStreamedVertexBuffer.AddVertices(quadVertices, 6, PrimitiveType::Triangles);
    mStreamedVertexBuffer.FlushBuffer(true);

    // Progress text
    if (progressText.Size() > 0)
    {
      glUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, textTransform.array);
      glBindTexture(GL_TEXTURE_2D, fontTexture->mId);
      mStreamedVertexBuffer.AddVertices(&progressText[0], progressText.Size(), PrimitiveType::Triangles);
      mStreamedVertexBuffer.FlushBuffer(true);
    }

    // Logo
    glUniformMatrix3fv(uvTransformLoc, 1, cTransposeMatrices, logoUvTransform.array);
    glUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, logoTransform.array);
    glBindTexture(GL_TEXTURE_2D, logoTexture->mId);
    mStreamedVertexBuffer.AddVertices(quadVertices, 6, PrimitiveType::Triangles);
    mStreamedVertexBuffer.FlushBuffer(true);
  }
  else
  {
    // Splash
    glUniformMatrix4fv(transformLoc, 1, cTransposeMatrices, splashTransform.array);
    glBindTexture(GL_TEXTURE_2D, splashTexture->mId);
    mStreamedVertexBuffer.AddVertices(quadVertices, 6, PrimitiveType::Triangles);
    mStreamedVertexBuffer.FlushBuffer(true);
  }

  glDisable(GL_BLEND);
  glUseProgram(0);

  SwapBuffers((HDC)mDeviceContext);
}

GlShader* OpenglRenderer::GetShader(ShaderKey& shaderKey)
{
  // Check if new shader is pending
  if (mShaderEntries.ContainsKey(shaderKey))
  {
    ShaderEntry& entry = mShaderEntries[shaderKey];

    CreateShader(entry);
    mShaderEntries.Erase(shaderKey);

    return mGlShaders.FindPointer(shaderKey);
  }

  // Find existing shader
  if (mGlShaders.ContainsKey(shaderKey))
    return mGlShaders.FindPointer(shaderKey);

  // No shader found
  return nullptr;
}

void OpenglRenderer::DoRenderTasks(RenderTasks* renderTasks, RenderQueues* renderQueues)
{
  mRenderTasks = renderTasks;
  mRenderQueues = renderQueues;

  forRange (RenderTaskRange& taskRange, mRenderTasks->mRenderTaskRanges.All())
    DoRenderTaskRange(taskRange);

  SwapBuffers((HDC)mDeviceContext);

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
    byte* task = &mRenderTasks->mRenderTaskBuffer.mRenderTaskData[taskIndex];

    switch (*task)
    {
      case RenderTaskType::ClearTarget:
      DoRenderTaskClearTarget((RenderTaskClearTarget*)task);
      taskIndex += sizeof(RenderTaskClearTarget);
      break;

      case RenderTaskType::RenderPass:
      DoRenderTaskRenderPass((RenderTaskRenderPass*)task);
      taskIndex += sizeof(RenderTaskRenderPass);
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

  glStencilMask(task->mStencilWriteMask);
  glDepthMask(true);

  glClearColor(task->mColor.x, task->mColor.y, task->mColor.z, task->mColor.w);
  glClearDepth(task->mDepth);
  glClearStencil(0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glStencilMask(0);
  glDepthMask(false);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenglRenderer::DoRenderTaskRenderPass(RenderTaskRenderPass* task)
{
  mViewportSize = IntVec2(task->mRenderSettings.mTargetsWidth, task->mRenderSettings.mTargetsHeight);
  if (mViewportSize.x == 0 || mViewportSize.y == 0)
    return;

  mShaderInputsId = task->mShaderInputsId;
  mRenderPassName = task->mRenderPassName;

  SetRenderSettings(task->mRenderSettings, mDriverSupport.mMultiTargetBlend);
  mClipMode = task->mRenderSettings.mScissorMode == ScissorMode::Enabled;

  SetRenderTargets(task->mRenderSettings);

  glViewport(0, 0, mViewportSize.x, mViewportSize.y);

  IndexRange viewNodeRange = mViewBlock->mRenderGroupRanges[task->mRenderGroupIndex];

  for (uint i = viewNodeRange.start; i < viewNodeRange.end; ++i)
  {
    ViewNode& viewNode = mViewBlock->mViewNodes[i];
    FrameNode& frameNode = mFrameBlock->mFrameNodes[viewNode.mFrameNodeIndex];

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
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

  ShaderKey shaderKey(compositeName, StringPair(String("PostVertex"), String()));
  GlShader* shader = GetShader(shaderKey);
  if (shader == nullptr)
    return;

  SetRenderTargets(task->mRenderSettings);
  SetRenderSettings(task->mRenderSettings, mDriverSupport.mMultiTargetBlend);

  glViewport(0, 0, mViewportSize.x, mViewportSize.y);

  SetShader(shader->mId);

  SetShaderParameters(mFrameBlock, mViewBlock);

  // Set Material or PostProcess fragment parameters
  mNextTextureSlot = 0;
  SetShaderParameters(resourceId, task->mShaderInputsId, mNextTextureSlot);
  SetShaderParameters(cGlobalShaderInputsId, task->mShaderInputsId, mNextTextureSlot);

  // draw fullscreen triangle
  glBindVertexArray(mTriangleArray);
  glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)0);
  glBindVertexArray(0);

  SetShader(0);
  SetRenderSettings(RenderSettings(), mDriverSupport.mMultiTargetBlend);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenglRenderer::DoRenderTaskBackBufferBlit(RenderTaskBackBufferBlit* task)
{
  GlTextureRenderData* renderData = (GlTextureRenderData*)task->mColorTarget;
  ScreenViewport viewport = task->mViewport;

  RECT rect;
  GetClientRect((HWND)mWindow, &rect);
  IntVec2 size = IntVec2(rect.right - rect.left, rect.bottom - rect.top);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, mSingleTargetFbo);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderData->mId, 0);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
  CheckFramebufferStatus();
  glBlitFramebuffer(0, 0, task->mTextureWidth, task->mTextureHeight, viewport.x, viewport.y, viewport.x + viewport.width, viewport.y + viewport.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  // Stretch texture to fill the whole back buffer
  //glBlitFramebuffer(0, 0, task->mTextureWidth, task->mTextureHeight, 0, 0, size.x, size.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void OpenglRenderer::DoRenderTaskTextureUpdate(RenderTaskTextureUpdate* task)
{
  AddTextureJob job;
  job.mRenderData = task->mRenderData;
  job.mWidth = task->mWidth;
  job.mHeight = task->mHeight;
  job.mType = task->mType;
  job.mFormat = task->mFormat;
  job.mAddressingX = task->mAddressingX;
  job.mAddressingY = task->mAddressingY;
  job.mFiltering = task->mFiltering;
  job.mCompareMode = task->mCompareMode;
  job.mCompareFunc = task->mCompareFunc;
  job.mAnisotropy = task->mAnisotropy;
  job.mMipMapping = task->mMipMapping;

  job.mMipCount = 0;
  job.mTotalDataSize = 0;
  job.mImageData = nullptr;
  job.mMipHeaders = nullptr;

  AddTexture(&job);
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
  ShaderKey shaderKey(materialData->mCompositeName, StringPair(mCoreVertexTypeNames[frameNode.mCoreVertexType], mRenderPassName));
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

  // On change of materials, material inputs followed by global inputs have to be reset
  if (materialData->mResourceId != mActiveMaterial)
  {
    SetShaderParameters((u64)materialData->mResourceId, mShaderInputsId, mNextTextureSlot);
    SetShaderParameters(cGlobalShaderInputsId, mShaderInputsId, mNextTextureSlot);

    mActiveMaterial = (u64)materialData->mResourceId;
  }

  // Per object shader inputs
  if (frameNode.mShaderInputRange.Count() != 0)
  {
    SetShaderParameters(frameNode.mShaderInputRange, mNextTextureSlot);
    // Since object input overrides could apply to other fragments that aren't from the material (i.e. RenderPass)
    // have to trigger a reset of all inputs, done by resetting active material, but shader does not have to be reset
    mActiveMaterial = 0;
  }

  GlTextureRenderData* textureData = (GlTextureRenderData*)frameNode.mTextureRenderData;
  if (textureData != nullptr)
  {
    // Don't need to use a permanent texture slot
    uint textureSlot = mNextTextureSlot;
    BindTexture(TextureType::Texture2D, textureSlot, textureData->mId, mDriverSupport.mSamplerObjects);
    SetShaderParameter(ShaderInputType::Texture, "HeightMapWeights", &textureSlot);
  }

  glBindVertexArray(meshData->mVertexArray);
  if (meshData->mIndexBuffer == 0)
    // If nothing is bound, glDrawArrays will invoke the shader pipeline the given number of times
    glDrawArrays(GlPrimitiveType(meshData->mPrimitiveType), 0, meshData->mIndexCount);
  else
    glDrawElements(GlPrimitiveType(meshData->mPrimitiveType), meshData->mIndexCount, GL_UNSIGNED_INT, (void*)0);
  glBindVertexArray(0);
}

void OpenglRenderer::DrawStreamed(ViewNode& viewNode, FrameNode& frameNode)
{
  GlMaterialRenderData* materialData = (GlMaterialRenderData*)frameNode.mMaterialRenderData;
  if (materialData == nullptr)
    return;

  // Shader permutation lookup for vertex type and render pass
  ShaderKey shaderKey(materialData->mCompositeName, StringPair(mCoreVertexTypeNames[frameNode.mCoreVertexType], mRenderPassName));
  GlShader* shader = GetShader(shaderKey);
  if (shader == nullptr)
    return;

  if (viewNode.mStreamedVertexCount == 0)
    return;

  ResourceId materialId = materialData->mResourceId;
  GLuint shaderId = shader->mId;
  GLuint textureId = 0;

  GlTextureRenderData* textureData = (GlTextureRenderData*)frameNode.mTextureRenderData;
  if (textureData != nullptr)
    textureId = textureData->mId;

  if (mCurrentLineWidth != frameNode.mBorderThickness)
  {
    mCurrentLineWidth = frameNode.mBorderThickness;
    mStreamedVertexBuffer.FlushBuffer(false);
    glLineWidth(frameNode.mBorderThickness);
  }

  if (mClipMode && frameNode.mClip != mCurrentClip)
  {
    mStreamedVertexBuffer.FlushBuffer(false);
    mCurrentClip = frameNode.mClip;
    glScissor((int)mCurrentClip.x, mViewportSize.y - (int)mCurrentClip.y - (int)mCurrentClip.w, (int)mCurrentClip.z, (int)mCurrentClip.w);
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
      SetShaderParameter(ShaderInputType::Texture, "SpriteSource", &mNextTextureSlot);
      ++mNextTextureSlot;
    }
  }

  // Have to force an independent draw call if object has individual shader inputs
  if (frameNode.mShaderInputRange.Count() != 0)
  {
    mStreamedVertexBuffer.FlushBuffer(false);
    SetShaderParameters(frameNode.mShaderInputRange, mNextTextureSlot);
    mActiveMaterial = 0;
  }

  uint vertexStart = viewNode.mStreamedVertexStart;
  uint vertexCount = viewNode.mStreamedVertexCount;
  mStreamedVertexBuffer.AddVertices(&mRenderQueues->mStreamedVertices[vertexStart], vertexCount, viewNode.mStreamedVertexType);
}

void OpenglRenderer::SetShaderParameter(ShaderInputType::Enum uniformType, StringParam name, void* data)
{
  GLint location = glGetUniformLocation(mActiveShader, name.c_str());
  mUniformFunctions[uniformType](location, 1, data);
}

void OpenglRenderer::SetShaderParameterMatrix(StringParam name, Mat3& transform)
{
  GLint location = glGetUniformLocation(mActiveShader, name.c_str());
  glUniformMatrix3fv(location, 1, cTransposeMatrices, transform.array);
}

void OpenglRenderer::SetShaderParameterMatrix(StringParam name, Mat4& transform)
{
  GLint location = glGetUniformLocation(mActiveShader, name.c_str());
  glUniformMatrix4fv(location, 1, cTransposeMatrices, transform.array);
}

void OpenglRenderer::SetShaderParameterMatrixInv(StringParam name, Mat3& transform)
{
  GLint location = glGetUniformLocation(mActiveShader, name.c_str());
  if (location != -1)
  {
    Mat3 inverse = transform.Inverted();
    glUniformMatrix3fv(location, 1, cTransposeMatrices, inverse.array);
  }
}

void OpenglRenderer::SetShaderParameterMatrixInv(StringParam name, Mat4& transform)
{
  GLint location = glGetUniformLocation(mActiveShader, name.c_str());
  if (location != -1)
  {
    Mat4 inverse = transform.Inverted();
    glUniformMatrix4fv(location, 1, cTransposeMatrices, inverse.array);
  }
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

      remappedBoneTransforms.PushBack(mRenderQueues->mSkinningBuffer[bufferIndex] * meshData->mBones[meshIndex].mBindTransform);
    }

    GLint location = glGetUniformLocation(mActiveShader, "BoneTransforms");
    glUniformMatrix4fv(location, remappedBoneTransforms.Size(), cTransposeMatrices, remappedBoneTransforms[0].array);
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
      // If any sampler attributes were set on the shader input then mSamplerSettings will be non-zero
      // If driver does not have sampler object support then this feature does nothing
      if (input.mSamplerSettings != 0 && mDriverSupport.mSamplerObjects)
      {
        u32 samplerSettings = input.mSamplerSettings;
        // Use texture settings as defaults so that only attributes specified on shader input differ from the texture
        SamplerSettings::FillDefaults(samplerSettings, textureData->mSamplerSettings);
        GLuint sampler = GetSampler(samplerSettings);
        glBindSampler(nextTextureSlot, sampler);
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
  ShaderKey shaderKey(entry.mComposite, StringPair(entry.mCoreVertex, entry.mRenderPass));

  GLuint shaderId = 0;
  CreateShader(entry.mVertexShader, entry.mGeometryShader, entry.mPixelShader, shaderId);

  // Shouldn't fail at this point. Not currently handling gl errors.
  ErrorIf(shaderId == 0, "Failed to compile or link shader.");

  GlShader shader;
  shader.mId = shaderId;

  // Must delete old shader after new one is created or something is getting incorrectly cached/generated
  if (mGlShaders.ContainsKey(shaderKey))
    glDeleteProgram(mGlShaders[shaderKey].mId);

  mGlShaders.Insert(shaderKey, shader);
}

void OpenglRenderer::CreateShader(StringParam vertexSource, StringParam geometrySource, StringParam pixelSource, GLuint& shader)
{
  //DebugPrint("%s\n", vertexSource.c_str());
  //DebugPrint("%s\n", geometrySource.c_str());
  //DebugPrint("%s\n", pixelSource.c_str());

  GLuint program = glCreateProgram();

  const GLchar* vertexSourceData = vertexSource.Data();
  GLint vertexSourceSize = vertexSource.SizeInBytes();
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSourceData, &vertexSourceSize);
  glCompileShader(vertexShader);
  CheckShader(vertexShader);
  glAttachShader(program, vertexShader);

  GLuint geometryShader = 0;
  if (!geometrySource.Empty())
  {
    const GLchar* geometrySourceData = geometrySource.Data();
    GLint geometrySourceSize = geometrySource.SizeInBytes();
    geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometryShader, 1, &geometrySourceData, &geometrySourceSize);
    glCompileShader(geometryShader);
    CheckShader(geometryShader);
    glAttachShader(program, geometryShader);
  }

  const GLchar* pixelSourceData = pixelSource.Data();
  GLint pixelSourceSize = pixelSource.SizeInBytes();
  GLuint pixelShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(pixelShader, 1, &pixelSourceData, &pixelSourceSize);
  glCompileShader(pixelShader);
  CheckShader(pixelShader);
  glAttachShader(program, pixelShader);

  glBindAttribLocation(program, VertexSemantic::Position, "attLocalPosition");
  glBindAttribLocation(program, VertexSemantic::Normal, "attLocalNormal");
  glBindAttribLocation(program, VertexSemantic::Tangent, "attLocalTangent");
  glBindAttribLocation(program, VertexSemantic::Bitangent, "attLocalBitangent");
  glBindAttribLocation(program, VertexSemantic::Uv, "attUv");
  glBindAttribLocation(program, VertexSemantic::UvAux, "attUvAux");
  glBindAttribLocation(program, VertexSemantic::Color, "attColor");
  glBindAttribLocation(program, VertexSemantic::ColorAux, "attColorAux");
  glBindAttribLocation(program, VertexSemantic::BoneIndices, "attBoneIndices");
  glBindAttribLocation(program, VertexSemantic::BoneWeights, "attBoneWeights");
  // Not implemented by geometry processor
  glBindAttribLocation(program, 10, "attAux0");
  glBindAttribLocation(program, 11, "attAux1");
  glBindAttribLocation(program, 12, "attAux2");
  glBindAttribLocation(program, 13, "attAux3");
  glBindAttribLocation(program, 14, "attAux4");
  glBindAttribLocation(program, 15, "attAux5");

  glLinkProgram(program);

  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE)
  {
    GLint infoLogLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    GLchar* strInfoLog = (GLchar*)alloca(infoLogLength + 1);
    glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
    ZPrint("Link Error\n%s\n", strInfoLog);
  }
  else
  {
    shader = program;
  }

  glDetachShader(program, vertexShader);
  glDetachShader(program, pixelShader);
  glDeleteShader(vertexShader);
  glDeleteShader(pixelShader);

  if (geometryShader != 0)
  {
    glDetachShader(program, geometryShader);
    glDeleteShader(geometryShader);
  }

  if (status == GL_FALSE)
    glDeleteProgram(program);
}

void OpenglRenderer::SetShader(GLuint shader)
{
  mActiveShader = shader;
  glUseProgram(mActiveShader);
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
  GlMeshRenderData* glRenderData = (GlMeshRenderData*)renderData;
  glDeleteVertexArrays(1, &glRenderData->mVertexArray);
  glDeleteBuffers(1, &glRenderData->mVertexBuffer);
  glDeleteBuffers(1, &glRenderData->mIndexBuffer);

  delete renderData;
}

void OpenglRenderer::DestroyRenderData(GlTextureRenderData* renderData)
{
  GlTextureRenderData* glRenderData = (GlTextureRenderData*)renderData;
  glDeleteTextures(1, &glRenderData->mId);
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
  glGenSamplers(1, &newSampler);

  TextureAddressing::Enum addressingX = SamplerSettings::AddressingX(samplerSettings);
  TextureAddressing::Enum addressingY = SamplerSettings::AddressingY(samplerSettings);
  TextureFiltering::Enum filtering = SamplerSettings::Filtering(samplerSettings);
  TextureCompareMode::Enum compareMode = SamplerSettings::CompareMode(samplerSettings);
  TextureCompareFunc::Enum compareFunc = SamplerSettings::CompareFunc(samplerSettings);

  glSamplerParameteri(newSampler, GL_TEXTURE_WRAP_S, GlTextureAddressing(addressingX));
  glSamplerParameteri(newSampler, GL_TEXTURE_WRAP_T, GlTextureAddressing(addressingY));
  glSamplerParameteri(newSampler, GL_TEXTURE_MIN_FILTER, GlTextureFilteringMin(filtering));
  glSamplerParameteri(newSampler, GL_TEXTURE_MAG_FILTER, GlTextureFilteringMag(filtering));
  glSamplerParameteri(newSampler, GL_TEXTURE_COMPARE_MODE, GlCompareMode(compareMode));
  glSamplerParameteri(newSampler, GL_TEXTURE_COMPARE_FUNC, GlCompareFunc(compareFunc));

  mSamplers.Insert(samplerSettings, newSampler);

  return newSampler;
}

void OpenglRenderer::DestroyUnusedSamplers()
{
  forRange (u32 id, mUnusedSamplers.All())
  {
    GLuint sampler = mSamplers[id];
    glDeleteSamplers(1, &sampler);
    mSamplers.Erase(id);
  }
  mUnusedSamplers.Clear();
  mUnusedSamplers.Append(mSamplers.Keys());
}

} // namespace Zero
