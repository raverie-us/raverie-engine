///////////////////////////////////////////////////////////////////////////////
///
/// \file DynamicAabbTreeBroadPhase.hpp
/// Declaration of the DynamicAabbTreeBroadPhase class.
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
class DynamicAabbTreeBroadPhase
  : public BaseDynamicAabbTreeBroadPhase<DynamicAabbTree<void*> >
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  typedef DynamicAabbTree<void*> TreeType;
  typedef TreeType::SelfQueryRange TreeSelfRange;

  DynamicAabbTreeBroadPhase();
  ~DynamicAabbTreeBroadPhase();
};

}//namespace Zero
