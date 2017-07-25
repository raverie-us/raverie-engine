///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Forward Declarations
class BitStream;

template<typename T>
inline Bits Measure(T& value)
{
  StaticAssert(MissingUserDefinedMeasure, false, "Requires a user-defined Measure function");
  return 0;
}

template<typename R>
inline Bits MeasureQuantized(const R& minValue_, const R& maxValue_, const R& quantum_)
{
  StaticAssert(MissingUserDefinedMeasureQuantized, false, "Requires a user-defined MeasureQuantized function");
  return 0;
}

template<typename T>
inline Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, T& value)
{
  StaticAssert(MissingUserDefinedSerialize, false, "Requires a user-defined Serialize function");
  return 0;
}

template<typename T, typename R>
inline Bits SerializeQuantized(SerializeDirection::Enum direction, BitStream& bitStream, T& value_, const R& minValue_, const R& maxValue_, const R& quantum_)
{
  StaticAssert(MissingUserDefinedSerializeQuantized, false, "Requires a user-defined SerializeQuantized function");
  return 0;
}

//---------------------------------------------------------------------------------//
//                                  BitStream                                      //
//---------------------------------------------------------------------------------//

//
// Member Functions
//

inline void BitStream::SetAlignment(BitAlignment::Enum alignment) const
{
  mAlignment = alignment;
}
inline BitAlignment::Enum BitStream::GetAlignment() const
{
  return mAlignment;
}

inline const byte* BitStream::GetData() const
{
  return mData;
}
inline byte* BitStream::GetDataExposed()
{
  return mData;
}

inline Bits BitStream::GetBitCapacity() const
{
  return BYTES_TO_BITS(GetByteCapacity());
}
inline Bytes BitStream::GetByteCapacity() const
{
  return mByteCapacity;
}

inline Bits BitStream::GetBitsWritten() const
{
  return mBitsWritten;
}
inline Bytes BitStream::GetBytesWritten() const
{
  return BITS_TO_BYTES(GetBitsWritten());
}

inline Bits BitStream::GetBitsUnwritten() const
{
  return GetBitCapacity() - GetBitsWritten();
}
inline Bytes BitStream::GetBytesUnwritten() const
{
  return BITS_TO_BYTES(GetBitsUnwritten());
}

inline Bits BitStream::GetBitsRead() const
{
  return mBitsRead;
}
inline Bytes BitStream::GetBytesRead() const
{
  return BITS_TO_BYTES(GetBitsRead());
}

inline Bits BitStream::GetBitsUnread() const
{
  return mBitsWritten - mBitsRead;
}
inline Bytes BitStream::GetBytesUnread() const
{
  return BITS_TO_BYTES(GetBitsUnread());
}

inline Bits BitStream::GetBitsSerialized(SerializeDirection::Enum direction) const
{
  return (direction == SerializeDirection::Write) ? mBitsWritten : mBitsRead;
}
inline Bytes BitStream::GetBytesSerialized(SerializeDirection::Enum direction) const
{
  return BITS_TO_BYTES(GetBitsSerialized(direction));
}

inline bool BitStream::IsEmpty() const
{
  return mBitsWritten ? false : true;
}

//
// Measure Operations
//

inline Bits BitStream::Measure(bool value)
{
  return 1;
}
inline Bits BitStream::Measure(const String& value)
{
  return BYTES_TO_BITS(value.SizeInBytes() + 1);
}

template<typename T>
inline R_ENABLE_IF(is_enum<T>::value, Bits) BitStream::Measure(T value)
{
  // Typedefs
  typedef EXACT_INT(BYTES_TO_BITS(sizeof(T))) intType;

  // Measure enum as integer
  return Measure((intType)value);
}
template<typename T>
inline R_ENABLE_IF(is_integral<T>::value, Bits) BitStream::Measure(T value)
{
  // Measure value
  return BYTES_TO_BITS(sizeof(value));
}
template<typename T>
inline R_ENABLE_IF(is_floating_point<T>::value, Bits) BitStream::Measure(T value)
{
  // Measure value
  return BYTES_TO_BITS(sizeof(value));
}
template<typename T, size_t Size>
inline Bits BitStream::Measure(const T (& array)[Size])
{
  Bits bitsWritten = 0;

  // Measure every element
  for(size_t i = 0; i < Size; ++i)
    bitsWritten += Measure(array[i]);

  return bitsWritten;
}
template<typename T>
inline Bits BitStream::Measure(const T* array, size_t size)
{
  Bits bitsWritten = 0;

  // Measure every element
  for(size_t i = 0; i < size; ++i)
    bitsWritten += Measure(array[i]);

  return bitsWritten;
}
template <Bits N, bool WrapAware>
inline Bits BitStream::Measure(UintN<N, WrapAware> value)
{
  return MeasureQuantized(value.min, value.max);
}
template<typename T>
inline R_ENABLE_IF(!is_scalar<T>::value, Bits) BitStream::Measure(const T& value)
{
  // Invoke user-defined function
  return Zero::Measure(const_cast<T&>(value));
}

