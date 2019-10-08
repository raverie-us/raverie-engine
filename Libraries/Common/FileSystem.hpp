// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

DeclareEnum4(FileSystemErrors, FileNotFound, FileNotAccessible, FileNotWritable, FileLocked);

ZeroShared extern const Rune cDirectorySeparatorRune;
ZeroShared extern const char cDirectorySeparatorCstr[];
ZeroShared extern bool cFileSystemCaseSensitive;

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
ZeroShared void AddVirtualFileSystemEntry(StringParam absolutePath, DataBlock* stealData, TimeType modifiedTime);

/// Called when we want to ensure that files that have been written are
/// persisted/saved to a location that can be recalled. Note that not every
/// platform or file system uses this function, and it will return false if it's
/// unused. Files will automatically be recalled when the file system
/// initializes. It is advised to call this in a location where mass saving
/// occurs. NOTE: This function will automatically be called when the
/// Common/Platform library shuts down.
ZeroShared bool PersistFiles();

/// Copies a file. Will spin lock if fails up to a max number of iterations.
/// (Calls CopyFileInternal) This operation will overwrite the destination file
/// if it exists.
ZeroShared bool CopyFile(StringParam dest, StringParam source);
/// The actual platform specific file copy function. This operation will
/// overwrite the destination file if it exists.
ZeroShared bool CopyFileInternal(StringParam dest, StringParam source);

/// Move a file. Must be folder to folder or file to file.
/// Will spin lock if fails up to a max number of iterations. (Calls
/// MoveFileInternal) This operation will overwrite the destination file if it
/// exists.
ZeroShared bool MoveFile(StringParam dest, StringParam source);
/// The actual platform specific function. Move a file. Must be folder to folder
/// or file to file. This operation will overwrite the destination file if it
/// exists.
ZeroShared bool MoveFileInternal(StringParam dest, StringParam source);

/// Deletes a file. Will spin lock if fails up to a max number of iterations.
/// (Calls DeleteFileInternal)
ZeroShared bool DeleteFile(StringParam file);
/// The actual platform specific function.
ZeroShared bool DeleteFileInternal(StringParam dest);

/// Delete an entire directory
ZeroShared bool DeleteDirectory(StringParam directory);
ZeroShared bool DeleteDirectoryContents(StringParam directory);

/// Makes sure the directory exists but that it is empty.
/// Any existing contents will be deleted.
ZeroShared bool EnsureEmptyDirectory(StringParam directory);

/// Create a directory.
ZeroShared void CreateDirectory(StringParam dest);

/// Create a directory and any parent directories required
ZeroShared void CreateDirectoryAndParents(StringParam directory);

/// Finds the first part of the given directory path that doesn't exist.
/// Primarily used for error message printing.
ZeroShared String FindFirstMissingDirectory(StringParam directory);

/// -1 Destination is older or does not exist.
///  0 Destination and source are the same.
///  1 Destination is newer.
ZeroShared int CheckFileTime(StringParam dest, StringParam source);

/// Returns the date/time of the file
ZeroShared bool GetFileDateTime(StringParam filePath, CalendarDateTime& result);

/// Is the dest file older than the source file?
ZeroShared inline bool DestinationIsOlder(StringParam dest, StringParam source)
{
  return CheckFileTime(dest, source) <= 0;
}

/// Update file time to current time.
ZeroShared bool SetFileToCurrentTime(StringParam filename);

/// Get the file last modified time.
ZeroShared TimeType GetFileModifiedTime(StringParam filename);

/// Get Size of file zero if not found
ZeroShared u64 GetFileSize(StringParam fileName);

// Does the file exist?
ZeroShared bool FileExists(StringParam filePath);

// Is the file exist and is writable?
ZeroShared bool FileWritable(StringParam filePath);

// Does the directory exist?
ZeroShared bool DirectoryExists(StringParam directoryPath);

// Use OS dependent behavior to strip the path of redundancies (such as .., and
// . where they are redundant) Side note: Windows has a few mechanisms that are
// partially incorrect in different places, so it is probably best for us to
// call Normalize using FilePath before Canonicalizing For example: Windows
// doesn't remove redundant slashes
ZeroShared String CanonicalizePath(StringParam directoryPath);

// Returns if this path is rooted (not relative)
ZeroShared bool PathIsRooted(StringParam directoryPath);

/// Special Paths

/// Get the current working directory for this process.
ZeroShared String GetWorkingDirectory();

/// Set the working directory for this process.
ZeroShared void SetWorkingDirectory(StringParam path);

/// Directory for application cache and config files.
ZeroShared String GetUserLocalDirectory();

/// Directory for user modifiable configuration files.
ZeroShared String GetUserDocumentsDirectory();

/// Directory for user modifiable files specific to a remote application.
ZeroShared String GetRemoteUserDocumentsApplicationDirectory(StringParam organization, StringParam applicationName);

/// Directory for user modifiable files specific to our application.
ZeroShared String GetUserDocumentsApplicationDirectory();

/// Directory to the application.
ZeroShared String GetApplicationDirectory();

/// Full Path to the application.
ZeroShared String GetApplication();

/// Get directory for temporary files.
ZeroShared String GetTemporaryDirectory();

/// Get an id string for a file. String returned
/// will be unique for every file on the system.
ZeroShared String UniqueFileId(StringParam fullpath);

class ZeroShared FileEntry
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
class ZeroShared FileRange
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
  ZeroDeclarePrivateData(FileRange, 720);
};

} // namespace Zero
