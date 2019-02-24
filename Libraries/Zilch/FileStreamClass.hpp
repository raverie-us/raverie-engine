// MIT Licensed (see LICENSE.md).

#pragma once
#ifndef ZILCH_FILE_STREAM_HPP
#  define ZILCH_FILE_STREAM_HPP

namespace Zilch
{
// The options should specify whether we want to read or write from a file, as
// well as if we want to allow others to read and write to the same file (via
// the share flags)
namespace FileMode
{
enum Enum
{
  // Reading will start at the beginning of the file
  // The file must exist or an exception will be thrown
  Read = (1 << 0),

  // Writing will start at the beginning of the file
  // If the file does not exist it will be created
  Write = (1 << 1),

  // Writing will start at the end of the file (implies the Write flag)
  // You cannot Seek to a position or Read from the stream in this mode
  // If the file does not exist it will be created
  Append = (1 << 2),

  // Allows others to read from the file at the same time
  ShareRead = (1 << 3),

  // Allows others to write to the file at the same time
  ShareWrite = (1 << 4),

  // Allows others to delete the file even if we are using it
  ShareDelete = (1 << 5),

  // Optimizes for sequential reading (in order). Random access is implied if
  // this flag is not set
  Sequential = (1 << 6),

  // All enums bound to Zilch must be of Integer size, so force it on all
  // platforms
  ForceIntegerSize = 0x7FFFFFFF
};
}

// A generic interface for reading and writing data to a stream (file, network,
// etc)
class ZeroShared FileStreamClass : public IStreamClass
{
public:
  ZilchDeclareType(FileStreamClass, TypeCopyMode::ReferenceType);

  // Constructor
  FileStreamClass(StringParam filePath, FileMode::Enum mode);
  FileStreamClass();
  FileStreamClass(const FileStreamClass& stream);
  ~FileStreamClass();

  // IStreamClass interface
  StreamCapabilities::Enum GetCapabilities() override;
  DoubleInteger GetPosition() override;
  DoubleInteger GetCount() override;
  bool Seek(DoubleInteger position, StreamOrigin::Enum origin) override;
  Integer Write(ArrayClass<Byte>& data, Integer byteStart, Integer byteCount) override;
  Integer WriteByte(Byte byte) override;
  Integer Read(ArrayClass<Byte>& data, Integer byteStart, Integer byteCount) override;
  Integer ReadByte() override;
  void Flush() override;
  void Close();

public:
  // The capabilities that we determined based on how we opened the file
  StreamCapabilities::Enum Capabilities;

  // The underlying operating system primitive that represents the file
  File InternalFile;
};
} // namespace Zilch

#endif