template<typename R>
inline R_ENABLE_IF(is_enum<R>::value && is_enum_or_integral<R>::value, Bits) BitStream::MeasureQuantized(R minValue_, R maxValue_, R quantum_)
{
  // Typedefs
  typedef EXACT_INT(BYTES_TO_BITS(sizeof(R))) intType;

  // Measure enum as integer
  return MeasureQuantized((intType)minValue_, (intType)maxValue_, (intType)quantum_);
}
template<typename R>
inline R_ENABLE_IF(is_integral<R>::value && is_integral<R>::value, Bits) BitStream::MeasureQuantized(R minValue_, R maxValue_, R quantum_)
{
  //
  // Setup
  //

  // Typedefs
  typedef typename conditional<is_signed<R>::value, R, R>::type V;
  typedef typename make_unsigned<V>::type                       UV;
  typedef double                                                F;

  // (Range should be valid)
  Assert(minValue_ <= maxValue_);

  // (Quantum should be non-zero and positive)
  Assert(quantum_ > 0);

  // Cast all parameters to the appropriate types
  V  minValue = V(minValue_);
  V  maxValue = V(maxValue_);
  UV quantum  = UV(quantum_);

  // Normalize range
  UV normalizedRange = UV(maxValue - minValue);

  // Quantize normalized range
  UV quantizedRange = DivCeil(normalizedRange, quantum);

  // Determine bits necessary to represent all possible values
  Bits bitSize = BitsNeededToRepresent(quantizedRange);
  return bitSize;
}
template<typename R>
inline R_ENABLE_IF(is_floating_point<R>::value, Bits) BitStream::MeasureQuantized(R minValue_, R maxValue_, R quantum_)
{
  //
  // Setup
  //

  // Typedefs
  typedef EXACT_UINT(BYTES_TO_BITS(sizeof(R))) V;
  typedef typename make_unsigned<V>::type      UV;
  typedef R                                    F;

  // (Range should be valid)
  Assert(minValue_ <= maxValue_);

  // (Quantum should be non-zero and positive)
  Assert(quantum_ > 0);

  // Cast all parameters to the appropriate types
  F minValue = F(minValue_);
  F maxValue = F(maxValue_);
  F quantum  = F(quantum_);

  // Normalize range
  F normalizedRange = (maxValue - minValue);
  Assert(normalizedRange >= 0); // (Should be positive)

  // Quantize normalized range
  UV quantizedRange = (UV)DivCeil(normalizedRange, quantum);

  // Determine bits necessary to represent all possible values
  Bits bitSize = BitsNeededToRepresent(quantizedRange);
  return bitSize;
}
template <Bits N, bool WrapAware, typename R>
inline R_ENABLE_IF(is_enum_or_integral<R>::value, Bits) BitStream::MeasureQuantized(R minValue_, R maxValue_, R quantum_)
{
  // Typedefs
  typedef typename UintN<N, WrapAware>::value_type value_type;

  // Measure UintN as integer
  return MeasureQuantized((value_type)minValue_, (value_type)maxValue_, (value_type)quantum_);
}
template<typename R>
inline R_ENABLE_IF(!is_scalar<R>::value, Bits) BitStream::MeasureQuantized(const R& minValue_, const R& maxValue_, const R& quantum_)
{
  // Invoke user-defined function
  return Zero::MeasureQuantized(minValue_, maxValue_, quantum_);
}

//
// Serialize Operations
//

