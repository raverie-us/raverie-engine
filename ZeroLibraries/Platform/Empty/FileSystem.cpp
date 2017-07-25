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
#include "Platform/FileSystem.hpp"

namespace Zero
{
//----------------------------------------------------------- File System Functions
const char  cDirectorySeparatorChar = '/';
const char* cDirectorySeparatorCstr = "/";

bool CopyFileInternal(StringRef dest, StringRef source)
{
  Error("Not implemented");
  return false;
}

bool MoveFileInternal(StringRef dest, StringRef source)
{
  Error("Not implemented");
  return false;
}

bool DeleteFileInternal(StringRef file)
{
  Error("Not implemented");
  return false;
}

void CreateDirectory(StringRef dest)
{
  Error("Not implemented");
}

void CreateDirectoryAndParents(StringRef directory)
{
  Error("Not implemented");
}

int CheckFileTime(StringRef dest, StringRef source)
{
  Error("Not implemented");
  return 0;
}

int SetFileToCurrentTime(StringRef filename)
{
  Error("Not implemented");
  return 0;
}

TimeType GetFileModifiedTime(StringRef filename)
{
  Error("Not implemented");
  return 0;
}

u32 GetFileSize(StringRef fileName)
{
  Error("Not implemented");
  return 0;
}

bool FileExists(StringRef filePath)
{
  Error("Not implemented");
  return false;
}

bool FileWritable(StringRef filePath)
{
  Error("Not implemented");
  return false;
}

bool DirectoryExists(StringRef directoryPath)
{
  Error("Not implemented");
  return false;
}

bool IsDirectory(StringRef directoryPath)
{
  Error("Not implemented");
  return false;
}

String CanonicalizePath(StringRef directoryPath)
{
  Error("Not implemented");
  return directoryPath;
}

String GetWorkingDirectory()
{
  Error("Not implemented");
  return String();
}

void SetWorkingDirectory(String path)
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

String UniqueFileId(StringRef fullpath)
{
  Error("Not implemented");
  return fullpath;
}

//----------------------------------------------------------- File Range
struct FileRangePrivateData
{
};

FileRange::FileRange(StringRef path)
{
  Error("Not implemented");
  ZeroConstructPrivateData(FileRangePrivateData);
}

FileRange::~FileRange()
{
  ZeroGetPrivateData(FileRangePrivateData);
  // Destructor logic
  ZeroDestructPrivateData(FileRangePrivateData);
}

bool FileRange::empty()
{
  ZeroGetPrivateData(FileRangePrivateData);
  return true;
}

cstr FileRange::front()
{
  ZeroGetPrivateData(FileRangePrivateData);
  return "";
}

void FileRange::popFront()
{
  ZeroGetPrivateData(FileRangePrivateData);
}

}//namespace Zero
