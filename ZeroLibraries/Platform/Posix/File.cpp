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

#pragma warning(disable: 4996)

namespace Zero
{
struct FilePrivateData
{
  FILE* mHandle;
  uint mFileSize;
};

cstr cBadFileMessage = "The file is missing, not in that location, or is "
  "protected.";

uint GetFileSize(FILE* file)
{
  int fd = fileno(file);
  struct stat st;
  fstat(fd, &st);
  return (uint)st.st_size;
}

DataBlock ReadFileIntoDataBlock(cstr filePath)
{
  FILE* file = fopen (filePath,"rb");
  if(file)
  {
    uint fileSize = GetFileSize(file);
    byte* buffer = (byte*)zAllocate(fileSize);
    fread(buffer, 1, fileSize, file);
    fclose(file);
    return DataBlock(buffer, fileSize);
  }

  return DataBlock();
}

byte * ReadFileIntoMemory(cstr filePath, size_t& fileSize, size_t extra)
{
  FILE* file = fopen (filePath,"rb");
  if(file)
  {
    fileSize = GetFileSize(file);
    byte* buffer = (byte*)zAllocate(fileSize+extra);
    fread(buffer, 1, fileSize, file);
    fclose(file);
    return buffer;
  }
  return NULL;
}

size_t WriteToFile(cstr filePath, byte* buffer, size_t bufferSize)
{
  FILE* file = fopen (filePath,"wb");
  if(file)
  {
    fwrite(buffer, 1, bufferSize,  file);
    fclose(file);
    return bufferSize;
  }
  return 0;
}

cstr ToFileMode(FileMode::Enum mode)
{
  switch(mode)
  {
    case FileMode::Read:
      return "rb";

    case FileMode::Append:
      return "ab";

    case FileMode::Write:
      return "wb";
  
    case FileMode::ReadWrite:
      return "r+b";
  }
  return "";
}

//Convert File Relative position to windows constant
uint ToOrigin(FileOrigin::Enum origin)
{
  switch(origin)
  {
  case FileOrigin::Begin:
    return SEEK_SET;
  case FileOrigin::End: 
    return SEEK_END;
  case FileOrigin::Current:
    return SEEK_CUR;
  }
  return SEEK_CUR;
}

File::File()
{
  ZeroConstructPrivateData(FilePrivateData);
  self->mHandle = NULL;
}

bool File::Open(StringParam filePath, FileMode::Enum mode, FileAccessPattern::Enum accessPattern, FileShare::Enum share, Status* status)
{
  ZeroGetPrivateData(FilePrivateData);
  cstr fmode = ToFileMode(mode);

  self->mHandle = fopen(filePath.c_str(), fmode);
  ReturnIf(self->mHandle == NULL, false, 
           "Failed to open file '%s'. %s", filePath.c_str(), cBadFileMessage);

  self->mFileSize = GetFileSize(self->mHandle);
  mFilePath = filePath;

  if(mode == FileMode::Append)
    Seek(self->mFileSize);
  return true;
}

void File::Close()
{
  ZeroGetPrivateData(FilePrivateData);
  if(self->mHandle != NULL)
  {
    fclose(self->mHandle);
    self->mHandle = NULL;
  }
}
    
bool File::IsOpen()
{
  ZeroGetPrivateData(FilePrivateData);
  return self->mHandle != NULL;
}
    

File::~File()
{
  Close();
}

FilePosition File::Tell()
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(self->mHandle == NULL, "File handle is not valid.");
  return ftell(self->mHandle);
}

size_t File::Size()
{
  ZeroGetPrivateData(FilePrivateData);
  return self->mFileSize;  
}

long long File::CurrentFileSize()
{
  ZeroGetPrivateData(FilePrivateData);
  return GetFileSize(self->mHandle);
}

bool File::Seek(FilePosition pos, FileOrigin::Enum origin)
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(self->mHandle == NULL, "File handle is not valid.");
  int result = fseek(self->mHandle, (long)pos, ToOrigin(origin) );
  // A result of '0' means success
  return (result == 0);
}

size_t File::Write(byte* data, size_t sizeInBytes)
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(self->mHandle == NULL, "File handle is not valid.");
  return fwrite(data, 1, sizeInBytes, self->mHandle);
}

size_t File::Read(byte* data, size_t sizeInBytes)
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(self->mHandle == NULL, "File handle is not valid.");
  return fread(data, 1, sizeInBytes, self->mHandle);
}

void File::Flush()
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(self->mHandle == NULL, "File handle is not valid.");
  fflush(self->mHandle);
}

}//namespace Zero
