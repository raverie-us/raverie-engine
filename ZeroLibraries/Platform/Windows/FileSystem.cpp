///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")

namespace Zero
{
const Rune  cDirectorySeparatorRune = Rune('\\');
const char* cDirectorySeparatorCstr = "\\";
bool cFileSystemCaseInsensitive = true;

void InitFileSystem()
{

}

void ShutdownFileSystem()
{

}

String GetWorkingDirectory()
{
  char temp[MAX_PATH+1];
  _getcwd(temp, MAX_PATH);
  ZeroStrCat(temp, MAX_PATH+1, cDirectorySeparatorCstr);
  return String(temp);
}

void SetWorkingDirectory(StringParam path)
{
  _chdir(path.c_str());
}

String GetUserLocalDirectory()
{
  wchar_t temp[MAX_PATH+1];
  // Consider making an option to set which path is returned for people who want to use networked drives with
  // proper permissions setup such that they work, same for the GetUserDocumentsDirectory function

  // SHGFP_TYPE_DEFAULT - Returns the default path for the requested folder
  // SHGFP_TYPE_CURRENT - Returns the redirect path for the requested folder
  SHGFP_TYPE pathType = SHGFP_TYPE_DEFAULT;

  // By calling get folder path with SHGFP_TYPE_DEFAULT it will return the path
  // %USERPROFILE%\AppData\Local even if the user has redirected it to a network drive
  // this avoids user permission errors caused by one drive or other networked drives
  SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, pathType, temp);
  return Narrow(temp);
}

String GetUserDocumentsDirectory()
{
  wchar_t temp[MAX_PATH+1];
  // SHGFP_TYPE_DEFAULT - Returns the default path for the requested folder
  // SHGFP_TYPE_CURRENT - Returns the redirect path for the requested folder
  SHGFP_TYPE pathType = SHGFP_TYPE_DEFAULT;

  // By calling get folder path with SHGFP_TYPE_DEFAULT it will return the path
  // %USERPROFILE%\Documents even if the user has redirected it to a network drive
  // this avoids user permission errors caused by one drive or other networked drives
  SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, pathType, temp);
  return Narrow(temp);
}

String GetApplicationDirectory()
{
  wchar_t temp[MAX_PATH+1];
  GetModuleFileName(NULL, temp, MAX_PATH);
  String fileName = Narrow(temp);

  ///FIX ME LATER!!!
  //return Zero::FilePath::GetDirectoryPath(fileName);
  StringRange found = fileName.FindLastOf(cDirectorySeparatorRune);
  if(found.Empty())
    return StringRange();

  return fileName.SubString(fileName.Begin(), found.Begin());
}

String GetApplication()
{
  wchar_t temp[MAX_PATH+1];
  GetModuleFileName(NULL, temp, MAX_PATH);
  return Narrow(temp);
}

String GetTemporaryDirectory()
{
  wchar_t tempPath[MAX_PATH];
  GetTempPath(MAX_PATH, tempPath);
  return Narrow(tempPath);
}

bool FileExists(StringParam filePath)
{
  DWORD attributes = GetFileAttributes(Widen(filePath).c_str());
  return (attributes!=INVALID_FILE_ATTRIBUTES);
}

bool FileWritable(StringParam filePath)
{
  DWORD attributes = GetFileAttributes(Widen(filePath).c_str());
  if(attributes == INVALID_FILE_ATTRIBUTES)
    return false;
  else
    return !(attributes & FILE_ATTRIBUTE_READONLY);
}

bool DirectoryExists(StringParam filePath)
{
  DWORD attributes = GetFileAttributes(Widen(filePath).c_str());
  return (attributes != INVALID_FILE_ATTRIBUTES);
}

bool IsDirectory(StringParam filePath)
{
  DWORD attributes = GetFileAttributes(Widen(filePath).c_str());
  return (attributes & FILE_ATTRIBUTE_DIRECTORY)!=0;
}

String CanonicalizePath(StringParam directoryPath)
{
  wchar_t buffer[MAX_PATH];
  if (PathCanonicalize(buffer, Widen(directoryPath).c_str()))
    return Narrow(buffer);
  else
    return directoryPath;
}

bool PathIsRooted(StringParam directoryPath)
{
  WString widePath = Widen(directoryPath);
  bool isRelative = PathIsRelative(widePath.c_str());
  return !isRelative;
}

void CreateDirectory(StringParam dest)
{
  BOOL success = ::CreateDirectoryW(Widen(dest).c_str(), NULL);
  if(!success)
  {
    DWORD lastErr = GetLastError();
    if(lastErr != ERROR_ALREADY_EXISTS)
      VerifyWin(success, "Failed to create the directory '%s'.", dest.c_str());
  }
}

