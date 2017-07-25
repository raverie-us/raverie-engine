///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "Utility/EnumDeclaration.hpp"
#include "Utility/BitMath.hpp"
#include "Utility/UintN.hpp"
#include "String/String.hpp"

//---------------------------------------------------------------------------------//
//                           BitStream Configuration                               //
//---------------------------------------------------------------------------------//

/// Use portable floating-point format?
/// For communication between platforms with differing floating-point formats
#define BITSTREAM_USE_PORTABLE_FLOATING_POINT 0

/// Default memory allocation byte size
/// Used if reserve is not called before first write
#define BITSTREAM_DEFAULT_RESERVE_BYTES POW2(5)
StaticAssertWithinRange(StaticAssertRange1, BYTES_TO_BITS(BITSTREAM_DEFAULT_RESERVE_BYTES), 1, Bits(-1));

/// Maximum memory allocation byte size
#define BITSTREAM_MAX_SIZE_BITS 28
#define BITSTREAM_MAX_BYTES POW2(BITSTREAM_MAX_SIZE_BITS)
StaticAssertWithinRange(StaticAssertRange2, BYTES_TO_BITS(BITSTREAM_MAX_BYTES), 1, Bits(-1));

namespace Zero
{

/// Bit alignment
DeclareEnum2(BitAlignment,
  Bit,   /// Bit aligned
  Byte); /// Byte aligned

/// Serialize direction
DeclareEnum2(SerializeDirection,
  Write, /// Perform a write
  Read); /// Perform a read

/// Forward Declarations
class BitStream;

/// Measures data
/// Returns the number of bits required to serialize the specified value
template<typename T>
Bits Measure(T& value);

/// Measures data bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
/// Returns the number of bits required to serialize the specified value
template<typename R>
Bits MeasureQuantized(const R& minValue_, const R& maxValue_, const R& quantum_);

/// Serializes data
/// Returns the number of bits serialized if successful, else 0
template<typename T>
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, T& value);

/// Serializes data bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
/// Returns the number of bits serialized if successful, else 0
template<typename T, typename R>
Bits SerializeQuantized(SerializeDirection::Enum direction, BitStream& bitStream, T& value_, const R& minValue_, const R& maxValue_, const R& quantum_);

//---------------------------------------------------------------------------------//
//                                  BitStream                                      //
//---------------------------------------------------------------------------------//

/// Bit-packed data stream
class BitStream
{
public:
  /// Default Constructor
  BitStream();
  /// Copy Constructor
  BitStream(const BitStream& rhs);
  /// Move Constructor
  BitStream(MoveReference<BitStream> rhs);

  /// Destructor
  ~BitStream();

  /// Copy Assignment Operator
  BitStream& operator =(const BitStream& rhs);
  /// Move Assignment Operator
  BitStream& operator =(MoveReference<BitStream> rhs);

  /// Comparison Operators
  bool operator ==(const BitStream& rhs) const;
  bool operator !=(const BitStream& rhs) const;
  bool operator  <(const BitStream& rhs) const;

  //
  // Member Functions
  //

  /// Sets the bit alignment
  /// Affects any successive write or read operations
  void SetAlignment(BitAlignment::Enum alignment) const;
  /// Returns the bit alignment
  BitAlignment::Enum GetAlignment() const;

  /// Returns the data array
  const byte* GetData() const;
  /// Returns the data array exposed for writing to the internal buffer directly.
  /// Be sure there's enough room by calling Reserve() prior to this call and inform
  /// the BitStream of data written by calling SetBitsWritten() or SetBytesWritten() afterwards.
  byte* GetDataExposed();

  /// Returns the data array capacity in bits
  inline Bits GetBitCapacity() const;
  /// Returns the data array capacity in bytes
  inline Bytes GetByteCapacity() const;

  /// Returns the number of bits written
  inline Bits GetBitsWritten() const;
  /// Returns the number of bytes written (Rounded up, ex. 12 bits = 2 bytes)
  inline Bytes GetBytesWritten() const;

  /// Returns the number of bits unwritten
  inline Bits GetBitsUnwritten() const;
  /// Returns the number of bytes unwritten (Rounded up, ex. 12 bits = 2 bytes)
  inline Bytes GetBytesUnwritten() const;

  /// Returns the number of bits read
  inline Bits GetBitsRead() const;
  /// Returns the number of bytes read (Rounded up, ex. 12 bits = 2 bytes)
  inline Bytes GetBytesRead() const;

