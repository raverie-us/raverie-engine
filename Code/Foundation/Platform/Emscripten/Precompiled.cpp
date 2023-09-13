// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

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

} // namespace Zero
