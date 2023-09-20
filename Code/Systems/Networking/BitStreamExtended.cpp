// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

//                              BitStreamExtended //

// Default Quantum Values
const int BitStreamExtended::DefaultIntegralQuantum = 1;
const float BitStreamExtended::DefaultFloatingPointQuantum = 0.0001;

RaverieDefineType(BitStreamExtended, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind destructor
  RaverieBindDestructor();

  // Bind default constructor
  RaverieBindDefaultConstructor();

  // Bind measure operations
  RaverieBindMethod(MeasureBoolean);

  RaverieBindMethod(MeasureInteger);
  RaverieBindMethod(MeasureDoubleInteger);
  RaverieBindMethod(MeasureInteger2);
  RaverieBindMethod(MeasureInteger3);
  RaverieBindMethod(MeasureInteger4);

  RaverieBindMethod(MeasureReal);
  RaverieBindMethod(MeasureDoubleReal);
  RaverieBindMethod(MeasureReal2);
  RaverieBindMethod(MeasureReal3);
  RaverieBindMethod(MeasureReal4);
  RaverieBindMethod(MeasureQuaternion);

  RaverieBindOverloadedMethod(MeasureString, RaverieStaticOverload(Bits));
  RaverieBindOverloadedMethod(MeasureString, RaverieStaticOverload(Bits, StringParam));

  // Bind measure half operations
  RaverieBindMethod(MeasureRealHalf);
  RaverieBindMethod(MeasureReal2Half);
  RaverieBindMethod(MeasureReal3Half);
  RaverieBindMethod(MeasureReal4Half);

  // Bind measure quantized operations
  RaverieBindOverloadedMethod(MeasureIntegerQuantized, RaverieStaticOverload(Bits, int, int));
  RaverieBindOverloadedMethod(MeasureIntegerQuantized, RaverieStaticOverload(Bits, int, int, int));

  RaverieBindOverloadedMethod(MeasureDoubleIntegerQuantized, RaverieStaticOverload(Bits, s64, s64));
  RaverieBindOverloadedMethod(MeasureDoubleIntegerQuantized, RaverieStaticOverload(Bits, s64, s64, s64));

  RaverieBindOverloadedMethod(MeasureInteger2Quantized,
                            RaverieStaticOverload(Bits, const Math::IntVector2&, const Math::IntVector2&));
  RaverieBindOverloadedMethod(
      MeasureInteger2Quantized,
      RaverieStaticOverload(Bits, const Math::IntVector2&, const Math::IntVector2&, const Math::IntVector2&));

  RaverieBindOverloadedMethod(MeasureInteger3Quantized,
                            RaverieStaticOverload(Bits, const Math::IntVector3&, const Math::IntVector3&));
  RaverieBindOverloadedMethod(
      MeasureInteger3Quantized,
      RaverieStaticOverload(Bits, const Math::IntVector3&, const Math::IntVector3&, const Math::IntVector3&));

  RaverieBindOverloadedMethod(MeasureInteger4Quantized,
                            RaverieStaticOverload(Bits, const Math::IntVector4&, const Math::IntVector4&));
  RaverieBindOverloadedMethod(
      MeasureInteger4Quantized,
      RaverieStaticOverload(Bits, const Math::IntVector4&, const Math::IntVector4&, const Math::IntVector4&));

  RaverieBindOverloadedMethod(MeasureRealQuantized, RaverieStaticOverload(Bits, float, float));
  RaverieBindOverloadedMethod(MeasureRealQuantized, RaverieStaticOverload(Bits, float, float, float));

  RaverieBindOverloadedMethod(MeasureDoubleRealQuantized, RaverieStaticOverload(Bits, double, double));
  RaverieBindOverloadedMethod(MeasureDoubleRealQuantized, RaverieStaticOverload(Bits, double, double, double));

  RaverieBindOverloadedMethod(MeasureReal2Quantized,
                            RaverieStaticOverload(Bits, const Math::Vector2&, const Math::Vector2&));
  RaverieBindOverloadedMethod(
      MeasureReal2Quantized,
      RaverieStaticOverload(Bits, const Math::Vector2&, const Math::Vector2&, const Math::Vector2&));

  RaverieBindOverloadedMethod(MeasureReal3Quantized,
                            RaverieStaticOverload(Bits, const Math::Vector3&, const Math::Vector3&));
  RaverieBindOverloadedMethod(
      MeasureReal3Quantized,
      RaverieStaticOverload(Bits, const Math::Vector3&, const Math::Vector3&, const Math::Vector3&));

  RaverieBindOverloadedMethod(MeasureReal4Quantized,
                            RaverieStaticOverload(Bits, const Math::Vector4&, const Math::Vector4&));
  RaverieBindOverloadedMethod(
      MeasureReal4Quantized,
      RaverieStaticOverload(Bits, const Math::Vector4&, const Math::Vector4&, const Math::Vector4&));

  // Bind write operations
  RaverieBindMethod(WriteBoolean);

  RaverieBindMethod(WriteInteger);
  RaverieBindMethod(WriteDoubleInteger);
  RaverieBindMethod(WriteInteger2);
  RaverieBindMethod(WriteInteger3);
  RaverieBindMethod(WriteInteger4);

  RaverieBindMethod(WriteReal);
  RaverieBindMethod(WriteDoubleReal);
  RaverieBindMethod(WriteReal2);
  RaverieBindMethod(WriteReal3);
  RaverieBindMethod(WriteReal4);
  RaverieBindMethod(WriteQuaternion);

  RaverieBindMethod(WriteString);

  // Bind write half operations
  RaverieBindMethod(WriteRealHalf);
  RaverieBindMethod(WriteReal2Half);
  RaverieBindMethod(WriteReal3Half);
  RaverieBindMethod(WriteReal4Half);

  // Bind write quantized operations
  RaverieBindOverloadedMethod(WriteIntegerQuantized, RaverieInstanceOverload(void, int, int, int));
  RaverieBindOverloadedMethod(WriteIntegerQuantized, RaverieInstanceOverload(void, int, int, int, int));

  RaverieBindOverloadedMethod(WriteDoubleIntegerQuantized, RaverieInstanceOverload(void, s64, s64, s64));
  RaverieBindOverloadedMethod(WriteDoubleIntegerQuantized, RaverieInstanceOverload(void, s64, s64, s64, s64));

  RaverieBindOverloadedMethod(
      WriteInteger2Quantized,
      RaverieInstanceOverload(void, const Math::IntVector2&, const Math::IntVector2&, const Math::IntVector2&));
  RaverieBindOverloadedMethod(
      WriteInteger2Quantized,
      RaverieInstanceOverload(
          void, const Math::IntVector2&, const Math::IntVector2&, const Math::IntVector2&, const Math::IntVector2&));

  RaverieBindOverloadedMethod(
      WriteInteger3Quantized,
      RaverieInstanceOverload(void, const Math::IntVector3&, const Math::IntVector3&, const Math::IntVector3&));
  RaverieBindOverloadedMethod(
      WriteInteger3Quantized,
      RaverieInstanceOverload(
          void, const Math::IntVector3&, const Math::IntVector3&, const Math::IntVector3&, const Math::IntVector3&));

  RaverieBindOverloadedMethod(
      WriteInteger4Quantized,
      RaverieInstanceOverload(void, const Math::IntVector4&, const Math::IntVector4&, const Math::IntVector4&));
  RaverieBindOverloadedMethod(
      WriteInteger4Quantized,
      RaverieInstanceOverload(
          void, const Math::IntVector4&, const Math::IntVector4&, const Math::IntVector4&, const Math::IntVector4&));

  RaverieBindOverloadedMethod(WriteRealQuantized, RaverieInstanceOverload(void, float, float, float));
  RaverieBindOverloadedMethod(WriteRealQuantized, RaverieInstanceOverload(void, float, float, float, float));

  RaverieBindOverloadedMethod(WriteDoubleRealQuantized, RaverieInstanceOverload(void, double, double, double));
  RaverieBindOverloadedMethod(WriteDoubleRealQuantized, RaverieInstanceOverload(void, double, double, double, double));

  RaverieBindOverloadedMethod(
      WriteReal2Quantized,
      RaverieInstanceOverload(void, const Math::Vector2&, const Math::Vector2&, const Math::Vector2&));
  RaverieBindOverloadedMethod(
      WriteReal2Quantized,
      RaverieInstanceOverload(
          void, const Math::Vector2&, const Math::Vector2&, const Math::Vector2&, const Math::Vector2&));

  RaverieBindOverloadedMethod(
      WriteReal3Quantized,
      RaverieInstanceOverload(void, const Math::Vector3&, const Math::Vector3&, const Math::Vector3&));
  RaverieBindOverloadedMethod(
      WriteReal3Quantized,
      RaverieInstanceOverload(
          void, const Math::Vector3&, const Math::Vector3&, const Math::Vector3&, const Math::Vector3&));

  RaverieBindOverloadedMethod(
      WriteReal4Quantized,
      RaverieInstanceOverload(void, const Math::Vector4&, const Math::Vector4&, const Math::Vector4&));
  RaverieBindOverloadedMethod(
      WriteReal4Quantized,
      RaverieInstanceOverload(
          void, const Math::Vector4&, const Math::Vector4&, const Math::Vector4&, const Math::Vector4&));

  // Bind can-read operations
  RaverieBindMethod(CanReadBoolean);

  RaverieBindMethod(CanReadInteger);
  RaverieBindMethod(CanReadDoubleInteger);
  RaverieBindMethod(CanReadInteger2);
  RaverieBindMethod(CanReadInteger3);
  RaverieBindMethod(CanReadInteger4);

  RaverieBindMethod(CanReadReal);
  RaverieBindMethod(CanReadDoubleReal);
  RaverieBindMethod(CanReadReal2);
  RaverieBindMethod(CanReadReal3);
  RaverieBindMethod(CanReadReal4);
  RaverieBindMethod(CanReadQuaternion);

  RaverieBindMethod(CanReadString);

  // Bind can-read half operations
  RaverieBindMethod(CanReadRealHalf);
  RaverieBindMethod(CanReadReal2Half);
  RaverieBindMethod(CanReadReal3Half);
  RaverieBindMethod(CanReadReal4Half);

  // Bind can-read quantized operations
  RaverieBindOverloadedMethod(CanReadIntegerQuantized, RaverieConstInstanceOverload(bool, int, int));
  RaverieBindOverloadedMethod(CanReadIntegerQuantized, RaverieConstInstanceOverload(bool, int, int, int));

  RaverieBindOverloadedMethod(CanReadDoubleIntegerQuantized, RaverieConstInstanceOverload(bool, s64, s64));
  RaverieBindOverloadedMethod(CanReadDoubleIntegerQuantized, RaverieConstInstanceOverload(bool, s64, s64, s64));

  RaverieBindOverloadedMethod(CanReadInteger2Quantized,
                            RaverieConstInstanceOverload(bool, const Math::IntVector2&, const Math::IntVector2&));
  RaverieBindOverloadedMethod(
      CanReadInteger2Quantized,
      RaverieConstInstanceOverload(bool, const Math::IntVector2&, const Math::IntVector2&, const Math::IntVector2&));

  RaverieBindOverloadedMethod(CanReadInteger3Quantized,
                            RaverieConstInstanceOverload(bool, const Math::IntVector3&, const Math::IntVector3&));
  RaverieBindOverloadedMethod(
      CanReadInteger3Quantized,
      RaverieConstInstanceOverload(bool, const Math::IntVector3&, const Math::IntVector3&, const Math::IntVector3&));

  RaverieBindOverloadedMethod(CanReadInteger4Quantized,
                            RaverieConstInstanceOverload(bool, const Math::IntVector4&, const Math::IntVector4&));
  RaverieBindOverloadedMethod(
      CanReadInteger4Quantized,
      RaverieConstInstanceOverload(bool, const Math::IntVector4&, const Math::IntVector4&, const Math::IntVector4&));

  RaverieBindOverloadedMethod(CanReadRealQuantized, RaverieConstInstanceOverload(bool, float, float));
  RaverieBindOverloadedMethod(CanReadRealQuantized, RaverieConstInstanceOverload(bool, float, float, float));

  RaverieBindOverloadedMethod(CanReadDoubleRealQuantized, RaverieConstInstanceOverload(bool, double, double));
  RaverieBindOverloadedMethod(CanReadDoubleRealQuantized, RaverieConstInstanceOverload(bool, double, double, double));

  RaverieBindOverloadedMethod(CanReadReal2Quantized,
                            RaverieConstInstanceOverload(bool, const Math::Vector2&, const Math::Vector2&));
  RaverieBindOverloadedMethod(
      CanReadReal2Quantized,
      RaverieConstInstanceOverload(bool, const Math::Vector2&, const Math::Vector2&, const Math::Vector2&));

  RaverieBindOverloadedMethod(CanReadReal3Quantized,
                            RaverieConstInstanceOverload(bool, const Math::Vector3&, const Math::Vector3&));
  RaverieBindOverloadedMethod(
      CanReadReal3Quantized,
      RaverieConstInstanceOverload(bool, const Math::Vector3&, const Math::Vector3&, const Math::Vector3&));

  RaverieBindOverloadedMethod(CanReadReal4Quantized,
                            RaverieConstInstanceOverload(bool, const Math::Vector4&, const Math::Vector4&));
  RaverieBindOverloadedMethod(
      CanReadReal4Quantized,
      RaverieConstInstanceOverload(bool, const Math::Vector4&, const Math::Vector4&, const Math::Vector4&));

  // Bind read operations
  RaverieBindMethod(ReadBoolean);

  RaverieBindMethod(ReadInteger);
  RaverieBindMethod(ReadDoubleInteger);
  RaverieBindMethod(ReadInteger2);
  RaverieBindMethod(ReadInteger3);
  RaverieBindMethod(ReadInteger4);

  RaverieBindMethod(ReadReal);
  RaverieBindMethod(ReadDoubleReal);
  RaverieBindMethod(ReadReal2);
  RaverieBindMethod(ReadReal3);
  RaverieBindMethod(ReadReal4);
  RaverieBindMethod(ReadQuaternion);

  RaverieBindMethod(ReadString);

  // Bind read half operations
  RaverieBindMethod(ReadRealHalf);
  RaverieBindMethod(ReadReal2Half);
  RaverieBindMethod(ReadReal3Half);
  RaverieBindMethod(ReadReal4Half);

  // Bind read quantized operations
  RaverieBindOverloadedMethod(ReadIntegerQuantized, RaverieConstInstanceOverload(int, int, int));
  RaverieBindOverloadedMethod(ReadIntegerQuantized, RaverieConstInstanceOverload(int, int, int, int));

  RaverieBindOverloadedMethod(ReadDoubleIntegerQuantized, RaverieConstInstanceOverload(s64, s64, s64));
  RaverieBindOverloadedMethod(ReadDoubleIntegerQuantized, RaverieConstInstanceOverload(s64, s64, s64, s64));

  RaverieBindOverloadedMethod(
      ReadInteger2Quantized,
      RaverieConstInstanceOverload(Math::IntVector2, const Math::IntVector2&, const Math::IntVector2&));
  RaverieBindOverloadedMethod(
      ReadInteger2Quantized,
      RaverieConstInstanceOverload(
          Math::IntVector2, const Math::IntVector2&, const Math::IntVector2&, const Math::IntVector2&));

  RaverieBindOverloadedMethod(
      ReadInteger3Quantized,
      RaverieConstInstanceOverload(Math::IntVector3, const Math::IntVector3&, const Math::IntVector3&));
  RaverieBindOverloadedMethod(
      ReadInteger3Quantized,
      RaverieConstInstanceOverload(
          Math::IntVector3, const Math::IntVector3&, const Math::IntVector3&, const Math::IntVector3&));

  RaverieBindOverloadedMethod(
      ReadInteger4Quantized,
      RaverieConstInstanceOverload(Math::IntVector4, const Math::IntVector4&, const Math::IntVector4&));
  RaverieBindOverloadedMethod(
      ReadInteger4Quantized,
      RaverieConstInstanceOverload(
          Math::IntVector4, const Math::IntVector4&, const Math::IntVector4&, const Math::IntVector4&));

  RaverieBindOverloadedMethod(ReadRealQuantized, RaverieConstInstanceOverload(float, float, float));
  RaverieBindOverloadedMethod(ReadRealQuantized, RaverieConstInstanceOverload(float, float, float, float));

  RaverieBindOverloadedMethod(ReadDoubleRealQuantized, RaverieConstInstanceOverload(double, double, double));
  RaverieBindOverloadedMethod(ReadDoubleRealQuantized, RaverieConstInstanceOverload(double, double, double, double));

  RaverieBindOverloadedMethod(ReadReal2Quantized,
                            RaverieConstInstanceOverload(Math::Vector2, const Math::Vector2&, const Math::Vector2&));
  RaverieBindOverloadedMethod(
      ReadReal2Quantized,
      RaverieConstInstanceOverload(Math::Vector2, const Math::Vector2&, const Math::Vector2&, const Math::Vector2&));

  RaverieBindOverloadedMethod(ReadReal3Quantized,
                            RaverieConstInstanceOverload(Math::Vector3, const Math::Vector3&, const Math::Vector3&));
  RaverieBindOverloadedMethod(
      ReadReal3Quantized,
      RaverieConstInstanceOverload(Math::Vector3, const Math::Vector3&, const Math::Vector3&, const Math::Vector3&));

  RaverieBindOverloadedMethod(ReadReal4Quantized,
                            RaverieConstInstanceOverload(Math::Vector4, const Math::Vector4&, const Math::Vector4&));
  RaverieBindOverloadedMethod(
      ReadReal4Quantized,
      RaverieConstInstanceOverload(Math::Vector4, const Math::Vector4&, const Math::Vector4&, const Math::Vector4&));

  // Bind bitstream methods
  RaverieBindMethod(GetBitCapacity);
  RaverieBindMethod(GetByteCapacity);

  RaverieBindMethod(GetBitsWritten);
  RaverieBindMethod(GetBytesWritten);

  RaverieBindMethod(GetBitsUnwritten);
  RaverieBindMethod(GetBytesUnwritten);

  RaverieBindMethod(GetBitsRead);
  RaverieBindMethod(GetBytesRead);

  RaverieBindMethod(GetBitsUnread);
  RaverieBindMethod(GetBytesUnread);

  RaverieBindMethod(IsEmpty);
  RaverieBindMethod(Reserve);
  RaverieBindMethod(Clear);

  // RaverieBindMethod(Unwrite);
  RaverieBindMethod(SetBitsWritten);
  RaverieBindMethod(SetBytesWritten);
  RaverieBindMethod(ClearBitsWritten);

  RaverieBindMethod(Unread);
  RaverieBindMethod(SetBitsRead);
  RaverieBindMethod(SetBytesRead);
  RaverieBindMethod(ClearBitsRead);

  // Make constructible in script
  type->CreatableInScript = true;
}

