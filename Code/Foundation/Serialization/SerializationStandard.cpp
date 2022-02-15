// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineStaticLibrary(SerializationLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(MetaSerialization);
  ZilchInitializeType(SerializationFilter);
  ZilchInitializeType(PrimitiveMetaSerialization<Integer>);
  ZilchInitializeType(PrimitiveMetaSerialization<Integer2>);
  ZilchInitializeType(PrimitiveMetaSerialization<Integer3>);
  ZilchInitializeType(PrimitiveMetaSerialization<Integer4>);
  ZilchInitializeType(PrimitiveMetaSerialization<String>);
  ZilchInitializeType(PrimitiveMetaSerialization<Boolean>);
  ZilchInitializeType(PrimitiveMetaSerialization<Real>);
  ZilchInitializeType(PrimitiveMetaSerialization<Real2>);
  ZilchInitializeType(PrimitiveMetaSerialization<Real3>);
  ZilchInitializeType(PrimitiveMetaSerialization<Real4>);
  ZilchInitializeType(PrimitiveMetaSerialization<Mat2>);
  ZilchInitializeType(PrimitiveMetaSerialization<Mat3>);
  ZilchInitializeType(PrimitiveMetaSerialization<Mat4>);
  ZilchInitializeType(PrimitiveMetaSerialization<Quat>);
  ZilchInitializeType(MetaStringSerialization);
  ZilchInitializeType(EnumMetaSerialization);

  ZeroBindSerializationPrimitiveExternal(Integer);
  ZeroBindSerializationPrimitiveExternal(Integer2);
  ZeroBindSerializationPrimitiveExternal(Integer3);
  ZeroBindSerializationPrimitiveExternal(Integer4);
  ZeroBindSerializationPrimitiveExternal(String);
  ZeroBindSerializationPrimitiveExternal(Boolean);
  ZeroBindSerializationPrimitiveExternal(Real);
  ZeroBindSerializationPrimitiveExternal(Real2);
  ZeroBindSerializationPrimitiveExternal(Real3);
  ZeroBindSerializationPrimitiveExternal(Real4);
  ZeroBindSerializationPrimitiveExternal(Mat2);
  ZeroBindSerializationPrimitiveExternal(Mat3);
  ZeroBindSerializationPrimitiveExternal(Mat4);
  ZeroBindSerializationPrimitiveExternal(Quat);
  ZeroBindSerializationPrimitiveExternal(Enum);

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

void SerializationLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  ZilchTypeId(Integer)->Add(new PrimitiveMetaSerialization<Integer>());
  ZilchTypeId(Integer2)->Add(new PrimitiveMetaSerialization<Integer2>());
  ZilchTypeId(Integer3)->Add(new PrimitiveMetaSerialization<Integer3>());
  ZilchTypeId(Integer4)->Add(new PrimitiveMetaSerialization<Integer4>());
  ZilchTypeId(String)->Add(new MetaStringSerialization);
  ZilchTypeId(Boolean)->Add(new PrimitiveMetaSerialization<Boolean>());
  ZilchTypeId(Real)->Add(new PrimitiveMetaSerialization<Real>());
  ZilchTypeId(Real2)->Add(new PrimitiveMetaSerialization<Real2>());
  ZilchTypeId(Real3)->Add(new PrimitiveMetaSerialization<Real3>());
  ZilchTypeId(Real4)->Add(new PrimitiveMetaSerialization<Real4>());
  ZilchTypeId(Mat2)->Add(new PrimitiveMetaSerialization<Mat2>());
  ZilchTypeId(Mat3)->Add(new PrimitiveMetaSerialization<Mat3>());
  ZilchTypeId(Mat4)->Add(new PrimitiveMetaSerialization<Mat4>());
  ZilchTypeId(Quat)->Add(new PrimitiveMetaSerialization<Quat>());
}

void SerializationLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Zero
