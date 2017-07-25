///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
void AddLinks(Cog* object, MetaSelection* selection)
{
  ObjectLinkRange range(object);
  for (; !range.Empty(); range.PopFront())
  {
    ObjectLinkEdge& edge = range.Front();
    Cog* otherCog = edge.GetOtherCog();
    if (selection->Contains(otherCog) || otherCog == object->GetSpace())
    {
      Cog* cog = edge.GetObjectLink()->GetOwner();
      selection->Add(cog);
    }
  }
}

void RemoveLinks(Cog* object, MetaSelection* selection)
{
  ObjectLinkRange range(object);
  for (; !range.Empty(); range.PopFront())
  {
    ObjectLinkEdge& edge = range.Front();
    Cog* otherCog = edge.GetOtherCog();
    if (selection->Contains(otherCog) || otherCog == object->GetSpace())
    {
      Cog* cog = edge.GetObjectLink()->GetOwner();
      selection->Remove(cog);
    }
  }
}

bool IsAncestorInSelection(Cog* cog, MetaSelection* selection)
{
  if(cog && cog->mHierarchyParent)
  {
    Cog* parent = cog->mHierarchyParent;
    if(selection->Contains(parent))
      return true;
    else
    {
      //check for parent parent
      return IsAncestorInSelection(parent, selection);
    }
  }

  return false;
}

bool IsAncestorPresent(const Cog* cog, const Array<CogId>& cogs)
{
  if(cog && cog->mHierarchyParent)
  {
    Cog* parent = cog->mHierarchyParent;
    if(cogs.Contains(parent->GetId()))
    {
      return true;
    }
    else
    {
        // check for parent parent
      return IsAncestorPresent(parent, cogs);
    }
  }

  return false;
}

bool IsAncestorPresent(Cog* cog, const Array<Handle>& metaObjects)
{
  if(cog && cog->mHierarchyParent)
  {
    Cog* parent = cog->mHierarchyParent;
    if(metaObjects.Contains(Handle(parent)))
    {
      return true;
    }
    else
    {
      // check for parent parent
      return IsAncestorPresent(parent, metaObjects);
    }
  }

  return false;
}

void FilterChildrenAndProtected(Array<Cog*>& cogs, MetaSelection* selection)
{
  MetaSelection::rangeType<Cog> r = selection->AllOfType<Cog>();
  for(;!r.Empty();r.PopFront())
  {
    Cog* object = r.Front();
    if(!object->mFlags.IsSet(CogFlags::Protected) &&
      !IsAncestorInSelection(object, selection))
    {
      cogs.PushBack(object);
    }
  }
}

void FilterChildrenAndProtected(const Array<CogId>& cogsIn, Array<Cog*>& cogsOut)
{
  forRange(Cog* object, cogsIn.All())
  {
    if(!object->mFlags.IsSet(CogFlags::Protected) && !IsAncestorPresent(object, cogsIn))
      cogsOut.PushBack(object);
  }
}

void FilterChildrenAndProtected(const Array<Handle>& objectsIn, Array<Handle>& objectsOut)
{
  forRange(Handle object, objectsIn.All())
  {
    Cog* cog = object.Get<Cog*>();
    if(!cog->mFlags.IsSet(CogFlags::Protected) && !IsAncestorPresent(cog, objectsIn))
      objectsOut.PushBack(object);
  }
}

} // namespace Zero
