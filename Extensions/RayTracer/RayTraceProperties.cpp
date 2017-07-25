///////////////////////////////////////////////////////////////////////////////
///
/// \file RayTraceProperties.cpp
/// Implementation of the RayTraceProperties class.
///
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "RayTraceProperties.hpp"
#include "Meta/MetaBinding.hpp"

namespace Zero
{

//--------------------------------------------------------- Ray Trace Properties
// Add meta data to this class
ZeroDefineType(RayTraceProperties);

// Bind methods and members for this object
void RayTraceProperties::InitializeMeta(MetaType* meta)
{
  BindBase(Component);
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindFieldProperty(mAttenuation);
  ZilchBindFieldProperty(mSpecularScalar);
  ZilchBindFieldProperty(mSpecularPower);
  BindCustomGetSetProperty(mElectricPermittivity, GetElectricPermittivity,
                                                  SetElectricPermittivity);
  BindCustomGetSetProperty(mMagneticPermeability, GetMagneticPermeability,
                                                  SetMagneticPermeability);
  ZilchBindFieldProperty(mIndexOfRefraction);
  ZilchBindFieldProperty(mUseTexture);
}

// Constructor
RayTraceProperties::RayTraceProperties(void)
{
  //
}

// Destructor
RayTraceProperties::~RayTraceProperties(void)
{
}

// Serialize the object
void RayTraceProperties::Serialize(Serializer& stream)
{
  const real defaultElectricPermittivity = real(1.0e6);
  const real defaultMagneticPermeability = real(1.0);
  const real defaultIndexOfRefraction = Math::Sqrt(defaultElectricPermittivity *
                                                   defaultMagneticPermeability);

  SerializeNameDefault(mAttenuation, Vec4(real(0.0), real(0.0), 
                                          real(0.0), real(0.0)));
  SerializeNameDefault(mSpecularScalar, real(0.4));
  SerializeNameDefault(mSpecularPower, real(90.0));
  SerializeNameDefault(mElectricPermittivity, defaultElectricPermittivity);
  SerializeNameDefault(mMagneticPermeability, defaultMagneticPermeability);
  SerializeNameDefault(mIndexOfRefraction, defaultIndexOfRefraction);
  SerializeNameDefault(mUseTexture, false);
}

const Vec4& RayTraceProperties::GetAttenuation(void) const
{
  return mAttenuation;
}

real RayTraceProperties::GetSpecularScalar(void) const
{
  return mSpecularScalar;
}

real RayTraceProperties::GetSpecularPower(void) const
{
  return mSpecularPower;
}

real RayTraceProperties::GetElectricPermittivity(void) const
{
  return mElectricPermittivity;
}

real RayTraceProperties::GetMagneticPermeability(void) const
{
  return mMagneticPermeability;
}

real RayTraceProperties::GetIndexOfRefraction(void) const
{
  return mIndexOfRefraction;
}

bool RayTraceProperties::GetUseTexture(void) const
{
  return mUseTexture;
}

void RayTraceProperties::SetElectricPermittivity(real electricPermittivity)
{
  mElectricPermittivity = Math::Max(electricPermittivity, 0.0f);
  mIndexOfRefraction = Math::Sqrt(mElectricPermittivity * 
                                  mMagneticPermeability);
}

void RayTraceProperties::SetMagneticPermeability(real magneticPermeability)
{
  mMagneticPermeability = Math::Max(magneticPermeability, 0.0f);
  mIndexOfRefraction = Math::Sqrt(mElectricPermittivity * 
                                  mMagneticPermeability);
}

void RayTraceProperties::SetUseTexture(bool useTexture)
{
  mUseTexture = useTexture;
}

}//namespace Zero
