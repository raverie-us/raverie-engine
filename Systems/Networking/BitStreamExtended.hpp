///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

//---------------------------------------------------------------------------------//
//                      BitStreamExtended Configuration                            //
//---------------------------------------------------------------------------------//

/// BitStreamExtended "small" size bits.
/// Determines the maximum bit size (bits written) a "small" BitStreamExtended may be serialized with.
/// If a BitStreamExtended's bit size is larger than this "small" maximum, we will use the "large" size instead.
/// (We use the last possible value as a bit flag to denote whether or not the BitStreamExtended is small or large. The idea is most BitStreams
/// will be small and thus it's preferable to use fewer bits declaring when it's size. But for extensibility purposes we allow larger sizes.)
#define BITSTREAM_EXTENDED_SMALL_SIZE_BITS 12
StaticAssertWithinRange(Range17, BITSTREAM_EXTENDED_SMALL_SIZE_BITS, 1, UINTMAX_BITS);

/// BitStreamExtended "large" size bits.
/// Determines the maximum bit size (bits written) a BitStreamExtended may be serialized with.
#define BITSTREAM_EXTENDED_LARGE_SIZE_BITS BITS_NEEDED_TO_REPRESENT(BYTES_TO_BITS(POW2(BITSTREAM_MAX_SIZE_BITS)) - 1)
StaticAssertWithinRange(Range18, BITSTREAM_EXTENDED_LARGE_SIZE_BITS, 1, UINTMAX_BITS);

namespace Zero
{

//---------------------------------------------------------------------------------//
//                              BitStreamExtended                                  //
//---------------------------------------------------------------------------------//

/// BitStreamExtended size bits.
static const Bits BitStreamExtendedSmallSizeBits = BITSTREAM_EXTENDED_SMALL_SIZE_BITS;
static const Bits BitStreamExtendedLargeSizeBits = BITSTREAM_EXTENDED_LARGE_SIZE_BITS;

/// BitStreamExtended size bits range.
static const Bits BitStreamExtendedSmallSizeBitsMin      = Bits(0);
static const Bits BitStreamExtendedSmallSizeBitsMax      = Bits(POW2(BitStreamExtendedSmallSizeBits) - 1);
static const Bits BitStreamExtendedSmallSizeIsLargeValue = BitStreamExtendedSmallSizeBitsMax;

static const Bits BitStreamExtendedLargeSizeBitsMin = Bits(0);
static const Bits BitStreamExtendedLargeSizeBitsMax = Bits(POW2(BitStreamExtendedLargeSizeBits) - 1);

/// Convenient, sliceable wrapper around BitStream.
/// Provides explicit serialization for commonly used types.
class BitStreamExtended : public BitStream
{
  // Default Quantum Values
  static const int   DefaultIntegralQuantum;
  static const float DefaultFloatingPointQuantum;

public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructors.
  BitStreamExtended();
  BitStreamExtended(const BitStream& rhs);

  /// Destructor.
  ~BitStreamExtended();

  /// Copy Assignment Operator.
  BitStreamExtended& operator =(const BitStream& rhs);

  //
  // Measure Operations
  //

  static Bits MeasureBoolean();

  static Bits MeasureInteger();
  static Bits MeasureDoubleInteger();
  static Bits MeasureInteger2();
  static Bits MeasureInteger3();
  static Bits MeasureInteger4();

  static Bits MeasureReal();
  static Bits MeasureDoubleReal();
  static Bits MeasureReal2();
  static Bits MeasureReal3();
  static Bits MeasureReal4();
  static Bits MeasureQuaternion();

  static Bits MeasureString();
  static Bits MeasureString(StringParam value);

  //
  // Measure Half Operations
  //

  static Bits MeasureRealHalf();
  static Bits MeasureReal2Half();
  static Bits MeasureReal3Half();
  static Bits MeasureReal4Half();

  //
  // Measure Quantized Operations
  //