inline Bits BitStream::SerializeBit(SerializeDirection::Enum direction, bool& value)
{
  return (direction == SerializeDirection::Write) ? WriteBit(value)
                                                  : ReadBit(value);
}
inline Bits BitStream::SerializeBits(SerializeDirection::Enum direction, byte* data, Bits dataBits)
{
  return (direction == SerializeDirection::Write) ? WriteBits(data, dataBits)
                                                  : ReadBits(data, dataBits);
}

inline Bits BitStream::SerializeByte(SerializeDirection::Enum direction, uint8& value)
{
  return (direction == SerializeDirection::Write) ? WriteByte(value)
                                                  : ReadByte(value);
}
inline Bits BitStream::SerializeBytes(SerializeDirection::Enum direction, byte* data, Bytes dataBytes)
{
  return (direction == SerializeDirection::Write) ? WriteBytes(data, dataBytes)
                                                  : ReadBytes(data, dataBytes);
}

inline Bits BitStream::Serialize(SerializeDirection::Enum direction, bool& value)
{
  return (direction == SerializeDirection::Write) ? Write(value)
                                                  : Read(value);
}
inline Bits BitStream::Serialize(SerializeDirection::Enum direction, String& value)
{
  return (direction == SerializeDirection::Write) ? Write(value)
                                                  : Read(value);
}
template<typename T>
inline R_ENABLE_IF(is_enum_or_integral<T>::value, Bits) BitStream::Serialize(SerializeDirection::Enum direction, T& value)
{
  return (direction == SerializeDirection::Write) ? Write(value)
                                                  : Read(value);
}
template<typename T>
inline R_ENABLE_IF(is_floating_point<T>::value, Bits) BitStream::Serialize(SerializeDirection::Enum direction, T& value)
{
  return (direction == SerializeDirection::Write) ? Write(value)
                                                  : Read(value);
}
template<typename T, size_t Size>
inline Bits BitStream::Serialize(SerializeDirection::Enum direction, const T (& array)[Size])
{
  Bits bitsSerialized = 0;

  // Serialize every element
  for(size_t i = 0; i < Size; ++i)
    bitsSerialized += Serialize(direction, array[i]);

  return bitsSerialized;
}
template<typename T>
inline Bits BitStream::Serialize(SerializeDirection::Enum direction, const T* array, size_t size)
{
  Bits bitsSerialized = 0;

  // Serialize every element
  for(size_t i = 0; i < size; ++i)
    bitsSerialized += Serialize(direction, array[i]);

  return bitsSerialized;
}
template <Bits N, bool WrapAware>
inline Bits BitStream::Serialize(SerializeDirection::Enum direction, UintN<N, WrapAware>& value)
{
  return (direction == SerializeDirection::Write) ? Write(value)
                                                  : Read(value);
}
template<typename T>
inline R_ENABLE_IF(!is_scalar<T>::value, Bits) BitStream::Serialize(SerializeDirection::Enum direction, T& value)
{
  // Invoke user-defined function
  return Zero::Serialize(direction, *this, value);
}

template<typename T, typename R>
R_ENABLE_IF(is_enum_or_integral<T>::value && is_enum_or_integral<R>::value, Bits) BitStream::SerializeQuantized(SerializeDirection::Enum direction, T& value_, R minValue_, R maxValue_, R quantum_)
{
  return (direction == SerializeDirection::Write) ? WriteQuantized(value_, minValue_, maxValue_, quantum_)
                                                  : ReadQuantized(value_, minValue_, maxValue_, quantum_);
}
template<typename T, typename R>
R_ENABLE_IF(is_floating_point<T>::value, Bits) BitStream::SerializeQuantized(SerializeDirection::Enum direction, T& value_, R minValue_, R maxValue_, R quantum_)
{
  return (direction == SerializeDirection::Write) ? WriteQuantized(value_, minValue_, maxValue_, quantum_)
                                                  : ReadQuantized(value_, minValue_, maxValue_, quantum_);
}
template <Bits N, bool WrapAware, typename R>
R_ENABLE_IF(is_enum_or_integral<R>::value, Bits) BitStream::SerializeQuantized(SerializeDirection::Enum direction, UintN<N, WrapAware>& value_, R minValue_, R maxValue_, R quantum_)
{
  return (direction == SerializeDirection::Write) ? WriteQuantized(value_, minValue_, maxValue_, quantum_)
                                                  : ReadQuantized(value_, minValue_, maxValue_, quantum_);
}
template<typename T, typename R>
R_ENABLE_IF(!is_scalar<T>::value, Bits) BitStream::SerializeQuantized(SerializeDirection::Enum direction, T& value_, const R& minValue_, const R& maxValue_, const R& quantum_)
{
  // Invoke user-defined function
  return Zero::SerializeQuantized(direction, *this, value_, minValue_, maxValue_, quantum_);
}

