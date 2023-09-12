// MIT Licensed (see LICENSE.md).
#pragma once

#include "Foundation/Support/SupportStandard.hpp"
#include "Foundation/Meta/MetaStandard.hpp"

#include "Foundation/Zilch/Precompiled.hpp"
using namespace Zilch;

namespace Zero
{

// Serialization library
class ZeroNoImportExport SerializationLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(SerializationLibrary);

  static void Initialize();
  static void Shutdown();
};

} // namespace Zero

#include "Serialization.hpp"
#include "ZeroContainers.hpp"
#include "SerializationBuilder.hpp"
#include "Tokenizer.hpp"
#include "SerializationUtility.hpp"
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