  static Bits MeasureIntegerQuantized(int minValue, int maxValue);
  static Bits MeasureIntegerQuantized(int minValue, int maxValue, int quantum);

  static Bits MeasureDoubleIntegerQuantized(s64 minValue, s64 maxValue);
  static Bits MeasureDoubleIntegerQuantized(s64 minValue, s64 maxValue, s64 quantum);

  static Bits MeasureInteger2Quantized(const Math::IntVector2& minValue, const Math::IntVector2& maxValue);
  static Bits MeasureInteger2Quantized(const Math::IntVector2& minValue, const Math::IntVector2& maxValue, const Math::IntVector2& quantum);

  static Bits MeasureInteger3Quantized(const Math::IntVector3& minValue, const Math::IntVector3& maxValue);
  static Bits MeasureInteger3Quantized(const Math::IntVector3& minValue, const Math::IntVector3& maxValue, const Math::IntVector3& quantum);

  static Bits MeasureInteger4Quantized(const Math::IntVector4& minValue, const Math::IntVector4& maxValue);
  static Bits MeasureInteger4Quantized(const Math::IntVector4& minValue, const Math::IntVector4& maxValue, const Math::IntVector4& quantum);

  static Bits MeasureRealQuantized(float minValue, float maxValue);
  static Bits MeasureRealQuantized(float minValue, float maxValue, float quantum);

  static Bits MeasureDoubleRealQuantized(double minValue, double maxValue);
  static Bits MeasureDoubleRealQuantized(double minValue, double maxValue, double quantum);

  static Bits MeasureReal2Quantized(const Math::Vector2& minValue, const Math::Vector2& maxValue);
  static Bits MeasureReal2Quantized(const Math::Vector2& minValue, const Math::Vector2& maxValue, const Math::Vector2& quantum);

  static Bits MeasureReal3Quantized(const Math::Vector3& minValue, const Math::Vector3& maxValue);
  static Bits MeasureReal3Quantized(const Math::Vector3& minValue, const Math::Vector3& maxValue, const Math::Vector3& quantum);

  static Bits MeasureReal4Quantized(const Math::Vector4& minValue, const Math::Vector4& maxValue);
  static Bits MeasureReal4Quantized(const Math::Vector4& minValue, const Math::Vector4& maxValue, const Math::Vector4& quantum);

  //
  // Write Operations
  //

  void WriteBoolean(bool value);

  void WriteInteger(int value);
  void WriteDoubleInteger(s64 value);
  void WriteInteger2(const Math::IntVector2& value);
  void WriteInteger3(const Math::IntVector3& value);
  void WriteInteger4(const Math::IntVector4& value);

  void WriteReal(float value);
  void WriteDoubleReal(double value);
  void WriteReal2(const Math::Vector2& value);
  void WriteReal3(const Math::Vector3& value);
  void WriteReal4(const Math::Vector4& value);
  void WriteQuaternion(const Math::Quaternion& value);

  void WriteString(StringParam value);

  //
  // Write Half Operations
  //

  void WriteRealHalf(float value);
  void WriteReal2Half(const Math::Vector2& value);
  void WriteReal3Half(const Math::Vector3& value);
  void WriteReal4Half(const Math::Vector4& value);

  //
  // Write Quantized Operations
  //

  void WriteIntegerQuantized(int value, int minValue, int maxValue);
  void WriteIntegerQuantized(int value, int minValue, int maxValue, int quantum);

  void WriteDoubleIntegerQuantized(s64 value, s64 minValue, s64 maxValue);
  void WriteDoubleIntegerQuantized(s64 value, s64 minValue, s64 maxValue, s64 quantum);

  void WriteInteger2Quantized(const Math::IntVector2& value, const Math::IntVector2& minValue, const Math::IntVector2& maxValue);
  void WriteInteger2Quantized(const Math::IntVector2& value, const Math::IntVector2& minValue, const Math::IntVector2& maxValue, const Math::IntVector2& quantum);

