///////////////////////////////////////////////////////////////////////////////
///
/// \file Thread.hpp
/// Declaration of the ExternalLibrary class.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
/// An externally loaded native library (example, a Windows .dll or *nix .so file)
class ZeroShared ExternalLibrary
{
public:
  ExternalLibrary();
  ~ExternalLibrary();

  // Is this a valid library or is it uninitialized?
  bool IsValid();

  // Loads a library in by file path
  void Load(Status& status, cstr filePath);

  // Unload the library (safe to call more than once)
  void Unload();

  // Read a function out of the external library by name
  void* GetFunctionByName(cstr name);

  // Redirect one function pointer to another
  // This function may return null, and is typically only for testing / advanced platform details
  // Returns the original callable version of the function
  void* Patch(void* oldFunctionPointer, void* newFunctionPointer);

  // Whether we unload the library when we're destructed (default true)
  bool mUnloadOnDestruction;

private:
  OsHandle mHandle;
};

}//namespace Zero
