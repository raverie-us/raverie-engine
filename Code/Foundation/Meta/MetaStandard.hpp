// MIT Licensed (see LICENSE.md).
#pragma once

// Standard includes
#include "Foundation/Geometry/GeometryStandard.hpp"

namespace Zero
{
class PropertyPath;
class MetaSelection;

// Meta library
class ZeroNoImportExport MetaLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(MetaLibrary);

  static void Initialize();
  static void Shutdown();
};

const Guid cInvalidUniqueId = (Guid)(u64)-1;
} // namespace Zero

// Project includes
#include "NativeTypeConversion.hpp"
#include "Utility/Singleton.hpp"
#include "Object.hpp"
#include "HandleManagers.hpp"
#include "ThreadSafeReferenceCounted.hpp"
#include "Tags.hpp"
#include "Event.hpp"
#include "CommonHandleManagers.hpp"
#include "ObjectRestoreState.hpp"
#include "PropertyHandle.hpp"
#include "MetaBindingExtensions.hpp"
#include "Notification.hpp"
#include "MetaDatabase.hpp"
#include "MetaHandles.hpp"
#include "MetaArray.hpp"
#include "MetaComposition.hpp"
#include "LocalModifications.hpp"
#include "MetaSelection.hpp"
#include "ZeroMetaArray.hpp"
#include "MetaMath.hpp"
#include "MetaEditorExtensions.hpp"
#include "SourceControl.hpp"
#include "MetaLibraryExtensions.hpp"
#include "AttributeExtension.hpp"
