// MIT Licensed (see LICENSE.md).
#include <Windows.h>
#include "../Windows/WString.hpp"
#define AT_FDCWD -100
#define R_OK 0x04
#define S_ISDIR(dir) true
#define ZeroAllPermissions (0777)

struct dirent
{
  const char* d_name;
};

struct DIR
{
  dirent mDirent;
  Zero::String mCurrent;
  HANDLE mHandle;
  WIN32_FIND_DATA mFindData;
};

DIR* opendir(const char* dirPath)
{
  DIR* result = new DIR();
  result->mDirent.d_name = result->mCurrent.c_str();
  result->mHandle = nullptr;
  memset(&result->mFindData, 0, sizeof(result->mFindData));

  if (dirPath == nullptr || strlen(dirPath) == 0)
    return result;

  // Copy String into temporary
  wchar_t path[MAX_PATH];
  Zero::WString wpath = Zero::Widen(dirPath);
  wcsncpy_s(path, MAX_PATH, wpath.c_str(), wpath.Size());

  // Check for trailing slash and add if not there
  if (path[wpath.Size() - 1] != '\\')
    ZeroStrCatW(path, MAX_PATH, L"\\");

  // Add the wildcard to get all files in directory
  ZeroStrCatW(path, MAX_PATH, L"*");

  // Begin Windows file iteration
  result->mHandle = FindFirstFile(path, &result->mFindData);

  if (result->mHandle == INVALID_HANDLE_VALUE)
    result->mHandle = nullptr;

  return result;
}

void closedir(DIR* dir)
{
  if (dir->mHandle)
    FindClose(dir->mHandle);

  delete dir;
}

dirent* readdir(DIR* dir)
{
  if (!dir->mHandle)
    return nullptr;

  dir->mCurrent = Zero::Narrow(dir->mFindData.cFileName);
  dir->mDirent.d_name = dir->mCurrent.c_str();

  BOOL hasNext = FindNextFile(dir->mHandle, &dir->mFindData);

  if (!hasNext)
  {
    // No more files
    FindClose(dir->mHandle);
    dir->mHandle = nullptr;
  }

  return &dir->mDirent;
}

int mkdir(const char* dirPath, uint chmod)
{
  return mkdir(dirPath);
}

void realpath(const char* path, char* buffer)
{
  Zero::String normalizedPath = Zero::FilePath::Normalize(path);
  size_t copySize = Math::Min(normalizedPath.SizeInBytes(), (size_t)Zero::File::MaxPath);
  memcpy(buffer, normalizedPath.c_str(), copySize);
  buffer[copySize] = 0;
}

int utimensat(int dirfd, const char* pathname, const struct timespec times[2], int flags)
{
  FILE* touch = fopen(pathname, "ab+");
  if (touch)
  {
    fclose(touch);
    return 0;
  }
  return 1;
}
