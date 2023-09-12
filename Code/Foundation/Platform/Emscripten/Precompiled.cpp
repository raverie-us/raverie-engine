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
}

using namespace Zero;

namespace Zero
{
extern SDL_Window* gSdlMainWindow;
}

// TODO(Trevor.Sundberg): Refactor the drag drop and open dialogs so that they can just directly input
// arrays of buffers (no need to write to the disk, read it and then write to the VFS, and read it again...).
bool CopyToVFS(StringParam file)
{
  FILE* sourceFile = fopen(file.c_str(), "rb");
  if (sourceFile == nullptr)
    return false;

  File destFile;
  if (!destFile.Open(file, FileMode::Write, FileAccessPattern::Sequential))
  {
    fclose(sourceFile);
    return false;
  }

  const int bufferSize = 4096;

  bool result = true;
  char buf[bufferSize];
  for (;;)
  {
    size_t bytesRead = fread(buf, 1, bufferSize, sourceFile);
    if (ferror(sourceFile))
    {
      result = false;
      break;
    }

    size_t written = destFile.Write((byte*)buf, bytesRead);
    if (!written)
    {
      result = false;
      break;
    }

    if (bytesRead != bufferSize)
      break;
  }

  fclose(sourceFile);
  return result;
}

extern "C" EMSCRIPTEN_KEEPALIVE void EmscriptenOnPaste(const char* text)
{
  if (!Shell::sInstance->mOnPaste)
    return;
  ClipboardData data;
  data.mHasText = true;
  data.mText = text;
  Shell::sInstance->mOnPaste(data, Shell::sInstance);
}

extern "C" EMSCRIPTEN_KEEPALIVE char* EmscriptenOnCopy(int cut)
{
  if (!Shell::sInstance->mOnCopy)
    return nullptr;
  ClipboardData data;
  Shell::sInstance->mOnCopy(data, !!cut, Shell::sInstance);
  ErrorIf(!data.mHasText && !data.mText.Empty(), "Clipboard Text was not empty, but HasText was not set");
  if (data.mHasText)
  {
    const size_t size = data.mText.SizeInBytes() + 1;
    char* buffer = (char*)malloc(size);
    memcpy(buffer, data.mText.c_str(), size);
    return buffer;
  }
  return nullptr;
}

extern "C" EMSCRIPTEN_KEEPALIVE void EmscriptenFileDropHandler(char* fileBuffer)
{
  // We're relying on the Emscripten instance only having one window with GL
  // setup. This could be changed to get a saved id somewhere for the primary
  // window.
  SDL_Window* sdlWindow = gSdlMainWindow;
  if (!sdlWindow)
    sdlWindow = SDL_GetGrabbedWindow();
  if (!sdlWindow)
    sdlWindow = SDL_GL_GetCurrentWindow();
  ReturnIf(!sdlWindow, , "Could not get SDL window for drag/drop");
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
    CopyToVFS(dropFile);

    it += stringSizeInBytes;
  }

  dropEvent.type = SDL_DROPCOMPLETE;
  dropEvent.file = nullptr;
  SDL_PushEvent((SDL_Event*)&dropEvent);

  // The filebuffer was allocated on the heap javascript side and we need to
  // free it
  free(fileBuffer);
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

EM_JS(void, EmscriptenOpenUrl, (cstr url), { window.open(UTF8ToString(url)); });

EM_JS(void, EmscriptenDownloadFile, (cstr fileName, const void* fileMemory, size_t fileSize), {
  downloadFileInMemory(UTF8ToString(fileName), fileMemory, fileSize);
});

namespace Os
{
bool ShellOpenApplication(StringParam file, StringParam parameters, StringParam workingDirectory)
{
  // Always assume the other application is in a sibling url directory
  // and that we can pass parameters as anything after the ?
  String name = FilePath::GetFileNameWithoutExtension(file);
  String url = "../../" + name + "/" + name + ".html?" + parameters;
  Os::OpenUrl(url.c_str());
  return true;
}

bool SupportsDownloadingFiles()
{
  return true;
}

void OpenUrl(cstr url)
{
  EmscriptenOpenUrl(url);
}
} // namespace Os

} // namespace Zero