//
// Write Operations
//

inline Bits BitStream::Write(bool value)
{
  return WriteBit(value);
}

template<typename T>
inline R_ENABLE_IF(is_enum<T>::value, Bits) BitStream::Write(T value)
{
  // Typedefs
  typedef EXACT_INT(BYTES_TO_BITS(sizeof(T))) intType;

  // Write enum as integer
  return Write((intType)value);
}
template<typename T>
inline R_ENABLE_IF(is_integral<T>::value, Bits) BitStream::Write(T value)
{
  // Flip to big endian
  value = NetworkFlip(value);

  // Write value
  return WriteBits((const byte*)&value, BYTES_TO_BITS(sizeof(value)));
}
template<typename T>
inline R_ENABLE_IF(is_floating_point<T>::value, Bits) BitStream::Write(T value)
{
  // Finite value (not NaN or infinity) check
  Assert(-std::numeric_limits<T>::max() <= value && value <= std::numeric_limits<T>::max());

#if BITSTREAM_USE_PORTABLE_FLOATING_POINT

  const Bits totalBits = BYTES_TO_BITS(sizeof(T));       // Total bits (sign + exp + mant)
  Bits       mantBits  = std::numeric_limits<T>::digits; // Mantissa bits
  Bits       expBits   = totalBits - mantBits;           // Exponent bits
  --mantBits;                                            // Sign bit
  Assert(totalBits == (1 + expBits + mantBits));

  // Typedefs
  typedef EXACT_UINT(BYTES_TO_BITS(sizeof(T))) uintType;
  typedef EXACT_INT(BYTES_TO_BITS(sizeof(T)))  intType;

  // Convert to IEEE-754
  uintType result;
  if(value == T(0))
    result = 0;
  else
  {
    T normValue = value;

    // Get sign bit, begin normalization
    uintType sign = 0;
    if(value < T(0))
    {
      sign = 1;
      normValue *= T(-1);
    }

    // Get exponent shift, finish normalization
    intType shift = 0;
    while(normValue >= T(2))
    {
      normValue /= T(2);
      ++shift;
    }
    while(normValue < T(1))
    {
      normValue *= T(2);
      --shift;
    }
    --normValue;

    // Get mantissa and exponent, stored as unsigned integers
    uintType mantissa = uintType(normValue * (T(uintType(1) << mantBits) + T(0.5)));
    uintType exponent = shift + (uintType(1) << (expBits - 1)) - 1;

    // Concatenate result (Sign | Exponent | Mantissa)
    result = (sign << (totalBits - 1)) | (exponent << (totalBits - expBits - 1)) | mantissa;
  }

  // Flip to big endian
  result = NetworkFlip(result);

  // Write the result
  return WriteBits((const byte*)&result, totalBits);

#else

  // Flip to big endian
  value = NetworkFlip(value);

  // Write the value
  return WriteBits((const byte*)&value, BYTES_TO_BITS(sizeof(value)));

#endif
}
template<typename T, size_t Size>
inline Bits BitStream::Write(const T (& array)[Size])
{
  Bits bitsWritten = 0;

  // Write every element
  for(size_t i = 0; i < Size; ++i)
    bitsWritten += Write(array[i]);

  return bitsWritten;
}
template<typename T>
inline Bits BitStream::Write(const T* array, size_t size)
{
  Bits bitsWritten = 0;

  // Write every element
  for(size_t i = 0; i < size; ++i)
    bitsWritten += Write(array[i]);

  return bitsWritten;
}
template <Bits N, bool WrapAware>
inline Bits BitStream::Write(UintN<N, WrapAware> value)
{
  return WriteQuantized(value.value(), value.min, value.max);
}
template<typename T>
inline R_ENABLE_IF(!is_scalar<T>::value, Bits) BitStream::Write(const T& value)
{
  // Invoke user-defined function
  return Zero::Serialize(SerializeDirection::Write, *this, const_cast<T&>(value));
}

