///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Standard includes
#include "Geometry/GeometryStandard.hpp"

namespace Zero
{
class PropertyPath;
class MetaSelection;

// Meta library
class ZeroNoImportExport MetaLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(MetaLibrary, "ZeroEngine");

  static void Initialize();
  static void Shutdown();
};

const Guid cInvalidUniqueId = (Guid)-1;
}//namespace Zero

// Project includes
#include "NativeTypeConversion.hpp"
#include "Singleton.hpp"
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
#include "ProxyObject.hpp"
#include "MetaComposition.hpp"
#include "LocalModifications.hpp"
#include "MetaSelection.hpp"
#include "ZeroMetaArray.hpp"
#include "MetaMath.hpp"
#include "MetaEditorExtensions.hpp"
#include "SourceControl.hpp"
#include "SimpleResourceFactory.hpp"
#include "MetaLibraryExtensions.hpp"
