///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{
  
//Templated functions so that code can be generate for each type of joint.

template <typename ListType>
void ClearFragmentList(ListType& jointList)
{
  typename ListType::range jointRange = jointList.All();
  while(!jointRange.Empty())
  {
    typename ListType::sub_reference joint = jointRange.Front();
    jointRange.PopFront();
    ListType::Unlink(&joint);
  }
}

template <typename ListType>
void UpdateDataFragmentList(ListType& jointList, MoleculeWalker& molecules)
{
  typename ListType::range jointRange = jointList.All();
  while(!jointRange.Empty())
  {
    typename ListType::sub_reference joint = jointRange.Front();
    jointRange.PopFront();
    joint.ComputeMolecules(molecules);
  }
}

template <typename ListType>
void WarmStartFragmentList(ListType& jointList, MoleculeWalker& molecules)
{
  typename ListType::range jointRange = jointList.All();
  while(!jointRange.Empty())
  {
    typename ListType::sub_reference joint = jointRange.Front();
    jointRange.PopFront();
    joint.WarmStart(molecules);
  }
}

template <typename ListType>
void IterateVelocitiesFragmentList(ListType& jointList, MoleculeWalker& molecules, uint iteration)
{
  typename ListType::range jointRange = jointList.All();
  while(!jointRange.Empty())
  {
    typename ListType::sub_reference joint = jointRange.Front();
    jointRange.PopFront();
    joint.Solve(molecules);
  }
}

template <typename ListType>
void SolvePositionsFragmentList(ListType& jointList, MoleculeWalker& molecules)
{

}

template <typename ListType>
void CommitFragmentList(ListType& jointList, MoleculeWalker& molecules)
{
  typename ListType::range jointRange = jointList.All();
  while(!jointRange.Empty())
  {
    typename ListType::sub_reference joint = jointRange.Front();
    jointRange.PopFront();
    joint.Commit(molecules);
  }
}

template <typename ListType>
void BatchEventsFragmentList(ListType& jointList)
{
  typename ListType::range jointRange = jointList.All();
  while(!jointRange.Empty())
  {
    typename ListType::sub_reference joint = jointRange.Front();
    jointRange.PopFront();
    joint.BatchEvents();
  }
}

template <typename ListType>
void DrawJointsFragmentList(ListType& jointList)
{
  typename ListType::range jointRange = jointList.All();
  while(!jointRange.Empty())
  {
    typename ListType::sub_reference joint = jointRange.Front();
    jointRange.PopFront();
    joint.DebugDraw();
  }
}

}//namespace Physics

}//namespace Zero
