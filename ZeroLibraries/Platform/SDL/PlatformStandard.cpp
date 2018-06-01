///////////////////////////////////////////////////////////////////////////////
///
/// \file OsShared.cpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

SDL_Cursor* gSDLCursors[SDL_NUM_SYSTEM_CURSORS] = { 0 };

//**************************************************************************************************
void PlatformLibrary::Initialize()
{
  SDL_Init(SDL_INIT_EVERYTHING);

  for (size_t i = 0; i < SDL_NUM_SYSTEM_CURSORS; ++i)
    gSDLCursors[i] = SDL_CreateSystemCursor((SDL_SystemCursor)i);
}

//**************************************************************************************************
void PlatformLibrary::Shutdown()
{
  for (size_t i = 0; i < SDL_NUM_SYSTEM_CURSORS; ++i)
    SDL_FreeCursor(gSDLCursors[i]);

  SDL_Quit();
}

}//namespace Zero
