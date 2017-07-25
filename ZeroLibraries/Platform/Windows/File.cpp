///////////////////////////////////////////////////////////////////////////////
///
/// \file File.cpp
/// Implementation of the file class for Windows.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Platform/File.hpp"
#include "Memory/Memory.hpp"
#include "String/FixedString.hpp"
#include "../FileEvents.hpp"

#define OSF_INVALID_HANDLE_VALUE ((intptr_t)INVALID_HANDLE_VALUE)

namespace Zero
{
const int File::PlatformMaxPath = MAX_PATH;

struct FilePrivateData
{
  FilePrivateData()
  {
    mHandle = INVALID_HANDLE_VALUE;
    mOsfHandle = OSF_INVALID_HANDLE_VALUE;
    mFileSize = 0;
  }

  HANDLE mHandle;
  intptr_t mOsfHandle;
  long long mFileSize;
};

const DWORD FILE_NO_SHARING = 0;
SECURITY_ATTRIBUTES* NOSECURITY = NULL;

cstr cBadFileMessage = "The file is missing, not in that location, or is protected.";

byte * ReadFileIntoMemory(cstr filePath, size_t& fileSize, size_t extra)
{
  HANDLE fileHandle = ::CreateFileW(Widen(filePath).c_str(), GENERIC_READ, FILE_SHARE_READ, 
                                    NOSECURITY, OPEN_EXISTING, 0, 0);
  
  CheckWin(fileHandle != INVALID_HANDLE_VALUE, "Failed to open file %s.", filePath);

  ReturnIf(fileHandle == INVALID_HANDLE_VALUE, NULL, 
           "Failed to open file '%s'. %s", filePath, cBadFileMessage);

  DWORD fileSizeInBytes = ::GetFileSize(fileHandle, NULL);
  byte* fileBuffer = (byte*)zAllocate(fileSizeInBytes+extra);
  if (fileBuffer == NULL)
  {
    ErrorIf(fileBuffer == NULL, 
            "Could not allocate enough memory for file '%s' into memory.", 
            filePath); 

    delete fileBuffer;
    ::CloseHandle(fileHandle);
    return NULL;
  }
  else
  {
    fileSize = fileSizeInBytes;
    DWORD bytesRead;
    BOOL readResult = ::ReadFile(fileHandle, fileBuffer, (DWORD)fileSize, 
                                 &bytesRead, NULL);
    ErrorIf(!readResult, "Could not Read file '%s'.", filePath); 
  }
  ::CloseHandle(fileHandle);
  return fileBuffer;
}

size_t WriteToFile(cstr filePath, const byte* data, size_t bufferSize)
{
  HANDLE fileHandle = ::CreateFileW(Widen(filePath).c_str(), GENERIC_WRITE, FILE_NO_SHARING,
                                    NOSECURITY, CREATE_ALWAYS, 0, 0); 
  ReturnIf(fileHandle == INVALID_HANDLE_VALUE, 0, "Failed to open destination "
           "file '%s'. %s", filePath, cBadFileMessage);

  FileModifiedState::BeginFileModified(filePath);

  DWORD bytesWritten = 0;
  BOOL writeResult = WriteFile(fileHandle, data, (DWORD)bufferSize,
                               &bytesWritten, 0);
  ::CloseHandle(fileHandle);

  FileModifiedState::EndFileModified(filePath);

  ReturnIf(writeResult == 0, 0, "Failed to write to destination file '%s'.", 
           filePath);

  return bufferSize;
}

DWORD ToWindowsFileMode(FileMode::Enum mode)
{
  switch(mode)
  {
    case FileMode::Read:
      return GENERIC_READ;

    case FileMode::Append:
    case FileMode::Write: 
      return GENERIC_WRITE;
  
    case FileMode::ReadWrite:
      return GENERIC_READ | GENERIC_WRITE;
  }
  return 0;
}

//Convert File Relative position to windows constant
DWORD ToWindowsRelative(FileOrigin::Enum relative)
{
  switch(relative)
  {
    case FileOrigin::Begin:
      return FILE_BEGIN;

    case FileOrigin::End: 
      return FILE_END;

    case FileOrigin::Current:
      return FILE_CURRENT;
  }
  return FILE_CURRENT;
}

DWORD ToWindowsDisposition(FileMode::Enum mode, FileAccessPattern::Enum pattern)
{
  DWORD disposition = 0;
  if(mode == FileMode::Write)
    disposition = CREATE_ALWAYS;
  else if(mode == FileMode::Append)
    disposition = OPEN_ALWAYS;
  else
    disposition = OPEN_EXISTING;
  return disposition;
}

DWORD ToWindowsFlags(FileMode::Enum mode, FileAccessPattern::Enum pattern)
{
  DWORD flags = 0;
  if(pattern == FileAccessPattern::Sequential)
    flags |= FILE_FLAG_SEQUENTIAL_SCAN;
  else
    flags |= FILE_FLAG_RANDOM_ACCESS;

  return flags;
}

//------------------------------------------------------------------------- File
File::File()
{
  ZeroConstructPrivateData(FilePrivateData);
  self->mHandle = INVALID_HANDLE_VALUE;
  self->mFileSize = -1;
  mFileMode = FileMode::Read;
}

File::~File()
{
  Close();
  ZeroDestructPrivateData(FilePrivateData);
}

size_t File::Size()
{
  ZeroGetPrivateData(FilePrivateData);
  return (size_t)self->mFileSize;
}

long long File::CurrentFileSize()
{
  ZeroGetPrivateData(FilePrivateData);
  // This should be upgraded to support 64 bit file sizes
  LARGE_INTEGER size;
  size.QuadPart = -1;
  ::GetFileSizeEx(self->mHandle, &size);
  return size.QuadPart;
}

bool File::Open(StringParam filePath, FileMode::Enum mode, FileAccessPattern::Enum accessPattern, FileShare::Enum share, Status* status)
{
  ZeroGetPrivateData(FilePrivateData);

  DWORD fileMode = ToWindowsFileMode(mode);
  DWORD flags = ToWindowsFlags(mode, accessPattern);
  DWORD disposition = ToWindowsDisposition(mode, accessPattern);
  DWORD sharingMode = FILE_NO_SHARING;

  if (share & FileShare::Unspecified)
  {
    if(mode == FileMode::Read)
      sharingMode = FILE_SHARE_READ;
  }
  else
  {
    if (share & FileShare::Read)
      sharingMode |= FILE_SHARE_READ;
    if (share & FileShare::Write)
      sharingMode |= FILE_SHARE_WRITE;
    if (share & FileShare::Delete)
      sharingMode |= FILE_SHARE_DELETE;
  }

  self->mHandle = ::CreateFileW(Widen(filePath).c_str(), fileMode, sharingMode, NOSECURITY, disposition,
                         flags, NULL);

  if(status == nullptr)
  {
    CheckWin(self->mHandle != INVALID_HANDLE_VALUE, "Failed to open file %s.", filePath.c_str());

    ReturnIf(self->mHandle == INVALID_HANDLE_VALUE, false, 
             "Failed to open file '%s'. %s", filePath.c_str(), cBadFileMessage);
  }
  else if(self->mHandle == INVALID_HANDLE_VALUE)
  {
    FillWindowsErrorStatus(*status);
    return false;
  }

  self->mFileSize = CurrentFileSize();
  mFilePath = filePath;

  if(mode == FileMode::Append)
    Seek(self->mFileSize);

  mFileMode = mode;
  if(mode != FileMode::Read)
    FileModifiedState::BeginFileModified(mFilePath);

  return true;
}

void File::Open(OsHandle handle, FileMode::Enum mode)
{
  ZeroGetPrivateData(FilePrivateData);
  self->mHandle = handle;
  self->mFileSize = CurrentFileSize();
  mFileMode = mode;
}

void File::Open(Status& status, FILE* file, FileMode::Enum mode)
{
  int descriptor = _fileno(file);
  if (descriptor == -1 || descriptor == -2)
  {
    status.SetFailed("The FILE pointer was not valid or bound to a stream");
    return;
  }

  ZeroGetPrivateData(FilePrivateData);
  self->mOsfHandle = _get_osfhandle(descriptor);
  HANDLE handle = (HANDLE)self->mOsfHandle;
  if (handle == INVALID_HANDLE_VALUE)
  {
    status.SetFailed("Unable to get a valid Win32 handle to the file descriptor");
    return;
  }

  self->mHandle = handle;
  self->mFileSize = CurrentFileSize();
  mFileMode = mode;
}

bool File::IsOpen()
{
  ZeroGetPrivateData(FilePrivateData);
  return self->mHandle != INVALID_HANDLE_VALUE;
}

void File::Close()
{
  ZeroGetPrivateData(FilePrivateData);
  if(self->mOsfHandle != OSF_INVALID_HANDLE_VALUE || self->mHandle != INVALID_HANDLE_VALUE)
  {
    if(mFileMode != FileMode::Read)
      FileModifiedState::EndFileModified(mFilePath);
    if(self->mOsfHandle != OSF_INVALID_HANDLE_VALUE)
      _close(self->mOsfHandle);
    else
      CloseHandle(self->mHandle);
    self->mOsfHandle = OSF_INVALID_HANDLE_VALUE;
    self->mHandle = INVALID_HANDLE_VALUE;
  }
}

FilePosition File::Tell()
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(self->mHandle == INVALID_HANDLE_VALUE, "File handle is not valid.");

