////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Dane Curbow
/// Copyright 2018, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// TODO PLATFORM find and define the max path through SDL, if not possible move this variable to where it will be possible for other platforms
const int File::PlatformMaxPath = 1024;

struct FilePrivateData
{
  FilePrivateData()
  {
    mFileData = nullptr;
  }

  bool IsValidFile()
  {
    return mFileData != nullptr && mFileData->type != SDL_RWOPS_UNKNOWN;
  }

  SDL_RWops* mFileData;
};

String FileModeToString(FileMode::Enum fileMode)
{
  if (fileMode == FileMode::Read)
    return String("rb");

  if (fileMode == FileMode::Write)
    return String("wb");

  if (fileMode == FileMode::Append)
    return String("ab");

  if (fileMode == FileMode::ReadWrite)
    return String("w+b");

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
  if (self->IsValidFile())
  {
    if (mFileMode != FileMode::Read)
      FileModifiedState::EndFileModified(mFilePath);

    SDL_RWclose(self->mFileData);
    self->mFileData = nullptr;
  }
}

FilePosition File::Tell()
{
  ZeroGetPrivateData(FilePrivateData);
  if (!self->IsValidFile())
    return 0;

  return SDL_RWtell(self->mFileData);
}

bool File::Seek(FilePosition pos, SeekOrigin::Enum rel)
{
  ZeroGetPrivateData(FilePrivateData);
  if (!self->IsValidFile())
    return false;
  
  int sdlWhence;
  switch (rel)
  {
    case SeekOrigin::Begin: sdlWhence = RW_SEEK_SET;
      break;
    case SeekOrigin::Current: sdlWhence = RW_SEEK_CUR;
      break;
    case SeekOrigin::End: sdlWhence = RW_SEEK_END;
      break;
    default: return false;
  }

  s64 ret = SDL_RWseek(self->mFileData, pos, sdlWhence);
  return (ret != -1);
}

size_t File::Write(byte* data, size_t sizeInBytes)
{
  ZeroGetPrivateData(FilePrivateData);
  if (!self->IsValidFile())
    return 0;

  size_t bytesWritten = 0;
  bytesWritten = SDL_RWwrite(self->mFileData, data, 1, sizeInBytes);
  return bytesWritten;
}

size_t File::Read(Status& status, byte* data, size_t sizeInBytes)
{
  ZeroGetPrivateData(FilePrivateData);
  if (!self->IsValidFile())
  {
    status.SetFailed("No file was open");
    return 0;
  }

  // SDL incorrectly throws an error when it's at the end of the file.
  return SDL_RWread(self->mFileData, data, 1, sizeInBytes);
}

bool File::HasData(Status& status)
{
  ZeroGetPrivateData(FilePrivateData);
  if (!self->IsValidFile())
  {
    status.SetFailed("No file was open");
    return false;
  }

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
  ZeroGetPrivateData(FilePrivateData);
  // SDL has no flush...
}

void File::Duplicate(Status& status, File& destinationFile)
{
  ZeroGetPrivateData(FilePrivateData);
  ZeroGetObjectPrivateData(FilePrivateData, &destinationFile, other);

  if (!self->IsValidFile() || !other->IsValidFile())
  {
    status.SetFailed("File is not valid. Open a valid file before attemping file operations.");
    return;
  }

  // Get this files data size in bytes
  size_t fileSizeInBytes = (size_t)SDL_RWsize(self->mFileData);
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
