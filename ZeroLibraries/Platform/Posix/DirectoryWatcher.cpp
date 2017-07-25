///////////////////////////////////////////////////////////////////////////////
///
/// \file DirectoryWatcher.hpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Platform/DirectoryWatcher.hpp"

namespace Zero
{

DirectoryWatcher::DirectoryWatcher(cstr directoryToWatch, CallbackFunction callback, void* callbackInstance)
{
  ZeroCStringCopy(mDirectoryToWatch, File::MaxPath, directoryToWatch, strlen(directoryToWatch));
  mCallbackInstance = callbackInstance;
  mCallback = callback;
}

DirectoryWatcher::~DirectoryWatcher()
{

}

void DirectoryWatcher::Shutdown()
{

}

OsInt DirectoryWatcher::RunThreadEntryPoint()
{
  return 0;
}

}//namespace Zero