template<typename T, typename R>
inline R_ENABLE_IF(is_enum<T>::value && is_enum_or_integral<R>::value, Bits) BitStream::WriteQuantized(T value_, R minValue_, R maxValue_, R quantum_)
{
  // Typedefs
  typedef EXACT_INT(BYTES_TO_BITS(sizeof(T))) intType;

  // Write enum as integer
  return WriteQuantized((intType)value_, (intType)minValue_, (intType)maxValue_, (intType)quantum_);
}
template<typename T, typename R>
inline R_ENABLE_IF(is_integral<T>::value && is_integral<R>::value, Bits) BitStream::WriteQuantized(T value_, R minValue_, R maxValue_, R quantum_)
{
  //
  // Setup
  //

  // Typedefs
  typedef typename conditional<is_signed<R>::value, R, T>::type V;
  typedef typename make_unsigned<V>::type                       UV;
  typedef double                                                F;

  // (Range should be valid)
  Assert(minValue_ <= maxValue_);

  // (Quantum should be non-zero and positive)
  Assert(quantum_ > 0);

  // Cast all parameters to the appropriate types
  V  value    = V(value_);
  V  minValue = V(minValue_);
  V  maxValue = V(maxValue_);
  UV quantum  = UV(quantum_);

  // Normalize range
  UV normalizedRange = UV(maxValue - minValue);

  // Quantize normalized range
  UV quantizedRange = DivCeil(normalizedRange, quantum);

  // Determine bits necessary to represent all possible values
  Bits bitSize = BitsNeededToRepresent(quantizedRange);

  //
  // Serialization
  //

  // Clamp value within range
  V clampedValue = Clamp(value, minValue, maxValue);

  // Normalize clamped value
  UV normalizedValue = UV(clampedValue - minValue);

  // Quantize normalized value
  UV quantizedValue = (UV)Round((F)normalizedValue / (F)quantum);

  // Left-shift quantized value
  UV leftShiftedQuantizedValue = (quantizedValue << (BYTES_TO_BITS(sizeof(UV)) - bitSize));

  // Convert to network byte order (big endian), which is naturally right-aligned, as necessary
  UV rightAlignedQuantizedValue = NetworkFlip(leftShiftedQuantizedValue);

  // Write the right-aligned quantized value
  return WriteBits((const byte*)&rightAlignedQuantizedValue, bitSize);
}
template<typename T, typename R>
inline R_ENABLE_IF(is_floating_point<T>::value, Bits) BitStream::WriteQuantized(T value_, R minValue_, R maxValue_, R quantum_)
{
  //
  // Setup
  //

  // Typedefs
  typedef EXACT_UINT(BYTES_TO_BITS(sizeof(T))) V;
  typedef typename make_unsigned<V>::type      UV;
  typedef T                                    F;

  // (Range should be valid)
  Assert(minValue_ <= maxValue_);

  // (Quantum should be non-zero and positive)
  Assert(quantum_ > 0);

  // Cast all parameters to the appropriate types
  F value    = F(value_);
  F minValue = F(minValue_);
  F maxValue = F(maxValue_);
  F quantum  = F(quantum_);

  // Normalize range
  F normalizedRange = (maxValue - minValue);
  Assert(normalizedRange >= 0); // (Should be positive)

  // Quantize normalized range
  UV quantizedRange = (UV)DivCeil(normalizedRange, quantum);

  // Determine bits necessary to represent all possible values
  Bits bitSize = BitsNeededToRepresent(quantizedRange);

  //
  // Serialization
  //

  // Clamp value within range
  F clampedValue = Clamp(value, minValue, maxValue);

  // Normalize clamped value
  F normalizedValue = (clampedValue - minValue);
  Assert(normalizedValue >= 0); // (Should be positive)

  // Quantize normalized value
  UV quantizedValue = (UV)Round(normalizedValue / quantum);

  // Left-shift quantized value
  UV leftShiftedQuantizedValue = (quantizedValue << (BYTES_TO_BITS(sizeof(UV)) - bitSize));

  // Convert to network byte order (big endian), which is naturally right-aligned, as necessary
  UV rightAlignedQuantizedValue = NetworkFlip(leftShiftedQuantizedValue);

  // Write the right-aligned quantized value
  return WriteBits((const byte*)&rightAlignedQuantizedValue, bitSize);
}
template <Bits N, bool WrapAware, typename R>
inline R_ENABLE_IF(is_enum_or_integral<R>::value, Bits) BitStream::WriteQuantized(UintN<N, WrapAware> value_, R minValue_, R maxValue_, R quantum_)
{
  // Typedefs
  typedef typename UintN<N, WrapAware>::value_type value_type;

  // Write UintN as integer
  return WriteQuantized(value_.value(), (value_type)minValue_, (value_type)maxValue_, (value_type)quantum_);
}
template<typename T, typename R>
inline R_ENABLE_IF(!is_scalar<T>::value, Bits) BitStream::WriteQuantized(const T& value_, const R& minValue_, const R& maxValue_, const R& quantum_)
{
  // Invoke user-defined function
  return Zero::SerializeQuantized(SerializeDirection::Write, *this, const_cast<T&>(value_), minValue_, maxValue_, quantum_);
}