  void WriteInteger3Quantized(const Math::IntVector3& value, const Math::IntVector3& minValue, const Math::IntVector3& maxValue);
  void WriteInteger3Quantized(const Math::IntVector3& value, const Math::IntVector3& minValue, const Math::IntVector3& maxValue, const Math::IntVector3& quantum);

  void WriteInteger4Quantized(const Math::IntVector4& value, const Math::IntVector4& minValue, const Math::IntVector4& maxValue);
  void WriteInteger4Quantized(const Math::IntVector4& value, const Math::IntVector4& minValue, const Math::IntVector4& maxValue, const Math::IntVector4& quantum);

  void WriteRealQuantized(float value, float minValue, float maxValue);
  void WriteRealQuantized(float value, float minValue, float maxValue, float quantum);

  void WriteDoubleRealQuantized(double value, double minValue, double maxValue);
  void WriteDoubleRealQuantized(double value, double minValue, double maxValue, double quantum);

  void WriteReal2Quantized(const Math::Vector2& value, const Math::Vector2& minValue, const Math::Vector2& maxValue);
  void WriteReal2Quantized(const Math::Vector2& value, const Math::Vector2& minValue, const Math::Vector2& maxValue, const Math::Vector2& quantum);

  void WriteReal3Quantized(const Math::Vector3& value, const Math::Vector3& minValue, const Math::Vector3& maxValue);
  void WriteReal3Quantized(const Math::Vector3& value, const Math::Vector3& minValue, const Math::Vector3& maxValue, const Math::Vector3& quantum);

  void WriteReal4Quantized(const Math::Vector4& value, const Math::Vector4& minValue, const Math::Vector4& maxValue);
  void WriteReal4Quantized(const Math::Vector4& value, const Math::Vector4& minValue, const Math::Vector4& maxValue, const Math::Vector4& quantum);

  //
  // Can-Read Operations
  //

  bool CanReadBoolean() const;

  bool CanReadInteger() const;
  bool CanReadDoubleInteger() const;
  bool CanReadInteger2() const;
  bool CanReadInteger3() const;
  bool CanReadInteger4() const;

  bool CanReadReal() const;
  bool CanReadDoubleReal() const;
  bool CanReadReal2() const;
  bool CanReadReal3() const;
  bool CanReadReal4() const;
  bool CanReadQuaternion() const;

  bool CanReadString() const;

  //
  // Can-Read Half Operations
  //

  bool CanReadRealHalf() const;
  bool CanReadReal2Half() const;
  bool CanReadReal3Half() const;
  bool CanReadReal4Half() const;

  //
  // Can-Read Quantized Operations
  //

  bool CanReadIntegerQuantized(int minValue, int maxValue) const;
  bool CanReadIntegerQuantized(int minValue, int maxValue, int quantum) const;

  bool CanReadDoubleIntegerQuantized(s64 minValue, s64 maxValue) const;
  bool CanReadDoubleIntegerQuantized(s64 minValue, s64 maxValue, s64 quantum) const;

  bool CanReadInteger2Quantized(const Math::IntVector2& minValue, const Math::IntVector2& maxValue) const;
  bool CanReadInteger2Quantized(const Math::IntVector2& minValue, const Math::IntVector2& maxValue, const Math::IntVector2& quantum) const;

  bool CanReadInteger3Quantized(const Math::IntVector3& minValue, const Math::IntVector3& maxValue) const;
  bool CanReadInteger3Quantized(const Math::IntVector3& minValue, const Math::IntVector3& maxValue, const Math::IntVector3& quantum) const;

  bool CanReadInteger4Quantized(const Math::IntVector4& minValue, const Math::IntVector4& maxValue) const;
  bool CanReadInteger4Quantized(const Math::IntVector4& minValue, const Math::IntVector4& maxValue, const Math::IntVector4& quantum) const;

  bool CanReadRealQuantized(float minValue, float maxValue) const;
  bool CanReadRealQuantized(float minValue, float maxValue, float quantum) const;

