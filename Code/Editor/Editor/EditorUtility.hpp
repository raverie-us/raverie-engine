// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class Cog;

// Get the text drawing position for a given cog
Vec3 GetObjectTextPosition(Cog* cog);

DeclareEnum2(IncludeMode, OnlyRoot, Children);
// Get an Aabb for a cog and its children
Aabb GetAabb(Cog* cog, IncludeMode::Type includeMode = IncludeMode::Children, bool world = true);
Aabb GetAabb(HandleParam metaObject, IncludeMode::Type includeMode = IncludeMode::Children, bool world = true);

// Get an Aabb for selection
Aabb GetAabb(MetaSelection* selection, IncludeMode::Type includeMode = IncludeMode::Children);

// Expand the aabb by this object size
void ExpandAabb(Cog* cog,
                Aabb& aabb,
                IncludeMode::Type includeMode = IncludeMode::Children,
                bool world = true,
                bool toParent = false,
                bool expandTransform = false);
void ExpandAabb(HandleParam metaObject,
                Aabb& aabb,
                IncludeMode::Type includeMode = IncludeMode::Children,
                bool world = true,
                bool toParent = false,
                bool expandTransform = false);
// Expand the aabb by this object's childrens' sizes only, but not the object's
// size itself.
void ExpandAabbChildrenOnly(HandleParam instance, Aabb& aabb, bool world = true, bool expandTransform = false);

// Get distance needed to full view an Aabb
float GetViewDistance(Aabb& aabb);

/// Takes in a code definition, validates, then displays it
void DisplayCodeDefinition(CodeDefinition& definition);

template <typename metaRangeType>
Aabb GetAabbFromObjects(metaRangeType objects, IncludeMode::Type includeMode = IncludeMode::Children)
{
  Aabb aabb = Aabb(Vec3::cZero, Vec3::cZero);

  if (objects.Empty())
    return aabb;

  // If this didn't happen before going through the range below, then the min
  // point of the aabb will always be (0,0,0), which is incorrect.
  aabb = GetAabb(objects.Front(), includeMode);

  forRange (Handle instance, objects)
  {
    Aabb currAabb = GetAabb(instance, includeMode);
    aabb.Combine(currAabb);
  }

  return aabb;
}

} // namespace Zero