  /// Returns the number of bits unread
  inline Bits GetBitsUnread() const;
  /// Returns the number of bytes unread (Rounded up, ex. 12 bits = 2 bytes)
  inline Bytes GetBytesUnread() const;

  /// Returns the number of bits serialized
  inline Bits GetBitsSerialized(SerializeDirection::Enum direction) const;
  /// Returns the number of bytes serialized (Rounded up, ex. 12 bits = 2 bytes)
  inline Bytes GetBytesSerialized(SerializeDirection::Enum direction) const;

  /// Returns true if the BitStream is empty (0 bits written), else false
  bool IsEmpty() const;

  /// Reserves at least the specified memory capacity, reallocating if necessary
  void Reserve(Bytes capacity);

  /// Clears all data written and resets the bitstream, optionally freeing reserved memory
  void Clear(bool freeMemory);

  //
  // Measure Operations
  //

  /// Measures a boolean value
  /// Returns the number of bits required to serialize the specified value
  static inline Bits Measure(bool value);
  /// Measures a string value
  /// Returns the number of bits required to serialize the specified value
  static inline Bits Measure(const String& value);

  /// Measures an enum value
  /// Returns the number of bits required to serialize the specified value
  template<typename T>
  static R_ENABLE_IF(is_enum<T>::value, Bits) Measure(T value);
  /// Measures an integral value
  /// Returns the number of bits required to serialize the specified value
  template<typename T>
  static R_ENABLE_IF(is_integral<T>::value, Bits) Measure(T value);
  /// Measures a floating-point value
  /// Returns the number of bits required to serialize the specified value
  template<typename T>
  static R_ENABLE_IF(is_floating_point<T>::value, Bits) Measure(T value);
  /// Measures a fixed array of values
  /// Returns the number of bits required to serialize the specified value
  template<typename T, size_t Size>
  static Bits Measure(const T (& array)[Size]);
  /// Measures a fixed array of values
  /// Returns the number of bits required to serialize the specified value
  template<typename T>
  static Bits Measure(const T* array, size_t size);
  /// Measures a UintN value
  /// Returns the number of bits required to serialize the specified value
  template <Bits N, bool WrapAware>
  static Bits Measure(UintN<N, WrapAware> value);
  /// Measures a user-defined value
  /// Returns the number of bits required to serialize the specified value
  template<typename T>
  static R_ENABLE_IF(!is_scalar<T>::value, Bits) Measure(const T& value);

  /// Measures an enum value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits required to serialize the specified value
  template<typename R>
  static R_ENABLE_IF(is_enum<R>::value && is_enum_or_integral<R>::value, Bits) MeasureQuantized(R minValue_, R maxValue_, R quantum_ = R(1));
  /// Measures an integral value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits required to serialize the specified value
  template<typename R>
  static R_ENABLE_IF(is_integral<R>::value && is_integral<R>::value, Bits) MeasureQuantized(R minValue_, R maxValue_, R quantum_ = R(1));
  /// Measures a floating-point value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits required to serialize the specified value
  template<typename R>
  static R_ENABLE_IF(is_floating_point<R>::value, Bits) MeasureQuantized(R minValue_, R maxValue_, R quantum_ = R(0.0001));
  /// Measures a UintN value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits required to serialize the specified value
  template <Bits N, bool WrapAware, typename R>
  static R_ENABLE_IF(is_enum_or_integral<R>::value, Bits) MeasureQuantized(R minValue_, R maxValue_, R quantum_ = R(1));
  /// Measures a user-defined value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits required to serialize the specified value
  template<typename R>
  static R_ENABLE_IF(!is_scalar<R>::value, Bits) MeasureQuantized(const R& minValue_, const R& maxValue_, const R& quantum_);

  //
  // Serialize Operations
  //

  /// Serializes a single bit
  /// Returns the number of bits serialized if successful, else 0
  inline Bits SerializeBit(SerializeDirection::Enum direction, bool& value);
  /// Serializes multiple bits
  /// Returns the number of bits serialized if successful, else 0
  inline Bits SerializeBits(SerializeDirection::Enum direction, byte* data, Bits dataBits);

