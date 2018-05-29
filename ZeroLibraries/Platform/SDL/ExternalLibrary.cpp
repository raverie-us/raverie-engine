////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Dane Curbow
/// Copyright 2018, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "SDL_loadso.h"

namespace Zero
{

ExternalLibrary::ExternalLibrary()
{
  mHandle = nullptr;
  mUnloadOnDestruction = true;
}

ExternalLibrary::~ExternalLibrary()
{
  if(mUnloadOnDestruction)
    Unload();
}

bool ExternalLibrary::IsValid()
{
  return mHandle != nullptr;
}

void ExternalLibrary::Load(Status& status, cstr filePath)
{
  mHandle = (void*)SDL_LoadObject(filePath);
  if(mHandle == nullptr)
  {
    String errorString = SDL_GetError();
    String message = String::Format("Failed to load external library: %s, %s", filePath, errorString.c_str());
    status.SetFailed(message);
  }
}

void ExternalLibrary::Unload()
{
  if(mHandle == nullptr)
    return;

  SDL_UnloadObject(mHandle);
  mHandle = nullptr;
}

void* ExternalLibrary::GetFunctionByName(cstr name)
{
  ReturnIf(mHandle == nullptr, nullptr, "Attempting to get a function from an invalid library");

  return SDL_LoadFunction(mHandle, name);
}

void* ExternalLibrary::Patch(void* oldFunctionPointer, void* newFunctionPointer)
{
#if defined(PLATFORM_32)
  // This implementation currently only works on Windows __stdcall functions that start with mov edi,edi
  // This typically includes all Windows functions
  // This also only works in x86, not x64
  byte* oldStart = (byte*)oldFunctionPointer;

  // Look for mov edi,edi
  if (oldStart[0] == 0x89 && oldStart[1] == 0xFF)
  {
    // Right behind mov edi,edi should be 5 nops
    byte* oldFunction = oldStart - 5;
    byte* newFunction = (byte*)newFunctionPointer;

    // Mark the page as writable
    DWORD oldProtect = 0;
    VirtualProtect(oldFunction, 7, PAGE_EXECUTE_READWRITE, &oldProtect);

    // Get the jump offset from the old function to the new function
    // We have to subtract 5 since that is the size of the relative jump 0xE9 opcode
    // and x86 counts the jump offset starting from the NEXT instruction (after 5)
    int offset = (newFunction - oldFunction) - 5;

    // Short relative jump backwards to the nops, where we're going to put a long relative jump
    oldFunction[5] = 0xEB;
    oldFunction[6] = (byte)-7;
    
    // Long relative jump to the new function
    oldFunction[0] = 0xE9;
    *(int*)(oldFunction + 1) = offset;

    // The original function can still be called by moving the function pointer
    // to right after where the mov edi,edi was (a 2 byte nop)
    byte* original = oldFunction + 7;
    return original;
  }
#endif

  return nullptr;
}

}//namespace Zero
