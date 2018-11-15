///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                  BitStream                                      //
//---------------------------------------------------------------------------------//

BitStream::BitStream()
{
  Initialize();
}
BitStream::BitStream(const BitStream& rhs)
  : mData(rhs.mByteCapacity ? new byte[rhs.mByteCapacity] : nullptr),
    mByteCapacity(rhs.mByteCapacity),
    mBitsWritten(rhs.mBitsWritten),
    mBitsRead(rhs.mBitsRead),
    mAlignment(rhs.mAlignment)
{
  if(mData)
    memcpy(mData, rhs.mData, rhs.mByteCapacity);
}
BitStream::BitStream(MoveReference<BitStream> rhs)
  : mData(rhs->mData),
    mByteCapacity(rhs->mByteCapacity),
    mBitsWritten(rhs->mBitsWritten),
    mBitsRead(rhs->mBitsRead),
    mAlignment(rhs->mAlignment)
{
  rhs->Initialize();
}

BitStream::~BitStream()
{
  Clear(true);
}

BitStream& BitStream::operator =(const BitStream& rhs)
{
  if(this != &rhs)
  {
    // Capacity too small to fit data?
    if(mByteCapacity < rhs.mByteCapacity)
      Reallocate(rhs.mByteCapacity, false);

    memcpy(mData, rhs.mData, rhs.mByteCapacity);
    mBitsWritten = rhs.mBitsWritten;
    mBitsRead    = rhs.mBitsRead;
    mAlignment   = rhs.mAlignment;
  }
  return *this;
}
BitStream& BitStream::operator =(MoveReference<BitStream> rhs)
{
  if(this != &rhs)
  {
    Clear(true);

    mData         = rhs->mData;
    mByteCapacity = rhs->mByteCapacity;
    mBitsWritten  = rhs->mBitsWritten;
    mBitsRead     = rhs->mBitsRead;
    mAlignment    = rhs->mAlignment;

    rhs->Initialize();
  }
  return *this;
}

bool BitStream::operator ==(const BitStream& rhs) const
{
  return (mByteCapacity == rhs.mByteCapacity)
      && memcmp(this->mData, rhs.mData, mByteCapacity) == 0;
}
bool BitStream::operator !=(const BitStream& rhs) const
{
  return !(*this == rhs);
}
bool BitStream::operator  <(const BitStream& rhs) const
{
  return (mByteCapacity < rhs.mByteCapacity)
      || memcmp(this->mData, rhs.mData, mByteCapacity) < 0;
}

//
// Member Functions
//

void BitStream::Reserve(Bytes capacity)
{
  // Additional space requested?
  if(capacity > mByteCapacity)
    Reallocate(capacity, true);
}

void BitStream::Clear(bool freeMemory)
{
  if(freeMemory)
  {
    // Free memory
    if(mData)
      delete [] mData;
    Initialize();
  }
  else
  {
    // Zero memory
    if(mData)
      memset(mData, 0, mByteCapacity);
    mBitsWritten = 0;
    mBitsRead    = 0;
    mAlignment   = BitAlignment::Bit;
  }
}

//
// Write Operations
//

