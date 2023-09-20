// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineStaticLibrary(SerializationLibrary)
{
  builder.CreatableInScriptDefault = false;

  RaverieInitializeType(MetaSerialization);
  RaverieInitializeType(SerializationFilter);
  RaverieInitializeType(PrimitiveMetaSerialization<Integer>);
  RaverieInitializeType(PrimitiveMetaSerialization<Integer2>);
  RaverieInitializeType(PrimitiveMetaSerialization<Integer3>);
  RaverieInitializeType(PrimitiveMetaSerialization<Integer4>);
  RaverieInitializeType(PrimitiveMetaSerialization<String>);
  RaverieInitializeType(PrimitiveMetaSerialization<Boolean>);
  RaverieInitializeType(PrimitiveMetaSerialization<Real>);
  RaverieInitializeType(PrimitiveMetaSerialization<Real2>);
  RaverieInitializeType(PrimitiveMetaSerialization<Real3>);
  RaverieInitializeType(PrimitiveMetaSerialization<Real4>);
  RaverieInitializeType(PrimitiveMetaSerialization<Mat2>);
  RaverieInitializeType(PrimitiveMetaSerialization<Mat3>);
  RaverieInitializeType(PrimitiveMetaSerialization<Mat4>);
  RaverieInitializeType(PrimitiveMetaSerialization<Quat>);
  RaverieInitializeType(MetaStringSerialization);
  RaverieInitializeType(EnumMetaSerialization);

  RaverieBindSerializationPrimitiveExternal(Integer);
  RaverieBindSerializationPrimitiveExternal(Integer2);
  RaverieBindSerializationPrimitiveExternal(Integer3);
  RaverieBindSerializationPrimitiveExternal(Integer4);
  RaverieBindSerializationPrimitiveExternal(String);
  RaverieBindSerializationPrimitiveExternal(Boolean);
  RaverieBindSerializationPrimitiveExternal(Real);
  RaverieBindSerializationPrimitiveExternal(Real2);
  RaverieBindSerializationPrimitiveExternal(Real3);
  RaverieBindSerializationPrimitiveExternal(Real4);
  RaverieBindSerializationPrimitiveExternal(Mat2);
  RaverieBindSerializationPrimitiveExternal(Mat3);
  RaverieBindSerializationPrimitiveExternal(Mat4);
  RaverieBindSerializationPrimitiveExternal(Quat);
  RaverieBindSerializationPrimitiveExternal(Enum);

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

void SerializationLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  RaverieTypeId(Integer)->Add(new PrimitiveMetaSerialization<Integer>());
  RaverieTypeId(Integer2)->Add(new PrimitiveMetaSerialization<Integer2>());
  RaverieTypeId(Integer3)->Add(new PrimitiveMetaSerialization<Integer3>());
  RaverieTypeId(Integer4)->Add(new PrimitiveMetaSerialization<Integer4>());
  RaverieTypeId(String)->Add(new MetaStringSerialization);
  RaverieTypeId(Boolean)->Add(new PrimitiveMetaSerialization<Boolean>());
  RaverieTypeId(Real)->Add(new PrimitiveMetaSerialization<Real>());
  RaverieTypeId(Real2)->Add(new PrimitiveMetaSerialization<Real2>());
  RaverieTypeId(Real3)->Add(new PrimitiveMetaSerialization<Real3>());
  RaverieTypeId(Real4)->Add(new PrimitiveMetaSerialization<Real4>());
  RaverieTypeId(Mat2)->Add(new PrimitiveMetaSerialization<Mat2>());
  RaverieTypeId(Mat3)->Add(new PrimitiveMetaSerialization<Mat3>());
  RaverieTypeId(Mat4)->Add(new PrimitiveMetaSerialization<Mat4>());
  RaverieTypeId(Quat)->Add(new PrimitiveMetaSerialization<Quat>());
}

void SerializationLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Raverie