  bool CanReadDoubleRealQuantized(double minValue, double maxValue) const;
  bool CanReadDoubleRealQuantized(double minValue, double maxValue, double quantum) const;

  bool CanReadReal2Quantized(const Math::Vector2& minValue, const Math::Vector2& maxValue) const;
  bool CanReadReal2Quantized(const Math::Vector2& minValue, const Math::Vector2& maxValue, const Math::Vector2& quantum) const;

  bool CanReadReal3Quantized(const Math::Vector3& minValue, const Math::Vector3& maxValue) const;
  bool CanReadReal3Quantized(const Math::Vector3& minValue, const Math::Vector3& maxValue, const Math::Vector3& quantum) const;

  bool CanReadReal4Quantized(const Math::Vector4& minValue, const Math::Vector4& maxValue) const;
  bool CanReadReal4Quantized(const Math::Vector4& minValue, const Math::Vector4& maxValue, const Math::Vector4& quantum) const;

  //
  // Read Operations
  //

  bool ReadBoolean() const;

  int ReadInteger() const;
  s64 ReadDoubleInteger() const;
  Math::IntVector2 ReadInteger2() const;
  Math::IntVector3 ReadInteger3() const;
  Math::IntVector4 ReadInteger4() const;

  float ReadReal() const;
  double ReadDoubleReal() const;
  Math::Vector2 ReadReal2() const;
  Math::Vector3 ReadReal3() const;
  Math::Vector4 ReadReal4() const;
  Math::Quaternion ReadQuaternion() const;

  String ReadString() const;

  //
  // Read Half Operations
  //

  float ReadRealHalf() const;
  Math::Vector2 ReadReal2Half() const;
  Math::Vector3 ReadReal3Half() const;
  Math::Vector4 ReadReal4Half() const;

  //
  // Read Quantized Operations
  //

  int ReadIntegerQuantized(int minValue, int maxValue) const;
  int ReadIntegerQuantized(int minValue, int maxValue, int quantum) const;

  s64 ReadDoubleIntegerQuantized(s64 minValue, s64 maxValue) const;
  s64 ReadDoubleIntegerQuantized(s64 minValue, s64 maxValue, s64 quantum) const;

  Math::IntVector2 ReadInteger2Quantized(const Math::IntVector2& minValue, const Math::IntVector2& maxValue) const;
  Math::IntVector2 ReadInteger2Quantized(const Math::IntVector2& minValue, const Math::IntVector2& maxValue, const Math::IntVector2& quantum) const;

  Math::IntVector3 ReadInteger3Quantized(const Math::IntVector3& minValue, const Math::IntVector3& maxValue) const;
  Math::IntVector3 ReadInteger3Quantized(const Math::IntVector3& minValue, const Math::IntVector3& maxValue, const Math::IntVector3& quantum) const;

  Math::IntVector4 ReadInteger4Quantized(const Math::IntVector4& minValue, const Math::IntVector4& maxValue) const;
  Math::IntVector4 ReadInteger4Quantized(const Math::IntVector4& minValue, const Math::IntVector4& maxValue, const Math::IntVector4& quantum) const;

  float ReadRealQuantized(float minValue, float maxValue) const;
  float ReadRealQuantized(float minValue, float maxValue, float quantum) const;

  double ReadDoubleRealQuantized(double minValue, double maxValue) const;
  double ReadDoubleRealQuantized(double minValue, double maxValue, double quantum) const;

  Math::Vector2 ReadReal2Quantized(const Math::Vector2& minValue, const Math::Vector2& maxValue) const;
  Math::Vector2 ReadReal2Quantized(const Math::Vector2& minValue, const Math::Vector2& maxValue, const Math::Vector2& quantum) const;

  Math::Vector3 ReadReal3Quantized(const Math::Vector3& minValue, const Math::Vector3& maxValue) const;
  Math::Vector3 ReadReal3Quantized(const Math::Vector3& minValue, const Math::Vector3& maxValue, const Math::Vector3& quantum) const;