inline Bits BitStream::Append(const BitStream& bitStream)
{
  return Append(bitStream, bitStream.GetBitsUnread());
}

inline Bits BitStream::AppendAll(const BitStream& value)
{
  value.ClearBitsRead();
  Bits result = Append(value);
  value.ClearBitsRead();
  return result;
}

inline Bits BitStream::AssignRemainder(const BitStream& value)
{
  Clear(false);
  return Append(value);
}

inline Bits BitStream::TrimFront()
{
  return TrimFront(GetBitsUnread());
}

// inline void BitStream::Unwrite(Bits bitsToUnwrite)
// {
//   mBitsWritten = (mBitsWritten > bitsToUnwrite) ? (mBitsWritten - bitsToUnwrite) : 0;
// }

inline void BitStream::SetBitsWritten(Bits bitsWritten)
{
  mBitsWritten = bitsWritten;
}
inline void BitStream::SetBytesWritten(Bytes bytesWritten)
{
  SetBitsWritten(BYTES_TO_BITS(bytesWritten));
}
inline void BitStream::ClearBitsWritten()
{
  mBitsWritten = 0;
}

//
// Read Operations
//

inline Bits BitStream::Read(bool& value) const
{
  return ReadBit(value);
}

template<typename T>
inline R_ENABLE_IF(is_enum<T>::value, Bits) BitStream::Read(T& value) const
{
  // Typedefs
  typedef EXACT_INT(BYTES_TO_BITS(sizeof(T))) intType;

  // Read enum as integer
  return Read((intType&)value);
}
template<typename T>
inline R_ENABLE_IF(is_integral<T>::value, Bits) BitStream::Read(T& value) const
{
  // Read the value
  Bits bitsRead = ReadBits((byte*)&value, BYTES_TO_BITS(sizeof(value)));
  if(!bitsRead) // Unable?
    return 0;

  // Flip from big endian
  value = NetworkFlip(value);

  // Result
  return bitsRead;
}
template<typename T>
inline R_ENABLE_IF(is_floating_point<T>::value, Bits) BitStream::Read(T& value) const
{
#if BITSTREAM_USE_PORTABLE_FLOATING_POINT

  const Bits totalBits = BYTES_TO_BITS(sizeof(T));       // Total bits (sign + exp + mant)
  Bits mantBits        = std::numeric_limits<T>::digits; // Mantissa bits
  Bits expBits         = totalBits - mantBits;           // Exponent bits
  --mantBits;                                            // Sign bit
  Assert(totalBits == (1 + expBits + mantBits));

  // Typedefs
  typedef EXACT_UINT(BYTES_TO_BITS(sizeof(T))) uintType;
  typedef EXACT_INT(BYTES_TO_BITS(sizeof(T)))  intType;

  // Read the result
  uintType result = 0;
  Bits bitsRead = ReadBits((byte*)&result, totalBits);
  if(!bitsRead) // Unable?
    return 0;

  // Flip from big endian
  result = NetworkFlip(result);

  // Convert from IEEE-754
  if(result == 0)
    value = T(0);
  else
  {
    T normValue = T(1); // Implicit 1.0

    // Read the mantissa
    normValue += T(result & ((uintType(1) << mantBits) - 1)) / T(uintType(1) << mantBits);

    // Read the exponent
    intType shift = ((result >> mantBits) & ((uintType(1) << expBits) - 1)) - ((uintType(1) << (expBits - 1)) - 1);
    while(shift > T(0))
    {
      normValue *= T(2);
      --shift;
    }
    while(shift < T(0))
    {
      normValue /= T(2);
      ++shift;
    }

    // Read the sign
    if((result >> (totalBits - 1)) & 1)
      normValue *= T(-1);

    value = normValue;
  }

  // Result
  return bitsRead;

#else

  // Read the value
  Bits bitsRead = ReadBits((byte*)&value, BYTES_TO_BITS(sizeof(value)));
  if(!bitsRead) // Unable?
    return 0;

  // Flip from big endian
  value = NetworkFlip(value);

  // Result
  return bitsRead;

#endif
}
template<typename T, size_t Size>
inline Bits BitStream::Read(T (& array)[Size])
{
  Bits bitsRead = 0;

  // Read every element
  for(size_t i = 0; i < Size; ++i)
    bitsRead += Read(array[i]);

  return bitsRead;
}
template<typename T>
inline Bits BitStream::Read(T* array, size_t size)
{
  Bits bitsRead = 0;

  // Read every element
  for(size_t i = 0; i < size; ++i)
    bitsRead += Read(array[i]);

  return bitsRead;
}
template <Bits N, bool WrapAware>
inline Bits BitStream::Read(UintN<N, WrapAware>& value) const
{
  return ReadQuantized(value.value(), value.min, value.max);
}
template<typename T>
inline R_ENABLE_IF(!is_scalar<T>::value, Bits) BitStream::Read(T& value) const
{
  // Invoke user-defined function
  return Zero::Serialize(SerializeDirection::Read, *const_cast<BitStream*>(this), value);
}