void CreateDirectoryAndParents(StringParam directory)
{
  char directoryPath[MAX_PATH];
  ZeroStrCpy(directoryPath, MAX_PATH, directory.c_str());
  uint size = directory.SizeInBytes();

  for(uint c = 0; c < size; ++c)
  {
    //When there is a directory separator
    if(directoryPath[c] == cDirectorySeparatorRune.value)
    {
      if (c > 0 && directoryPath[c - 1] == ':')
        continue;
      //Null terminate
      directoryPath[c] = '\0';
      //Create directory
      CreateDirectory(directoryPath);
      //remove null terminator
      directoryPath[c] = cDirectorySeparatorRune.value;
    }
  }

  //directories no longer have a trailing '/' so we have to
  //explicitly create the final directory
  CreateDirectory(directory);
}

bool CopyFileInternal(StringParam dest, StringParam source)
{
  BOOL success = ::CopyFileW(Widen(source).c_str(), Widen(dest).c_str(), FALSE);
  VerifyWin(success, "Failed to copy file. %s to %s.", source.c_str(), dest.c_str());
  return success!=0;
}

bool MoveFileInternal(StringParam dest, StringParam source)
{
  BOOL success = ::MoveFileEx(Widen(source).c_str(), Widen(dest).c_str(), MOVEFILE_REPLACE_EXISTING);
  VerifyWin(success, "Failed to move file. %s to %s.", source.c_str(), dest.c_str());
  return success!=0;
}

bool DeleteFileInternal(StringParam file)
{
  BOOL success = ::DeleteFileW(Widen(file).c_str());
  VerifyWin(success, "Failed to delete file %s.", file.c_str());
  return success!=0;
}

bool DeleteDirectory(StringParam directory)
{
  if(!DirectoryExists(directory))
    return false;

  FileRange range(directory);
  for(; !range.Empty(); range.PopFront())
  {
    String name = range.Front();
    String fullName = BuildString(directory, cDirectorySeparatorCstr, name);
    if(IsDirectory(fullName))
      DeleteDirectory(fullName);
    else
      DeleteFile(fullName);
  }

  //this is the only part that needs to be updated platform specific
  BOOL success = ::RemoveDirectoryW(Widen(directory).c_str());
  VerifyWin(success, "Failed to delete directory %s.", directory.c_str());
  return success != 0;
}

TimeType SystemTimeToTimeType(SYSTEMTIME& systemTime)
{
  //Build a TimeType using mktime and systemTime
  tm newTime;
  memset(&newTime, 0, sizeof(tm));
  //tm_year is based at 1900
  newTime.tm_year = systemTime.wYear - 1900;
  //tm_mday is zero based
  newTime.tm_mon = systemTime.wMonth - 1;
  newTime.tm_mday = systemTime.wDay;
  newTime.tm_hour = systemTime.wHour;
  newTime.tm_min = systemTime.wMinute;
  newTime.tm_sec = systemTime.wSecond;
  return mktime(&newTime);
}

TimeType GetFileModifiedTime(StringParam filename)
{
  WIN32_FILE_ATTRIBUTE_DATA fileInfo;
  BOOL success = GetFileAttributesEx(Widen(filename).c_str(), GetFileExInfoStandard, (void*)&fileInfo);
  CheckWin(success, "Failed to get GetFileAttributesEx.");

  // Convert to system time so the time can be parsed
  SYSTEMTIME modifiedSystemTime;
  FileTimeToSystemTime(&fileInfo.ftLastWriteTime, &modifiedSystemTime);

  // Convert and return
  return SystemTimeToTimeType(modifiedSystemTime);
}

int SetFileToCurrentTime(StringParam filename)
{
  // Need a file handle to do file time operations
  StackHandle sourceFile;
  sourceFile = CreateFile(Widen(filename).c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, 0, NULL);

  FILETIME fileTime;
  SYSTEMTIME systemTime;
  BOOL result;

  // Gets the current system time
  GetSystemTime(&systemTime); 
  // Converts the current system time to file time format
  SystemTimeToFileTime(&systemTime, &fileTime);

  // Sets last-write time of the file 
  // to the converted current system time 
  result = SetFileTime(sourceFile,
    (LPFILETIME) NULL, 
    (LPFILETIME) NULL, 
    &fileTime);

  VerifyWin(result, "Failed to set file time.");

  return result;
}

u32 GetFileSize(StringParam fileName)
{
  WIN32_FILE_ATTRIBUTE_DATA fileInfo;
  BOOL success = GetFileAttributesEx(Widen(fileName).c_str(), GetFileExInfoStandard, (void*)&fileInfo);
  CheckWin(success, "Failed to get GetFileAttributesEx.");
  if(!success)
    return 0;
  return (u32)fileInfo.nFileSizeLow;
}

int CheckFileTime(StringParam dest, StringParam source)
{
  // Source should always exist
  WIN32_FILE_ATTRIBUTE_DATA sourceInfo;
  BOOL success = GetFileAttributesEx(Widen(source).c_str(), GetFileExInfoStandard, (void*)&sourceInfo);
  // If their was some error still return dest is older
  if(!success) return -1;

  WIN32_FILE_ATTRIBUTE_DATA destInfo;
  success = GetFileAttributesEx(Widen(dest).c_str(), GetFileExInfoStandard, (void*)&destInfo);
  // If dest does not exist it is older
  if(!success) return -1;

  // Do file time comparison
  int result = CompareFileTime(&destInfo.ftLastWriteTime, &sourceInfo.ftLastWriteTime);

  return result;
}

