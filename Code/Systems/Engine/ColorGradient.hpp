// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Specifies a range of colors that are interpolated when sampled.
class ColorGradient : public DataResource
{
public:
  RaverieDeclareType(ColorGradient, TypeCopyMode::ReferenceType);

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

class ColorGradientManager : public ResourceManager
{
public:
  DeclareResourceManager(ColorGradientManager, ColorGradient);

  ColorGradientManager(BoundType* resourceType);
  ColorGradient* CreateNewResourceInternal(StringParam name) override;
};

} // namespace Raverie