BitStreamExtended::BitStreamExtended() : BitStream()
{
}
BitStreamExtended::BitStreamExtended(const BitStream& rhs) : BitStream(rhs)
{
}

BitStreamExtended::~BitStreamExtended()
{
}

BitStreamExtended& BitStreamExtended::operator=(const BitStream& rhs)
{
  BitStream::operator=(rhs);
  return *this;
}

//
// Measure Operations
//

Bits BitStreamExtended::MeasureBoolean()
{
  return 1;
}

Bits BitStreamExtended::MeasureInteger()
{
  return BYTES_TO_BITS(sizeof(int));
}
Bits BitStreamExtended::MeasureDoubleInteger()
{
  return BYTES_TO_BITS(sizeof(s64));
}
Bits BitStreamExtended::MeasureInteger2()
{
  return (BYTES_TO_BITS(sizeof(int)) * 2);
}
Bits BitStreamExtended::MeasureInteger3()
{
  return (BYTES_TO_BITS(sizeof(int)) * 3);
}
Bits BitStreamExtended::MeasureInteger4()
{
  return (BYTES_TO_BITS(sizeof(int)) * 4);
}

Bits BitStreamExtended::MeasureReal()
{
  return BYTES_TO_BITS(sizeof(float));
}
Bits BitStreamExtended::MeasureDoubleReal()
{
  return BYTES_TO_BITS(sizeof(double));
}
Bits BitStreamExtended::MeasureReal2()
{
  return (BYTES_TO_BITS(sizeof(float)) * 2);
}
Bits BitStreamExtended::MeasureReal3()
{
  return (BYTES_TO_BITS(sizeof(float)) * 3);
}
Bits BitStreamExtended::MeasureReal4()
{
  return (BYTES_TO_BITS(sizeof(float)) * 4);
}
Bits BitStreamExtended::MeasureQuaternion()
{
  return (BYTES_TO_BITS(sizeof(float)) * 4);
}