  /// Serializes a single byte
  /// Returns the number of bits serialized if successful, else 0
  inline Bits SerializeByte(SerializeDirection::Enum direction, uint8& value);
  /// Serializes multiple bytes
  /// Returns the number of bits serialized if successful, else 0
  inline Bits SerializeBytes(SerializeDirection::Enum direction, byte* data, Bytes dataBytes);

  /// Serializes a boolean value
  /// Returns the number of bits serialized if successful, else 0
  inline Bits Serialize(SerializeDirection::Enum direction, bool& value);
  /// Serializes a string value
  /// Returns the number of bits serialized if successful, else 0
  inline Bits Serialize(SerializeDirection::Enum direction, String& value);
  /// Serializes an integral value
  /// Returns the number of bits serialized if successful, else 0
  template<typename T>
  R_ENABLE_IF(is_enum_or_integral<T>::value, Bits) Serialize(SerializeDirection::Enum direction, T& value);
  /// Serializes a floating-point value
  /// Returns the number of bits serialized if successful, else 0
  template<typename T>
  R_ENABLE_IF(is_floating_point<T>::value, Bits) Serialize(SerializeDirection::Enum direction, T& value);
  /// Serializes a fixed array of values
  /// Returns the number of bits written if successful, else 0
  template<typename T, size_t Size>
  Bits Serialize(SerializeDirection::Enum direction, const T (& array)[Size]);
  /// Serializes a fixed array of values
  /// Returns the number of bits written if successful, else 0
  template<typename T>
  Bits Serialize(SerializeDirection::Enum direction, const T* array, size_t size);
  /// Serializes a UintN value
  /// Returns the number of bits serialized if successful, else 0
  template <Bits N, bool WrapAware>
  Bits Serialize(SerializeDirection::Enum direction, UintN<N, WrapAware>& value);
  /// Serializes a user-defined value
  /// Returns the number of bits serialized if successful, else 0
  template<typename T>
  R_ENABLE_IF(!is_scalar<T>::value, Bits) Serialize(SerializeDirection::Enum direction, T& value);

  /// Serializes an enum or integral value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits serialized if successful, else 0
  template<typename T, typename R>
  R_ENABLE_IF(is_enum_or_integral<T>::value && is_enum_or_integral<R>::value, Bits) SerializeQuantized(SerializeDirection::Enum direction, T& value_, R minValue_, R maxValue_, R quantum_ = R(1));
  /// Serializes a floating-point value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits serialized if successful, else 0
  template<typename T, typename R>
  R_ENABLE_IF(is_floating_point<T>::value, Bits) SerializeQuantized(SerializeDirection::Enum direction, T& value_, R minValue_, R maxValue_, R quantum_ = R(0.0001));
  /// Serializes a UintN value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits serialized if successful, else 0
  template <Bits N, bool WrapAware, typename R>
  R_ENABLE_IF(is_enum_or_integral<R>::value, Bits) SerializeQuantized(SerializeDirection::Enum direction, UintN<N, WrapAware>& value_, R minValue_, R maxValue_, R quantum_ = R(1));
  /// Serializes a user-defined value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits serialized if successful, else 0
  template<typename T, typename R>
  R_ENABLE_IF(!is_scalar<T>::value, Bits) SerializeQuantized(SerializeDirection::Enum direction, T& value_, const R& minValue_, const R& maxValue_, const R& quantum_);

  //
  // Write Operations
  //

  /// Writes a single bit
  /// Returns the number of bits written if successful, else 0
  Bits WriteBit(bool value);
  /// Writes multiple bits
  /// Returns the number of bits written if successful, else 0
  Bits WriteBits(const byte* data, Bits dataBits);

  /// Writes a single byte
  /// Returns the number of bits written if successful, else 0
  Bits WriteByte(uint8 value);
  /// Writes multiple bytes
  /// Returns the number of bits written if successful, else 0
  Bits WriteBytes(const byte* data, Bytes dataBytes);

  /// Writes a boolean value
  /// Returns the number of bits written if successful, else 0
  inline Bits Write(bool value);
  /// Writes a string value
  /// Returns the number of bits written if successful, else 0
  Bits Write(const String& value);

