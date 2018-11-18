////////////////////////////////////////////////////////////////////////////////
///
/// \file TilePaletteProperty.cpp
/// Implementation of the property interface for TilePalette selections.
///
/// Authors: Nathan Carlson
/// Copyright 2014, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------- TilePaletteProperty
//******************************************************************************
PropertyState TilePaletteProperty::GetValue(HandleParam object, PropertyPathParam propertyPath)
{
  TilePaletteView* paletteView = object.Get<TilePaletteView*>();

  Property* property = propertyPath.GetPropertyFromRoot(object);

  if (property->Name == "TilePalette")
    return paletteView->GetTilePaletteState();
  else if (property->Name == "Archetype")
    return paletteView->GetArchetypeState();
  else if (property->Name == "Sprite")
    return paletteView->GetSpriteState();
  else if (property->Name == "Collision")
    return paletteView->GetCollisionState();
  else if (property->Name == "Mergeable")
    return paletteView->GetMergeableState();
  else
    return PropertyState();
}

}//namespace Zero
