///////////////////////////////////////////////////////////////////////////////
///
/// \file File.cpp
/// Implementation of the Os file class.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Platform/File.hpp"

namespace Zero
{
//----------------------------------------------------------- File Functions
byte* ReadFileIntoMemory(cstr path, size_t& fileSize, size_t extra)
{
  Error("Not implemented");
  return NULL;
}

DataBlock ReadFileIntoDataBlock(cstr path)
{
  Error("Not implemented");
  return DataBlock();
}

uint WriteToFile(cstr filePath, byte * pData, size_t bufferSize)
{
  Error("Not implemented");
  return 0;
}

//----------------------------------------------------------- File
struct FilePrivateData
{
};

File::File()
{
  Error("Not implemented");
  ZeroConstructPrivateData(FilePrivateData);
}

File::~File()
{
  ZeroGetPrivateData(FilePrivateData);
  // Destruction logic
  ZeroDestructPrivateData(FilePrivateData);
}

bool File::Open(StringParam filePath, FileMode::Enum mode, FileAccessPattern::Enum accessPattern, FileShare::Enum share, Status* status)
{
  ZeroGetPrivateData(FilePrivateData);
  return false;
}

void File::Close()
{
  ZeroGetPrivateData(FilePrivateData);
}

FilePosition File::Tell()
{
  ZeroGetPrivateData(FilePrivateData);
  return 0;
}

bool File::Seek(FilePosition filePosition, FileOrigin::Enum origin)
{
  ZeroGetPrivateData(FilePrivateData);
  return false;
}

size_t File::Write(byte* data, size_t sizeInBytes)
{
  ZeroGetPrivateData(FilePrivateData);
  return 0;
}

size_t File::Read(byte* data, size_t sizeInBytes)
{
  ZeroGetPrivateData(FilePrivateData);
  return 0;
}

size_t File::Size()
{
  ZeroGetPrivateData(FilePrivateData);
  return 0;
}

long long File::CurrentFileSize()
{
  ZeroGetPrivateData(FilePrivateData);
  return 0;
}

bool File::IsOpen()
{
  ZeroGetPrivateData(FilePrivateData);
  return false;
}

void File::Flush()
{
  ZeroGetPrivateData(FilePrivateData);
}

}//namespace Zero
