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

bool CopyFileInternal(StringParam dest, StringParam source)
{
  Error("Not implemented");
  return false;
}

bool MoveFileInternal(StringParam dest, StringParam source)
{
  Error("Not implemented");
  return false;
}

bool DeleteFileInternal(StringParam file)
{
  Error("Not implemented");
  return false;
}

bool DeleteDirectory(StringParam directory)
{
  Error("Not implemented");
  return false;
}

bool PathIsRooted(StringParam directoryPath)
{
  Error("Not implemented");
  return false;
}

void CreateDirectory(StringParam dest)
{
  Error("Not implemented");
}

void CreateDirectoryAndParents(StringParam directory)
{
  Error("Not implemented");
}

String FindFirstMissingDirectory(StringParam directory)
{
  Error("Not implemented");
  return String();
}

int CheckFileTime(StringParam dest, StringParam source)
{
  Error("Not implemented");
  return 0;
}

bool SetFileToCurrentTime(StringParam filename)
{
  Error("Not implemented");
  return false;
}

TimeType GetFileModifiedTime(StringParam filename)
{
  Error("Not implemented");
  return 0;
}

u64 GetFileSize(StringParam fileName)
{
  Error("Not implemented");
  return 0;
}

bool FileExists(StringParam filePath)
{
  Error("Not implemented");
  return false;
}

bool FileWritable(StringParam filePath)
{
  Error("Not implemented");
  return false;
}

bool DirectoryExists(StringParam directoryPath)
{
  Error("Not implemented");
  return false;
}

bool IsDirectory(StringParam directoryPath)
{
  Error("Not implemented");
  return false;
}

String CanonicalizePath(StringParam directoryPath)
{
  Error("Not implemented");
  return directoryPath;
}

String GetWorkingDirectory()
{
  Error("Not implemented");
  return String();
}

void SetWorkingDirectory(StringParam path)
{
  Error("Not implemented");
}

String GetUserLocalDirectory()
{
  Error("Not implemented");
  return String();
}

String GetUserDocumentsDirectory()
{
  Error("Not implemented");
  return String();
}

String GetApplicationDirectory()
{
  Error("Not implemented");
  return String();
}

String GetApplication()
{
  Error("Not implemented");
  return String();
}

String GetTemporaryDirectory()
{
  Error("Not implemented");
  return String();
}

String UniqueFileId(StringParam fullpath)
{
  Error("Not implemented");
  return String();
}

//----------------------------------------------------------- File Range
struct FileRangePrivateData
{
};

FileRange::FileRange(StringParam path)
{
  Error("Not implemented");
}

FileRange::~FileRange()
{
  Error("Not implemented");
}

bool FileRange::Empty()
{
  Error("Not implemented");
  return true;
}

String FileRange::Front()
{
  Error("Not implemented");
  return String();
}

FileEntry FileRange::FrontEntry()
{
  Error("Not implemented");
  return FileEntry();
}

void FileRange::PopFront()
{
  Error("Not implemented");
}

}//namespace Zero
