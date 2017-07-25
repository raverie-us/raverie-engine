/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_STREAM_HPP
#define ZILCH_STREAM_HPP

namespace Zilch
{
  class IStreamClass;
  class AsciiEncoding;
  class Utf8Encoding;

  /// Lets us query what a stream is capable of doing before we try it
  /// Otherwise we'll cause an exception to be thrown for doing something invalid
  namespace StreamCapabilities
  {
    enum Enum
    {
      None      = 0,
      Read      = 1,
      Write     = 2,
      Seek      = 4,
      GetCount  = 8,
      SetCount  = 16,

      /// All enums bound to Zilch must be of Integer size, so force it on all platforms
      ForceIntegerSize = 0x7FFFFFFF
    };
  }

  /// When we seek in a stream, we can specifiy to seek from the start, end, etc
  // Note: This must match up to the Zero::FileOrigin
  namespace StreamOrigin
  {
    enum Enum
    {
      Current,
      Start,
      End,

      /// All enums bound to Zilch must be of Integer size, so force it on all platforms
      ForceIntegerSize = 0x7FFFFFFF
    };
  }

  /// An encoding is a generic interface for writing out strings to any format (wide character, Utf16, etc)
  class ZeroShared IEncoding
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    /// Ascii encoding will strip wide characters and turn them into spaces
    /// All characters that fit within a single byte will be directly written to the stream
    static AsciiEncoding& GetAscii();

    /// Writes every rune to the file encoded in Utf8 form
    /// (the Zilch string is already in UTF8 form, so it would be written directly)
    static Utf8Encoding& GetUtf8();

    /// Write whatever bytes are needed to represent a single rune
    /// Returns the number of bytes written to the stream
    virtual Integer Write(Rune rune, IStreamClass& stream);

