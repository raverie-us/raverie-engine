///////////////////////////////////////////////////////////////////////////////
///
/// \file OsShared.hpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class ZeroNoImportExport PlatformLibrary
{
public:
  static void Initialize();
  static void Shutdown();
};

}//namespace Zero