Bits BitStreamExtended::MeasureString()
{
  return BYTES_TO_BITS(sizeof(char));
}
Bits BitStreamExtended::MeasureString(StringParam value)
{
  return BitStream::Measure(value);
}

//
// Measure Half Operations
//

Bits BitStreamExtended::MeasureRealHalf()
{
  return BYTES_TO_BITS(sizeof(u16));
}
Bits BitStreamExtended::MeasureReal2Half()
{
  return (BYTES_TO_BITS(sizeof(u16)) * 2);
}
Bits BitStreamExtended::MeasureReal3Half()
{
  return (BYTES_TO_BITS(sizeof(u16)) * 3);
}
Bits BitStreamExtended::MeasureReal4Half()
{
  return (BYTES_TO_BITS(sizeof(u16)) * 4);
}

//
// Measure Quantized Operations
//

Bits BitStreamExtended::MeasureIntegerQuantized(int minValue, int maxValue)
{
  return MeasureIntegerQuantized(minValue, maxValue, DefaultIntegralQuantum);
}
Bits BitStreamExtended::MeasureIntegerQuantized(int minValue, int maxValue, int quantum)
{
  return BitStream::MeasureQuantized(minValue, maxValue, quantum);
}

Bits BitStreamExtended::MeasureDoubleIntegerQuantized(s64 minValue, s64 maxValue)
{
  return MeasureDoubleIntegerQuantized(minValue, maxValue, DefaultIntegralQuantum);
}
Bits BitStreamExtended::MeasureDoubleIntegerQuantized(s64 minValue, s64 maxValue, s64 quantum)
{
  return BitStream::MeasureQuantized(minValue, maxValue, quantum);
}

Bits BitStreamExtended::MeasureInteger2Quantized(const Math::IntVector2& minValue, const Math::IntVector2& maxValue)
{
  return MeasureInteger2Quantized(minValue, maxValue, Math::IntVector2(DefaultIntegralQuantum, DefaultIntegralQuantum));
}
Bits BitStreamExtended::MeasureInteger2Quantized(const Math::IntVector2& minValue,
                                                 const Math::IntVector2& maxValue,
                                                 const Math::IntVector2& quantum)
{
  return BitStream::MeasureQuantized(minValue.x, maxValue.x, quantum.x) +
         BitStream::MeasureQuantized(minValue.y, maxValue.y, quantum.y);
}

Bits BitStreamExtended::MeasureInteger3Quantized(const Math::IntVector3& minValue, const Math::IntVector3& maxValue)
{
  return MeasureInteger3Quantized(
      minValue, maxValue, Math::IntVector3(DefaultIntegralQuantum, DefaultIntegralQuantum, DefaultIntegralQuantum));
}
Bits BitStreamExtended::MeasureInteger3Quantized(const Math::IntVector3& minValue,
                                                 const Math::IntVector3& maxValue,
                                                 const Math::IntVector3& quantum)
{
  return BitStream::MeasureQuantized(minValue.x, maxValue.x, quantum.x) +
         BitStream::MeasureQuantized(minValue.y, maxValue.y, quantum.y) +
         BitStream::MeasureQuantized(minValue.z, maxValue.z, quantum.z);
}

Bits BitStreamExtended::MeasureInteger4Quantized(const Math::IntVector4& minValue, const Math::IntVector4& maxValue)
{
  return MeasureInteger4Quantized(
      minValue,
      maxValue,
      Math::IntVector4(DefaultIntegralQuantum, DefaultIntegralQuantum, DefaultIntegralQuantum, DefaultIntegralQuantum));
}
Bits BitStreamExtended::MeasureInteger4Quantized(const Math::IntVector4& minValue,
                                                 const Math::IntVector4& maxValue,
                                                 const Math::IntVector4& quantum)
{
  return BitStream::MeasureQuantized(minValue.x, maxValue.x, quantum.x) +
         BitStream::MeasureQuantized(minValue.y, maxValue.y, quantum.y) +
         BitStream::MeasureQuantized(minValue.z, maxValue.z, quantum.z) +
         BitStream::MeasureQuantized(minValue.w, maxValue.w, quantum.w);
}

Bits BitStreamExtended::MeasureRealQuantized(float minValue, float maxValue)
{
  return MeasureRealQuantized(minValue, maxValue, DefaultFloatingPointQuantum);
}
Bits BitStreamExtended::MeasureRealQuantized(float minValue, float maxValue, float quantum)
{
  return BitStream::MeasureQuantized(minValue, maxValue, quantum);
}

Bits BitStreamExtended::MeasureDoubleRealQuantized(double minValue, double maxValue)
{
  return MeasureDoubleRealQuantized(minValue, maxValue, DefaultFloatingPointQuantum);
}
Bits BitStreamExtended::MeasureDoubleRealQuantized(double minValue, double maxValue, double quantum)
{
  return BitStream::MeasureQuantized(minValue, maxValue, quantum);
}

Bits BitStreamExtended::MeasureReal2Quantized(const Math::Vector2& minValue, const Math::Vector2& maxValue)
{
  return MeasureReal2Quantized(minValue, maxValue, Math::Vector2(DefaultFloatingPointQuantum));
}
Bits BitStreamExtended::MeasureReal2Quantized(const Math::Vector2& minValue,
                                              const Math::Vector2& maxValue,
                                              const Math::Vector2& quantum)
{
  return BitStream::MeasureQuantized(minValue.x, maxValue.x, quantum.x) +
         BitStream::MeasureQuantized(minValue.y, maxValue.y, quantum.y);
}

Bits BitStreamExtended::MeasureReal3Quantized(const Math::Vector3& minValue, const Math::Vector3& maxValue)
{
  return MeasureReal3Quantized(minValue, maxValue, Math::Vector3(DefaultFloatingPointQuantum));
}
Bits BitStreamExtended::MeasureReal3Quantized(const Math::Vector3& minValue,
                                              const Math::Vector3& maxValue,
                                              const Math::Vector3& quantum)
{
  return BitStream::MeasureQuantized(minValue.x, maxValue.x, quantum.x) +
         BitStream::MeasureQuantized(minValue.y, maxValue.y, quantum.y) +
         BitStream::MeasureQuantized(minValue.z, maxValue.z, quantum.z);
}

Bits BitStreamExtended::MeasureReal4Quantized(const Math::Vector4& minValue, const Math::Vector4& maxValue)
{
  return MeasureReal4Quantized(minValue, maxValue, Math::Vector4(DefaultFloatingPointQuantum));
}
Bits BitStreamExtended::MeasureReal4Quantized(const Math::Vector4& minValue,
                                              const Math::Vector4& maxValue,
                                              const Math::Vector4& quantum)
{
  return BitStream::MeasureQuantized(minValue.x, maxValue.x, quantum.x) +
         BitStream::MeasureQuantized(minValue.y, maxValue.y, quantum.y) +
         BitStream::MeasureQuantized(minValue.z, maxValue.z, quantum.z) +
         BitStream::MeasureQuantized(minValue.w, maxValue.w, quantum.w);
}

//
// Write Operations
//

void BitStreamExtended::WriteBoolean(bool value)
{
  Bits bitsWritten = BitStream::Write(value);
  Assert(bitsWritten == MeasureBoolean());
}

void BitStreamExtended::WriteInteger(int value)
{
  Bits bitsWritten = BitStream::Write(value);
  Assert(bitsWritten == MeasureInteger());
}
void BitStreamExtended::WriteDoubleInteger(s64 value)
{
  Bits bitsWritten = BitStream::Write(value);
  Assert(bitsWritten == MeasureDoubleInteger());
}
void BitStreamExtended::WriteInteger2(const Math::IntVector2& value)
{
  Bits bitsWritten = 0;
  bitsWritten += BitStream::Write(value.x);
  bitsWritten += BitStream::Write(value.y);
  Assert(bitsWritten == MeasureInteger2());
}
void BitStreamExtended::WriteInteger3(const Math::IntVector3& value)
{
  Bits bitsWritten = 0;
  bitsWritten += BitStream::Write(value.x);
  bitsWritten += BitStream::Write(value.y);
  bitsWritten += BitStream::Write(value.z);
  Assert(bitsWritten == MeasureInteger3());
}
void BitStreamExtended::WriteInteger4(const Math::IntVector4& value)
{
  Bits bitsWritten = 0;
  bitsWritten += BitStream::Write(value.x);
  bitsWritten += BitStream::Write(value.y);
  bitsWritten += BitStream::Write(value.z);
  bitsWritten += BitStream::Write(value.w);
  Assert(bitsWritten == MeasureInteger4());
}

