/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2015, DigiPen Institute of Technology
\**************************************************************/

// Include protection
#pragma once
#ifndef ZILCH_SAMPLE_PLUGIN_HPP
#define ZILCH_SAMPLE_PLUGIN_HPP

#define ZeroImportDll
#include "Zilch.hpp"
using namespace Zilch;

//***************************************************************************
ZilchDeclareStaticLibraryAndPlugin(SampleLibrary, SamplePlugin);

//***************************************************************************
class Sample
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  static void Run(StringBuilderExtended* builder, ArrayClass<Byte>* bytes);
};

// End header protection
#endif
