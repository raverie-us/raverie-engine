// MIT Licensed (see LICENSE.md).
#pragma once

#include "Foundation/Support/SupportStandard.hpp"
#include "Foundation/Meta/MetaStandard.hpp"

#include "Foundation/RaverieLanguage/Precompiled.hpp"
using namespace Raverie;

namespace Raverie
{

// Serialization library
class SerializationLibrary : public Raverie::StaticLibrary
{
public:
  RaverieDeclareStaticLibraryInternals(SerializationLibrary);

  static void Initialize();
  static void Shutdown();
};

} // namespace Raverie

#include "Serialization.hpp"
#include "Containers.hpp"
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
