/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  ZilchDefineExternalBaseType(StreamCapabilities::Enum, TypeCopyMode::ValueType, builder, type)
  {
    ZilchFullBindEnum(builder, type, SpecialType::Flags);
    ZilchFullBindEnumValue(builder, type, StreamCapabilities::None,     "None");
    ZilchFullBindEnumValue(builder, type, StreamCapabilities::Read,     "Read");
    ZilchFullBindEnumValue(builder, type, StreamCapabilities::Write,    "Write");
    ZilchFullBindEnumValue(builder, type, StreamCapabilities::Seek,     "Seek");
    ZilchFullBindEnumValue(builder, type, StreamCapabilities::GetCount, "GetCount");
    ZilchFullBindEnumValue(builder, type, StreamCapabilities::SetCount, "SetCount");
  }

  //***************************************************************************
  ZilchDefineExternalBaseType(StreamOrigin::Enum, TypeCopyMode::ValueType, builder, type)
  {
    ZilchFullBindEnum(builder, type, SpecialType::Enumeration);
    ZilchFullBindEnumValue(builder, type, StreamOrigin::Start,    "Start");
    ZilchFullBindEnumValue(builder, type, StreamOrigin::Current,  "Current");
    ZilchFullBindEnumValue(builder, type, StreamOrigin::End,      "End");
  }

  //***************************************************************************
  ZilchDefineType(IEncoding, builder, type)
  {
    type->HandleManager = ZilchManagerId(PointerManager);

    // Even though this is an interface, because it is native, it must have a constructor that can be implemented
    ZilchFullBindDestructor(builder, type, IEncoding);
    ZilchFullBindConstructor(builder, type, IEncoding, nullptr);

    ZilchFullBindMethod(builder, type, &IEncoding::Write, ZilchNoOverload, "Write", nullptr)->IsVirtual = true;
    ZilchFullBindMethod(builder, type, &IEncoding::Read,  ZilchNoOverload, "Read",  nullptr)->IsVirtual = true;

    ZilchFullBindGetterSetter(builder, type, &IEncoding::GetAscii, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "Ascii");
    ZilchFullBindGetterSetter(builder, type, &IEncoding::GetUtf8, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "Utf8");
  }

  //***************************************************************************
  ZilchDefineType(AsciiEncoding, builder, type)
  {
  }

  //***************************************************************************
  ZilchDefineType(Utf8Encoding, builder, type)
  {
  }

  //***************************************************************************
  ZilchDefineType(IStreamClass, builder, type)
  {
    // Even though this is an interface, because it is native, it must have a constructor that can be implemented
    ZilchFullBindDestructor(builder, type, IStreamClass);
    ZilchFullBindConstructor(builder, type, IStreamClass, nullptr);

    ZilchFullBindGetterSetter(builder, type, &IStreamClass::GetCapabilities, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "Capabilities");
    ZilchFullBindGetterSetter(builder, type, &IStreamClass::GetPosition, ZilchNoOverload, &IStreamClass::SetPosition, ZilchNoOverload, "Position");
    ZilchFullBindGetterSetter(builder, type, &IStreamClass::GetCount, ZilchNoOverload, &IStreamClass::SetCount, ZilchNoOverload, "Count");
    
    ZilchFullBindMethod(builder, type, &IStreamClass::Seek, ZilchNoOverload, "Seek", "position, origin")->IsVirtual = true;
    ZilchFullBindMethod(builder, type, &IStreamClass::Write, (Integer (IStreamClass::*)(ArrayClass<Byte>&, Integer, Integer)), "Write", "data, byteStart, byteCount")->IsVirtual = true;
    ZilchFullBindMethod(builder, type, &IStreamClass::WriteByte, ZilchNoOverload, "WriteByte", nullptr)->IsVirtual = true;
    ZilchFullBindMethod(builder, type, &IStreamClass::Read, (Integer (IStreamClass::*)(ArrayClass<Byte>&, Integer, Integer)), "Read", "data, byteStart, byteCount")->IsVirtual = true;
    ZilchFullBindMethod(builder, type, &IStreamClass::ReadByte, ZilchNoOverload, "ReadByte", nullptr)->IsVirtual = true;
    ZilchFullBindMethod(builder, type, &IStreamClass::Flush, ZilchNoOverload, "Flush", nullptr)->IsVirtual = true;

    // Extensions (these are not virtual)
    ZilchFullBindMethod(builder, type, &IStreamClass::Write, (Integer (IStreamClass::*)(ArrayClass<Byte>&)), "Write", "data");
    ZilchFullBindMethod(builder, type, &IStreamClass::WriteText, (Integer (IStreamClass::*)(StringParam, IEncoding&)), "WriteText", "text, sourceStreamEncoding")->IsVirtual = true;
    ZilchFullBindMethod(builder, type, &IStreamClass::WriteText, (Integer (IStreamClass::*)(StringParam)), "WriteText", "text")->IsVirtual = true;
    //ZilchFullBindMethod(builder, type, &IStreamClass::Read, (ArrayClass<Byte> (IStreamClass::*)(Integer)), "Read", "byteCount");
    ZilchFullBindMethod(builder, type, &IStreamClass::ReadLine, (String (IStreamClass::*)(IEncoding&)), "ReadLine", nullptr)->IsVirtual = true;
    ZilchFullBindMethod(builder, type, &IStreamClass::ReadLine, (String (IStreamClass::*)()), "ReadLine", nullptr)->IsVirtual = true;
    ZilchFullBindMethod(builder, type, &IStreamClass::ReadAllText, (String (IStreamClass::*)(IEncoding&)), "ReadAllText", nullptr)->IsVirtual = true;
    ZilchFullBindMethod(builder, type, &IStreamClass::ReadAllText, (String (IStreamClass::*)()), "ReadAllText", nullptr)->IsVirtual = true;
  }

  //***************************************************************************
  AsciiEncoding& IEncoding::GetAscii()
  {
    static AsciiEncoding encoding;
    return encoding;
  }
  
  //***************************************************************************
  Utf8Encoding& IEncoding::GetUtf8()
  {
    static Utf8Encoding encoding;
    return encoding;
  }
  
  //***************************************************************************
  Integer IEncoding::Write(Rune rune, IStreamClass& stream)
  {
    ExecutableState::CallingState->ThrowNotImplementedException();
    return 0;
  }
  
  //***************************************************************************
  Rune IEncoding::Read(IStreamClass& stream)
  {
    ExecutableState::CallingState->ThrowNotImplementedException();
    return Rune();
  }

  //***************************************************************************
  Integer AsciiEncoding::Write(Rune rune, IStreamClass& stream)
  {
    if (rune.mValue.value > 255)
      return stream.WriteByte((Byte)'?');
    else
      return stream.WriteByte((Byte)rune.mValue.value);
  }

  //***************************************************************************
  Rune AsciiEncoding::Read(IStreamClass& stream)
  {
    // Read a single byte from the stream, if its not valid, then return null
    Integer byte = stream.ReadByte();
    if (byte == -1)
      return Rune();
    return Rune(byte);
  }
  
  //***************************************************************************
  Integer Utf8Encoding::Write(Rune rune, IStreamClass& stream)
  {
    byte utf8Bytes[4] = { 0 };
    size_t encodedByteCount = Zero::UTF8::UnpackUtf8RuneIntoBuffer(rune.mValue, utf8Bytes);

    for (size_t i = 0; i < encodedByteCount; ++i)
    {
      // If we fail to write out a byte, return the amount we've already written (works because of 0 based indexing)
      if (stream.WriteByte(utf8Bytes[i]) == 0)
        return i;
    }

    // We wrote the full rune out!
    return encodedByteCount;
  }

  //***************************************************************************
  Rune Utf8Encoding::Read(IStreamClass& stream)
  {
    // Read the first rune byte
    byte utf8Bytes[4] = { 0 };
    Integer firstByte = stream.ReadByte();
    if (firstByte == -1)
      return Rune();

    utf8Bytes[0] = (byte)firstByte;

    size_t encodedByteCount = Zero::UTF8::EncodedCodepointLength((byte)firstByte);

    // We start at 1 since we've already read the first byte
    for (size_t i = 1; i < encodedByteCount; ++i)
    {
      Integer readByte = stream.ReadByte();
      if (readByte == -1)
        return Rune();

      utf8Bytes[i] = (byte)readByte;
    }

    Rune rune = Zero::UTF8::ReadUtf8Rune(utf8Bytes);
    return rune;
  }
  
  //***************************************************************************
  StreamCapabilities::Enum IStreamClass::GetCapabilities()
  {
    return StreamCapabilities::None;
  }
  
  //***************************************************************************
  DoubleInteger IStreamClass::GetPosition()
  {
    ExecutableState::CallingState->ThrowNotImplementedException();
    return 0;
  }

  //***************************************************************************
  void IStreamClass::SetPosition(DoubleInteger position)
  {
    // Attempt to seek to the given position relative to the start
    if (this->Seek(position, StreamOrigin::Start) == false)
    {
      // We failed the first seek, check if the position was negative (if so, seek to the start of the stream)
      if (position < 0)
      {
        this->Seek(0, StreamOrigin::Start);
      }
      // Also check if the position was beyond the size of the stream
      // This is valid for some streams, however we already know the above seek failed
      // Clamp to the end of the stream
      else if (position > this->GetCount())
      {
        this->Seek(0, StreamOrigin::End);
      }
    }
  }
  
  //***************************************************************************
  DoubleInteger IStreamClass::GetCount()
  {
    if ((this->GetCapabilities() & StreamCapabilities::GetCount) == 0)
      ExecutableState::CallingState->ThrowException("This stream does not support the GetCount capability");
    return 0;
  }

  //***************************************************************************
  void IStreamClass::SetCount(DoubleInteger count)
  {
    if ((this->GetCapabilities() & StreamCapabilities::SetCount) == 0)
      ExecutableState::CallingState->ThrowException("This stream does not support the SetCount capability");
  }
  
  //***************************************************************************
  bool IStreamClass::Seek(DoubleInteger position, StreamOrigin::Enum origin)
  {
    if ((this->GetCapabilities() & StreamCapabilities::Seek) == 0)
      ExecutableState::CallingState->ThrowException("This stream does not support the Seek capability");
    return false;
  }
  
  //***************************************************************************
  Integer IStreamClass::Write(ArrayClass<Byte>& data, Integer byteStart, Integer byteCount)
  {
    if ((this->GetCapabilities() & StreamCapabilities::Write) == 0)
    {
      ExecutableState::CallingState->ThrowException("This stream does not support the Write capability");
      return 0;
    }
    
    IStreamClass::ValidateArray(data, byteStart, byteCount, false);
    return 0;
  }
  
  //***************************************************************************
  Integer IStreamClass::WriteByte(Byte byte)
  {
    if ((this->GetCapabilities() & StreamCapabilities::Write) == 0)
      ExecutableState::CallingState->ThrowException("This stream does not support the Write capability");
    return 0;
  }
  
  //***************************************************************************
  Integer IStreamClass::Read(ArrayClass<Byte>& data, Integer byteStart, Integer byteCount)
  {
    if ((this->GetCapabilities() & StreamCapabilities::Read) == 0)
    {
      ExecutableState::CallingState->ThrowException("This stream does not support the Read capability");
      return 0;
    }

    IStreamClass::ValidateArray(data, byteStart, byteCount, true);
    return 0;
  }
  
  //***************************************************************************
  Integer IStreamClass::ReadByte()
  {
    if ((this->GetCapabilities() & StreamCapabilities::Read) == 0)
      ExecutableState::CallingState->ThrowException("This stream does not support the Read capability");
    return 0;
  }

  //***************************************************************************
  void IStreamClass::Flush()
  {
  }
  
  //***************************************************************************
  bool IStreamClass::ValidateArray(ArrayClass<Byte>& data, Integer byteStart, Integer byteCount, bool resizeArrayIfNeeded)
  {
    // We don't allow a negative starting value
    if (byteStart < 0)
    {
      ExecutableState::CallingState->ThrowException("The parameter 'byteStart' cannot be negative");
      return false;
    }

    // We don't allow a negative count value
    if (byteCount < 0)
    {
      ExecutableState::CallingState->ThrowException("The parameter 'byteCount' cannot be negative");
      return false;
    }
    
    // Compute the end in bytes and get the current size of the array
    Integer byteEnd = byteStart + byteCount;
    Integer currentCount = (Integer)data.NativeArray.Size();

    // If the byte range goes outside the current array
    if (byteEnd > currentCount)
    {
      // If we allow resizing...
      if (resizeArrayIfNeeded)
      {
        // Resize the array to go all the way to the end, and then zero out the bytes
        data.NativeArray.Resize(byteEnd);
        memset(data.NativeArray.Data() + currentCount, 0, byteEnd - currentCount);
        data.Modified();
      }
      else
      {
        ExecutableState::CallingState->ThrowException("The byte range exceeds the size of the array");
        return false;
      }
    }

    // We didn't throw an exception, so it must be valid
    return true;
  }
  
  //***************************************************************************
  Integer IStreamClass::Write(ArrayClass<Byte>& data)
  {
    return this->Write(data, 0, data.NativeArray.Size());
  }

  //***************************************************************************
  Integer IStreamClass::WriteRune(Rune rune)
  {
    return IEncoding::GetUtf8().Write(rune, *this);
  }

  //***************************************************************************
  Integer IStreamClass::WriteRune(Rune rune, IEncoding& destinationStreamEncoding)
  {
    return destinationStreamEncoding.Write(rune, *this);
  }

  //***************************************************************************
  Rune IStreamClass::ReadRune()
  {
    if ((this->GetCapabilities() & StreamCapabilities::Read) == 0)
      ExecutableState::CallingState->ThrowException("This stream does not support the Read capability");
    return 0;
  }
  
  //***************************************************************************
  Integer IStreamClass::WriteText(StringParam text, IEncoding& destinationStreamEncoding)
  {
    Integer totalWritten = 0;

    // Technically this should be iterating through runes
    ZilchTodo("Unicode");
    StringRange range = text.All();
    ZilchForEach(Zero::Rune r, range)
    {
      // Use whatever encoding they passed in to write out the rune (should be virtual)
      Integer amountWritten = destinationStreamEncoding.Write(r, *this);
      if (amountWritten == 0)
        break;

      // Accumulate how much was written (may be multiple bytes for each rune, depending on the encoding)
      totalWritten += amountWritten;
    }
    return totalWritten;
  }
  
  //***************************************************************************
  Integer IStreamClass::WriteText(StringParam text)
  {
    return this->WriteText(text, IEncoding::GetUtf8());
  }

  //***************************************************************************
  String IStreamClass::ReadLine(IEncoding& sourceStreamEncoding)
  {
    // Appends all the runes together
    StringBuilder builder;

    ZilchLoop
    {
      // The encoding will translate bytes from the stream into valid characters (runes)
      // If reading fails for any reason, encoding should always return the null character
      Rune rune = sourceStreamEncoding.Read(*this);

      // We disregard carriage return. Maybe we shouldn't do this, but it simplifies
      // our logic and doesn't cause it to read ahead in the stream
      // Only very old systems use just the CR to mean newline
      // If this is specifically just the LF (newline character)
      if (rune.mValue == '\n')
      {
        builder.Append('\n');
        return builder.ToString();
      }

      // If this is the end of a the stream (or an error occurred)
      if (rune.mValue == '\0')
        return builder.ToString();

      // We concatenate all read runes together (should automatically encode Utf8 because our strings are Utf8)
      builder.Append(rune.mValue);
    }
  }

  //***************************************************************************
  String IStreamClass::ReadLine()
  {
    return this->ReadLine(IEncoding::GetUtf8());
  }
  
  //***************************************************************************
  String IStreamClass::ReadAllText(IEncoding& sourceStreamEncoding)
  {
    // Appends all the runes together
    StringBuilder builder;

    ZilchLoop
    {
      // The encoding will translate bytes from the stream into valid characters (runes)
      // If reading fails for any reason, encoding should always return the null character
      Rune rune = sourceStreamEncoding.Read(*this);

      // If this is the end of a the stream (or an error occurred)
      if (rune.mValue == '\0')
        return builder.ToString();

      // We concatenate all read runes together (should automatically encode Utf8 because our strings are Utf8)
      builder.Append(rune.mValue);
    }
  }

  //***************************************************************************
  String IStreamClass::ReadAllText()
  {
    return this->ReadAllText(IEncoding::GetUtf8());
  }
}
