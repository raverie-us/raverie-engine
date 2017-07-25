///////////////////////////////////////////////////////////////////////////////
///
/// \file ColorGradient.hpp
/// Declaration of the ColorGradient resource.
///
/// Authors: Joshua Claeys
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//--------------------------------------------------------------- Color Gradient
class ColorGradient : public DataResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  ColorGradient();

  /// Set default data.
  void SetDefaults() override;

  /// Serialize the gradient to/from a file.
  void Serialize(Serializer& stream) override;

  /// Initializes the gradient.
  void Initialize();

  /// Inserts a new color into the gradient at the given interpolant value.
  void Insert(Vec4 color, float interpolant);

  /// Inserts a new color into the gradient at the given interpolant value.
  void Insert(ByteColor color, float interpolant);

  /// Sample the curve at the given t.
  Vec4 Sample(float t);

  Gradient<Vec4> mGradient;
};

//------------------------------------------------------- Color Gradient Manager
class ColorGradientManager : public ResourceManager
{
public:
  DeclareResourceManager(ColorGradientManager, ColorGradient);

  ColorGradientManager(BoundType* resourceType);
  ColorGradient* CreateNewResourceInternal(StringParam name) override;
};

}//namespace Zero
