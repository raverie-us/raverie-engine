///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//**************************************************************************************************
void CommonLibrary::Initialize()
{
  //Start the memory system used for all systems and containers.
  Memory::Root::Initialize();
}

//**************************************************************************************************
void CommonLibrary::Shutdown()
{
  Memory::Shutdown();
}

}//namespace Zero