void BitStreamExtended::WriteReal(float value)
{
  Bits bitsWritten = BitStream::Write(value);
  Assert(bitsWritten == MeasureReal());
}
void BitStreamExtended::WriteDoubleReal(double value)
{
  Bits bitsWritten = BitStream::Write(value);
  Assert(bitsWritten == MeasureDoubleReal());
}
void BitStreamExtended::WriteReal2(const Math::Vector2& value)
{
  Bits bitsWritten = 0;
  bitsWritten += BitStream::Write(value.x);
  bitsWritten += BitStream::Write(value.y);
  Assert(bitsWritten == MeasureReal2());
}
void BitStreamExtended::WriteReal3(const Math::Vector3& value)
{
  Bits bitsWritten = 0;
  bitsWritten += BitStream::Write(value.x);
  bitsWritten += BitStream::Write(value.y);
  bitsWritten += BitStream::Write(value.z);
  Assert(bitsWritten == MeasureReal3());
}
void BitStreamExtended::WriteReal4(const Math::Vector4& value)
{
  Bits bitsWritten = 0;
  bitsWritten += BitStream::Write(value.x);
  bitsWritten += BitStream::Write(value.y);
  bitsWritten += BitStream::Write(value.z);
  bitsWritten += BitStream::Write(value.w);
  Assert(bitsWritten == MeasureReal4());
}
void BitStreamExtended::WriteQuaternion(const Math::Quaternion& value)
{
  Bits bitsWritten = 0;
  bitsWritten += BitStream::Write(value.x);
  bitsWritten += BitStream::Write(value.y);
  bitsWritten += BitStream::Write(value.z);
  bitsWritten += BitStream::Write(value.w);
  Assert(bitsWritten == MeasureReal4());
}

void BitStreamExtended::WriteString(StringParam value)
{
  Bits bitsWritten = BitStream::Write(value);
  Assert(bitsWritten == MeasureString(value));
}

//
// Write Half Operations
//

void BitStreamExtended::WriteRealHalf(float value)
{
  Bits bitsWritten = BitStream::Write(HalfFloatConverter::ToHalfFloat(value));
  Assert(bitsWritten == MeasureRealHalf());
}
void BitStreamExtended::WriteReal2Half(const Math::Vector2& value)
{
  Bits bitsWritten = 0;
  bitsWritten += BitStream::Write(HalfFloatConverter::ToHalfFloat(value.x));
  bitsWritten += BitStream::Write(HalfFloatConverter::ToHalfFloat(value.y));
  Assert(bitsWritten == MeasureReal2Half());
}
void BitStreamExtended::WriteReal3Half(const Math::Vector3& value)
{
  Bits bitsWritten = 0;
  bitsWritten += BitStream::Write(HalfFloatConverter::ToHalfFloat(value.x));
  bitsWritten += BitStream::Write(HalfFloatConverter::ToHalfFloat(value.y));
  bitsWritten += BitStream::Write(HalfFloatConverter::ToHalfFloat(value.z));
  Assert(bitsWritten == MeasureReal3Half());
}
void BitStreamExtended::WriteReal4Half(const Math::Vector4& value)
{
  Bits bitsWritten = 0;
  bitsWritten += BitStream::Write(HalfFloatConverter::ToHalfFloat(value.x));
  bitsWritten += BitStream::Write(HalfFloatConverter::ToHalfFloat(value.y));
  bitsWritten += BitStream::Write(HalfFloatConverter::ToHalfFloat(value.z));
  bitsWritten += BitStream::Write(HalfFloatConverter::ToHalfFloat(value.w));
  Assert(bitsWritten == MeasureReal4Half());
}

//
// Write Quantized Operations
//

void BitStreamExtended::WriteIntegerQuantized(int value, int minValue, int maxValue)
{
  return WriteIntegerQuantized(value, minValue, maxValue, DefaultIntegralQuantum);
}
void BitStreamExtended::WriteIntegerQuantized(int value, int minValue, int maxValue, int quantum)
{
  Bits bitsWritten = BitStream::WriteQuantized(value, minValue, maxValue, quantum);
  Assert(bitsWritten == MeasureIntegerQuantized(minValue, maxValue, quantum));
}

void BitStreamExtended::WriteDoubleIntegerQuantized(s64 value, s64 minValue, s64 maxValue)
{
  return WriteDoubleIntegerQuantized(value, minValue, maxValue, DefaultIntegralQuantum);
}
void BitStreamExtended::WriteDoubleIntegerQuantized(s64 value, s64 minValue, s64 maxValue, s64 quantum)
{
  Bits bitsWritten = BitStream::WriteQuantized(value, minValue, maxValue, quantum);
  Assert(bitsWritten == MeasureDoubleIntegerQuantized(minValue, maxValue, quantum));
}

void BitStreamExtended::WriteInteger2Quantized(const Math::IntVector2& value,
                                               const Math::IntVector2& minValue,
                                               const Math::IntVector2& maxValue)
{
  return WriteInteger2Quantized(
      value, minValue, maxValue, Math::IntVector2(DefaultIntegralQuantum, DefaultIntegralQuantum));
}
void BitStreamExtended::WriteInteger2Quantized(const Math::IntVector2& value,
                                               const Math::IntVector2& minValue,
                                               const Math::IntVector2& maxValue,
                                               const Math::IntVector2& quantum)
{
  Bits bitsWritten = 0;
  bitsWritten += BitStream::WriteQuantized(value.x, minValue.x, maxValue.x, quantum.x);
  bitsWritten += BitStream::WriteQuantized(value.y, minValue.y, maxValue.y, quantum.y);
  Assert(bitsWritten == MeasureInteger2Quantized(minValue, maxValue, quantum));
}

void BitStreamExtended::WriteInteger3Quantized(const Math::IntVector3& value,
                                               const Math::IntVector3& minValue,
                                               const Math::IntVector3& maxValue)
{
  return WriteInteger3Quantized(
      value,
      minValue,
      maxValue,
      Math::IntVector3(DefaultIntegralQuantum, DefaultIntegralQuantum, DefaultIntegralQuantum));
}
void BitStreamExtended::WriteInteger3Quantized(const Math::IntVector3& value,
                                               const Math::IntVector3& minValue,
                                               const Math::IntVector3& maxValue,
                                               const Math::IntVector3& quantum)
{
  Bits bitsWritten = 0;
  bitsWritten += BitStream::WriteQuantized(value.x, minValue.x, maxValue.x, quantum.x);
  bitsWritten += BitStream::WriteQuantized(value.y, minValue.y, maxValue.y, quantum.y);
  bitsWritten += BitStream::WriteQuantized(value.z, minValue.z, maxValue.z, quantum.z);
  Assert(bitsWritten == MeasureInteger3Quantized(minValue, maxValue, quantum));
}

void BitStreamExtended::WriteInteger4Quantized(const Math::IntVector4& value,
                                               const Math::IntVector4& minValue,
                                               const Math::IntVector4& maxValue)
{
  return WriteInteger4Quantized(
      value,
      minValue,
      maxValue,
      Math::IntVector4(DefaultIntegralQuantum, DefaultIntegralQuantum, DefaultIntegralQuantum, DefaultIntegralQuantum));
}
void BitStreamExtended::WriteInteger4Quantized(const Math::IntVector4& value,
                                               const Math::IntVector4& minValue,
                                               const Math::IntVector4& maxValue,
                                               const Math::IntVector4& quantum)
{
  Bits bitsWritten = 0;
  bitsWritten += BitStream::WriteQuantized(value.x, minValue.x, maxValue.x, quantum.x);
  bitsWritten += BitStream::WriteQuantized(value.y, minValue.y, maxValue.y, quantum.y);
  bitsWritten += BitStream::WriteQuantized(value.z, minValue.z, maxValue.z, quantum.z);
  bitsWritten += BitStream::WriteQuantized(value.w, minValue.w, maxValue.w, quantum.w);
  Assert(bitsWritten == MeasureInteger4Quantized(minValue, maxValue, quantum));
}

void BitStreamExtended::WriteRealQuantized(float value, float minValue, float maxValue)
{
  return WriteRealQuantized(value, minValue, maxValue, DefaultFloatingPointQuantum);
}
void BitStreamExtended::WriteRealQuantized(float value, float minValue, float maxValue, float quantum)
{
  Bits bitsWritten = BitStream::WriteQuantized(value, minValue, maxValue, quantum);
  Assert(bitsWritten == MeasureRealQuantized(minValue, maxValue, quantum));
}

void BitStreamExtended::WriteDoubleRealQuantized(double value, double minValue, double maxValue)
{
  return WriteDoubleRealQuantized(value, minValue, maxValue, DefaultFloatingPointQuantum);
}
void BitStreamExtended::WriteDoubleRealQuantized(double value, double minValue, double maxValue, double quantum)
{
  Bits bitsWritten = BitStream::WriteQuantized(value, minValue, maxValue, quantum);
  Assert(bitsWritten == MeasureDoubleRealQuantized(minValue, maxValue, quantum));
}

