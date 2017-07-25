///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//**************************************************************************************************
ZilchDefineStaticLibrary(SerializationLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(MetaSerialization);
  ZilchInitializeType(SerializationFilter);
  ZilchInitializeType(PrimitiveMetaSerialization<Integer>);
  ZilchInitializeType(PrimitiveMetaSerialization<String>);
  ZilchInitializeType(PrimitiveMetaSerialization<Boolean>);
  ZilchInitializeType(PrimitiveMetaSerialization<Real>);
  ZilchInitializeType(PrimitiveMetaSerialization<Real2>);
  ZilchInitializeType(PrimitiveMetaSerialization<Real3>);
  ZilchInitializeType(PrimitiveMetaSerialization<Real4>);
  ZilchInitializeType(PrimitiveMetaSerialization<Mat3>);
  ZilchInitializeType(PrimitiveMetaSerialization<Mat4>);
  ZilchInitializeType(PrimitiveMetaSerialization<Quat>);
  ZilchInitializeType(MetaStringSerialization);
  ZilchInitializeType(EnumMetaSerialization);

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

//**************************************************************************************************
void SerializationLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  ZilchTypeId(Integer)->Add(new PrimitiveMetaSerialization<Integer>());
  ZilchTypeId(String)->Add(new MetaStringSerialization);
  ZilchTypeId(Boolean)->Add(new PrimitiveMetaSerialization<Boolean>());
  ZilchTypeId(Real)->Add(new PrimitiveMetaSerialization<Real>());
  ZilchTypeId(Real2)->Add(new PrimitiveMetaSerialization<Real2>());
  ZilchTypeId(Real3)->Add(new PrimitiveMetaSerialization<Real3>());
  ZilchTypeId(Real4)->Add(new PrimitiveMetaSerialization<Real4>());
  ZilchTypeId(Mat3)->Add(new PrimitiveMetaSerialization<Mat3>());
  ZilchTypeId(Mat4)->Add(new PrimitiveMetaSerialization<Mat4>());
  ZilchTypeId(Quat)->Add(new PrimitiveMetaSerialization<Quat>());
  ZilchTypeId(Enum)->Add(new EnumMetaSerialization());
}

//**************************************************************************************************
void SerializationLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

}//namespace Zero
