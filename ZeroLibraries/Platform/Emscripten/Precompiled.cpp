///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "../Empty/ComPort.cpp"
#include "../Empty/CrashHandler.cpp"
#include "../Empty/Debug.cpp"
#include "../Empty/DebugSymbolInformation.cpp"
#include "../Empty/DirectoryWatcher.cpp"
#include "../Empty/ExecutableResource.cpp"
#include "../Empty/Intrinsics.cpp"
#include "../Empty/Registry.cpp"
#include "../Empty/Socket.cpp"
#include "../Empty/Thread.cpp"
#include "../Empty/ThreadSync.cpp"
#include "../Posix/FileSystem.cpp"
#include "../STL/Atomic.cpp"
#include "../STL/File.cpp"
#include "../STL/FpControl.cpp"
#include "../STL/Process.cpp"
#include "../SDL/Audio.cpp"
#include "../SDL/ExternalLibrary.cpp"
#include "../SDL/Main.cpp"
#include "../SDL/OpenglRendererSDL.cpp"
#include "../SDL/Peripherals.cpp"
#include "../SDL/PlatformStandard.cpp"
#include "../SDL/Resolution.cpp"
#include "../SDL/Shell.cpp"
#include "../SDL/Timer.cpp"
#include "../SDL/Utilities.cpp"
#include "../OpenGL/OpenglRenderer.cpp"
#include "../OpenGL/OpenglRenderer.hpp"

int vsprintf_s(char* buffer, size_t numberOfElements, const char* format, va_list args)
{
  return vsnprintf(buffer, numberOfElements, format, args);
}

int sprintf_s(char* buffer, size_t numberOfElements, const char* format, ...)
{
  va_list args;
  va_start(args, format);
  int result = vsnprintf(buffer, numberOfElements, format, args);
  va_end(args);
  return result;
}

int swprintf_s(wchar_t* buffer, size_t numberOfElements, const wchar_t* format, ...)
{
  va_list args;
  va_start(args, format);
  int result = vswprintf(buffer, numberOfElements, format, args);
  va_end(args);
  return result;
}

errno_t strcat_s(char* dest, rsize_t destsz, const char* src)
{
  if (strlen(src) >= destsz)
  {
    if (dest && destsz != 0)
      *dest = '\0';
    return 1;
  }

  strcat(dest, src);
  return 0;
}

errno_t wcscat_s(wchar_t* dest, rsize_t destsz, const wchar_t* src)
{
  if (wcslen(src) >= destsz)
  {
    if (dest && destsz != 0)
      *dest = '\0';
    return 1;
  }

  wcscat(dest, src);
  return 0;
}

errno_t strcpy_s(char* dest, rsize_t destsz, const char* src)
{
  if (strlen(src) >= destsz)
  {
    if (dest && destsz != 0)
      *dest = '\0';
    return 1;
  }
    
  strcpy(dest, src);
  return 0;
}

errno_t wcscpy_s(wchar_t* dest, rsize_t destsz, const wchar_t* src)
{
  if (wcslen(src) >= destsz)
  {
    if (dest && destsz != 0)
      *dest = '\0';
    return 1;
  }

  wcscpy(dest, src);
  return 0;
}

errno_t strncpy_s(char *dest, rsize_t destsz, const char *src, rsize_t count)
{
  if (count > destsz)
  {
    if (dest && destsz != 0)
      *dest = '\0';
    return 1;
  }

  strncpy(dest, src, count);
  return 0;
}

void glDrawBuffer(GLenum buf)
{
  GLenum drawBuffers[8] = { buf, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_NONE };
  glDrawBuffers(1, drawBuffers);
}

static const char* const cInvalidWebGl = "This function should not be called when running Emscripten: WebGL functionality was not properly queried";

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

//EM_JS(const char*, EmscriptenShellOpenFile, (bool multiSelect, const char* accept),
//{
//  if (!document) return null;
//  var input = document.createElement('input');
//  input.type = 'file';
//  
//  if (multiSelect)
//    input.multiple = true;
//  
//  input.accept = UTF8ToString(accept);
//  
//  input.onchange = function(event)
//  {
//     var fileList = input.files;
//     console.log(fileList);
//  };
//  
//  // Simulate clicking on the input.
//  input.click();
//  
//  return mallocStringUTF8('test.png');
//});
//
//bool Shell::OpenFile(FileDialogInfo& config)
//{
//  // We have no way of selecting a folder, so for now we just enable multi-select.
//  bool multiSelect =
//    config.Flags & FileDialogFlags::MultiSelect ||
//    config.flags & FileDialogFlags::Folder;
//  
//  StringBuilder acceptExtensions;
//  forRange(FileDialogFilter& filter, config.mSearchFilters)
//  {
//    if (acceptExtensions.GetSize() != 0)
//      acceptExtensions.Append(',');
//    
//    forEach(Rune rune, filter.mFilter)
//    {
//      if (rune == '*')
//        // Do nothing, we don't use wild-cards here.
//      else if (rune == ';')
//        acceptExtensions.Append(',');
//      else
//        acceptExtensions.Append(rune);
//    }
//  }
//  String accept = acceptExtensions.ToString();
//  
//  const char* fileList = EmscriptenShellOpenFile(multiSelect, accept.c_str());
//  ZPrint("File Open List: %s\n", fileList);
//  free(fileList);
//  return true;
//}