  Math::Vector4 ReadReal4Quantized(const Math::Vector4& minValue, const Math::Vector4& maxValue) const;
  Math::Vector4 ReadReal4Quantized(const Math::Vector4& minValue, const Math::Vector4& maxValue, const Math::Vector4& quantum) const;

  //
  // Event Operations
  //

  // TODO: Make this more accurate
  /// Returns true if an event can be read from the bitstream, else false.
  bool CanReadEvent() const { return GetBitsUnread(); }

  /// Writes an event to the bitstream.
  /// Returns true if successful, else false.
  bool WriteEvent(Event* event);

  /// Reads an event from the bitstream.
  /// Returns a new event if successful, else nullptr.
  HandleOf<Event> ReadEvent(GameSession* gameSession) const;
};

//---------------------------------------------------------------------------------//
//                             Serialize Functions                                 //
//---------------------------------------------------------------------------------//

//
// Variant
//

/// Serializes a non-empty Variant.
/// Will not serialize the stored type ID, the deserializer must default construct the variant as the expected type before deserializing.
/// Returns the number of bits serialized if successful, else 0.
inline Bits SerializeKnownExtendedVariant(SerializeDirection::Enum direction, BitStream& bitStream, Variant& value)
{
  // Get starting bits count
  Bits startBits = bitStream.GetBitsSerialized(direction);

  // Variant is a basic native type?
  if(value.GetNativeType()->mIsBasicNativeType)
  {
    // Serialize basic native type
    return SerializeKnownBasicVariant(direction, bitStream, value);
  }
  // Variant is an Any type?
  else if(value.Is<Any>())
  {
    // Determine the type contained in the Any
    Any& anyValue = value.GetOrError<Any>();

    // Any is an enum type?
    if(anyValue.StoredType->IsEnum())
    {
      // Get enum bound type
      BoundType* enumBoundType = Type::GetBoundType(anyValue.StoredType);
      if(!enumBoundType) // Unable?
      {
        Assert(false);
        return 0;
      }

      // Get enum values count
      uint enumValueCount = enumBoundType->AllProperties.Size();

      // Serialize enum in range defined by possible enum values
      uint& enumValue    = *(uint*)anyValue.GetData();
      uint  enumValueMin = 0;
      uint  enumValueMax = (enumValueCount == 0) ? 0 : (enumValueCount - 1);

      return bitStream.SerializeQuantized(direction, enumValue, enumValueMin, enumValueMax);
    }
    // Any is an resource type?
    else if(anyValue.StoredType->IsA(ZilchTypeId(Resource)))
    {
      Bits startBits = bitStream.GetBitsSerialized(direction);

      // Write?
      if(direction == SerializeDirection::Write)
      {
        // Get resource
        Resource* resource = anyValue.Get<Resource*>();

        // Get resource ID
        u64 resourceId = (u64)(resource ? resource->mResourceId : ResourceId(0));

        // Write resource ID
        if(!bitStream.Write(resourceId)) // Unable?
          return 0;
      }
      // Read?
      else
      {
        // Read resource ID
        u64 resourceId = 0;
        if(!bitStream.Read(resourceId)) // Unable?
          return 0;

        // Resource specified?
        if(resourceId)
        {
          // Find resource
          Resource* resource = Z::gResources->GetResource(resourceId);

          // Unable to find resource?
          if(!resource)
          {
            // Set default resource
            DoNotifyWarning("BitStream", "Unable to find resource specified remotely, using default resource instead");
            anyValue.DefaultConstruct(anyValue.StoredType);
          }
          // Found resource?
          else
          {
            // Set found resource
            anyValue = resource;
          }
        }
        // Resource not specified?
        else
        {
          // Set null resource
          anyValue = (Resource*)nullptr;
        }
      }

      return bitStream.GetBitsSerialized(direction) - startBits;
    }
    // Any is a bitstream extended type?
    else if(anyValue.StoredType == ZilchTypeId(BitStreamExtended))
    {
      // Get bitstream extended
      BitStreamExtended* bitStreamExtended = anyValue.Get<BitStreamExtended*>();

      // Serialize bitstream extended
      return bitStream.Serialize(direction, *bitStreamExtended);
    }
    // Any is another type?
    else
    {
      // Unsupported type
      DoNotifyError("BitStream", "Unable to serialize Variant Any value");
      return 0;
    }
  }
  // Variant is another type?
  else
  {
    // Unsupported type
    DoNotifyError("BitStream", "Unable to serialize Variant value");
    return 0;
  }
}

/// Returns true if BitStream can serialize the specified zilch type, else false.
inline bool BitStreamCanSerializeType(Type* zilchType)
{
  // Get basic native type (or null)
  NativeType* basicNativeType = ZilchTypeToBasicNativeType(zilchType);

  //    Is a basic any type?
  // OR Is an enum?
  // OR Is a resource?
  // OR Is a bitstream extended?
  return (basicNativeType != nullptr)
      || (zilchType->IsEnum())
      || (zilchType->IsA(ZilchTypeId(Resource)))
      || (zilchType == ZilchTypeId(BitStreamExtended));
}

/// Serializes a Variant.
/// Returns the number of bits serialized if successful, else 0.
inline Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, Variant& value)
{
  // Variants serialized through the standard BitStream Read/Write/Serialize functions, etc.
  // must contain known types that we support (which can be queried via BitStreamCanSerializeAnyType)
  return SerializeKnownExtendedVariant(direction, bitStream, value);
}