Bits BitStream::WriteBit(bool value)
{
  // Ensure there's enough space before writing
  ReallocateIfNecessary(1);

  // Get full bytes and remaining bits written
  Bytes fullBytesWritten = DIV8(mBitsWritten);
  Bits  remBitsWritten   = MOD8(mBitsWritten);

  // Write value at cursor
  byte* writeCursor = mData + fullBytesWritten;
  ASSIGN_LBIT(value, writeCursor, remBitsWritten);

  ++mBitsWritten;

  // Success
  return 1;
}
Bits BitStream::WriteBits(const byte* data, Bits dataBits)
{
  // There must be data to write
  Assert(data && dataBits);
  // Data size must not exceed maximum possible BitStream size
  Assert(dataBits <= BYTES_TO_BITS(BITSTREAM_MAX_BYTES));
  // Data must not overlap with internal BitStream memory
  Assert(!MemoryIsOverlapping(mData, mByteCapacity, data, BITS_TO_BYTES(dataBits)));

  // Byte Alignment?
  if(mAlignment == BitAlignment::Byte)
  {
    // Ensure we are byte aligned before continuing
    WriteUntilByteAligned();

    // Ensure the data is byte aligned before continuing
    Bits remDataBits = MOD8(dataBits);
    if(remDataBits) // Not byte aligned?
    {
      // Byte align data
      // (We simply end up writing extra bits from the data to write another full byte)
      dataBits += (8 - remDataBits);

      // Data should now be byte aligned
      Assert(!MOD8(dataBits));
    }
  }

  // Ensure there is enough space before continuing
  ReallocateIfNecessary(dataBits);

  // Get full bytes and remaining bits written
  Bytes fullBytesWritten       = DIV8(mBitsWritten);
  Bits  remBitsWritten         = MOD8(mBitsWritten);
  Bytes fullDataBytes          = DIV8(dataBits);
  Bits  remDataBits            = MOD8(dataBits);
  byte* writeCursor            = mData + fullBytesWritten;
  byte* dataCursor             = (byte*)data;
  bool  writeCursorByteAligned = !remBitsWritten;

  Assert(BYTES_TO_BITS(fullBytesWritten) + remBitsWritten == mBitsWritten
      && BYTES_TO_BITS(fullDataBytes)    + remDataBits    == dataBits);

  // Write full data bytes first (if any)
  if(writeCursorByteAligned && fullDataBytes)
  {
    // Byte aligned?
    // Write bytes using memcpy
    memcpy(writeCursor, dataCursor, fullDataBytes);
    dataCursor       += fullDataBytes;
    fullBytesWritten += fullDataBytes;
    writeCursor      += fullDataBytes;
  }
  else
  {
    // Not byte aligned?
    // Write bytes manually
    for(Bytes k = 0; k < fullDataBytes; ++k)
    {
      //Write a data byte
      for(Bits i = 0; i < 8; ++i)
      {
        // Write value at cursor
        ASSIGN_LBIT(*dataCursor & LBIT(i), writeCursor, remBitsWritten);

        ++remBitsWritten;

        // Write cursor reached a byte boundary?
        if(remBitsWritten == 8)
        {
          remBitsWritten = 0;
          ++fullBytesWritten;
          ++writeCursor;
        }
      }

      ++dataCursor;
    }
  }

  // Write remaining data bits second (if any)
  for(Bits i = 0; i < remDataBits; ++i)
  {
    // Write value at cursor
    ASSIGN_LBIT(*dataCursor & LBIT(i), writeCursor, remBitsWritten);

    ++remBitsWritten;

    // Write cursor reached a byte boundary?
    if(remBitsWritten == 8)
    {
      remBitsWritten = 0;
      ++fullBytesWritten;
      ++writeCursor;
    }
  }

  mBitsWritten += dataBits;

  // Success
  return dataBits;
}

Bits BitStream::WriteByte(uint8 value)
{
  return WriteBits((byte*)&value, BYTES_TO_BITS(1));
}
Bits BitStream::WriteBytes(const byte* data, Bytes dataBytes)
{
  return WriteBits(data, BYTES_TO_BITS(dataBytes));
}

Bits BitStream::Write(const String& value)
{
  return WriteBits((const byte*)value.c_str(), BYTES_TO_BITS(value.SizeInBytes() + 1));
}

Bits BitStream::WriteUntilByteAligned()
{
  // Not byte aligned?
  Bits remBitsWritten = MOD8(mBitsWritten);
  if(remBitsWritten)
  {
    // Write pad bits
    Bits padBits = 8 - remBitsWritten;
    for(Bits i = 0; i < padBits; ++i)
      WriteBit(false);

    // Write cursor should now be byte aligned
    Assert(!MOD8(mBitsWritten));

    // Success
    return padBits;
  }

  // Success
  return 0;
}

Bits BitStream::Append(const BitStream& bitStream, Bits dataBits)
{
  // Reserve space as necessary
  ReallocateIfNecessary(dataBits);

  // Append up to unread bits available
  dataBits = std::min(dataBits, bitStream.GetBitsUnread());
  if(dataBits == 0)
    return 0;
  Bits remAppendBits = dataBits;

  // Append non-byte-aligned bits (if any)
  Bits remBitsRead = MOD8(bitStream.mBitsRead);
  if(remBitsRead) // Not byte aligned?
  {
    // Read/Write bits
    bool value;
    Bits remBits = 8 - remBitsRead;
    for(Bits i = 0; i < remBits && remAppendBits; ++i, --remAppendBits)
    {
      // Read bit value from other bitstream
      if(!bitStream.ReadBit(value)) // Unable?
      {
        // Failure
        Unread(i);
        return 0;
      }

      // Write bit value to our bitstream
      WriteBit(value);
    }
  }

  // Append byte-aligned bits (if any)
  if(remAppendBits)
  {
    // Read cursor should now be byte aligned
    Assert(!MOD8(bitStream.mBitsRead));

    // Write remaining bits from their bitstream to ours
    WriteBits(bitStream.GetData() + DIV8(bitStream.mBitsRead), remAppendBits);
    bitStream.mBitsRead += remAppendBits; // Advance other bitstream's read cursor
    Assert(bitStream.mBitsRead <= bitStream.mBitsWritten);
  }

  // Success
  return dataBits;
}

