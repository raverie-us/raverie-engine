///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorUtility.hpp
/// Declaration of the Editor support classes EditorSpace and Selection.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Cog;

// Get the text drawing position for a given object
Vec3 GetObjectTextPosition(Cog* object);

DeclareEnum2(IncludeMode, OnlyRoot, Children);
// Get an Aabb for an object and its children
Aabb GetAabb(Cog* object, IncludeMode::Type includeMode = IncludeMode::Children);
Aabb GetAabb(HandleParam metaObject, IncludeMode::Type includeMode = IncludeMode::Children);

// Get an Aabb for selection
Aabb GetAabb(MetaSelection* selection, IncludeMode::Type includeMode = IncludeMode::Children);

// Expand the aabb by this object size
void ExpandAabb(Cog* object, Aabb& aabb, IncludeMode::Type includeMode= IncludeMode::Children, bool world = true);
void ExpandAabb(HandleParam metaObject, Aabb& aabb, IncludeMode::Type includeMode = IncludeMode::Children, bool world = true);

// Get distance needed to full view an Aabb
float GetViewDistance(Aabb& aabb);

/// Takes in a code definition, validates, then displays it
void DisplayCodeDefinition(CodeDefinition& definition);

template <typename metaRangeType>
Aabb GetAabbFromObjects(metaRangeType objects, IncludeMode::Type includeMode = IncludeMode::Children)
{
  if(objects.Empty())
    return Aabb(Vec3::cZero, Vec3::cZero);

  Aabb aabb = GetAabb(objects.Front(), includeMode);

  forRange(Handle instance, objects)
  {
    Aabb currAabb = GetAabb(instance, includeMode);
    aabb.Combine(currAabb);
  }

  return aabb;
}

}//namespace Zero