/// Serializes a BitStreamExtended.
/// Returns the number of bits serialized if successful, else 0.
inline Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, BitStreamExtended& value)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Get bitstream extended size
    Bits valueBits = value.GetBitsWritten();

    // Bitstream extended is too large to write?
    if(valueBits > BitStreamExtendedLargeSizeBitsMax)
    {
      DoNotifyError("BitStream", String::Format("BitStream is too large to write at %u bytes. BitStreams must be less than %u bytes", BITS_TO_BYTES(valueBits), BITS_TO_BYTES(POW2(BitStreamExtendedLargeSizeBits))));
      return 0;
    }

    // Is a small bitstream?
    if(valueBits < BitStreamExtendedSmallSizeIsLargeValue)
    {
      // Write small bitstream extended size
      bitStream.WriteQuantized(valueBits, BitStreamExtendedSmallSizeBitsMin, BitStreamExtendedSmallSizeBitsMax);
    }
    // Is a large bitstream?
    else
    {
      // Write small bitstream extended size
      // But instead write the 'Is Large' Value
      bitStream.WriteQuantized(BitStreamExtendedSmallSizeIsLargeValue, BitStreamExtendedSmallSizeBitsMin, BitStreamExtendedSmallSizeBitsMax);

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
    ReturnIf(!bitStream.ReadQuantized(valueBits, BitStreamExtendedSmallSizeBitsMin, BitStreamExtendedSmallSizeBitsMax), 0);

    // 'Is Large' Value?
    if(valueBits == BitStreamExtendedSmallSizeIsLargeValue)
    {
      // Read large bitstream extended size
      ReturnIf(!bitStream.ReadQuantized(valueBits, BitStreamExtendedLargeSizeBitsMin, BitStreamExtendedLargeSizeBitsMax), 0);
    }

    // (There should be enough bits available to be read)
    Assert(bitStream.GetBitsUnread() >= valueBits);

    // Clear current bitstream extended data (if any)
    value.Clear(false);

    // Read bitstream extended data
    if(value.Append(bitStream, valueBits) != valueBits) // Unable?
    {
      DoNotifyError("BitStream", "Error reading BitStream data.");
      return 0;
    }
    Assert(value.GetBitsWritten() == valueBits);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

} // namespace Zero