Bits BitStream::TrimFront(Bits dataBits)
{
  Bits originalSize = GetBitsWritten();

  // Create trimmed copy and overwrite this
  BitStream copy;
  copy.Append(*this, dataBits);
  *this = ZeroMove(copy);

  // Success
  Assert(originalSize >= GetBitsWritten());
  return originalSize - GetBitsWritten();
}

//
// Read Operations
//

Bits BitStream::ReadBit(bool& value) const
{
  // No more bits available to read?
  if(!GetBitsUnread())
    return 0; // Failure

  // Get full bytes and remaining bits read
  Bytes fullBytesRead = DIV8(mBitsRead);
  Bits  remBitsRead   = MOD8(mBitsRead);

  // Read value at cursor
  const byte* readCursor = mData + fullBytesRead;
  value = *readCursor & LBIT(remBitsRead) ? true : false;

  ++mBitsRead;

  // Success
  return 1;
}
Bits BitStream::ReadBits(byte* data, Bits dataBits) const
{
  // There must be data to read
  Assert(data && dataBits);
  // Data size must not exceed maximum possible BitStream size
  Assert(dataBits <= BYTES_TO_BITS(BITSTREAM_MAX_BYTES));
  // Data must not overlap with internal BitStream memory
  Assert(!MemoryIsOverlapping(mData, mByteCapacity, data, BITS_TO_BYTES(dataBits)));

  // Byte Alignment?
  Bits bitsRead1 = 0;
  if(mAlignment == BitAlignment::Byte)
  {
    // Ensure we are byte aligned before continuing
    bitsRead1 = ReadUntilByteAligned();

    // Ensure the data is byte aligned before continuing
    Bits remDataBits = MOD8(dataBits);
    if(remDataBits) // Not byte aligned?
    {
      // Byte align data
      // (We simply end up reading extra bits from the data to read another full byte)
      dataBits += (8 - remDataBits);

      // Data should now be byte aligned
      Assert(!MOD8(dataBits));
    }
  }

  // Ensure there is enough unread data before continuing
  if(dataBits > GetBitsUnread())
  {
    // Failure
    Unread(bitsRead1);
    return 0;
  }

  // Zero data memory
  memset(data, 0, BITS_TO_BYTES(dataBits));

  // Get full bytes and remaining bits read
  Bytes fullBytesRead         = DIV8(mBitsRead);
  Bits  remBitsRead           = MOD8(mBitsRead);
  Bytes fullDataBytes         = DIV8(dataBits);
  Bits  remDataBits           = MOD8(dataBits);
  byte* readCursor            = mData + fullBytesRead;
  byte* dataCursor            = data;
  bool  readCursorByteAligned = !remBitsRead;

  Assert(BYTES_TO_BITS(fullBytesRead) + remBitsRead == mBitsRead
      && BYTES_TO_BITS(fullDataBytes) + remDataBits == dataBits);

  // Read full data bytes first (if any)
  if(readCursorByteAligned && fullDataBytes)
  {
    // Byte aligned?
    // Read bytes using memcpy
    memcpy(dataCursor, readCursor, fullDataBytes);
    dataCursor    += fullDataBytes;
    fullBytesRead += fullDataBytes;
    readCursor    += fullDataBytes;
  }
  else
  {
    // Not byte aligned?
    // Read bytes manually
    for(Bytes k = 0; k < fullDataBytes; ++k)
    {
      // Read a data byte
      for(Bits i = 0; i < 8; ++i)
      {
        // Write value at data cursor
        ASSIGN_LBIT(*readCursor & LBIT(remBitsRead), dataCursor, i);

        ++remBitsRead;

        // Read cursor reached a byte boundary?
        if(remBitsRead == 8)
        {
          remBitsRead = 0;
          ++fullBytesRead;
          ++readCursor;
        }
      }

      ++dataCursor;
    }
  }

  // Read remaining data bits second (if any)
  for(Bits i = 0; i < remDataBits; ++i)
  {
    // Write value at data cursor
    ASSIGN_LBIT(*readCursor & LBIT(remBitsRead), dataCursor, i);

    ++remBitsRead;

    // Read cursor reached a byte boundary?
    if(remBitsRead == 8)
    {
      remBitsRead = 0;
      ++fullBytesRead;
      ++readCursor;
    }
  }

  mBitsRead += dataBits;

  // Success
  return dataBits;
}

