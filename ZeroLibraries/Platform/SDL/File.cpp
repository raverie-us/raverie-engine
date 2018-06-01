////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Dane Curbow
/// Copyright 2018, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Platform/File.hpp"
#include "SDL_rwops.h"

namespace Zero
{

// TODO PLATFORM find and define the max path through SDL, if not possible move this variable to where it will be possible for other platforms
const int File::PlatformMaxPath = 256;

struct FilePrivateData
{
  FilePrivateData()
  {
    mFileData = nullptr;
  }

  bool IsValidFile()
  {
    if (mFileData == nullptr || mFileData->type == SDL_RWOPS_UNKNOWN)
      return false;
    
    return true;
  }

  SDL_RWops* mFileData;
};

String FileModeToString(FileMode::Enum fileMode)
{
  if (fileMode == FileMode::Read)
    return String("r");

  if (fileMode == FileMode::Write)
    return String("w");

  if (fileMode == FileMode::Append)
    return String("a");

  if (fileMode == FileMode::ReadWrite)
    return String("rw");

  return String();
}

//------------------------------------------------------------------------- File
File::File()
{
  ZeroConstructPrivateData(FilePrivateData);
}

File::~File()
{
  Close();
  ZeroDestructPrivateData(FilePrivateData);
}

size_t File::Size()
{
  ZeroGetPrivateData(FilePrivateData);
  return (size_t)SDL_RWsize(self->mFileData);
}

long long File::CurrentFileSize()
{
  ZeroGetPrivateData(FilePrivateData);
  return SDL_RWsize(self->mFileData);
}

bool File::Open(StringParam filePath, FileMode::Enum mode, FileAccessPattern::Enum accessPattern, FileShare::Enum share, Status* status)
{
  ZeroGetPrivateData(FilePrivateData);
  
  String fileMode = FileModeToString(mode);
  self->mFileData = SDL_RWFromFile(filePath.c_str(), fileMode.c_str());

  if (self->mFileData == nullptr)
  {
    String errorString = SDL_GetError();
    Warn("Failed to open file '%s'. %s", filePath.c_str(), errorString.c_str());
    return false;
  }

  return true;
}

void File::Open(OsHandle handle, FileMode::Enum mode)
{
  Error("SDL does not support opening files via OSHandles");
}

void File::Open(Status& status, FILE* file, FileMode::Enum mode)
{
  ZeroGetPrivateData(FilePrivateData);
  self->mFileData = SDL_RWFromFP(file, SDL_TRUE);

  if (self->mFileData == nullptr)
  {
    String errorString = SDL_GetError();
    String message = String::Format("Failed to open file: %s", errorString.c_str());
    status.SetFailed(message);
    return;
  }

  mFileMode = mode;
}

bool File::IsOpen()
{
  ZeroGetPrivateData(FilePrivateData);
  return self->mFileData != nullptr;
}

void File::Close()
{
  ZeroGetPrivateData(FilePrivateData);
  if (self->mFileData != nullptr || self->mFileData->type != SDL_RWOPS_UNKNOWN)
  {
    if (mFileMode != FileMode::Read)
      FileModifiedState::EndFileModified(mFilePath);

    SDL_RWclose(self->mFileData);
  }
}

FilePosition File::Tell()
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(!self->IsValidFile(), "File is not valid. Open a valid file before attemping file operations.");

  return SDL_RWtell(self->mFileData);
}

bool File::Seek(FilePosition pos, FileOrigin::Enum rel)
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(!self->IsValidFile(), "File is not valid. Open a valid file before attemping file operations.");
  
  int sdlWhence;
  switch (rel)
  {
    case FileOrigin::Begin: sdlWhence = RW_SEEK_SET;
      break;
    case FileOrigin::Current: sdlWhence = RW_SEEK_CUR;
      break;
    case FileOrigin::End: sdlWhence = RW_SEEK_END;
      break;
    default: return false;
  }

  s64 ret = SDL_RWseek(self->mFileData, pos, sdlWhence);
  return (ret != -1);
}

size_t File::Write(byte* data, size_t sizeInBytes)
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(!self->IsValidFile(), "File is not valid. Open a valid file before attemping file operations.");

  size_t bytesWritten = 0;
  bytesWritten = SDL_RWwrite(self->mFileData, data, 1, sizeInBytes);
  return bytesWritten;
}

size_t File::Read(Status& status, byte* data, size_t sizeInBytes)
{
  ZeroGetPrivateData(FilePrivateData);
  // We don't assert here because its legal to close the handle from another thread,
  // and attempt a read operation (which will fail, expectedly)
  size_t bytesRead = 0;
  bool result = SDL_RWread(self->mFileData, data, 1, sizeInBytes);
  if (result)
    status.SetSucceeded();
  else
  {
    String errorString = SDL_GetError();
    String message = String::Format("Failed to read file '%s': %s", mFilePath.c_str(), errorString.c_str());
    status.SetFailed(message);
  }
  return bytesRead;
}

bool File::HasData(Status& status)
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(!self->IsValidFile(), "File is not valid. Open a valid file before attemping file operations.");

  if (self->mFileData->type != SDL_RWOPS_UNKNOWN)
  {
    int64 bytesRead = SDL_RWsize(self->mFileData);
    if(bytesRead > 0)
    {
      status.SetSucceeded();
    }
    else
    {
      String errorString = SDL_GetError();
      String message = String::Format("Failed to read file '%s': %s", mFilePath.c_str(), errorString.c_str());
      status.SetFailed(message);
    }

    return bytesRead > 0;
  }

  status.SetFailed("Unknown file handle type. Only files or pipes are allowed");
  return false;
}

void File::Flush()
{
  //TODO DANE
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(!self->IsValidFile(), "File is not valid. Open a valid file before attemping file operations.");
  //FlushFileBuffers(self->mHandle);
}

void File::Duplicate(Status& status, File& destinationFile)
{
  ZeroGetPrivateData(FilePrivateData);
  ZeroGetObjectPrivateData(FilePrivateData, &destinationFile, other);
  ErrorIf(!self->IsValidFile(), "File is not valid. Open a valid file before attemping file operations.");
  ErrorIf(!other->IsValidFile(), "File is not valid. Open a valid file before attemping file operations.");

  // Get this files data size in bytes
  size_t fileSizeInBytes = (size_t)SDL_RWsize(self->mFileData);
  byte* buffer = new byte[fileSizeInBytes];
  
  // Read this files data
  Read(status, buffer, fileSizeInBytes);
  if (status.Failed())
  {
    delete buffer;
    Warn(status.Message.c_str());
  }

  // Write the duplicate data into the other file
  size_t ret = destinationFile.Write(buffer, fileSizeInBytes);
  delete buffer;
  WarnIf(ret != fileSizeInBytes, "Failed to duplicate original file");
}

}//namespace Zero
