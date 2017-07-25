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
#include "Platform/File.hpp"
#include "Platform/FileSystem.hpp"
#include "String/StringBuilder.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

namespace Zero
{

const char  cDirectorySeparatorChar = '/';
const char* cDirectorySeparatorCstr = "/";


void InitFileSystem()
{

}

void ShutdownFileSystem()
{

}

bool CopyFileInternal(StringRef dest, StringRef source)
{
  FILE* sourceFile = fopen(source.c_str(), "rb");
  if(sourceFile == NULL)
    return false;

  FILE* destFile = fopen(dest.c_str(), "wb");
  if(destFile == NULL)
  { 
    fclose(sourceFile);
    return false;
  }

  const int bufferSize = 4096;

  char buf[bufferSize];
  for(;;)
  {
    ssize_t bytesRead = fread(buf, 1, bufferSize, sourceFile);
    int written = fwrite(buf, bytesRead, 1, destFile);
    if (bytesRead!=bufferSize) 
      break;
  }

  fclose(sourceFile);
  fclose(destFile);
  return true;
}

bool VerifyPosix(int returnCode, cstr operation)
{
  if(returnCode != 0)
  {
    ZPrint("While: %s Error: %s\n", operation, strerror(errno) );
    return false;
  }
  else
  {
    return true;
  }
}

bool MoveFileInternal(StringRef dest, StringRef source)
{
  return  VerifyPosix(rename(source.c_str(), dest.c_str()), "MoveFile");
}

bool DeleteFileInternal(StringRef filename)
{
  return VerifyPosix(unlink(filename.c_str()), "DeleteFile");
}

bool DeleteDirectory(StringRef name)
{
  return VerifyPosix(rmdir(name.c_str()), "DeleteDirectory");
}

void CreateDirectory(StringRef dest)
{
  int failCode = mkdir(dest.c_str(),
    S_IRUSR | S_IWUSR | S_IXUSR |
    S_IRGRP | S_IWGRP | S_IXGRP |
    S_IROTH | S_IXOTH );

  if(failCode != 0)
  {
    // If the error is anything except already exists
    if(errno != EEXIST)
      ZPrint("Failed to create directory %s Error: %s\n", dest.c_str(), strerror(errno) );
  }
}

void CreateDirectoryAndParents(StringRef directory)
{
  char directoryPath[File::MaxPath];
  ZeroStrCpy(directoryPath, File::MaxPath, directory.c_str());
  uint size = strlen(directoryPath);
  for(uint c=0;c<size;++c)
  {
    //When their is a directory separator
    if(directoryPath[c] == cDirectorySeparatorChar && c > 0)
    {
      //Null terminate
      directoryPath[c] = '\0';
      //Create directory
      CreateDirectory(directoryPath);
      //remove null terminator
      directoryPath[c] = cDirectorySeparatorChar;
    }
  }

  // Finally create the directory
  CreateDirectory(directoryPath);
}


int CheckFileTime(StringRef dest, StringRef source)
{
  struct stat destStat;
  if(stat(dest.c_str(), &destStat)!=0)
    return -1;

  struct stat sourceStat;
  if(stat(source.c_str(), &sourceStat)!=0)
    return 1;

  if(destStat.st_mtime > sourceStat.st_mtime)
    return 1;

  if(destStat.st_mtime == sourceStat.st_mtime)
    return 0;
  else
    return -1;
}

time_t GetFileModifiedTime(StringRef file)
{
  struct stat fileStat;
  if(stat(file.c_str(), &fileStat)!=0)
    return (time_t)fileStat.st_mtime;
  return 0;
}

int SetFileToCurrentTime(StringRef filename)
{
  return VerifyPosix(utimensat(AT_FDCWD, filename.c_str(), NULL, 0), "Updating File Time");
}

u32 GetFileSize(StringRef fileName)
{
  struct stat st;
  stat(fileName.c_str(), &st);
  return (uint)st.st_size;
}

bool FileExists(StringRef filePath)
{
  struct stat st;
  return stat(filePath.c_str(), &st) != -1;
}

bool DirectoryExists(StringRef directoryPath)
{
  struct stat st;
  if(stat(directoryPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
    return true;
  return false;
}

// Return true if it is directory path and it exists
bool IsDirectory(StringRef directoryPath)
{
  struct stat st;
  if(stat(directoryPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
    return true;
  return false;
}

String CanonicalizePath(StringRef directoryPath)
{
  Error("CanonicalizePath not yet supported"); 
  return directoryPath;
}

bool FileWritable(StringRef filePath)
{
  return access(filePath.c_str(), R_OK) == 0;
}

String UniqueFileId(StringRef fullpath)
{
   char buffer[File::MaxPath] = {0};
   realpath(fullpath.c_str(), buffer);

  //on unix the path is unique
  return fullpath;
}

String GetWorkingDirectory()
{
  char temp[File::MaxPath+1];
  getcwd(temp, File::MaxPath);
  return temp;
}

void SetWorkingDirectory(String path)
{
  chdir(path.c_str());
}

String GetUserLocalDirectory()
{
  // Use the standard ~/.cache location
  char local[File::MaxPath+1] = {0};
  ZeroStrCpy(local, File::MaxPath, getenv("HOME"));
  ZeroStrCat(local, File::MaxPath, "/.cache");
  return local;
}

String GetUserDocumentsDirectory()
{
  return getenv("HOME");
}

String GetTemporaryDirectory()
{
  return "/tmp";
}

String GetApplication()
{
  return String();
}

String GetApplicationDirectory()
{
  return String();
}

struct FileRangePrivateData
{
  DIR* mDir;
  struct dirent* mEntry;
};

FileRange::FileRange(StringRef search)
{ 
  ZeroConstructPrivateData(FileRangePrivateData);
  DIR* dir = opendir(search.c_str());
  if(dir)
  {
    self->mDir = dir;
    self->mEntry = readdir(dir);
  }
  else
  {
    self->mDir = NULL;
    self->mEntry = NULL;
  }
}

FileRange::~FileRange()
{
  ZeroGetPrivateData(FileRangePrivateData);
  closedir(self->mDir);
}

bool FileRange::empty()
{
  ZeroGetPrivateData(FileRangePrivateData);
  return self->mEntry == NULL;
}

cstr FileRange::front()
{
  ZeroGetPrivateData(FileRangePrivateData);
  return self->mEntry->d_name;
}

void FileRange::popFront()
{
  ZeroGetPrivateData(FileRangePrivateData);
  self->mEntry = readdir(self->mDir);

  //Get rid of "." and ".." directory results.
  if(!empty() && strcmp(front(), ".") == 0)
    popFront();

  if(!empty() && strcmp(front(), "..") == 0)
    popFront();
}

}//namespace Zero
