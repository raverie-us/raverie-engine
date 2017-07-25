///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

// Using static GLEW
#define GLEW_STATIC
#pragma warning( disable : 4005  ) 
// Include glew before OpenGl
#include <GL/glew.h>
#include <GL/wglew.h>
// Include OpenGl
#include <GL/GL.h>


// Link Libraries
#pragma comment(lib, "OpenGL32.Lib")
#ifdef NDEBUG
#pragma comment(lib, "glew32s.lib")
#else
#pragma comment(lib, "glew32sd.lib")
#endif

PIXELFORMATDESCRIPTOR Setup32Bit()
{
  PIXELFORMATDESCRIPTOR pfd =
  {
    sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd
    1,                                // version number
    PFD_DRAW_TO_WINDOW |              // support window
    PFD_SUPPORT_OPENGL |              // support OpenGL
    PFD_DOUBLEBUFFER,                 // double buffered
    PFD_TYPE_RGBA,                    // RGBA type
    24,                               // 24-bit color depth
    0, 0, 0, 0, 0, 0,                 // color bits ignored
    8,                                // 8-bit alpha color
    0,                                // shift bit ignored
    0,                                // no accumulation buffer
    0, 0, 0, 0,                       // accum bits ignored
    32,                               // 32-bit z-buffer
    0,                                // no stencil buffer
    0,                                // no auxiliary buffer
    PFD_MAIN_PLANE,                   // main layer
    0,                                // reserved
    0, 0, 0                           // layer masks ignored
  };

  return pfd;
}

struct GlTextureEnums
{
  GLint mInternalFormat;
  GLint mFormat;
  GLint mType;
};
// Two enum formats, the depth format and color
GlTextureEnums gTextureEnums[] =
{
  {GL_RGBA32F, GL_RGBA, GL_FLOAT},
  {GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT}
};

DeclareEnum5(VertexSemantic,
              Position,
              Normal,
              TexCoord0,
              TexCoord1,
              None);

GlslRenderer::GlslRenderer(void* context)
{
  InitializeSystem(context);
  InitializeBuffers();
  InitializeTargets();

  mZilchFragmentTypeToGlsl.Insert(FragmentType::Vertex, GL_VERTEX_SHADER);
  mZilchFragmentTypeToGlsl.Insert(FragmentType::Geometry, GL_GEOMETRY_SHADER);
  mZilchFragmentTypeToGlsl.Insert(FragmentType::Pixel, GL_FRAGMENT_SHADER);
}

GlslRenderer::~GlslRenderer()
{
  glDeleteFramebuffers(1, &mMultiTargetFbo);

  glDeleteBuffers(1, &mTriangleVertex);
  glDeleteBuffers(1, &mTriangleIndex);
  glDeleteVertexArrays(1, &mTriangleArray);
}

void GlslRenderer::InitializeSystem(void* context)
{
  HDC drawContext = (HDC)context;
  PIXELFORMATDESCRIPTOR pfd = Setup32Bit();
  // Choose the device context's best available pixel format match
  GLuint pixelFormat = ChoosePixelFormat(drawContext, &pfd);

  // Set that pixel format as the active format
  BOOL success = SetPixelFormat(drawContext, pixelFormat, &pfd);
  ErrorIf(!success, "Failed to set pixel format.");

  // Create the shared context
  HGLRC sharedContext = wglCreateContext(drawContext);
  wglMakeCurrent(drawContext, sharedContext);

  GLenum glewInitStatus = glewInit();
  if(glewInitStatus != GLEW_OK)
    Error("GLEW failed to initialize with error: %d", glewInitStatus);
}

void GlslRenderer::InitializeBuffers()
{
  uint triangleIndices[] = {0, 1, 2};

  glGenVertexArrays(1, &mTriangleArray);
  glBindVertexArray(mTriangleArray);

  glGenBuffers(1, &mTriangleVertex);
  glBindBuffer(GL_ARRAY_BUFFER, mTriangleVertex);
  glBufferData(GL_ARRAY_BUFFER, sizeof(RenderVertex) * 3, mFullScreenTriangleVerts, GL_STATIC_DRAW);

  glEnableVertexAttribArray(VertexSemantic::Position);
  glVertexAttribPointer(VertexSemantic::Position, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void*)offsetof(RenderVertex, mPosition));
  glEnableVertexAttribArray(VertexSemantic::TexCoord0);
  glVertexAttribPointer(VertexSemantic::TexCoord0, 2, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void*)offsetof(RenderVertex, mUv));

  glGenBuffers(1, &mTriangleIndex);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mTriangleIndex);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * 3, triangleIndices, GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void GlslRenderer::InitializeTargets()
{
  glGenFramebuffers(1, &mMultiTargetFbo);

  // appears that this can be set once on an fbo, not sure if there are performance penalties for having everything always active
  glBindFramebuffer(GL_FRAMEBUFFER, mMultiTargetFbo);
  GLenum buffers[4];
  for(uint i = 0; i < 4; ++i)
    buffers[i] = GL_COLOR_ATTACHMENT0 + i;
  glDrawBuffers(4, buffers);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  mTextureIds.Resize(mMaxRenderTargets);
  for(uint i = 0; i < mMaxRenderTargets; ++i)
  {
    glGenTextures(1, &mTextureIds[i]);
    glBindTexture(GL_TEXTURE_2D, mTextureIds[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, gTextureEnums[0].mInternalFormat, mScreenWidth, mScreenHeight, 0, gTextureEnums[0].mFormat, gTextureEnums[0].mType, NULL);
  }
}

bool GlslRenderer::CompileShader(StringParam filePath, StringParam shaderSource, FragmentType::Enum type, ErrorReporter& reporter)
{
  // Get the glsl shader type id
  GLenum shaderType = FragmentTypeToShaderType(type, reporter);
  
  int shaderId;
  return CompileShaderInternal(filePath, shaderSource, shaderType, shaderId, reporter);
}

bool GlslRenderer::CompileAndLinkShader(Array<FragmentInfo>& fragments, ErrorReporter& reporter)
{
  int programId;
  return CompileAndLinkInternal(fragments, programId, reporter);
}

void GlslRenderer::RunPostProcess(Array<FragmentInfo>& fragments, RenderResult& result, ErrorReporter& reporter)
{
  // Compile the shaders
  int programId;
  bool success = CompileAndLinkInternal(fragments, programId, reporter);

  // Bind the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, mMultiTargetFbo);
  for(uint i = 0; i < mMaxRenderTargets; ++i)
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, mTextureIds[i], 0);

  // Validate the framebuffer
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE)
    DebugPrint("Framebuffer incomplete\n");

  // Clear the target
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Start using the shader
  glUseProgram(programId);

  // Render to the target
  glBindVertexArray(mTriangleArray);
  glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)0);
  glBindVertexArray(0);

  // Stop using the shader
  glUseProgram(0);

  // Collect the results
  for(size_t i = 0; i < mMaxRenderTargets; ++i)
  {
    // Setup which target to read from
    glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
    // Read 1 pixel from that target
    glReadPixels(1, 1, 1, 1, GL_RGBA, GL_FLOAT, result.mData + i);
  }
}