  /// Writes an enum value
  /// Returns the number of bits written if successful, else 0
  template<typename T>
  R_ENABLE_IF(is_enum<T>::value, Bits) Write(T value);
  /// Writes an integral value
  /// Returns the number of bits written if successful, else 0
  template<typename T>
  R_ENABLE_IF(is_integral<T>::value, Bits) Write(T value);
  /// Writes a floating-point value
  /// Returns the number of bits written if successful, else 0
  template<typename T>
  R_ENABLE_IF(is_floating_point<T>::value, Bits) Write(T value);
  /// Writes a fixed array of values
  /// Returns the number of bits written if successful, else 0
  template<typename T, size_t Size>
  Bits Write(const T (& array)[Size]);
  /// Writes a fixed array of values
  /// Returns the number of bits written if successful, else 0
  template<typename T>
  Bits Write(const T* array, size_t size);
  /// Writes a UintN value
  /// Returns the number of bits written if successful, else 0
  template <Bits N, bool WrapAware>
  Bits Write(UintN<N, WrapAware> value);
  /// Writes a user-defined value
  /// Returns the number of bits written if successful, else 0
  template<typename T>
  R_ENABLE_IF(!is_scalar<T>::value, Bits) Write(const T& value);

  /// Writes an enum value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits written if successful, else 0
  template<typename T, typename R>
  R_ENABLE_IF(is_enum<T>::value && is_enum_or_integral<R>::value, Bits) WriteQuantized(T value_, R minValue_, R maxValue_, R quantum_ = R(1));
  /// Writes an integral value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits written if successful, else 0
  template<typename T, typename R>
  R_ENABLE_IF(is_integral<T>::value && is_integral<R>::value, Bits) WriteQuantized(T value_, R minValue_, R maxValue_, R quantum_ = R(1));
  /// Writes a floating-point value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits written if successful, else 0
  template<typename T, typename R>
  R_ENABLE_IF(is_floating_point<T>::value, Bits) WriteQuantized(T value_, R minValue_, R maxValue_, R quantum_ = R(0.0001));
  /// Writes a UintN value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits written if successful, else 0
  template <Bits N, bool WrapAware, typename R>
  R_ENABLE_IF(is_enum_or_integral<R>::value, Bits) WriteQuantized(UintN<N, WrapAware> value_, R minValue_, R maxValue_, R quantum_ = R(1));
  /// Writes a user-defined value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits written if successful, else 0
  template<typename T, typename R>
  R_ENABLE_IF(!is_scalar<T>::value, Bits) WriteQuantized(const T& value_, const R& minValue_, const R& maxValue_, const R& quantum_);

  /// Writes until a byte boundary is reached
  /// Returns the number of bits written
  Bits WriteUntilByteAligned();

  /// Appends to the back of the BitStream (Writes up to dataBits, Read from specified bitStream)
  /// Returns the number of bits appended
  Bits Append(const BitStream& bitStream, Bits dataBits);
  Bits Append(const BitStream& bitStream);

  /// Appends the entirety of the value BitStream to back of this BitStream
  /// Returns the number of bits appended
  Bits AppendAll(const BitStream& value);

  /// Clears this BitStream and appends the unread remainder of the value BitStream
  /// Returns the number of bits appended
  Bits AssignRemainder(const BitStream& value);

  /// Trims the front of the BitStream (Writes up to dataBits bits to a copy, Read from this bitStream, and overwrites this bitStream with the copy)
  /// Returns the number of bits trimmed
  Bits TrimFront(Bits dataBits);
  Bits TrimFront();

  // /// Unwrites the specified number of bits
  // void Unwrite(Bits bitsToUnwrite);

  /// Sets the number of bits written
  void SetBitsWritten(Bits bitsWritten);
  /// Sets the number of bytes written
  void SetBytesWritten(Bytes bytesWritten);
  /// Clears the number of bits written
  void ClearBitsWritten();

  //
  // Read Operations
  //

  /// Reads a single bit
  /// Returns the number of bits read if successful, else 0
  Bits ReadBit(bool& value) const;
  /// Reads multiple bits
  /// Returns the number of bits read if successful, else 0
  Bits ReadBits(byte* data, Bits dataBits) const;

  /// Reads a single byte
  /// Returns the number of bits read if successful, else 0
  Bits ReadByte(uint8& value) const;
  /// Reads multiple bytes
  /// Returns the number of bits read if successful, else 0
  Bits ReadBytes(byte* data, Bytes dataBytes) const;

  /// Reads a boolean value
  /// Returns the number of bits read if successful, else 0
  inline Bits Read(bool& value) const;
  /// Reads a string value
  /// Returns the number of bits read if successful, else 0
  Bits Read(String& value) const;