    /// Read whatever bytes are needed to represent a single rune
    /// If the stream Read or ReadByte returns that it read no data, this will return the null Rune
    virtual Rune Read(IStreamClass& stream);
  };
  
  /// Ascii encoding will strip wide characters and turn them into spaces
  /// All characters that fit within a single byte will be directly written to the stream
  class ZeroShared AsciiEncoding : public IEncoding
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);
    Integer Write(Rune rune, IStreamClass& stream) override;
    Rune Read(IStreamClass& stream) override;
  };
  
  /// Ascii encoding will strip wide characters and turn them into spaces
  /// All characters that fit within a single byte will be directly written to the stream
  class ZeroShared Utf8Encoding : public IEncoding
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);
    Integer Write(Rune rune, IStreamClass& stream) override;
    Rune Read(IStreamClass& stream) override;
  };

  /// A generic interface for reading and writing data to a stream (file, network, etc)
  class ZeroShared IStreamClass
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    /// Lets us query what a stream is capable of doing before we try it
    virtual StreamCapabilities::Enum GetCapabilities();

    /// Get where we are in the stream relative to the beginning, in bytes
    /// Note: The setter is non-virtual, and requires the Seek capability
    virtual DoubleInteger GetPosition();

    /// Gets or sets how long our stream is in bytes
    /// If the stream is capable of getting the count but it fails then it will return -1
    /// Check the stream capabilities for both GetCount and SetCount before reading or writing to this value
    virtual DoubleInteger GetCount();
    virtual void SetCount(DoubleInteger count);

    /// Moves the stream to a given position (relative to an origin)
    /// Returns true if the seek fully succeeded, or false otherwise
    /// Check the stream capabilities for Seek before calling this function
    virtual bool Seek(DoubleInteger position, StreamOrigin::Enum origin);

    /// Writes an amount of data to the stream and returns the amount that was actually written
    /// This will throw if the byteStart and byteCount exceeds the array size
    /// Check the stream capabilities for Write before calling this function
    virtual Integer Write(ArrayClass<Byte>& data, Integer arrayByteStart, Integer arrayByteCount);

    /// Writes a single byte of data to the stream (should be implemented for efficiency)
    /// Returns 1 if the single byte was written, or 0
    /// Check the stream capabilities for Write before calling this function
    virtual Integer WriteByte(Byte byte);

    /// Reads data from the stream into an array
    /// If the array is not large enough to store the data at the start/with the given count, it will be resized
    /// Check the stream capabilities for Read before calling this function
    virtual Integer Read(ArrayClass<Byte>& data, Integer arrayByteStart, Integer arrayByteCount);

    /// Reads a single byte of data or returns -1 if the byte could not be read
    /// Check the stream capabilities for Write before calling this function
    virtual Integer ReadByte();

    /// Some streams will buffer data before writing it out to whatever hardware device it targets
    /// This is an optimization because memory is generally much faster, however, if you want to force the
    /// stream to write out all its data, then call this function
    virtual void Flush();

    // Extensions below (non virtual functions that are just helpers)

    /// If the flag 'resize' is true, then we will attempt to resize the array to fit the data
    /// Otherwise we will throw exceptions for ranges that go outside the array
    /// Invalid values will always cause exceptions to be thrown (such as a negative value)
    /// Returns true if it succeeds (or resizes) or false if it throws an exception
    static bool ValidateArray(ArrayClass<Byte>& data, Integer byteStart, Integer byteCount, bool resizeArrayIfNeeded);
    
    /// Directly set the position where we are in the stream (always from the beginning, in bytes)
    /// Invokes Seek, and if it fails it will attempt to clamp the position either to 0 or to the end of the stream and Seek again
    /// Check the stream capabilities for Seek before setting the position
    void SetPosition(DoubleInteger position);

    /// Writes an entire array of bytes to the stream and returns the amount that was actually written
    /// Check the stream capabilities for Write before calling this function
    Integer Write(ArrayClass<Byte>& data);

    /// Writes a rune to the stream
    /// Returns the number of bytes that were written to the stream
    /// Check the stream capabilities for Write before calling this function
    Integer WriteRune(Rune rune, IEncoding& destinationStreamEncoding);

    /// Writes a rune to the stream
    /// Returns the number of bytes that were written to the stream
    /// Check the stream capabilities for Write before calling this function
    /// Assumes Utf8 encoding
    Integer WriteRune(Rune rune);

    /// Writes a string range to the stream with a specified encoding (IEncoding.Ascii, etc)
    /// Returns the number of bytes that were written to the stream
    /// This function will not consume the string range
    /// Check the stream capabilities for Write before calling this function
    Integer WriteText(StringParam text, IEncoding& destinationStreamEncoding);

    /// Same as the above WriteText, but assumes Utf8 encoding
    Integer WriteText(StringParam text);

    /// Reads a single rune from the file using the encoding
    Rune ReadRune(IEncoding& destinationStreamEncoding);
    
    /// Reads a single rune assumign the encoding is Utf8
    Rune ReadRune();

    // Reads data from the stream and returns it into an array
    // The length of the array will indicate how much was actually read
    // Check the stream capabilities for Read before calling this function
    // Note: The memory is allocated within Zilch's Heap Manager and must be freed after using ObjectToHandle
    // This function is not enabled until we fix binding being able to return Handle<ArrayClass<Byte> >
    //ArrayClass<Byte>* Read(Integer byteCount);

    /// Reads bytes and converts them into runes via the specified encoding until we reach the '\n', '\0', or the end of the stream
    /// Note: '\r' is not used as a line terminator because that format is out-dated
    /// Read line always returns the line including the '\n' at the end
    /// If a line does include a '\n' at the end, it means we reached the end of the stream (or an error occurred)
    /// An empty string will be returned if ReadLine is called again after it already reached the end of the stream
    String ReadLine(IEncoding& sourceStreamEncoding);
    
    /// Same as the above ReadLine, but assumes Utf8 encoding
    String ReadLine();
    
    /// Reads bytes and converts them into runes via the specified encoding until we reach '\0' or the end of the stream
    String ReadAllText(IEncoding& sourceStreamEncoding);

    /// Same as the above ReadAllText, but assumes Utf8 encoding
    String ReadAllText();
  };
}

#endif
