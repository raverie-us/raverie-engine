///////////////////////////////////////////////////////////////////////////////
///
/// \file File.hpp
/// Declaration of the Os file class.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
// File Read Write Mode
DeclareEnum4(FileMode,
     // Open file for reading, reading starts at beginning of file
     // If the file does not exist opening will fail
     Read,
     // Open file for writing, if the file exists it be truncated to zero
     // If the file does not exist it will be created
     Write,
     // Open file for writing, writing starts at the end of the file
     // If the file does not exist it will be created
     Append,
     // Open for reading and writing, reading/writing starts at the beginning of file
     // If the file does not exist it will be created
     ReadWrite);

// Hint on how the file will be accessed
DeclareEnum2(FileAccessPattern, Sequential, Random);

// What permissions others have when accessing the same file
DeclareBitField4(FileShare, Read, Write, Delete, Unspecified);

// Position in file
typedef u64 FilePosition;

// Used in file Seek
DeclareEnum3(FileOrigin, Current, Begin, End);

ZeroShared byte* ReadFileIntoMemory(cstr path, size_t& fileSize, size_t extra = 0);
ZeroShared DataBlock ReadFileIntoDataBlock(cstr path);
ZeroShared String ReadFileIntoString(StringParam path);
ZeroShared size_t WriteToFile(cstr filePath, const byte* data, size_t bufferSize);

// Auxiliary functions defined once for every platform
ZeroShared bool CompareFile(Status& status, StringParam filePath1, StringParam filePath2);
ZeroShared bool CompareFileAndString(Status& status, StringParam filePath, StringParam string);

/// Os file class
class ZeroShared File
{
public:
  static const int MaxPath = 260;
  static const int PlatformMaxPath;

  File();
  ~File();
  
  /// Open the file
  /// We take an optional status (this should eventually be refactored, but we wanted to keep current asserting functionality)
  bool Open(StringParam filePath, FileMode::Enum mode, FileAccessPattern::Enum accessPattern, FileShare::Enum share = FileShare::Unspecified, Status* status = nullptr);

  /// Creates a file from an OsHandle (cannot fail, assumes the OS handle is valid)
  void Open(OsHandle handle, FileMode::Enum mode);

  /// Creates a file from a standard C FILE pointer
  /// The most common case is passing in stdin, stdout, and stderr
  void Open(Status& status, FILE* file, FileMode::Enum mode);

  /// Close the file
  void Close();
  
  /// Current read/write position in the file
  FilePosition Tell();
  
  /// Move the read write position to a new filePosition relative to origin
  bool Seek(FilePosition filePosition, FileOrigin::Enum origin = FileOrigin::Begin);
  
  /// Write data to the file
  size_t Write(byte* data, size_t sizeInBytes);
  
  /// Read data from the file
  size_t Read(Status& status, byte* data, size_t sizeInBytes);

  /// Force all reads / write to the disk
  void Flush();
  
  /// Size of the file when opened (not current)
  /// Returns -1 if it fails to get the size of the file
  size_t Size();
  
  /// Size of the file when currently
  /// Returns -1 if it fails to get the size of the file
  long long CurrentFileSize();
  
  /// Is the file currently open?
  bool IsOpen();

  /// Duplicates this file into the destination file. Assumes that this file handle is valid.
  /// Also assumes both files were created in this application's process.
  void Duplicate(Status& status, File& destinationFile) const;

private:
  ZeroDeclarePrivateData(File, 50);

  String mFilePath;
  FileMode::Enum mFileMode;
};

}//namespace Zero
