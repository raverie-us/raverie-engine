///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

inline u32 EndianSwap(u32 x)
{
  return (x>>24) | ((x<<8) & 0x00FF0000) | ((x>>8) & 0x0000FF00) | (x<<24);
}

inline u64 EndianSwap(u64 x)
{
  return (x>>56) | 
    ((x<<40) & 0x00FF000000000000ULL) |
    ((x<<24) & 0x0000FF0000000000ULL) |
    ((x<<8)  & 0x000000FF00000000ULL) |
    ((x>>8)  & 0x00000000FF000000ULL) |
    ((x>>24) & 0x0000000000FF0000ULL) |
    ((x>>40) & 0x000000000000FF00ULL) |
    (x<<56);
}

template<typename bufferType, typename type>
void Read(bufferType& buffer, type& data)
{
  Status status;
  buffer.Read(status, (byte*)&data , sizeof(type));
}

template<typename bufferType, typename type>
void Write(bufferType& buffer, type& data)
{
  buffer.Write((byte*)&data , sizeof(type));
}

template<typename bufferType, typename stringType>
void ReadString(bufferType& buffer, uint size, stringType& str)
{
  Status status;
  byte* data = (byte*)alloca(size+1);
  buffer.Read(status, data, size);
  data[size] = '\0';
  str = (char*)data;
}

template<typename bufferType, typename type>
void PeekType(bufferType& buffer, type& data)
{
  Read(buffer, data);
  int offset = (int)sizeof(data);
  buffer.Seek(-offset, FileOrigin::Current);
}

void WriteStringRangeToFile(StringParam path, StringRange range);

DeclareEnum2(FilterResult, Ignore, Include);
class FileFilter
{
public:
  virtual FilterResult::Enum Filter(StringParam filename) = 0;
};

class FilterFileRegex : public FileFilter
{
public:
  String mAccept;
  String mIgnore;
  FilterFileRegex(StringParam accept, StringParam ignore)
    :mAccept(accept), mIgnore(ignore)
  {
  }
  FilterResult::Enum Filter(StringParam filename) override;
};

class ExtensionFilterFile : public FileFilter
{
public:
  String mExtension;
  bool mCaseSensative;

  ExtensionFilterFile(StringParam extension, bool caseSensative = false)
    :mExtension(extension), mCaseSensative(caseSensative)
  {
  }

  FilterResult::Enum Filter(StringParam filename) override;
};

// Move the contents of a folder.
void MoveFolderContents(StringParam dest, StringParam source, FileFilter* filter = 0);

// Copy the contents of a folder.
void CopyFolderContents(StringParam dest, StringParam source, FileFilter* filter = 0);

// Finds all files in the given path
void FindFilesRecursively(StringParam path, Array<String>& foundFiles);

// Finds files matching a filter in the given path
void FindFilesRecursively(StringParam path, Array<String>& foundFiles, FileFilter* filter);

// Get a time stamp.
String GetTimeAndDateStamp();

// Get the date
String GetDate();

// Make a time stamped backup of a file.
void BackUpFile(StringParam backupPath, StringParam fileName);

// Data Block

//Allocate a Data Block
DataBlock AllocateBlock(size_t size);

//Free memory in data block
void FreeBlock(DataBlock& block);

//Clone (not just copy) new memory will be allocated
void CloneBlock(DataBlock& destBlock, const DataBlock& source);

u64 ComputeFolderSizeRecursive(StringParam folder);
String HumanReadableFileSize(u64 bytes);

}//namespace Zero
