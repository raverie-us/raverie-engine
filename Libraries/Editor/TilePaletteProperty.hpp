////////////////////////////////////////////////////////////////////////////////
///
/// \file TilePaletteProperty.hpp
/// Declaration of the property interface for TilePalette selections.
///
/// Authors: Nathan Carlson
/// Copyright 2014, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------- TilePaletteProperty
class TilePaletteProperty : public PropertyInterface
{
public:
  /// Returns whether or not the value is valid. For example, it could be
  /// invalid if this is a multi-selection and there is a conflict between
  /// the values on multiple objects.
  PropertyState GetValue(HandleParam object, PropertyPathParam property) override;
};

}//namespace Zero