void BitStreamExtended::WriteReal2Quantized(const Math::Vector2& value,
                                            const Math::Vector2& minValue,
                                            const Math::Vector2& maxValue)
{
  return WriteReal2Quantized(value, minValue, maxValue, Math::Vector2(DefaultFloatingPointQuantum));
}
void BitStreamExtended::WriteReal2Quantized(const Math::Vector2& value,
                                            const Math::Vector2& minValue,
                                            const Math::Vector2& maxValue,
                                            const Math::Vector2& quantum)
{
  Bits bitsWritten = 0;
  bitsWritten += BitStream::WriteQuantized(value.x, minValue.x, maxValue.x, quantum.x);
  bitsWritten += BitStream::WriteQuantized(value.y, minValue.y, maxValue.y, quantum.y);
  Assert(bitsWritten == MeasureReal2Quantized(minValue, maxValue, quantum));
}

void BitStreamExtended::WriteReal3Quantized(const Math::Vector3& value,
                                            const Math::Vector3& minValue,
                                            const Math::Vector3& maxValue)
{
  return WriteReal3Quantized(value, minValue, maxValue, Math::Vector3(DefaultFloatingPointQuantum));
}
void BitStreamExtended::WriteReal3Quantized(const Math::Vector3& value,
                                            const Math::Vector3& minValue,
                                            const Math::Vector3& maxValue,
                                            const Math::Vector3& quantum)
{
  Bits bitsWritten = 0;
  bitsWritten += BitStream::WriteQuantized(value.x, minValue.x, maxValue.x, quantum.x);
  bitsWritten += BitStream::WriteQuantized(value.y, minValue.y, maxValue.y, quantum.y);
  bitsWritten += BitStream::WriteQuantized(value.z, minValue.z, maxValue.z, quantum.z);
  Assert(bitsWritten == MeasureReal3Quantized(minValue, maxValue, quantum));
}

void BitStreamExtended::WriteReal4Quantized(const Math::Vector4& value,
                                            const Math::Vector4& minValue,
                                            const Math::Vector4& maxValue)
{
  return WriteReal4Quantized(value, minValue, maxValue, Math::Vector4(DefaultFloatingPointQuantum));
}
void BitStreamExtended::WriteReal4Quantized(const Math::Vector4& value,
                                            const Math::Vector4& minValue,
                                            const Math::Vector4& maxValue,
                                            const Math::Vector4& quantum)
{
  Bits bitsWritten = 0;
  bitsWritten += BitStream::WriteQuantized(value.x, minValue.x, maxValue.x, quantum.x);
  bitsWritten += BitStream::WriteQuantized(value.y, minValue.y, maxValue.y, quantum.y);
  bitsWritten += BitStream::WriteQuantized(value.z, minValue.z, maxValue.z, quantum.z);
  bitsWritten += BitStream::WriteQuantized(value.w, minValue.w, maxValue.w, quantum.w);
  Assert(bitsWritten == MeasureReal4Quantized(minValue, maxValue, quantum));
}

//
// Can-Read Operations
//

bool BitStreamExtended::CanReadBoolean() const
{
  return BitStream::GetBitsUnread() >= MeasureBoolean();
}

bool BitStreamExtended::CanReadInteger() const
{
  return BitStream::GetBitsUnread() >= MeasureInteger();
}
bool BitStreamExtended::CanReadDoubleInteger() const
{
  return BitStream::GetBitsUnread() >= MeasureDoubleInteger();
}
bool BitStreamExtended::CanReadInteger2() const
{
  return BitStream::GetBitsUnread() >= MeasureInteger2();
}
bool BitStreamExtended::CanReadInteger3() const
{
  return BitStream::GetBitsUnread() >= MeasureInteger3();
}
bool BitStreamExtended::CanReadInteger4() const
{
  return BitStream::GetBitsUnread() >= MeasureInteger4();
}

bool BitStreamExtended::CanReadReal() const
{
  return BitStream::GetBitsUnread() >= MeasureReal();
}
bool BitStreamExtended::CanReadDoubleReal() const
{
  return BitStream::GetBitsUnread() >= MeasureDoubleReal();
}
bool BitStreamExtended::CanReadReal2() const
{
  return BitStream::GetBitsUnread() >= MeasureReal2();
}
bool BitStreamExtended::CanReadReal3() const
{
  return BitStream::GetBitsUnread() >= MeasureReal3();
}
bool BitStreamExtended::CanReadReal4() const
{
  return BitStream::GetBitsUnread() >= MeasureReal4();
}
bool BitStreamExtended::CanReadQuaternion() const
{
  return BitStream::GetBitsUnread() >= MeasureReal4();
}

bool BitStreamExtended::CanReadString() const
{
  return BitStream::GetBitsUnread() >= MeasureString();
}

//
// Can-Read Half Operations
//

bool BitStreamExtended::CanReadRealHalf() const
{
  return BitStream::GetBitsUnread() >= MeasureRealHalf();
}
bool BitStreamExtended::CanReadReal2Half() const
{
  return BitStream::GetBitsUnread() >= MeasureReal2Half();
}
bool BitStreamExtended::CanReadReal3Half() const
{
  return BitStream::GetBitsUnread() >= MeasureReal3Half();
}
bool BitStreamExtended::CanReadReal4Half() const
{
  return BitStream::GetBitsUnread() >= MeasureReal4Half();
}

//
// Can-Read Quantized Operations
//

bool BitStreamExtended::CanReadIntegerQuantized(int minValue, int maxValue) const
{
  return BitStream::GetBitsUnread() >= MeasureIntegerQuantized(minValue, maxValue);
}
bool BitStreamExtended::CanReadIntegerQuantized(int minValue, int maxValue, int quantum) const
{
  return BitStream::GetBitsUnread() >= MeasureIntegerQuantized(minValue, maxValue, quantum);
}

bool BitStreamExtended::CanReadDoubleIntegerQuantized(s64 minValue, s64 maxValue) const
{
  return BitStream::GetBitsUnread() >= MeasureDoubleIntegerQuantized(minValue, maxValue);
}
bool BitStreamExtended::CanReadDoubleIntegerQuantized(s64 minValue, s64 maxValue, s64 quantum) const
{
  return BitStream::GetBitsUnread() >= MeasureDoubleIntegerQuantized(minValue, maxValue, quantum);
}

bool BitStreamExtended::CanReadInteger2Quantized(const Math::IntVector2& minValue,
                                                 const Math::IntVector2& maxValue) const
{
  return BitStream::GetBitsUnread() >= MeasureInteger2Quantized(minValue, maxValue);
}
bool BitStreamExtended::CanReadInteger2Quantized(const Math::IntVector2& minValue,
                                                 const Math::IntVector2& maxValue,
                                                 const Math::IntVector2& quantum) const
{
  return BitStream::GetBitsUnread() >= MeasureInteger2Quantized(minValue, maxValue, quantum);
}

bool BitStreamExtended::CanReadInteger3Quantized(const Math::IntVector3& minValue,
                                                 const Math::IntVector3& maxValue) const
{
  return BitStream::GetBitsUnread() >= MeasureInteger3Quantized(minValue, maxValue);
}
bool BitStreamExtended::CanReadInteger3Quantized(const Math::IntVector3& minValue,
                                                 const Math::IntVector3& maxValue,
                                                 const Math::IntVector3& quantum) const
{
  return BitStream::GetBitsUnread() >= MeasureInteger3Quantized(minValue, maxValue, quantum);
}

bool BitStreamExtended::CanReadInteger4Quantized(const Math::IntVector4& minValue,
                                                 const Math::IntVector4& maxValue) const
{
  return BitStream::GetBitsUnread() >= MeasureInteger4Quantized(minValue, maxValue);
}
bool BitStreamExtended::CanReadInteger4Quantized(const Math::IntVector4& minValue,
                                                 const Math::IntVector4& maxValue,
                                                 const Math::IntVector4& quantum) const
{
  return BitStream::GetBitsUnread() >= MeasureInteger4Quantized(minValue, maxValue, quantum);
}

bool BitStreamExtended::CanReadRealQuantized(float minValue, float maxValue) const
{
  return BitStream::GetBitsUnread() >= MeasureRealQuantized(minValue, maxValue);
}
bool BitStreamExtended::CanReadRealQuantized(float minValue, float maxValue, float quantum) const
{
  return BitStream::GetBitsUnread() >= MeasureRealQuantized(minValue, maxValue, quantum);
}

bool BitStreamExtended::CanReadDoubleRealQuantized(double minValue, double maxValue) const
{
  return BitStream::GetBitsUnread() >= MeasureDoubleRealQuantized(minValue, maxValue);
}
bool BitStreamExtended::CanReadDoubleRealQuantized(double minValue, double maxValue, double quantum) const
{
  return BitStream::GetBitsUnread() >= MeasureDoubleRealQuantized(minValue, maxValue, quantum);
}

bool BitStreamExtended::CanReadReal2Quantized(const Math::Vector2& minValue, const Math::Vector2& maxValue) const
{
  return BitStream::GetBitsUnread() >= MeasureReal2Quantized(minValue, maxValue);
}
bool BitStreamExtended::CanReadReal2Quantized(const Math::Vector2& minValue,
                                              const Math::Vector2& maxValue,
                                              const Math::Vector2& quantum) const
{
  return BitStream::GetBitsUnread() >= MeasureReal2Quantized(minValue, maxValue, quantum);
}

bool BitStreamExtended::CanReadReal3Quantized(const Math::Vector3& minValue, const Math::Vector3& maxValue) const
{
  return BitStream::GetBitsUnread() >= MeasureReal3Quantized(minValue, maxValue);
}
bool BitStreamExtended::CanReadReal3Quantized(const Math::Vector3& minValue,
                                              const Math::Vector3& maxValue,
                                              const Math::Vector3& quantum) const
{
  return BitStream::GetBitsUnread() >= MeasureReal3Quantized(minValue, maxValue, quantum);
}

