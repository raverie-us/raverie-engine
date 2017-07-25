///////////////////////////////////////////////////////////////////////////////
///
/// \file RayTraceProperties.hpp
/// Declaration of the RayTraceProperties class.
///
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Engine/Component.hpp"

namespace Zero
{

//--------------------------------------------------------- Ray Trace Properties
class RayTraceProperties : public Component
{
public:
  // Meta Initialization
  ZeroDeclareType(RayTraceProperties);
  static void InitializeMeta(MetaType* meta);

  // Constructor / Destructor
  RayTraceProperties();
  ~RayTraceProperties();

  /// Component interface
  void Serialize(Serializer& stream) override;

  const Vec4& GetAttenuation() const;
  real GetSpecularScalar() const;
  real GetSpecularPower() const;
  real GetElectricPermittivity() const;
  real GetMagneticPermeability() const;
  real GetIndexOfRefraction() const;
  bool GetUseTexture() const;

  void SetElectricPermittivity(real electricPermittivity);
  void SetMagneticPermeability(real magneticPermeability);
  void SetUseTexture(bool useTexture);

private:
  Vec4 mAttenuation;          //Color of the material inside the object
  real mSpecularScalar;       //Fraction of incident that's specularly reflected
  real mSpecularPower;        //Governs the dispersion of the light
  real mElectricPermittivity; //Amount a material distorts an electric field
  real mMagneticPermeability; //Amount a material distorts an magnetic field
  real mIndexOfRefraction;
  bool mUseTexture;
};

}//namespace Zero