int GlslRenderer::FragmentTypeToShaderType(FragmentType::Enum fragmentType, ErrorReporter& reporter)
{
  int glslType = mZilchFragmentTypeToGlsl.FindValue(fragmentType, -1);
  // We only handle vertex and pixel right now
  if(glslType == -1)
    reporter.Report("Invalid shader type");
  return glslType;
}

bool GlslRenderer::CompileShaderInternal(StringParam filePath, StringParam shaderSource, int shaderType, int& shaderId, ErrorReporter& reporter)
{
  shaderId = glCreateShader(shaderType);

  // Load the code
  cstr code = shaderSource.Data();
  GLint length = shaderSource.SizeInBytes();
  glShaderSource(shaderId, 1, &code, &length);

  // Compile the shader
  glCompileShader(shaderId);

  // Pull the info log from the shader compilation (do this regardless of compilation error)
  GLint infoLogLength;
  glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
  GLchar* strInfoLog = (char*)alloca(infoLogLength + 1);
  glGetShaderInfoLog(shaderId, infoLogLength, NULL, strInfoLog);
  String log = strInfoLog;

  // Check for compiler errors
  GLint status = 0;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
  
  String workDir = GetWorkingDirectory();
  String fullPath = FilePath::Combine(workDir, filePath);
  Array<LogMessage> warnings;
  Array<LogMessage> errors;
  // Extract warnings and errors
  StringSplitRange lines = log.Split("\n");
  for(; !lines.Empty(); lines.PopFront())
  {
    StringRange line = lines.Front();
    StringRange lineNumber = line.FindRangeExclusive("(", ")");

    if(line.Contains("warning"))
      warnings.PushBack(LogMessage(fullPath, lineNumber, line));
    else if(line.Contains("error"))
      errors.PushBack(LogMessage(fullPath, lineNumber, line));
  }
  
  // Always report warnings
  if(!warnings.Empty())
    reporter.ReportCompilationWarning(warnings);

  // Report errors if compilation failed
  if(status == GL_FALSE)
  {
    reporter.ReportCompilationError(errors);
    return false;
  }

  return true;
}

bool GlslRenderer::LinkInternal(const Array<int>& shaderIds, int& programId, ErrorReporter& reporter)
{
  //create the shader program
  programId = glCreateProgram();

  // Attach shaders to program
  for(size_t i = 0; i < shaderIds.Size(); ++i)
  {
    glAttachShader(programId, shaderIds[i]);
  }

  glBindAttribLocation(programId, VertexSemantic::Position, "attPosition");
  glBindAttribLocation(programId, VertexSemantic::TexCoord0, "attUv");

  // Link the program vertex shader and pixel shader
  glLinkProgram(programId);

  // Check for linker errors
  GLint status;
  glGetProgramiv(programId, GL_LINK_STATUS, &status);
  if(status == GL_FALSE)
  {
    // Print out linker errors
    GLint infoLogLength;
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);

    GLchar* strInfoLog = (GLchar*)alloca(infoLogLength + 1);
    glGetProgramInfoLog(programId, infoLogLength, NULL, strInfoLog);

    // Not sure what to do for file/line here so pass empty string?
    reporter.ReportLinkerError(String(), String(), strInfoLog);
    return false;
  }
  return true;
}

bool GlslRenderer::CompileAndLinkInternal(Array<FragmentInfo>& fragments, int& programId, ErrorReporter& reporter)
{
  // Compile the shaders
  bool success = true;
  Array<int> shaderIds;

  for(size_t i = 0; i < fragments.Size(); ++i)
  {
    FragmentInfo& fragmentInfo = fragments[i];
    // If this fragment had no code then it didn't exist so skip it
    if(fragmentInfo.mFragmentCode.Empty())
      continue;

    // Find out what the glsl shader type is
    int glShaderType = FragmentTypeToShaderType(fragmentInfo.mFragmentType, reporter);

    int shaderId;
    success &= CompileShaderInternal(fragmentInfo.mFilePath, fragmentInfo.mFragmentCode, glShaderType, shaderId, reporter);
    shaderIds.PushBack(shaderId);
  }

  // Link if we succeeded to compile
  if(success)
    success &= LinkInternal(shaderIds, programId, reporter);

  return success;
}