  /// Reads an enum value
  /// Returns the number of bits read if successful, else 0
  template<typename T>
  R_ENABLE_IF(is_enum<T>::value, Bits) Read(T& value) const;
  /// Reads an integral value
  /// Returns the number of bits read if successful, else 0
  template<typename T>
  R_ENABLE_IF(is_integral<T>::value, Bits) Read(T& value) const;
  /// Reads a floating-point value
  /// Returns the number of bits read if successful, else 0
  template<typename T>
  R_ENABLE_IF(is_floating_point<T>::value, Bits) Read(T& value) const;
  /// Reads a fixed array of values
  /// Returns the number of bits read if successful, else 0
  template<typename T, size_t Size>
  Bits Read(T (& array)[Size]);
  /// Reads a fixed array of values
  /// Returns the number of bits read if successful, else 0
  template<typename T>
  Bits Read(T* array, size_t size);
  /// Reads a UintN value
  /// Returns the number of bits read if successful, else 0
  template <Bits N, bool WrapAware>
  Bits Read(UintN<N, WrapAware>& value) const;
  /// Reads a user-defined value
  /// Returns the number of bits read if successful, else 0
  template<typename T>
  R_ENABLE_IF(!is_scalar<T>::value, Bits) Read(T& value) const;

  /// Reads an enum value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits read if successful, else 0
  template<typename T, typename R>
  R_ENABLE_IF(is_enum<T>::value && is_enum_or_integral<R>::value, Bits) ReadQuantized(T& value_, R minValue_, R maxValue_, R quantum_ = R(1)) const;
  /// Reads an integral value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits read if successful, else 0
  template<typename T, typename R>
  R_ENABLE_IF(is_integral<T>::value && is_integral<R>::value, Bits) ReadQuantized(T& value_, R minValue_, R maxValue_, R quantum_ = R(1)) const;
  /// Reads a floating-point value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits read if successful, else 0
  template<typename T, typename R>
  R_ENABLE_IF(is_floating_point<T>::value, Bits) ReadQuantized(T& value_, R minValue_, R maxValue_, R quantum_ = R(0.0001)) const;
  /// Reads a UintN value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits read if successful, else 0
  template <Bits N, bool WrapAware, typename R>
  R_ENABLE_IF(is_enum_or_integral<R>::value, Bits) ReadQuantized(UintN<N, WrapAware>& value_, R minValue_, R maxValue_, R quantum_ = R(1)) const;
  /// Reads a user-defined value bound within an inclusive range discretized to the nearest quantum interval value (using only the bits necessary to represent all possible values)
  /// Returns the number of bits read if successful, else 0
  template<typename T, typename R>
  R_ENABLE_IF(!is_scalar<T>::value, Bits) ReadQuantized(T& value_, const R& minValue_, const R& maxValue_, const R& quantum_) const;

  /// Returns the byte length of the string to be read next (including null terminator), else 0
  inline Bytes PeekStringBytes() const;

  /// Reads until a byte boundary is reached
  /// Returns the number of bits read
  inline Bits ReadUntilByteAligned() const;

  /// Unreads the specified number of bits
  inline void Unread(Bits bitsToUnread) const;

  /// Sets the number of bits read
  inline void SetBitsRead(Bits bitsRead) const;
  /// Sets the number of bytes read
  inline void SetBytesRead(Bytes bytesRead) const;
  /// Clears the number of bits read
  inline void ClearBitsRead() const;

protected:
  //
  // Helper Functions
  //

  /// Initializes the BitStream
  void Initialize();
  /// Reallocates if necessary to fit the additional bits
  void ReallocateIfNecessary(Bits additionalBits);
  /// Reallocates to the specified capacity, copying data if copyData is enabled
  void Reallocate(Bytes capacity, bool copyData);

  /// Binary data array
  byte*                      mData;
  /// Binary data capacity
  Bytes                      mByteCapacity;
  /// Also next write position
  Bits                       mBitsWritten;
  /// Also next read position
  mutable Bits               mBitsRead;
  /// Internal bit alignment policy
  mutable BitAlignment::Enum mAlignment;
};

/// Returns value as a binary string
template<typename T>
String GetBinaryString(const T& value);

/// Returns the BitStream as a binary string
/// Note: Resets the read cursor
String GetBinaryString(const BitStream& bitStream, Bytes bytesPerLine = 8);

} // namespace Zero

// Includes
#include "BitStream.inl"