template<typename T, typename R>
R_ENABLE_IF(is_enum<T>::value && is_enum_or_integral<R>::value, Bits) BitStream::ReadQuantized(T& value_, R minValue_, R maxValue_, R quantum_) const
{
  // Typedefs
  typedef EXACT_INT(BYTES_TO_BITS(sizeof(T))) intType;

  // Read enum as integer
  return ReadQuantized((intType&)value_, (intType)minValue_, (intType)maxValue_, (intType)quantum_);
}
template<typename T, typename R>
R_ENABLE_IF(is_integral<T>::value && is_integral<R>::value, Bits) BitStream::ReadQuantized(T& value_, R minValue_, R maxValue_, R quantum_) const
{
  //
  // Setup
  //

  // Typedefs
  typedef typename conditional<is_signed<R>::value, R, T>::type V;
  typedef typename make_unsigned<V>::type                       UV;
  typedef double                                                F;

  // (Range should be valid)
  Assert(minValue_ <= maxValue_);

  // (Quantum should be non-zero and positive)
  Assert(quantum_ > 0);

  // Cast all parameters to the appropriate types
  T& value    = value_;
  V  minValue = V(minValue_);
  V  maxValue = V(maxValue_);
  UV quantum  = UV(quantum_);

  // Normalize range
  UV normalizedRange = UV(maxValue - minValue);

  // Quantize normalized range
  UV quantizedRange = DivCeil(normalizedRange, quantum);

  // Determine bits necessary to represent all possible values
  Bits bitSize = BitsNeededToRepresent(quantizedRange);

  //
  // Deserialization
  //

  // Read the right-aligned quantized value
  UV rightAlignedQuantizedValue;
  Bits bitsRead = ReadBits((byte*)&rightAlignedQuantizedValue, bitSize);
  if(!bitsRead) // Unable?
    return 0;

  // Convert from network byte order (big endian), which is naturally right-aligned, as necessary
  UV leftShiftedQuantizedValue = NetworkFlip(rightAlignedQuantizedValue);

  // Right-shift left-shifted quantized value
  UV quantizedValue = (leftShiftedQuantizedValue >> (BYTES_TO_BITS(sizeof(UV)) - bitSize));

  // Dequantize value
  UV normalizedValue = (quantizedValue * quantum);

  // Denormalize value
  V clampedValue = ((V)normalizedValue + minValue);

  // Set value
  value = (T)clampedValue;
  return bitsRead;
}
template<typename T, typename R>
R_ENABLE_IF(is_floating_point<T>::value, Bits) BitStream::ReadQuantized(T& value_, R minValue_, R maxValue_, R quantum_) const
{
  //
  // Setup
  //

  // Typedefs
  typedef EXACT_UINT(BYTES_TO_BITS(sizeof(T))) V;
  typedef typename make_unsigned<V>::type      UV;
  typedef T                                    F;

  // (Range should be valid)
  Assert(minValue_ <= maxValue_);

  // (Quantum should be non-zero and positive)
  Assert(quantum_ > 0);

  // Cast all parameters to the appropriate types
  T& value   = value_;
  F minValue = F(minValue_);
  F maxValue = F(maxValue_);
  F quantum  = F(quantum_);

  // Normalize range
  F normalizedRange = (maxValue - minValue);
  Assert(normalizedRange >= 0); // (Should be positive)

  // Quantize normalized range
  UV quantizedRange = (UV)DivCeil(normalizedRange, quantum);

  // Determine bits necessary to represent all possible values
  Bits bitSize = BitsNeededToRepresent(quantizedRange);

  //
  // Deserialization
  //

  // Read the right-aligned quantized value
  UV rightAlignedQuantizedValue;
  Bits bitsRead = ReadBits((byte*)&rightAlignedQuantizedValue, bitSize);
  if(!bitsRead) // Unable?
    return 0;

  // Convert from network byte order (big endian), which is naturally right-aligned, as necessary
  UV leftShiftedQuantizedValue = NetworkFlip(rightAlignedQuantizedValue);

  // Right-shift left-shifted quantized value
  UV quantizedValue = (leftShiftedQuantizedValue >> (BYTES_TO_BITS(sizeof(UV)) - bitSize));

  // Dequantize value
  F normalizedValue = ((F)quantizedValue * quantum);
  Assert(normalizedValue >= 0); // (Should be positive)

  // Denormalize value
  F clampedValue = (normalizedValue + minValue);

  // Set value
  value = (T)clampedValue;
  return bitsRead;
}
template <Bits N, bool WrapAware, typename R>
R_ENABLE_IF(is_enum_or_integral<R>::value, Bits) BitStream::ReadQuantized(UintN<N, WrapAware>& value_, R minValue_, R maxValue_, R quantum_) const
{
  // Typedefs
  typedef typename UintN<N, WrapAware>::value_type value_type;

  // Read UintN as integer
  return ReadQuantized(value_.value(), (value_type)minValue_, (value_type)maxValue_, (value_type)quantum_);
}
template<typename T, typename R>
R_ENABLE_IF(!is_scalar<T>::value, Bits) BitStream::ReadQuantized(T& value_, const R& minValue_, const R& maxValue_, const R& quantum_) const
{
  // Invoke user-defined function
  return Zero::SerializeQuantized(SerializeDirection::Read, *const_cast<BitStream*>(this), value_, minValue_, maxValue_, quantum_);
}

inline void BitStream::Unread(Bits bitsToUnread) const
{
  mBitsRead = (mBitsRead > bitsToUnread) ? (mBitsRead - bitsToUnread) : 0;
}

inline void BitStream::SetBitsRead(Bits bitsRead) const
{
  mBitsRead = bitsRead;
}
inline void BitStream::SetBytesRead(Bytes bytesRead) const
{
  SetBitsRead(BYTES_TO_BITS(bytesRead));
}
inline void BitStream::ClearBitsRead() const
{
  mBitsRead = 0;
}

template<typename T>
inline String GetBinaryString(const T& value)
{
  StringBuilder result;
  byte* valueCursor = (byte*)&value;

  // Read every byte (from left to right)
  for(uint i = 0; i < sizeof(value); ++i)
  {
    // Read every bit in the byte (from left to right)
    for(uint j = 0; j < 8; ++j)
      result += *valueCursor & LBIT(j) ? '1' : '0';

    // Next byte
    result += ' ';
    ++valueCursor;
  }

  return result.ToString();
}

} // namespace Zero
