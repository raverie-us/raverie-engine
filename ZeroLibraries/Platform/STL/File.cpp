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

namespace Zero
{
struct FilePrivateData
{
  FILE* mHandle;
  uint mFileSize;
};

cstr cBadFileMessage = "The file is missing, not in that location, or is protected.";

uint GetFileSize(FILE* file)
{
  long current = ftell(file);
  fseek(file, 0L, SEEK_END);
  uint size = (uint)ftell(file);
  fseek(file, current, SEEK_SET);
  return size;
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
uint ToOrigin(SeekOrigin::Enum origin)
{
  switch(origin)
  {
  case SeekOrigin::Begin:
    return SEEK_SET;
  case SeekOrigin::End: 
    return SEEK_END;
  case SeekOrigin::Current:
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
  if (self->mHandle == nullptr)
  {
    if (status)
      status->SetFailed(String::Format("Failed to open file '%s'. %s", filePath.c_str(), cBadFileMessage));
    return false;
  }

  self->mFileSize = GetFileSize(self->mHandle);
  mFilePath = filePath;

  if(mode == FileMode::Append)
    Seek(self->mFileSize);
  return true;
}

void File::Open(OsHandle handle, FileMode::Enum mode)
{
  // Our OsHandle would be a FILE*.
  Status status;
  return Open(status, (FILE*)handle, mode);
}

void File::Open(Status& status, FILE* file, FileMode::Enum mode)
{
  ZeroGetPrivateData(FilePrivateData);
  self->mHandle = file;
  self->mFileSize = GetFileSize(self->mHandle);
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

bool File::Seek(FilePosition pos, SeekOrigin::Enum origin)
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(self->mHandle == NULL, "File handle is not valid.");
  int result = fseek(self->mHandle, (long)pos, ToOrigin(origin));
  // A result of '0' means success
  return (result == 0);
}

size_t File::Write(byte* data, size_t sizeInBytes)
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(self->mHandle == NULL, "File handle is not valid.");
  return fwrite(data, 1, sizeInBytes, self->mHandle);
}

size_t File::Read(Status& status, byte* data, size_t sizeInBytes)
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(self->mHandle == NULL, "File handle is not valid.");
  size_t amount = fread(data, 1, sizeInBytes, self->mHandle);
  if (amount != sizeInBytes && ferror(self->mHandle))
    status.SetFailed("Error reading file data.");
  return amount;
}

bool File::HasData(Status& status)
{
  return (s64)Tell() < (s64)CurrentFileSize();
}

void File::Flush()
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(self->mHandle == NULL, "File handle is not valid.");
  fflush(self->mHandle);
}

void File::Duplicate(Status& status, File& destinationFile)
{
  ZeroGetPrivateData(FilePrivateData);
  ZeroGetObjectPrivateData(FilePrivateData, &destinationFile, other);

  if (!self->mHandle || !other->mHandle)
  {
    status.SetFailed("File is not valid. Open a valid file before attemping file operations.");
    return;
  }

  // Get this files data size in bytes
  size_t fileSizeInBytes = (size_t)CurrentFileSize();
  byte* buffer = new byte[fileSizeInBytes];

  // Read this files data
  size_t ret = Read(status, buffer, fileSizeInBytes);
  WarnIf(ret != fileSizeInBytes, "Failed to duplicate original file");
  if (status.Failed())
  {
    delete buffer;
    return;
  }

  // Write the duplicate data into the other file
  ret = destinationFile.Write(buffer, fileSizeInBytes);
  delete buffer;
  WarnIf(ret != fileSizeInBytes, "Failed to duplicate original file");
}

}//namespace Zero
