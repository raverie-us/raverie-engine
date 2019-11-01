// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

// This is only used to make it compile for easy testing
#if defined(WelderTargetOsWindows)
#  include "FileSystemWindowsEmulation.inl"
#else
#  include <unistd.h>
#  include <dirent.h>
#  define ZeroAllPermissions (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH)
#endif

namespace Zero
{

const Rune cDirectorySeparatorRune = '/';
const char cDirectorySeparatorCstr[] = "/";
const String cDirectorySeparatorString("/");
bool cFileSystemCaseSensitive = false;

static const String cCurrentDirectory(".");
static const String cParentDirectory("..");

FileSystemInitializer::FileSystemInitializer(PopulateVirtualFileSystem callback, void* userData)
{
#if defined(WelderTargetOsEmscripten)
  // Calling this will allow the outside product to populate the
  // virtual file system by calling 'AddVirtualFileSystemEntry'.
  if (callback)
    callback(userData);
#endif
}

FileSystemInitializer::~FileSystemInitializer()
{
}

void AddVirtualFileSystemEntry(StringParam absolutePath, DataBlock* stealData, TimeType modifiedTime)
{
  ErrorIf(!PathIsRooted(absolutePath),
          "The given path should have been an absolute/rooted path '%s'",
          absolutePath.c_str());

  // Create our entries for our files based on name, data, and modified time
  // If the size is 0, then it's a directory
  if (stealData == nullptr || stealData->Data == nullptr || stealData->Size == 0)
  {
    CreateDirectoryAndParents(absolutePath);
  }
  else
  {
    // Make sure the directory exists before we try and open a file inside of
    // it.
    String directory = FilePath::GetDirectoryPath(absolutePath);
    CreateDirectoryAndParents(directory);

    // The file may already exist if it was persisted. If so, don't write over
    // it!
    if (!FileExists(absolutePath))
    {
      FILE* file = fopen(absolutePath.c_str(), "wb");

      ReturnIf(!file,
               ,
               "Could not open file '%s', was the parent directory '%s' "
               "created? (dir exists: %d, file exists: %d)",
               absolutePath.c_str(),
               directory.c_str(),
               (int)DirectoryExists(directory),
               (int)FileExists(absolutePath));

      size_t written = fwrite(stealData->Data, 1, stealData->Size, file);
      ErrorIf(written != stealData->Size, "Could not write all data to file '%s'", absolutePath.c_str());

      fclose(file);
      struct utimbuf times;
      times.actime = (time_t)modifiedTime;
      times.modtime = (time_t)modifiedTime;
      utime(absolutePath.c_str(), &times);
    }
  }
}

bool PersistFiles()
{
#if defined(WelderTargetOsEmscripten)
  EM_ASM(FS.syncfs(false, console.log));
  ZPrint("Persisting files...\n");
  return true;
#else
  return false;
#endif
}

bool CopyFileInternal(StringParam dest, StringParam source)
{
  FILE* sourceFile = fopen(source.c_str(), "rb");
  if (sourceFile == nullptr)
    return false;

  FILE* destFile = fopen(dest.c_str(), "wb");
  if (destFile == nullptr)
  {
    fclose(sourceFile);
    return false;
  }

  const int bufferSize = 4096;

  bool result = true;
  char buf[bufferSize];
  for (;;)
  {
    size_t bytesRead = fread(buf, 1, bufferSize, sourceFile);
    if (ferror(sourceFile))
    {
      result = false;
      break;
    }

    int written = fwrite(buf, bytesRead, 1, destFile);
    if (!written)
    {
      result = false;
      break;
    }
    if (ferror(destFile))
    {
      result = false;
      break;
    }

    if (bytesRead != bufferSize)
      break;
  }

  fclose(sourceFile);
  fclose(destFile);
  return result;
}

bool VerifyPosix(int returnCode, cstr operation)
{
  if (returnCode != 0)
  {
    ZPrint("While: %s Error: %s\n", operation, strerror(errno));
    return false;
  }
  else
  {
    return true;
  }
}

bool MoveFileInternal(StringParam dest, StringParam source)
{
  return VerifyPosix(rename(source.c_str(), dest.c_str()), "MoveFile");
}

bool DeleteFileInternal(StringParam filename)
{
  return VerifyPosix(unlink(filename.c_str()), "DeleteFile");
}

bool DeleteDirectory(StringParam name)
{
  return VerifyPosix(rmdir(name.c_str()), "DeleteDirectory");
}

void CreateDirectory(StringParam dest)
{
  int failCode = mkdir(dest.c_str(), ZeroAllPermissions);

  if (failCode != 0)
  {
    // If the error is anything except already exists
    if (errno != EEXIST)
      ZPrint("Failed to create directory '%s': %s\n", dest.c_str(), strerror(errno));
  }
}

void CreateDirectoryAndParents(StringParam directory)
{
  if (directory.Empty())
    return;

  // Normalize it first to ensure we don't have any double slashes.
  // This also ensures we don't have a trailing separator.
  String path = FilePath::Normalize(directory);

  // Copy the string (including null) to the temp buffer since we're going to
  // modify it by adding nulls.
  char* temp = (char*)alloca(directory.SizeInBytes() + 1);
  memcpy(temp, directory.c_str(), directory.SizeInBytes() + 1);

  // We don't want to attempt to create the root in case the path is rooted
  // however, we always know we can at least skip the first character even if it
  // isn't rooted because we know a directory name will have at least one
  // character.
  for (char* it = temp + 1; *it != 0; ++it)
  {
    // If we hit the directory separator, temporarily substitute a null
    // character and call make directory, then put it back and continue walking.
    if (*it == '/')
    {
      *it = 0;
      CreateDirectory(temp);
      *it = '/';
    }
  }

  // Finally make the entire directory.
  CreateDirectory(temp);
}

time_t GetFileModifiedTime(StringParam file)
{
  struct stat st;
  if (stat(file.c_str(), &st) == 0)
    return (time_t)st.st_mtime;
  return 0;
}

bool SetFileToCurrentTime(StringParam filename)
{
  return VerifyPosix(utime(filename.c_str(), nullptr), "Updating File Time");
}

u64 GetFileSize(StringParam fileName)
{
  struct stat st;
  if (stat(fileName.c_str(), &st) == 0)
    return (uint)st.st_size;
  return 0;
}

bool FileExists(StringParam filePath)
{
  struct stat st;
  return stat(filePath.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

bool DirectoryExists(StringParam directoryPath)
{
  struct stat st;
  return stat(directoryPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

String CanonicalizePath(StringParam directoryPath)
{
  return FilePath::Normalize(directoryPath);
}

bool PathIsRooted(StringParam directoryPath)
{
  return directoryPath.StartsWith(cDirectorySeparatorString);
}

bool FileWritable(StringParam filePath)
{
  return access(filePath.c_str(), R_OK) == 0;
}

String UniqueFileId(StringParam fullpath)
{
  char buffer[File::MaxPath + 1] = {0};
  realpath(fullpath.c_str(), buffer);

  // on unix the path is unique
  return fullpath;
}

String GetWorkingDirectory()
{
  char temp[File::MaxPath + 1];
  getcwd(temp, File::MaxPath);
  return temp;
}

void SetWorkingDirectory(StringParam path)
{
  chdir(path.c_str());
}

String GetUserLocalDirectory()
{
  // Use the standard ~/.cache location
  char local[File::MaxPath + 1] = {0};
  ZeroStrCpy(local, File::MaxPath, GetUserDocumentsDirectory().c_str());
  ZeroStrCat(local, File::MaxPath, "/.cache");
  return local;
}

String GetUserDocumentsDirectory()
{
  const char* home = getenv("HOME");
  if (home && strlen(home) != 0)
    return home;

  const char* userProfile = getenv("USERPROFILE");
  if (userProfile && strlen(userProfile) != 0)
  {
    String documents = FilePath::Combine(userProfile, "Documents");
    if (DirectoryExists(documents))
      return documents;
  }

  return "/Documents/";
}

String GetTemporaryDirectory()
{
  const char* temp = getenv("TMPDIR");
  if (temp && strlen(temp) != 0)
    return temp;

  temp = getenv("TEMP");
  if (temp && strlen(temp) != 0)
    return temp;

  temp = getenv("TMP");
  if (temp && strlen(temp) != 0)
    return temp;

  return FilePath::GetDirectoryPath(tmpnam(nullptr));
}

String GetApplication()
{
  // The first entry in the command line arguments should be our executable.
  ReturnIf(
      gCommandLineArguments.Empty(), "/Main.app", "The command line arguments should not be empty, were they set?");
  return gCommandLineArguments.Front();
}

struct FileRangePrivateData
{
  DIR* mDir;
  struct dirent* mEntry;
};

FileRange::FileRange(StringParam search) : mPath(search)
{
  ZeroConstructPrivateData(FileRangePrivateData);
  DIR* dir = opendir(search.c_str());
  self->mEntry = nullptr;
  self->mDir = dir;

  PopFront();
}

FileRange::~FileRange()
{
  ZeroGetPrivateData(FileRangePrivateData);
  if (self->mDir)
    closedir(self->mDir);
  ZeroDestructPrivateData(FileRangePrivateData);
}

bool FileRange::Empty()
{
  ZeroGetPrivateData(FileRangePrivateData);
  return self->mEntry == nullptr;
}

String FileRange::Front()
{
  ZeroGetPrivateData(FileRangePrivateData);
  return self->mEntry->d_name;
}

FileEntry FileRange::FrontEntry()
{
  ZeroGetPrivateData(FileRangePrivateData);
  FileEntry entry;
  entry.mFileName = self->mEntry->d_name;
  entry.mPath = mPath;
  entry.mSize = GetFileSize(FilePath::Combine(mPath, entry.mFileName));
  return entry;
}

void FileRange::PopFront()
{
  ZeroGetPrivateData(FileRangePrivateData);
  if (!self->mDir)
    return;

  self->mEntry = readdir(self->mDir);

  // Get rid of "." and ".." directory results.
  if (!Empty() && Front() == cCurrentDirectory)
    PopFront();

  if (!Empty() && Front() == cParentDirectory)
    PopFront();
}

} // namespace Zero
