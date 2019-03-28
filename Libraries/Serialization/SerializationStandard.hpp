// MIT Licensed (see LICENSE.md).
#pragma once

#include "Support/SupportStandard.hpp"
#include "PlatformStandard.hpp"
#include "Meta/MetaStandard.hpp"

#include "Zilch/Precompiled.hpp"
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

} // namespace Zero

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
#include "SerializationUtility.hpp"
#include "LegacyDataTreeParser.hpp"
#include "DataTreeTokenizer.hpp"
#include "DataTreeParser.hpp"