bool GetFileDateTime(StringParam filePath, CalendarDateTime& result)
{
  if(!FileExists(filePath))
    return false;

  //get the file time
  WIN32_FILE_ATTRIBUTE_DATA sourceInfo;
  BOOL success = GetFileAttributesEx(Widen(filePath).c_str(), GetFileExInfoStandard, (void*)&sourceInfo);

  //convert that to the system time (which has the year, months, day, etc...)
  SYSTEMTIME systemTime;
  FileTimeToSystemTime(&sourceInfo.ftLastWriteTime, &systemTime);

  //gotta convert to the local time zone (should really convert to pacific time, but it's too much work)
  SYSTEMTIME localSystemTime;
  SystemTimeToTzSpecificLocalTime(NULL, &systemTime, &localSystemTime);

  result.Year = localSystemTime.wYear;
  result.Month = localSystemTime.wMonth;
  result.Day = localSystemTime.wDay;
  result.Hour = localSystemTime.wHour;
  result.Minutes = localSystemTime.wMinute;
  result.Seconds = localSystemTime.wSecond;
  
  return true;
}

struct FileRangePrivateData
{
  cstr mCurrent;
  HANDLE mHandle;
  WIN32_FIND_DATA mFindData;
};


FileRange::FileRange(StringParam filePath)
{
  ZeroConstructPrivateData(FileRangePrivateData);
  mPath = filePath;
  if(mPath.Empty())
  {
    Error("Cannot create a file range from an empty directory/path string (working directory as empty string not supported)");
    self->mHandle = NULL;
    return;
  }

  // Copy String into temporary
  uint size = mPath.SizeInBytes();
  wchar_t path[MAX_PATH];
  ZeroCStringCopyW(path, MAX_PATH, Widen(mPath).c_str(), Widen(mPath).Size());

  // Check for trailing slash and add if not there
  if(path[size-1] != '\\')
    ZeroStrCatW(path, MAX_PATH, L"\\");

  // Add the wildcard to get all files in directory
  ZeroStrCatW(path, MAX_PATH, L"*");

  // Begin Windows file iteration
  self->mHandle = FindFirstFile(path, &self->mFindData);

  if(self->mHandle == INVALID_HANDLE_VALUE)
  {
    self->mHandle = NULL;
  }
  else
  {
    //Skip rid of "." and ".." directory results.
    if (wcscmp(self->mFindData.cFileName, L".") == 0)
      this->PopFront();
    if (self->mHandle && wcscmp(self->mFindData.cFileName, L"..") == 0)
      this->PopFront();
  }
}

FileRange::~FileRange()
{
  ZeroGetPrivateData(FileRangePrivateData);

  if(self->mHandle)
    FindClose(self->mHandle);

  ZeroDestructPrivateData(FileRangePrivateData);
}

bool FileRange::Empty()
{
  ZeroGetPrivateData(FileRangePrivateData);
  return self->mHandle==NULL;
}

String FileRange::Front()
{
  ZeroGetPrivateData(FileRangePrivateData);
  return Narrow(self->mFindData.cFileName);
}

FileEntry FileRange::frontEntry()
{
  ZeroGetPrivateData(FileRangePrivateData);

  LARGE_INTEGER largeInt;
  largeInt.LowPart = self->mFindData.nFileSizeLow;
  largeInt.HighPart = self->mFindData.nFileSizeHigh;

  FileEntry entry;
  entry.mFileName = Narrow(self->mFindData.cFileName);
  entry.mSize = largeInt.QuadPart;
  entry.mPath = mPath;
  return entry;
}

void FileRange::PopFront()
{
  ZeroGetPrivateData(FileRangePrivateData);
  BOOL hasNext = FindNextFile(self->mHandle, &self->mFindData);

  if(!hasNext)
  {
    // No more files
    FindClose(self->mHandle);
    self->mHandle = NULL;
  }
}

String UniqueFileId(StringParam fullpath)
{
#ifdef FILE_NAME_NORMALIZED
  StackHandle fileHandle;

  fileHandle = CreateFile(Widen(fullpath).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
    OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

  if(fileHandle == INVALID_HANDLE_VALUE)
  {
    //no file just return path.
    fileHandle = cInvalidHandle;
    return fullpath;
  }

  wchar_t fixedFullPath[MAX_PATH+1] = {0};
  DWORD size = GetFinalPathNameByHandle(fileHandle, fixedFullPath, MAX_PATH, FILE_NAME_NORMALIZED);

  if(size == 0)
    return fullpath;

  return Narrow(fixedFullPath);
#else
  return fullpath;
#endif
}

}//namespace Zero