Bits BitStream::ReadByte(uint8& value) const
{
  return ReadBits((byte*)&value, BYTES_TO_BITS(1));
}
Bits BitStream::ReadBytes(byte* data, Bytes dataBytes) const
{
  return ReadBits(data, BYTES_TO_BITS(dataBytes));
}

Bits BitStream::Read(String& value) const
{
  // Peek string size
  Bytes stringBytes = PeekStringBytes();
  if(!stringBytes) // Unable?
  {
    // Failure
    return 0;
  }

  // Ignore NULL terminator for String
  --stringBytes;

  // Non-empty string?
  Bits bitsRead1 = 0;
  if(stringBytes)
  {
    // Allocate string node to write string into
    StringNode* node = String::AllocateNode(stringBytes);

    // Read string
    bitsRead1 = ReadBits((byte*)node->Data, BYTES_TO_BITS(stringBytes));
    if(!bitsRead1)
    {
      // Failure
      node->release();
      return 0;
    }

    // Assign output string
    value = String(node);
  }
  else
  {
    // Clear output string
    value.Clear();
  }

  // Skip NULL terminator for String
  Bits bitsRead2 = BYTES_TO_BITS(1);
  mBitsRead += bitsRead2;

  // Success
  return bitsRead1 + bitsRead2;
}

Bytes BitStream::PeekStringBytes() const
{
  // Store original read cursor
  Bits origBitsRead = mBitsRead;

  // Read bytes until NULL
  uint8 value;
  Bytes stringBytes = 0;
  do
  {
    // Unable?
    if(!ReadByte(value))
      return 0; // Failure

    ++stringBytes;

  } while(value);

  // Restore original read cursor
  mBitsRead = origBitsRead;

  // Success
  return stringBytes;
}

Bits BitStream::ReadUntilByteAligned() const
{
  // Not byte aligned?
  Bits remBitsRead = MOD8(mBitsRead);
  if(remBitsRead)
  {
    // Read pad bits
    bool padValue;
    Bits padBits = 8 - remBitsRead;
    for(Bits i = 0; i < padBits; ++i)
    {
      if(!ReadBit(padValue)) // Unable?
      {
        // Failure
        Unread(i);
        return 0;
      }
    }

    // Read cursor should now be byte aligned
    Assert(!MOD8(mBitsRead));

    // Success
    return padBits;
  }

  // Success
  return 0;
}

//
// Helper Functions
//

void BitStream::Initialize()
{
  mData         = nullptr;
  mByteCapacity = 0;
  mBitsWritten  = 0;
  mBitsRead     = 0;
  mAlignment    = BitAlignment::Bit;
}
void BitStream::ReallocateIfNecessary(Bits additionalBits)
{
  // Need to reallocate to fit the additional bits?
  if(BYTES_TO_BITS(mByteCapacity) < mBitsWritten + additionalBits)
  {
    const Bytes growSize = (Bytes)(mByteCapacity * 2) + BITS_TO_BYTES(additionalBits);

    // First allocation?
    if(!mByteCapacity)
      Reallocate(std::max(growSize, (Bytes)BITSTREAM_DEFAULT_RESERVE_BYTES), true); // Use whichever is larger
    else
      Reallocate(growSize, true);
  }
}
void BitStream::Reallocate(Bytes capacity, bool copyData)
{
  Assert(capacity <= BITSTREAM_MAX_BYTES);

  byte* temp         = mData;
  Bytes tempCapacity = GetByteCapacity();
  mData              = new byte[capacity];
  mByteCapacity      = capacity;

  Assert(mByteCapacity > tempCapacity);

  // First allocation?
  if(!temp)
    memset(mData, 0, mByteCapacity); // Zero memory
  // Copying data?
  else if(copyData)
  {
    memcpy(mData, temp, tempCapacity);                             // Copy previous memory space
    memset(mData + tempCapacity, 0, mByteCapacity - tempCapacity); // Zero the rest
  }

  if(temp)
    delete [] temp;
}

String GetBinaryString(const BitStream& bitStream, Bytes bytesPerLine)
{
  StringBuilder result;
  bitStream.ClearBitsRead();

  bool bitSet;
  Bits  bitCount  = 0;
  Bytes byteCount = 0;
  while(bitStream.Read(bitSet))
  {
    // Space every 8 bits
    if(bitCount == 8)
    {
      bitCount = 0;
      ++byteCount;

      // BytesPerLine enabled and has been reached?
      if(bytesPerLine && byteCount == bytesPerLine)
      {
        // Newline
        result += '\n';
        byteCount = 0;
      }
      else // Space
        result += ' ';
    }

    // Output the bit
    result += bitSet ? '1' : '0';
    ++bitCount;
  }

  bitStream.ClearBitsRead();
  return result.ToString();
}

} // namespace Zero