bool BitStreamExtended::CanReadReal4Quantized(const Math::Vector4& minValue, const Math::Vector4& maxValue) const
{
  return BitStream::GetBitsUnread() >= MeasureReal4Quantized(minValue, maxValue);
}
bool BitStreamExtended::CanReadReal4Quantized(const Math::Vector4& minValue,
                                              const Math::Vector4& maxValue,
                                              const Math::Vector4& quantum) const
{
  return BitStream::GetBitsUnread() >= MeasureReal4Quantized(minValue, maxValue, quantum);
}

//
// Read Operations
//

bool BitStreamExtended::ReadBoolean() const
{
  Assert(CanReadBoolean());

  bool value = false;
  Bits bitsRead = BitStream::Read(value);
  Assert(bitsRead == MeasureBoolean());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Boolean value (%u bits) from BitStream "
                                     "(%u / %u bits read/written)",
                                     MeasureBoolean(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}

int BitStreamExtended::ReadInteger() const
{
  Assert(CanReadInteger());

  int value = 0;
  Bits bitsRead = BitStream::Read(value);
  Assert(bitsRead == MeasureInteger());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Integer value (%u bits) from BitStream "
                                     "(%u / %u bits read/written)",
                                     MeasureInteger(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}
s64 BitStreamExtended::ReadDoubleInteger() const
{
  Assert(CanReadDoubleInteger());

  s64 value = 0;
  Bits bitsRead = BitStream::Read(value);
  Assert(bitsRead == MeasureDoubleInteger());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read DoubleInteger value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureDoubleInteger(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}
Math::IntVector2 BitStreamExtended::ReadInteger2() const
{
  Assert(CanReadInteger2());

  Math::IntVector2 value;
  Bits bitsRead = 0;
  bitsRead += BitStream::Read(value.x);
  bitsRead += BitStream::Read(value.y);
  Assert(bitsRead == MeasureInteger2());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Integer2 value (%u bits) from BitStream "
                                     "(%u / %u bits read/written)",
                                     MeasureInteger2(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}
Math::IntVector3 BitStreamExtended::ReadInteger3() const
{
  Assert(CanReadInteger3());

  Math::IntVector3 value;
  Bits bitsRead = 0;
  bitsRead += BitStream::Read(value.x);
  bitsRead += BitStream::Read(value.y);
  bitsRead += BitStream::Read(value.z);
  Assert(bitsRead == MeasureInteger3());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Integer3 value (%u bits) from BitStream "
                                     "(%u / %u bits read/written)",
                                     MeasureInteger3(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}
Math::IntVector4 BitStreamExtended::ReadInteger4() const
{
  Assert(CanReadInteger4());

  Math::IntVector4 value;
  Bits bitsRead = 0;
  bitsRead += BitStream::Read(value.x);
  bitsRead += BitStream::Read(value.y);
  bitsRead += BitStream::Read(value.z);
  bitsRead += BitStream::Read(value.w);
  Assert(bitsRead == MeasureInteger4());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Integer4 value (%u bits) from BitStream "
                                     "(%u / %u bits read/written)",
                                     MeasureInteger4(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}

float BitStreamExtended::ReadReal() const
{
  Assert(CanReadReal());

  float value = 0;
  Bits bitsRead = BitStream::Read(value);
  Assert(bitsRead == MeasureReal());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Real value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureReal(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}
double BitStreamExtended::ReadDoubleReal() const
{
  Assert(CanReadDoubleReal());

  double value = 0;
  Bits bitsRead = BitStream::Read(value);
  Assert(bitsRead == MeasureDoubleReal());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read DoubleReal value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureDoubleReal(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}
Math::Vector2 BitStreamExtended::ReadReal2() const
{
  Assert(CanReadReal2());

  Math::Vector2 value;
  Bits bitsRead = 0;
  bitsRead += BitStream::Read(value.x);
  bitsRead += BitStream::Read(value.y);
  Assert(bitsRead == MeasureReal2());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Real2 value (%u bits) from BitStream "
                                     "(%u / %u bits read/written)",
                                     MeasureReal2(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}
Math::Vector3 BitStreamExtended::ReadReal3() const
{
  Assert(CanReadReal3());

  Math::Vector3 value;
  Bits bitsRead = 0;
  bitsRead += BitStream::Read(value.x);
  bitsRead += BitStream::Read(value.y);
  bitsRead += BitStream::Read(value.z);
  Assert(bitsRead == MeasureReal3());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Real3 value (%u bits) from BitStream "
                                     "(%u / %u bits read/written)",
                                     MeasureReal3(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}
Math::Vector4 BitStreamExtended::ReadReal4() const
{
  Assert(CanReadReal4());

  Math::Vector4 value;
  Bits bitsRead = 0;
  bitsRead += BitStream::Read(value.x);
  bitsRead += BitStream::Read(value.y);
  bitsRead += BitStream::Read(value.z);
  bitsRead += BitStream::Read(value.w);
  Assert(bitsRead == MeasureReal4());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Real4 value (%u bits) from BitStream "
                                     "(%u / %u bits read/written)",
                                     MeasureReal4(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}
Math::Quaternion BitStreamExtended::ReadQuaternion() const
{
  Assert(CanReadQuaternion());

  Math::Quaternion value;
  Bits bitsRead = 0;
  bitsRead += BitStream::Read(value.x);
  bitsRead += BitStream::Read(value.y);
  bitsRead += BitStream::Read(value.z);
  bitsRead += BitStream::Read(value.w);
  Assert(bitsRead == MeasureQuaternion());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Quaternion value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureQuaternion(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}

String BitStreamExtended::ReadString() const
{
  Assert(CanReadString());

  String value;
  Bits bitsRead = BitStream::Read(value);
  Assert(bitsRead == MeasureString());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read String value (%u bits) from BitStream "
                                     "(%u / %u bits read/written)",
                                     MeasureString(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}

//
// BitStreamExtended::Read Half Operations
//

float BitStreamExtended::ReadRealHalf() const
{
  Assert(CanReadRealHalf());

  u16 value = 0;
  Bits bitsRead = BitStream::Read(value);
  Assert(bitsRead == MeasureRealHalf());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Real half value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureRealHalf(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return HalfFloatConverter::ToFloat(value);
}
Math::Vector2 BitStreamExtended::ReadReal2Half() const
{
  Assert(CanReadReal2Half());

  u16 value[2] = {0};
  Bits bitsRead = 0;
  bitsRead += BitStream::Read(value[0]);
  bitsRead += BitStream::Read(value[1]);
  Assert(bitsRead == MeasureReal2Half());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Real2 half value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureReal2Half(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return Math::Vector2(HalfFloatConverter::ToFloat(value[0]), HalfFloatConverter::ToFloat(value[1]));
}
Math::Vector3 BitStreamExtended::ReadReal3Half() const
{
  Assert(CanReadReal3Half());

  u16 value[3] = {0};
  Bits bitsRead = 0;
  bitsRead += BitStream::Read(value[0]);
  bitsRead += BitStream::Read(value[1]);
  bitsRead += BitStream::Read(value[2]);
  Assert(bitsRead == MeasureReal3Half());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Real3 half value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureReal3Half(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return Math::Vector3(HalfFloatConverter::ToFloat(value[0]),
                       HalfFloatConverter::ToFloat(value[1]),
                       HalfFloatConverter::ToFloat(value[2]));
}
Math::Vector4 BitStreamExtended::ReadReal4Half() const
{
  Assert(CanReadReal4Half());

  u16 value[4] = {0};
  Bits bitsRead = 0;
  bitsRead += BitStream::Read(value[0]);
  bitsRead += BitStream::Read(value[1]);
  bitsRead += BitStream::Read(value[2]);
  bitsRead += BitStream::Read(value[3]);
  Assert(bitsRead == MeasureReal4Half());

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Real4 half value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureReal4Half(),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return Math::Vector4(HalfFloatConverter::ToFloat(value[0]),
                       HalfFloatConverter::ToFloat(value[1]),
                       HalfFloatConverter::ToFloat(value[2]),
                       HalfFloatConverter::ToFloat(value[3]));
}

//
// BitStreamExtended::Read Quantized Operations
//

int BitStreamExtended::ReadIntegerQuantized(int minValue, int maxValue) const
{
  return ReadIntegerQuantized(minValue, maxValue, DefaultIntegralQuantum);
}
int BitStreamExtended::ReadIntegerQuantized(int minValue, int maxValue, int quantum) const
{
  Assert(CanReadIntegerQuantized(minValue, maxValue, quantum));

  int value = 0;
  Bits bitsRead = BitStream::ReadQuantized(value, minValue, maxValue, quantum);
  Assert(bitsRead == MeasureIntegerQuantized(minValue, maxValue, quantum));

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Integer quantized value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureIntegerQuantized(minValue, maxValue, quantum),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}

s64 BitStreamExtended::ReadDoubleIntegerQuantized(s64 minValue, s64 maxValue) const
{
  return ReadDoubleIntegerQuantized(minValue, maxValue, DefaultIntegralQuantum);
}
s64 BitStreamExtended::ReadDoubleIntegerQuantized(s64 minValue, s64 maxValue, s64 quantum) const
{
  Assert(CanReadDoubleIntegerQuantized(minValue, maxValue, quantum));

  s64 value = 0;
  Bits bitsRead = BitStream::ReadQuantized(value, minValue, maxValue, quantum);
  Assert(bitsRead == MeasureDoubleIntegerQuantized(minValue, maxValue, quantum));

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read DoubleInteger quantized value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureDoubleIntegerQuantized(minValue, maxValue, quantum),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}

Math::IntVector2 BitStreamExtended::ReadInteger2Quantized(const Math::IntVector2& minValue,
                                                          const Math::IntVector2& maxValue) const
{
  return ReadInteger2Quantized(minValue, maxValue, Math::IntVector2(DefaultIntegralQuantum, DefaultIntegralQuantum));
}
Math::IntVector2 BitStreamExtended::ReadInteger2Quantized(const Math::IntVector2& minValue,
                                                          const Math::IntVector2& maxValue,
                                                          const Math::IntVector2& quantum) const
{
  Assert(CanReadInteger2Quantized(minValue, maxValue, quantum));

  Math::IntVector2 value;
  Bits bitsRead = 0;
  bitsRead += BitStream::ReadQuantized(value.x, minValue.x, maxValue.x, quantum.x);
  bitsRead += BitStream::ReadQuantized(value.y, minValue.y, maxValue.y, quantum.y);
  Assert(bitsRead == MeasureInteger2Quantized(minValue, maxValue, quantum));

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Integer2 quantized value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureInteger2Quantized(minValue, maxValue, quantum),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}

Math::IntVector3 BitStreamExtended::ReadInteger3Quantized(const Math::IntVector3& minValue,
                                                          const Math::IntVector3& maxValue) const
{
  return ReadInteger3Quantized(
      minValue, maxValue, Math::IntVector3(DefaultIntegralQuantum, DefaultIntegralQuantum, DefaultIntegralQuantum));
}
Math::IntVector3 BitStreamExtended::ReadInteger3Quantized(const Math::IntVector3& minValue,
                                                          const Math::IntVector3& maxValue,
                                                          const Math::IntVector3& quantum) const
{
  Assert(CanReadInteger3Quantized(minValue, maxValue, quantum));

  Math::IntVector3 value;
  Bits bitsRead = 0;
  bitsRead += BitStream::ReadQuantized(value.x, minValue.x, maxValue.x, quantum.x);
  bitsRead += BitStream::ReadQuantized(value.y, minValue.y, maxValue.y, quantum.y);
  bitsRead += BitStream::ReadQuantized(value.z, minValue.z, maxValue.z, quantum.z);
  Assert(bitsRead == MeasureInteger3Quantized(minValue, maxValue, quantum));

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Integer3 quantized value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureInteger3Quantized(minValue, maxValue, quantum),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}

Math::IntVector4 BitStreamExtended::ReadInteger4Quantized(const Math::IntVector4& minValue,
                                                          const Math::IntVector4& maxValue) const
{
  return ReadInteger4Quantized(
      minValue,
      maxValue,
      Math::IntVector4(DefaultIntegralQuantum, DefaultIntegralQuantum, DefaultIntegralQuantum, DefaultIntegralQuantum));
}
Math::IntVector4 BitStreamExtended::ReadInteger4Quantized(const Math::IntVector4& minValue,
                                                          const Math::IntVector4& maxValue,
                                                          const Math::IntVector4& quantum) const
{
  Assert(CanReadInteger4Quantized(minValue, maxValue, quantum));

  Math::IntVector4 value;
  Bits bitsRead = 0;
  bitsRead += BitStream::ReadQuantized(value.x, minValue.x, maxValue.x, quantum.x);
  bitsRead += BitStream::ReadQuantized(value.y, minValue.y, maxValue.y, quantum.y);
  bitsRead += BitStream::ReadQuantized(value.z, minValue.z, maxValue.z, quantum.z);
  bitsRead += BitStream::ReadQuantized(value.w, minValue.w, maxValue.w, quantum.w);
  Assert(bitsRead == MeasureInteger4Quantized(minValue, maxValue, quantum));

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Integer4 quantized value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureInteger4Quantized(minValue, maxValue, quantum),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}

float BitStreamExtended::ReadRealQuantized(float minValue, float maxValue) const
{
  return ReadRealQuantized(minValue, maxValue, DefaultFloatingPointQuantum);
}
float BitStreamExtended::ReadRealQuantized(float minValue, float maxValue, float quantum) const
{
  Assert(CanReadRealQuantized(minValue, maxValue, quantum));

  float value = 0;
  Bits bitsRead = BitStream::ReadQuantized(value, minValue, maxValue, quantum);
  Assert(bitsRead == MeasureRealQuantized(minValue, maxValue, quantum));

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Real quantized value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureRealQuantized(minValue, maxValue, quantum),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}

double BitStreamExtended::ReadDoubleRealQuantized(double minValue, double maxValue) const
{
  return ReadDoubleRealQuantized(minValue, maxValue, DefaultFloatingPointQuantum);
}
double BitStreamExtended::ReadDoubleRealQuantized(double minValue, double maxValue, double quantum) const
{
  Assert(CanReadDoubleRealQuantized(minValue, maxValue, quantum));

  double value = 0;
  Bits bitsRead = BitStream::ReadQuantized(value, minValue, maxValue, quantum);
  Assert(bitsRead == MeasureDoubleRealQuantized(minValue, maxValue, quantum));

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read DoubleReal quantized value (%u bits) "
                                     "from BitStream (%u / %u bits read/written)",
                                     MeasureDoubleRealQuantized(minValue, maxValue, quantum),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}

Math::Vector2 BitStreamExtended::ReadReal2Quantized(const Math::Vector2& minValue, const Math::Vector2& maxValue) const
{
  return ReadReal2Quantized(minValue, maxValue, Math::Vector2(DefaultFloatingPointQuantum));
}
Math::Vector2 BitStreamExtended::ReadReal2Quantized(const Math::Vector2& minValue,
                                                    const Math::Vector2& maxValue,
                                                    const Math::Vector2& quantum) const
{
  Assert(CanReadReal2Quantized(minValue, maxValue, quantum));

  Math::Vector2 value;
  Bits bitsRead = 0;
  bitsRead += BitStream::ReadQuantized(value.x, minValue.x, maxValue.x, quantum.x);
  bitsRead += BitStream::ReadQuantized(value.y, minValue.y, maxValue.y, quantum.y);
  Assert(bitsRead == MeasureReal2Quantized(minValue, maxValue, quantum));

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Real2 quantized value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureReal2Quantized(minValue, maxValue, quantum),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}

Math::Vector3 BitStreamExtended::ReadReal3Quantized(const Math::Vector3& minValue, const Math::Vector3& maxValue) const
{
  return ReadReal3Quantized(minValue, maxValue, Math::Vector3(DefaultFloatingPointQuantum));
}
Math::Vector3 BitStreamExtended::ReadReal3Quantized(const Math::Vector3& minValue,
                                                    const Math::Vector3& maxValue,
                                                    const Math::Vector3& quantum) const
{
  Assert(CanReadReal3Quantized(minValue, maxValue, quantum));

  Math::Vector3 value;
  Bits bitsRead = 0;
  bitsRead += BitStream::ReadQuantized(value.x, minValue.x, maxValue.x, quantum.x);
  bitsRead += BitStream::ReadQuantized(value.y, minValue.y, maxValue.y, quantum.y);
  bitsRead += BitStream::ReadQuantized(value.z, minValue.z, maxValue.z, quantum.z);
  Assert(bitsRead == MeasureReal3Quantized(minValue, maxValue, quantum));

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Real3 quantized value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureReal3Quantized(minValue, maxValue, quantum),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}

Math::Vector4 BitStreamExtended::ReadReal4Quantized(const Math::Vector4& minValue, const Math::Vector4& maxValue) const
{
  return ReadReal4Quantized(minValue, maxValue, Math::Vector4(DefaultFloatingPointQuantum));
}
Math::Vector4 BitStreamExtended::ReadReal4Quantized(const Math::Vector4& minValue,
                                                    const Math::Vector4& maxValue,
                                                    const Math::Vector4& quantum) const
{
  Assert(CanReadReal4Quantized(minValue, maxValue, quantum));

  Math::Vector4 value;
  Bits bitsRead = 0;
  bitsRead += BitStream::ReadQuantized(value.x, minValue.x, maxValue.x, quantum.x);
  bitsRead += BitStream::ReadQuantized(value.y, minValue.y, maxValue.y, quantum.y);
  bitsRead += BitStream::ReadQuantized(value.z, minValue.z, maxValue.z, quantum.z);
  bitsRead += BitStream::ReadQuantized(value.w, minValue.w, maxValue.w, quantum.w);
  Assert(bitsRead == MeasureReal4Quantized(minValue, maxValue, quantum));

  if (!bitsRead)
  {
    DoNotifyException("BitStream",
                      String::Format("Unable to read Real4 quantized value (%u bits) from "
                                     "BitStream (%u / %u bits read/written)",
                                     MeasureReal4Quantized(minValue, maxValue, quantum),
                                     BitStream::GetBitsRead(),
                                     BitStream::GetBitsWritten()));
  }

  return value;
}

//
// Event Operations
//

bool BitStreamExtended::WriteEvent(Event* event)
{
  // Null event?
  if (!event)
  {
    Assert(false);
    return false;
  }

  // Get event type
  BoundType* eventType = RaverieVirtualTypeId(event);
  if (!eventType)
  {
    Assert(false);
    return false;
  }

  // Write meta type ID
  // (TODO: This may need to change to work cross-platform since compile time
  // type IDs rely on template instantiation order) (Safest way is to send
  // strings of the type name, and use a lookup table to optimize)
  BitStream::Write(eventType->Name); // TODO: Refactor this to use ItemCache

  // Write event ID
  // (TODO: Optimize this with a lookup table)
  BitStream::Write(event->EventId);

  //
  // Write Properties
  //

  // For all properties
  MemberRange<Property> properties = eventType->GetProperties(Members::InheritedInstanceExtension);
  forRange (Property* property, properties)
  {
    // Is the script source property?
    if (property->Name == cScriptSource)
      continue; // Skip property (we don't need to serialize this)

    // Is the event ID property?
    if (property->Name == cEventId)
      continue; // Skip property (we manually serialize this above)

    // Is a net peer ID property?
    if (property->HasAttribute(PropertyAttributes::cNetPeerId))
      continue; // Skip property (will be set automatically by NetPeer after
                // receiving the event)

    // Is a net property?
    if (property->HasAttribute(PropertyAttributes::cNetProperty))
    {
      // (Should be a valid net property type)
      Assert(IsValidNetPropertyType(property->PropertyType));

      // Is Cog type?
      if (property->PropertyType == RaverieTypeId(Cog))
      {
        // Get cog as net object ID
        // (Using ReplicaId to take advantage of WriteQuantized)
        ReplicaId netObjectId = GetNetPropertyCogAsNetObjectId(property, event);

        // Write net object ID
        BitStream::Write(netObjectId);
      }
      // Is CogPath type?
      else if (property->PropertyType == RaverieTypeId(CogPath))
      {
        // Get cog path value
        Any cogPathAny = property->GetValue(event);
        if (!cogPathAny.IsHoldingValue()) // Unable?
          DoNotifyError("BitStream",
                        "Error getting CogPath NetProperty - Unable to get "
                        "property instance value");
        CogPath* cogPath = cogPathAny.Get<CogPath*>();

        // Get cog path string
        String cogPathString = cogPath ? cogPath->GetPath() : String();

        // Write cog path string
        BitStream::Write(cogPathString);
      }
      // Is any other type?
      else
      {
        // Get any value
        Any anyValue = property->GetValue(event);

        // Attempt to convert basic any value to variant value
        Variant variantValue = ConvertBasicAnyToVariant(anyValue);
        if (variantValue.IsEmpty()) // Unable? (The any's stored type is not a
                                    // basic native type?)
        {
          // Assign the any value itself to the variant value
          // (Some property types like enums, resources, and bitstream are
          // expected to be wrapped in an any this way)
          variantValue = anyValue;
        }

        // Is serialization supported for the underlying type?
        if (BitStreamCanSerializeType(anyValue.StoredType))
        {
          // Write variant
          BitStream::Write(variantValue);
        }
        else
          DoNotifyError("BitStream",
                        "Unable to serialize property - Serialization is not "
                        "supported by the property type");
      }
    }
  }

  // Success
  return true;
}
HandleOf<Event> BitStreamExtended::ReadEvent(GameSession* gameSession) const
{
  // Get NetPeer (if available)
  NetPeer* netPeer = gameSession->has(NetPeer);

  // Read event type name
  String eventTypeName;
  if (!BitStream::Read(eventTypeName)) // Unable?
  {
    Assert(false);
    return nullptr;
  }

  // Get event type
  BoundType* eventType = MetaDatabase::GetInstance()->FindType(eventTypeName);
  if (!eventType)
  {
    Assert(false);
    return nullptr;
  }

  // Create event
  HandleOf<Event> eventHandle = ExecutableState::CallingState->AllocateDefaultConstructed<Event>(eventType);
  Event* event = eventHandle;
  if (!event) // Unable?
  {
    Assert(false);
    return nullptr;
  }

  // Read event ID
  if (!BitStream::Read(event->EventId)) // Unable?
  {
    Assert(false);
    return nullptr;
  }

  //
  // Read Properties
  //

  // For all properties
  MemberRange<Property> properties = eventType->GetProperties(Members::InheritedInstanceExtension);
  forRange (Property* property, properties)
  {
    // Is the script source property?
    if (property->Name == cScriptSource)
      continue; // Skip property (we don't need to serialize this)

    // Is the event ID property?
    if (property->Name == cEventId)
      continue; // Skip property (we manually serialize this above)

    // Is a net peer ID property?
    if (property->HasAttribute(PropertyAttributes::cNetPeerId))
      continue; // Skip property (will be set automatically by NetPeer after
                // receiving the event)

    // Is a net property?
    if (property->HasAttribute(PropertyAttributes::cNetProperty))
    {
      // (Should be a valid net property type)
      Assert(IsValidNetPropertyType(property->PropertyType));

      // Is Cog type?
      if (property->PropertyType == RaverieTypeId(Cog))
      {
        // NetPeer not provided?
        if (!netPeer)
        {
          DoNotifyError("BitStream",
                        "Unable to serialize [NetProperty] Cog property - "
                        "GameSession must have a NetPeer component");
          return nullptr;
        }

        // Read net object ID
        // (Using ReplicaId to take advantage of ReadQuantized)
        ReplicaId netObjectId;
        if (!BitStream::Read(netObjectId)) // Unable?
        {
          Assert(false);
          return nullptr;
        }

        // Set cog as net object ID
        SetNetPropertyCogAsNetObjectId(property, event, netPeer, netObjectId.value());
      }
      // Is CogPath type?
      else if (property->PropertyType == RaverieTypeId(CogPath))
      {
        // Get cog path value
        Any cogPathAny = property->GetValue(event);
        if (!cogPathAny.IsHoldingValue()) // Unable?
          DoNotifyError("BitStream",
                        "Error getting CogPath NetProperty - Unable to get "
                        "property instance value");
        CogPath* cogPath = cogPathAny.Get<CogPath*>();

        // Read cog path string
        String cogPathString;
        if (!BitStream::Read(cogPathString)) // Unable?
        {
          Assert(false);
          return nullptr;
        }

        // Set cog path string
        if (cogPath)
          cogPath->SetPath(cogPathString);
      }
      // Is any other type?
      else
      {
        // Get any value
        Any anyValue = property->GetValue(event);

        // Attempt to convert basic any value to variant value
        Variant variantValue = ConvertBasicAnyToVariant(anyValue);
        if (variantValue.IsEmpty()) // Unable? (The any's stored type is not a
                                    // basic native type?)
        {
          // Assign the any value itself to the variant value
          // (Some property types like enums, resources, and bitstream are
          // expected to be wrapped in an any this way)
          variantValue = anyValue;
        }

        // Is serialization supported for the underlying type?
        if (BitStreamCanSerializeType(anyValue.StoredType))
        {
          // Read variant
          if (!BitStream::Read(variantValue)) // Unable?
          {
            Assert(false);
            return nullptr;
          }

          // Attempt to convert basic variant value to any value
          Any anyValue = ConvertBasicVariantToAny(variantValue);
          if (!anyValue.IsHoldingValue()) // Unable? (The variant's stored type
                                          // is not a basic native type?)
          {
            // Get the any value itself from the variant value
            // (Some property types like enums, resources, and bitstream are
            // expected to be wrapped in an any this way)
            anyValue = variantValue.GetOrError<Any>();
          }

          // Set the property value
          property->SetValue(event, anyValue);
        }
        else
          DoNotifyError("BitStream",
                        "Unable to serialize property - Serialization is not "
                        "supported by the property type");
      }
    }
  }

  // Success
  return event;
}

//
// Serialize template specializations
//

/// Serializes a Variant.
/// Returns the number of bits serialized if successful, else 0.
template <>
Bits Serialize<Variant>(SerializeDirection::Enum direction, BitStream& bitStream, Variant& value)
{
  // Variants serialized through the standard BitStream Read/Write/Serialize
  // functions, etc. must contain known types that we support (which can be
  // queried via BitStreamCanSerializeAnyType)
  return SerializeKnownExtendedVariant(direction, bitStream, value);
}

/// Serializes a BitStreamExtended.
/// Returns the number of bits serialized if successful, else 0.
template <>
Bits Serialize<BitStreamExtended>(SerializeDirection::Enum direction, BitStream& bitStream, BitStreamExtended& value)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Get bitstream extended size
    Bits valueBits = value.GetBitsWritten();

    // Bitstream extended is too large to write?
    if (valueBits > BitStreamExtendedLargeSizeBitsMax)
    {
      DoNotifyError("BitStream",
                    String::Format("BitStream is too large to write at %u bytes. "
                                   "BitStreams must be less than %u bytes",
                                   BITS_TO_BYTES(valueBits),
                                   BITS_TO_BYTES(POW2(BitStreamExtendedLargeSizeBits))));
      return 0;
    }

    // Is a small bitstream?
    if (valueBits < BitStreamExtendedSmallSizeIsLargeValue)
    {
      // Write small bitstream extended size
      bitStream.WriteQuantized(valueBits, BitStreamExtendedSmallSizeBitsMin, BitStreamExtendedSmallSizeBitsMax);
    }
    // Is a large bitstream?
    else
    {
      // Write small bitstream extended size
      // But instead write the 'Is Large' Value
      bitStream.WriteQuantized(
          BitStreamExtendedSmallSizeIsLargeValue, BitStreamExtendedSmallSizeBitsMin, BitStreamExtendedSmallSizeBitsMax);

      // Write large bitstream extended size
      bitStream.WriteQuantized(valueBits, BitStreamExtendedLargeSizeBitsMin, BitStreamExtendedLargeSizeBitsMax);
    }

    // Write bitstream extended data
    bitStream.AppendAll(value);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read small bitstream extended size
    Bits valueBits = 0;
    ReturnIf(!bitStream.ReadQuantized(valueBits, BitStreamExtendedSmallSizeBitsMin, BitStreamExtendedSmallSizeBitsMax),
             0,
             "");

    // 'Is Large' Value?
    if (valueBits == BitStreamExtendedSmallSizeIsLargeValue)
    {
      // Read large bitstream extended size
      ReturnIf(
          !bitStream.ReadQuantized(valueBits, BitStreamExtendedLargeSizeBitsMin, BitStreamExtendedLargeSizeBitsMax),
          0,
          "");
    }

    // (There should be enough bits available to be read)
    Assert(bitStream.GetBitsUnread() >= valueBits);

    // Clear current bitstream extended data (if any)
    value.Clear(false);

    // Read bitstream extended data
    if (value.Append(bitStream, valueBits) != valueBits) // Unable?
    {
      DoNotifyError("BitStream", "Error reading BitStream data.");
      return 0;
    }
    Assert(value.GetBitsWritten() == valueBits);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

} // namespace Raverie
