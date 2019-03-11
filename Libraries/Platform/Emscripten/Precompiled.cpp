// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

extern "C" EMSCRIPTEN_KEEPALIVE void* Znaj(unsigned int n)
{
  return malloc((size_t)n);
}

extern "C" EMSCRIPTEN_KEEPALIVE void* Znwj(unsigned int n)
{
  return malloc((size_t)n);
}

extern "C" EMSCRIPTEN_KEEPALIVE void* _Znaj(unsigned int n)
{
  return malloc((size_t)n);
}

extern "C" EMSCRIPTEN_KEEPALIVE void* _Znwj(unsigned int n)
{
  return malloc((size_t)n);
}

void glDrawBuffer(GLenum buf)
{
  GLenum drawBuffers[8] = {buf, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE};
  glDrawBuffers(1, drawBuffers);
}

static const char* const cInvalidWebGl = "This function should not be called when running Emscripten: WebGL "
                                         "functionality was not properly queried";

void glBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
  Error(cInvalidWebGl);
}

void glBlendEquationi(GLuint buf, GLenum mode)
{
  Error(cInvalidWebGl);
}

void glBlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
  Error(cInvalidWebGl);
}

void glBlendFunci(GLuint buf, GLenum sfactor, GLenum dfactor)
{
  Error(cInvalidWebGl);
}

void glEnablei(GLenum cap, GLuint index)
{
  Error(cInvalidWebGl);
}

void glDisablei(GLenum cap, GLuint index)
{
  Error(cInvalidWebGl);
};

using namespace Zero;

extern "C" EMSCRIPTEN_KEEPALIVE void EmscriptenFileDropHandler(char* fileBuffer)
{
  // We're relying on the Emscripten instance only having one window with GL
  // setup. This could be changed to get a saved id somewhere for the primary
  // window.
  SDL_Window* sdlWindow = SDL_GL_GetCurrentWindow();
  ReturnIf(!sdlWindow, , "Could not get window from SDL_GL_GetCurrentWindow");
  Uint32 windowID = SDL_GetWindowID(sdlWindow);

  // Create the drop event
  SDL_DropEvent dropEvent;
  dropEvent.windowID = windowID;
  dropEvent.type = SDL_DROPBEGIN;
  dropEvent.file = nullptr;
  SDL_PushEvent((SDL_Event*)&dropEvent);

  char* it = fileBuffer;

  // Create an SDL_DropEvent for each file dropped
  while (*it != '\0')
  {
    dropEvent.type = SDL_DROPFILE;
    // Copy the file into a new cstr for the event, must be
    // freed in the SDL event handler +1 for the null terminator.
    size_t stringSizeInBytes = strlen(it) + 1;
    char* dropFile = (char*)SDL_malloc(stringSizeInBytes);
    memcpy(dropFile, (char*)it, stringSizeInBytes);
    dropEvent.file = dropFile;
    // Set the window id and queue the event
    SDL_PushEvent((SDL_Event*)&dropEvent);

    it += stringSizeInBytes;
  }

  dropEvent.type = SDL_DROPCOMPLETE;
  dropEvent.file = nullptr;
  SDL_PushEvent((SDL_Event*)&dropEvent);

  // The filebuffer was allocated on the heap javascript side and we need to
  // free it
  free(fileBuffer);
}

EM_JS(void, EmscriptenShellOpenFileBegin, (bool multiple, const char* accept, void* configPointer), {
  if (!document)
    return;
  shellOpenFile(multiple, UTF8ToString(accept), configPointer);
});

extern "C" EMSCRIPTEN_KEEPALIVE void EmscriptenShellOpenFileEnd(char* fileBuffer, void* configPointer)
{
  FileDialogInfo& config = *(FileDialogInfo*)configPointer;

  // Note the the 'fileBuffer' can be null if we cancelled, so we check it
  // before iterating.
  if (fileBuffer)
  {
    // Loop through all the files and add them to the config.
    char* it = fileBuffer;
    while (*it != '\0')
    {
      String file(it);
      config.mFiles.PushBack(file);

      it += file.SizeInBytes() + 1;
    }
    free(fileBuffer);
  }

  // Invoke the user's provided callback.
  if (config.mCallback)
    config.mCallback(config.mFiles, config.mUserData);
}

namespace Zero
{

void Shell::OpenFile(FileDialogInfo& config)
{
  // We have no way of selecting a folder, so for now we just enable
  // multi-select.
  bool multiple = config.Flags & FileDialogFlags::MultiSelect || config.Flags & FileDialogFlags::Folder;

  StringBuilder acceptExtensions;
  forRange (FileDialogFilter& filter, config.mSearchFilters)
  {
    if (acceptExtensions.GetSize() != 0)
      acceptExtensions.Append(',');

    // Filter out all the wildcard stars '*' since html input does not use them.
    forRange (Rune rune, filter.mFilter)
    {
      if (rune == ';')
        acceptExtensions.Append(',');
      else if (rune != '*')
        acceptExtensions.Append(rune);
    }
  }

  String accept = acceptExtensions.ToString();
  EmscriptenShellOpenFileBegin(multiple, accept.c_str(), &config);
}

} // namespace Zero
