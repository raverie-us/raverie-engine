// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"
#include "SDL.h"
#include "OpenglRenderer.hpp"

namespace Zero
{
class OpenglRendererSDL : public OpenglRenderer
{
public:
  OpenglRendererSDL(OsHandle windowHandle, String& error);
  ~OpenglRendererSDL() override;
};

OpenglRendererSDL::OpenglRendererSDL(OsHandle windowHandle, String& error)
{
  SDL_Window* window = (SDL_Window*)windowHandle;

  SDL_GLContext deviceContext = SDL_GL_CreateContext(window);

  // If we end up using OpenGL, then set our OpenGL version.
  // SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions
  // are disabled
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  // 3.2 is part of the modern versions of OpenGL, but most video cards whould
  // be able to run it
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  // We don't want the back buffer to be multi-sampled because we can't blit a
  // frame buffer to it.
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

  // Turn on double buffering with a 24bit Z buffer.
  // You may need to change this to 16 or 32 for your system
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // Call the base initialize now that we've created the OpenGL context.
  Initialize(windowHandle, deviceContext, nullptr, error);
}

OpenglRendererSDL::~OpenglRendererSDL()
{
  // Must call this before we destroy the OpenGL context.
  Shutdown();

  SDL_GL_DeleteContext((SDL_GLContext)mDeviceContext);
}

Renderer* CreateRenderer(OsHandle windowHandle, String& error)
{
  return new OpenglRendererSDL(windowHandle, error);
}

void zglSetSwapInterval(OpenglRenderer* renderer, int interval)
{
// On Emscripten we don't want to set this because the browser emits errors.
#if !defined(WelderTargetOsEmscripten)
  SDL_GL_SetSwapInterval(interval);
#endif
}

IntVec2 zglGetWindowRenderableSize(OpenglRenderer* renderer)
{
  // Use a default size just in case the call fails.
  IntVec2 size(1024, 768);
  SDL_GL_GetDrawableSize((SDL_Window*)renderer->mWindow, &size.x, &size.y);
  return size;
}

void zglSwapBuffers(OpenglRenderer* renderer)
{
  SDL_GL_SwapWindow((SDL_Window*)renderer->mWindow);
}

} // namespace Zero
