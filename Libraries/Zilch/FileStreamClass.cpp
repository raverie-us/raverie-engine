// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Zilch
{
ZilchDefineExternalBaseType(FileMode::Enum, TypeCopyMode::ValueType, builder, type)
{
  ZilchFullBindEnum(builder, type, SpecialType::Flags);
  ZilchFullBindEnumValue(builder, type, FileMode::Read, "Read");
  ZilchFullBindEnumValue(builder, type, FileMode::Write, "Write");
  ZilchFullBindEnumValue(builder, type, FileMode::Append, "Append");
  ZilchFullBindEnumValue(builder, type, FileMode::ShareRead, "ShareRead");
  ZilchFullBindEnumValue(builder, type, FileMode::ShareWrite, "ShareWrite");
  ZilchFullBindEnumValue(builder, type, FileMode::ShareDelete, "ShareDelete");
  ZilchFullBindEnumValue(builder, type, FileMode::Sequential, "Sequential");
}

ZilchDefineType(FileStreamClass, builder, type)
{
  // Even though this is an interface, because it is native, it must have a
  // constructor that can be implemented
  ZilchFullBindDestructor(builder, type, FileStreamClass);
  ZilchFullBindConstructor(builder, type, FileStreamClass, "filePath, mode", StringParam, FileMode::Enum);
  ZilchFullBindConstructor(builder, type, FileStreamClass, nullptr);
  ZilchBindConstructor(const FileStreamClass&);

  ZilchFullBindMethod(builder, type, &FileStreamClass::Close, ZilchNoOverload, "Close", nullptr);
}

FileStreamClass::FileStreamClass(StringParam filePath, FileMode::Enum mode)
{
  Zero::FileShare::Enum zeroShare = (Zero::FileShare::Enum)0;
  Zero::FileMode::Enum zeroMode = Zero::FileMode::Read;
  Zero::FileAccessPattern::Enum zeroAccessPattern = Zero::FileAccessPattern::Random;

  bool read = (mode & FileMode::Read) != 0;
  bool write = (mode & FileMode::Write) != 0;
  bool append = (mode & FileMode::Append) != 0;

  if (append && read)
  {
    ExecutableState::CallingState->ThrowException("Cannot Append and Read from the same FileStreamClass");
    return;
  }

  // Translate our mode flags into the Zero enum
  if (append)
    zeroMode = Zero::FileMode::Append;
  else if (read && write)
    zeroMode = Zero::FileMode::ReadWrite;
  else if (write)
    zeroMode = Zero::FileMode::Write;
  else
    zeroMode = Zero::FileMode::Read;

  // Enable any optimizations
  if (mode & FileMode::Sequential)
    zeroAccessPattern = Zero::FileAccessPattern::Sequential;

  // We always have these capabilities
  this->Capabilities = (StreamCapabilities::Enum)(StreamCapabilities::GetCount | StreamCapabilities::Seek);

  if (mode & FileMode::ShareRead)
    zeroShare = (Zero::FileShare::Enum)(zeroShare | Zero::FileShare::Read);
  if (mode & FileMode::ShareWrite)
    zeroShare = (Zero::FileShare::Enum)(zeroShare | Zero::FileShare::Write);
  if (mode & FileMode::ShareDelete)
    zeroShare = (Zero::FileShare::Enum)(zeroShare | Zero::FileShare::Delete);

  // Setup stream capabilities based on flags passed in
  if (read)
    this->Capabilities = (StreamCapabilities::Enum)(this->Capabilities | StreamCapabilities::Read);
  if (write)
    this->Capabilities = (StreamCapabilities::Enum)(this->Capabilities | StreamCapabilities::Write);

  // Create the path if it doesn't exist and we're in a writing file mode (all
  // of these should create the file according to the windows standard but
  // currently ReadWrite might not actually do this)
  if (zeroMode == Zero::FileMode::Write || zeroMode == Zero::FileMode::ReadWrite || zeroMode == Zero::FileMode::Append)
  {
    String directoryPath = Zero::FilePath::GetDirectoryPath(filePath);
    CreateDirectoryAndParents(directoryPath);
  }

  // Open the file and throw an exception if we fail
  Status status;
  this->InternalFile.Open(filePath, zeroMode, zeroAccessPattern, zeroShare, &status);
  if (status.Failed())
  {
    String message = String::Format("Unable to open the file '%s': %s", filePath.c_str(), status.Message.c_str());
    ExecutableState::CallingState->ThrowException(message);
  }
}

FileStreamClass::FileStreamClass()
{
  Capabilities = (StreamCapabilities::Enum)0;
}

FileStreamClass::FileStreamClass(const FileStreamClass& stream)
{
  // Duplicate the file handle
  Status status;
  const_cast<FileStreamClass&>(stream).InternalFile.Duplicate(status, InternalFile);

  Capabilities = stream.Capabilities;
}

FileStreamClass::~FileStreamClass()
{
  if (this->InternalFile.IsOpen())
    Close();
}

StreamCapabilities::Enum FileStreamClass::GetCapabilities()
{
  return this->Capabilities;
}

DoubleInteger FileStreamClass::GetPosition()
{
  return this->InternalFile.Tell();
}

DoubleInteger FileStreamClass::GetCount()
{
  return this->InternalFile.CurrentFileSize();
}

bool FileStreamClass::Seek(DoubleInteger position, StreamOrigin::Enum origin)
{
  return this->InternalFile.Seek(position, (Zero::SeekOrigin::Enum)origin);
}

Integer FileStreamClass::Write(ArrayClass<Byte>& data, Integer byteStart, Integer byteCount)
{
  IStreamClass::Write(data, byteStart, byteCount);
  if (ExecutableState::GetCallingReport().HasThrownExceptions())
    return 0;

  return (Integer)this->InternalFile.Write(data.NativeArray.Data() + byteStart, (size_t)byteCount);
}

Integer FileStreamClass::WriteByte(Byte byte)
{
  return (Integer)this->InternalFile.Write(&byte, 1);
}

Integer FileStreamClass::Read(ArrayClass<Byte>& data, Integer byteStart, Integer byteCount)
{
  Status status;
  IStreamClass::Read(data, byteStart, byteCount);
  if (ExecutableState::GetCallingReport().HasThrownExceptions())
    return 0;

  return (Integer)this->InternalFile.Read(status, data.NativeArray.Data() + byteStart, (size_t)byteCount);
}

Integer FileStreamClass::ReadByte()
{
  Status status;
  IStreamClass::ReadByte();
  if (ExecutableState::GetCallingReport().HasThrownExceptions())
    return 0;

  // Read a single byte
  Byte byte = 0;
  size_t bytesRead = this->InternalFile.Read(status, &byte, 1);

  // If we didn't read anything, then return -1 (we're returning an Integer)
  if (bytesRead == 0)
    return -1;

  // Otherwise, return just the byte
  return (Integer)byte;
}

void FileStreamClass::Flush()
{
  this->InternalFile.Flush();
}

void FileStreamClass::Close()
{
  this->InternalFile.Close();
}

} // namespace Zilch
