// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// The BroadPhase interface for the DynamicAabbTree. Unlike the tree itself,
/// this keeps track of internal pairs and figures out what actually
/// needs to be queried for self intersections.
class DynamicAabbTreeBroadPhase : public BaseDynamicAabbTreeBroadPhase<DynamicAabbTree<void*>>
{
public:
  RaverieDeclareType(DynamicAabbTreeBroadPhase, TypeCopyMode::ReferenceType);

  typedef DynamicAabbTree<void*> TreeType;
  typedef TreeType::SelfQueryRange TreeSelfRange;

  DynamicAabbTreeBroadPhase();
  ~DynamicAabbTreeBroadPhase();
};

} // namespace Raverie