  LARGE_INTEGER move;
  move.QuadPart = 0;

  LARGE_INTEGER newPosition;
  BOOL success = SetFilePointerEx(self->mHandle, move,  &newPosition, FILE_CURRENT);

  return newPosition.QuadPart;
}

bool File::Seek(FilePosition pos, FileOrigin::Enum rel)
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(self->mHandle == INVALID_HANDLE_VALUE, "File handle is not valid.");
  LARGE_INTEGER move;
  move.QuadPart = pos;

  uint winRel = ToWindowsRelative(rel);

  LARGE_INTEGER newPosition;
  BOOL success = SetFilePointerEx(self->mHandle, move,  &newPosition, winRel);
  return (success != 0);
}

size_t File::Write(byte* data, size_t sizeInBytes)
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(self->mHandle == INVALID_HANDLE_VALUE, "File handle is not valid.");
  DWORD bytesWritten = 0;
  WriteFile(self->mHandle, data, sizeInBytes, &bytesWritten, NULL);
  return bytesWritten;
}

size_t File::Read(Status& status, byte* data, size_t sizeInBytes)
{
  ZeroGetPrivateData(FilePrivateData);
  // We don't assert here because its legal to close the handle from another thread,
  // and attempt a read operation (which will fail, expectedly)
  DWORD bytesRead = 0;
  bool result = ReadFile(self->mHandle, data, sizeInBytes, &bytesRead, NULL);
  if (result)
    status.SetSucceeded();
  else
  {
    int errorCode = (int)GetLastError();
    String errorString = ToErrorString(errorCode);
    String message = String::Format("Failed to read file '%s': %s", mFilePath, errorString.c_str());
    status.SetFailed(message, errorCode);
  }
  return bytesRead;
}

void File::Flush()
{
  ZeroGetPrivateData(FilePrivateData);
  ErrorIf(self->mHandle == INVALID_HANDLE_VALUE, "File handle is not valid.");
  FlushFileBuffers(self->mHandle);
}

void File::Duplicate(Status& status, File& destinationFile) const
{
  ZeroGetPrivateData(FilePrivateData);
  ZeroGetObjectPrivateData(FilePrivateData, &destinationFile, other);

  HANDLE process = GetCurrentProcess();

  BOOL result = DuplicateHandle(
    process, self->mHandle, 
    process, &other->mHandle,
    0,
    FALSE, // Make it un-inheritable.
    DUPLICATE_SAME_ACCESS);

  if(result == FALSE)
    WinReturnIfStatus(status);
}

}//namespace Zero
