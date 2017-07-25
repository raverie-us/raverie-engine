///////////////////////////////////////////////////////////////////////////////
///
/// \file HierarchyRange.cpp
/// Implementation of the Hierarchy component range.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

bool NameCondition::operator()(const Cog& cog)
{
  return const_cast<Cog&>(cog).GetName() == this->Name;
}

bool NameCondition::operator()(const Cog* cog)
{
  return const_cast<Cog*>(cog)->GetName() == this->Name;
}

HierarchyRange HierarchyRange::EntireTree(Cog* object)
{
  return HierarchyRange(GetRoot(object));
}

HierarchyRange HierarchyRange::SubTree(Cog* object)
{
  return HierarchyRange(object);
}

HierarchyRange::HierarchyRange()
{
  mCurrent = nullptr;
  mStarting = nullptr;
}

HierarchyRange::HierarchyRange(Cog* s)
{
  mStarting = s;
  //Start on the most left node
  mCurrent = MostLeft(s);
}

bool HierarchyRange::Empty()
{
  return mCurrent == nullptr;
}

void HierarchyRange::PopFront()
{
  ErrorIf(Empty(), "Attempting to popFront on an empty HierarchyRange");
  //simple stackless tree transversal
  if(mCurrent->GetParent() && mCurrent != mStarting)
  {
    //Get the next sibling if no next sibling move to the parent
    Hierarchy* p = mCurrent->GetParent()->has(Hierarchy);
    Cog* nextSib = (Cog*)HierarchyList::Next(mCurrent);
    if(nextSib == p->Children.End())
    {
      // We don't want to go above our starting point in the tree
      if(mCurrent->GetParent() != mStarting)
        mCurrent = mCurrent->GetParent();
      else
        mCurrent = nullptr;
    }
    else
      mCurrent = MostLeft(nextSib);
  }
  else
    mCurrent = nullptr;
}

HierarchyRange::ref_type HierarchyRange::Front()
{
  return *mCurrent;
}

Cog* HierarchyRange::MostLeft(Cog* object)
{
  while(Hierarchy* p = object->has(Hierarchy))
  {
    if(!p->Children.Empty())
      object = &p->Children.Front();
    else
      return object;
  }
  return object;
}

Cog* HierarchyRange::GetRoot(Cog* object)
{
  while(object->GetParent() != nullptr)
    object = object->GetParent();
  return object;
}

}//namespace Zero
