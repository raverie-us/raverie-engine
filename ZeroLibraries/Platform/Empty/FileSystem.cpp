///////////////////////////////////////////////////////////////////////////////
///
/// \file FileSystem.cpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
//----------------------------------------------------------- File System Functions
const Rune  cDirectorySeparatorRune = Rune('/');
const char cDirectorySeparatorCstr[] = "/";
bool cFileSystemCaseInsensitive = false;

String gWorkingDirectory;

bool CopyFileInternal(StringParam dest, StringParam source)
{
  return false;
}

bool MoveFileInternal(StringParam dest, StringParam source)
{
  return false;
}

bool DeleteFileInternal(StringParam file)
{
  return false;
}

bool DeleteDirectory(StringParam directory)
{
  return false;
}

bool PathIsRooted(StringParam directoryPath)
{
  return false;
}

void CreateDirectory(StringParam dest)
{
}

void CreateDirectoryAndParents(StringParam directory)
{
}

int CheckFileTime(StringParam dest, StringParam source)
{
  return 0;
}

bool SetFileToCurrentTime(StringParam filename)
{
  return false;
}

TimeType GetFileModifiedTime(StringParam filename)
{
  return 0;
}

u64 GetFileSize(StringParam fileName)
{
  return 0;
}

bool FileExists(StringParam filePath)
{
  return false;
}

bool FileWritable(StringParam filePath)
{
  return false;
}

bool DirectoryExists(StringParam directoryPath)
{
  return false;
}

bool IsDirectory(StringParam directoryPath)
{
  return false;
}

String CanonicalizePath(StringParam directoryPath)
{
  return directoryPath;
}

String GetWorkingDirectory()
{
  return gWorkingDirectory;
}

void SetWorkingDirectory(StringParam path)
{
  gWorkingDirectory = path;
}

String GetUserLocalDirectory()
{
  return gWorkingDirectory;
}

String GetUserDocumentsDirectory()
{
  return gWorkingDirectory;
}

String GetApplicationDirectory()
{
  return gWorkingDirectory;
}

String GetApplication()
{
  return String();
}

String GetTemporaryDirectory()
{
  return String();
}

String UniqueFileId(StringParam fullpath)
{
  return String();
}

//----------------------------------------------------------- File Range
struct FileRangePrivateData
{
};

FileRange::FileRange(StringParam path)
{
}

FileRange::~FileRange()
{
}

bool FileRange::Empty()
{
  return true;
}

String FileRange::Front()
{
  return String();
}

FileEntry FileRange::FrontEntry()
{
  return FileEntry();
}

void FileRange::PopFront()
{
}

}//namespace Zero
