// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"
#include "../OpenGL/OpenglRenderer.hpp"

namespace Zero
{
class OpenglRendererEmscripten : public OpenglRenderer
{
public:
  OpenglRendererEmscripten(OsHandle windowHandle, String& error);
  ~OpenglRendererEmscripten() override;
};

OpenglRendererEmscripten::OpenglRendererEmscripten(OsHandle windowHandle, String& error)
{
  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.antialias = false;
  attrs.majorVersion = 2;
  attrs.minorVersion = 0;
  attrs.alpha = false;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE deviceContext = emscripten_webgl_create_context("#canvas", &attrs);
  emscripten_webgl_make_context_current(deviceContext);

  // Call the base initialize now that we've created the OpenGL context.
  Initialize(windowHandle, (OsHandle)(uintptr_t)deviceContext, nullptr, error);
}

OpenglRendererEmscripten::~OpenglRendererEmscripten()
{
  // Must call this before we destroy the OpenGL context.
  Shutdown();

  emscripten_webgl_destroy_context((EMSCRIPTEN_WEBGL_CONTEXT_HANDLE)mDeviceContext);
}

Renderer* CreateRenderer(OsHandle windowHandle, String& error)
{
  return new OpenglRendererEmscripten(windowHandle, error);
}

void zglSetSwapInterval(OpenglRenderer* renderer, int interval)
{
}

IntVec2 zglGetWindowRenderableSize(OpenglRenderer* renderer)
{
  // Use a default size just in case the call fails.
  IntVec2 size(1024, 768);
  emscripten_webgl_get_drawing_buffer_size((EMSCRIPTEN_WEBGL_CONTEXT_HANDLE)renderer->mDeviceContext, &size.x, &size.y);
  return size;
}

void zglSwapBuffers(OpenglRenderer* renderer)
{
  emscripten_webgl_commit_frame();
}

} // namespace Zero
