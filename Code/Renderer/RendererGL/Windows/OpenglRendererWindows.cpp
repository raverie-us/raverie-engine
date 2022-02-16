// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"
#include "OpenglRenderer.hpp"

namespace Zero
{
class OpenglRendererWindows : public OpenglRenderer
{
public:
  OpenglRendererWindows(OsHandle windowHandle, String& error);
  ~OpenglRendererWindows() override;
};

OpenglRendererWindows::OpenglRendererWindows(OsHandle windowHandle, String& error)
{
  HWND window = (HWND)windowHandle;
  HDC deviceContext = GetDC(window);

  PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR), // size of this pfd
      1,                             // version number
      PFD_DRAW_TO_WINDOW |           // support window
          PFD_SUPPORT_OPENGL |       // support OpenGL
          PFD_DOUBLEBUFFER,          // double buffered
      PFD_TYPE_RGBA,                 // RGBA type
      32,                            // 32-bit color depth
      0,
      0,
      0,
      0,
      0,
      0, // color bits ignored
      0, // no alpha buffer
      0, // shift bit ignored
      0, // no accumulation buffer
      0,
      0,
      0,
      0,              // accum bits ignored
      0,              // no z-buffer
      0,              // no stencil buffer
      0,              // no auxiliary buffer
      PFD_MAIN_PLANE, // main layer
      0,              // reserved
      0,
      0,
      0 // layer masks ignored
  };

  int pixelFormat = ChoosePixelFormat(deviceContext, &pfd);
  BOOL success = SetPixelFormat(deviceContext, pixelFormat, &pfd);
  ErrorIf(!success, "Failed to set pixel format.");

  HGLRC renderContext = wglCreateContext(deviceContext);
  wglMakeCurrent(deviceContext, renderContext);

  // Call the base initialize now that we've created the OpenGL context.
  Initialize(windowHandle, deviceContext, renderContext, error);
}

OpenglRendererWindows::~OpenglRendererWindows()
{
  // Must call this before we destroy the OpenGL context.
  Shutdown();

  wglMakeCurrent(NULL, NULL);
  wglDeleteContext((HGLRC)mRenderContext);
}

Renderer* CreateRenderer(OsHandle windowHandle, String& error)
{
  return new OpenglRendererWindows(windowHandle, error);
}

void zglSetSwapInterval(OpenglRenderer* renderer, int interval)
{
  wglSwapIntervalEXT(interval);
}

IntVec2 zglGetWindowRenderableSize(OpenglRenderer* renderer)
{
  RECT rect;
  GetClientRect((HWND)renderer->mWindow, &rect);
  IntVec2 size = IntVec2(rect.right - rect.left, rect.bottom - rect.top);
  return size;
}

void zglSwapBuffers(OpenglRenderer* renderer)
{
  SwapBuffers((HDC)renderer->mDeviceContext);
}

} // namespace Zero
