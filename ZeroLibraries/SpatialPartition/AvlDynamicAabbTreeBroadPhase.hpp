///////////////////////////////////////////////////////////////////////////////
///
/// \file AvlDynamicAabbTreeBroadPhase.hpp
/// Declaration of the AvlDynamicAabbTreeBroadPhase class.
/// 
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

///The BroadPhase interface for the DynamicAabbTree. Unlike the tree itself,
///this keeps track of internal pairs and figures out what actually
///needs to be queried for self intersections.
class AvlDynamicAabbTreeBroadPhase
  : public BaseDynamicAabbTreeBroadPhase<AvlDynamicAabbTree<void*> >
{
public:
  typedef AvlDynamicAabbTree<void*> TreeType;

  AvlDynamicAabbTreeBroadPhase();
  ~AvlDynamicAabbTreeBroadPhase();

  ZilchDeclareType(TypeCopyMode::ReferenceType);
};

}//namespace Zero
