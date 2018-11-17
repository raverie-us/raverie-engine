///////////////////////////////////////////////////////////////////////////////
///
/// \file SpatialPartitionStandard.hpp
/// Standard includes for the SpatialPartition library.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Standard includes
#include "Geometry/GeometryStandard.hpp"
#include "Serialization/SerializationStandard.hpp"

namespace Zero
{

// SpatialPartition library
class ZeroNoImportExport SpatialPartitionLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(SpatialPartitionLibrary, "ZeroEngine");

  static void Initialize();
  static void Shutdown();
};

}//namespace Zero

// Project includes
#include "BroadPhaseProxy.hpp"
#include "ProxyCast.hpp"
#include "BroadPhase.hpp"
#include "SimpleCastCallbacks.hpp"
#include "BroadPhaseRanges.hpp"
#include "BaseDynamicAabbTreeBroadPhase.hpp"
#include "DynamicTreeHelpers.hpp"
#include "BaseDynamicAabbTree.hpp"
#include "AvlDynamicAabbTree.hpp"
#include "DynamicAabbTree.hpp"
#include "DynamicAabbTreeBroadPhase.hpp"
#include "AvlDynamicAabbTreeBroadPhase.hpp"
#include "BaseNSquared.hpp"
#include "NSquared.hpp"
#include "NSquaredBroadPhase.hpp"
#include "BoundingBox.hpp"
#include "BoundingBoxBroadPhase.hpp"
#include "BoundingSphere.hpp"
#include "BoundingSphereBroadPhase.hpp"
#include "SapContainers.hpp"
#include "Sap.hpp"
#include "SapBroadPhase.hpp"
#include "AabbTreeNode.hpp"
#include "AabbTreeMethods.hpp"
#include "StaticAabbTree.hpp"
#include "StaticAabbTreeBroadPhase.hpp"
#include "BroadPhasePackage.hpp"
#include "BroadPhaseCreator.hpp"
#include "BroadPhaseTracker.hpp"
