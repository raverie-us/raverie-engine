// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

DeclareEnum4(FileSystemErrors, FileNotFound, FileNotAccessible, FileNotWritable, FileLocked);

extern const Rune cDirectorySeparatorRune;
extern const char cDirectorySeparatorCstr[];
extern bool cFileSystemCaseSensitive;

/// Some platforms require initialization of thier file system (mounting
/// devices, virtual files, etc). Create this object on the stack and keep it
/// alive until you're done (with scopes).
class FileSystemInitializer
{
public:
  typedef void (*PopulateVirtualFileSystem)(void* userData);
  FileSystemInitializer(PopulateVirtualFileSystem callback = nullptr, void* userData = nullptr);
  ~FileSystemInitializer();
};

/// Creates a virtual file or directory (only when running in virtual mode).
/// An empty/0-size or null data block indicates a directory rather than a file.
/// Only called during the PopulateVirtualFileSystem callback.
void AddVirtualFileSystemEntry(StringParam absolutePath, DataBlock* stealData, TimeType modifiedTime);

/// Copies a file. Will spin lock if fails up to a max number of iterations.
/// (Calls CopyFileInternal) This operation will overwrite the destination file
/// if it exists.
bool CopyFile(StringParam dest, StringParam source);
/// The actual platform specific file copy function. This operation will
/// overwrite the destination file if it exists.
bool CopyFileInternal(StringParam dest, StringParam source);

/// Move a file. Must be folder to folder or file to file.
/// Will spin lock if fails up to a max number of iterations. (Calls
/// MoveFileInternal) This operation will overwrite the destination file if it
/// exists.
bool MoveFile(StringParam dest, StringParam source);
/// The actual platform specific function. Move a file. Must be folder to folder
/// or file to file. This operation will overwrite the destination file if it
/// exists.
bool MoveFileInternal(StringParam dest, StringParam source);

/// Deletes a file. Will spin lock if fails up to a max number of iterations.
/// (Calls DeleteFileInternal)
bool DeleteFile(StringParam file);
/// The actual platform specific function.
bool DeleteFileInternal(StringParam dest);

/// Delete an entire directory
bool DeleteDirectory(StringParam directory);
bool DeleteDirectoryContents(StringParam directory);

/// Makes sure the directory exists but that it is empty.
/// Any existing contents will be deleted.
bool EnsureEmptyDirectory(StringParam directory);

/// Create a directory.
void CreateDirectory(StringParam dest);

/// Create a directory and any parent directories required
void CreateDirectoryAndParents(StringParam directory);

/// Finds the first part of the given directory path that doesn't exist.
/// Primarily used for error message printing.
String FindFirstMissingDirectory(StringParam directory);

/// -1 Destination is older or does not exist.
///  0 Destination and source are the same.
///  1 Destination is newer.
int CheckFileTime(StringParam dest, StringParam source);

/// Returns the date/time of the file
bool GetFileDateTime(StringParam filePath, CalendarDateTime& result);

/// Is the dest file older than the source file?
inline bool DestinationIsOlder(StringParam dest, StringParam source)
{
  return CheckFileTime(dest, source) <= 0;
}

/// Update file time to current time.
bool SetFileToCurrentTime(StringParam filename);

/// Get the file last modified time.
TimeType GetFileModifiedTime(StringParam filename);

/// Get Size of file zero if not found
u64 GetFileSize(StringParam fileName);

// Does the file exist?
bool FileExists(StringParam filePath);

// Is the file exist and is writable?
bool FileWritable(StringParam filePath);

// Does the directory exist?
bool DirectoryExists(StringParam directoryPath);

// Use OS dependent behavior to strip the path of redundancies (such as .., and
// . where they are redundant) Side note: Windows has a few mechanisms that are
// partially incorrect in different places, so it is probably best for us to
// call Normalize using FilePath before Canonicalizing For example: Windows
// doesn't remove redundant slashes
String CanonicalizePath(StringParam directoryPath);

// Returns if this path is rooted (not relative)
bool PathIsRooted(StringParam directoryPath);

/// Special Paths

/// Get the current working directory for this process.
String GetWorkingDirectory();

/// Set the working directory for this process.
void SetWorkingDirectory(StringParam path);

/// Directory for application cache and config files.
String GetUserLocalDirectory();

/// Directory for user modifiable configuration files.
String GetUserDocumentsDirectory();

/// Directory for user modifiable files specific to a remote application.
String GetRemoteUserDocumentsApplicationDirectory(StringParam organization, StringParam applicationName);

/// Directory for user modifiable files specific to our application.
String GetUserDocumentsApplicationDirectory();

/// Directory to the application.
String GetApplicationDirectory();

/// Full Path to the application.
String GetApplication();

/// Get directory for temporary files.
String GetTemporaryDirectory();

/// Get an id string for a file. String returned
/// will be unique for every file on the system.
String UniqueFileId(StringParam fullpath);

class FileEntry
{
public:
  String mPath;
  String mFileName;
  u64 mSize;

  String GetFullPath()
  {
    return FilePath::Combine(mPath, mFileName);
  }
};

/// File range for iterating over files in a directory
/// This only returns the file name (must be combined with the full path)
class FileRange
{
public:
  // Path of directory to walk
  FileRange(StringParam filePath);
  ~FileRange();

  // Range interface
  bool Empty();
  String Front();
  // This function should be used over front (and should eventually replace it)
  FileEntry FrontEntry();

  void PopFront();

  FileRange& All()
  {
    return *this;
  }
  const FileRange& All() const
  {
    return *this;
  }

private:
  String mPath;
  RaverieDeclarePrivateData(FileRange, 720);
};

} // namespace Raverie
