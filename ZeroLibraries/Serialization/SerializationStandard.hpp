///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Support/SupportStandard.hpp"
#include "Platform/PlatformStandard.hpp"
#include "Meta/MetaStandard.hpp"

#include "Zilch/Zilch.hpp"
using namespace Zilch;

namespace Zero
{

// Serialization library
class ZeroNoImportExport SerializationLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(SerializationLibrary, "ZeroEngine");

  static void Initialize();
  static void Shutdown();
};

}

#include "Serialization.hpp"
#include "ZeroContainers.hpp"
#include "SerializationBuilder.hpp"
#include "Tokenizer.hpp"
#include "Text.hpp"
#include "Binary.hpp"
#include "DataTreeNode.hpp"
#include "DataTree.hpp"
#include "Simple.hpp"
#include "DefaultSerializer.hpp"
#include "Tokenizer.hpp"
#include "SerializationTraits.hpp"
#include "EnumSerialization.hpp"
#include "MetaSerialization.hpp"
#include "MathSerialization.hpp"
