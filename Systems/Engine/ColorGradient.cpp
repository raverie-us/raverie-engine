///////////////////////////////////////////////////////////////////////////////
///
/// \file ColorGradient.cpp
/// Implementation of the ColorGradient resource.
///
/// Authors: Joshua Claeys
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//--------------------------------------------------------------- Color Gradient
ZilchDefineType(ColorGradient, builder, type)
{
  ZilchBindMethod(Sample);
}

/// Constructor.
ColorGradient::ColorGradient()
{

}

/// Meta object initialization.

/// Set default data.
void ColorGradient::SetDefaults()
{
  Insert(Color::Black, 0.0f);
  Insert(Color::White, 1.0f);
}

/// Serialize the gradient to/from a file.
void ColorGradient::Serialize(Serializer& stream)
{
  SerializeName(mGradient);
}

/// Initializes the gradient.
void ColorGradient::Initialize()
{

}

/// Inserts a new color into the gradient at the given interpolant value.
void ColorGradient::Insert(Vec4 color, float interpolant)
{
  mGradient.Insert(color, interpolant);
}

/// Inserts a new color into the gradient at the given interpolant value.
void ColorGradient::Insert(ByteColor color, float interpolant)
{
  mGradient.Insert(ToFloatColor(color), interpolant);
}

/// Sample the curve at the given t.
Vec4 ColorGradient::Sample(float t)
{
  return mGradient.Sample(t);
}

//------------------------------------------------------- Color Gradient Manager
ImplementResourceManager(ColorGradientManager, ColorGradient);

ColorGradientManager::ColorGradientManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  AddLoader("ColorGradient", new TextDataFileLoader<ColorGradientManager>());
  DefaultResourceName = "DefaultColorGradient";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.ColorGradient.data"));
  mCanCreateNew = true;
  mCanDuplicate = true;
  mExtension = DataResourceExtension;
}

ColorGradient* ColorGradientManager::CreateNewResourceInternal(StringParam name)
{
  ColorGradient* gradient = new ColorGradient();
  gradient->SetDefaults();
  gradient->Initialize();
  return gradient;
}

}//namespace Zero
