// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "FileSupport.hpp"
#include "Platform/FileSystem.hpp"
#include "String/StringBuilder.hpp"
#include "Regex/Regex.hpp"
#include "Platform/FilePath.hpp"
#include "Utility/Time.hpp"

extern unsigned char VirtualFileSystemData[];
extern unsigned int VirtualFileSystemSize;

namespace Zero
{
void WriteStringRangeToFile(StringParam path, StringRange range)
{
  WriteToFile(path.c_str(), (byte*)range.Data(), range.SizeInBytes());
}

bool MoveFolderContents(StringParam dest, StringParam source, FileFilter* filter)
{
  ReturnIf(source.Empty(),
           false,
           "Cannot copy from an empty directory/path string (working directory "
           "as empty string not supported)");
  ReturnIf(dest.Empty(),
           false,
           "Cannot copy to an empty directory/path string (working directory "
           "as empty string not supported)");

  CreateDirectoryAndParents(dest);

  bool success = true;
  FileRange files(source);
  for (; !files.Empty(); files.PopFront())
  {
    String sourceFile = FilePath::Combine(source, files.Front());
    String destFile = FilePath::Combine(dest, files.Front());

    // Filter
    FilterResult::Enum filterResult = FilterResult::Include;
    if (filter)
      filterResult = filter->Filter(sourceFile);

    // Skip
    if (filterResult == FilterResult::Ignore)
      continue;

    if (DirectoryExists(sourceFile))
    {
      success &= MoveFolderContents(destFile, sourceFile);
    }
    else
    {
      success &= MoveFile(destFile, sourceFile);
    }
  }
  return success;
}

void CopyFolderContents(StringParam dest, StringParam source, FileFilter* filter)
{
  CreateDirectoryAndParents(dest);

  FileRange files(source);
  for (; !files.Empty(); files.PopFront())
  {
    // If the path is a directory enumerate it
    String sourceFile = FilePath::Combine(source, files.Front());
    String destFile = FilePath::Combine(dest, files.Front());

    // Filter
    FilterResult::Enum filterResult = FilterResult::Include;
    if (filter)
      filterResult = filter->Filter(sourceFile);

    // Skip the file
    if (filterResult == FilterResult::Ignore)
      continue;

    if (DirectoryExists(sourceFile))
    {
      CopyFolderContents(destFile, sourceFile, filter);
    }
    else
    {
      CopyFile(destFile, sourceFile);
    }
  }
}

void FindFilesRecursively(StringParam path, Array<String>& foundFiles)
{
  // If this is a file then just add this to the results and return
  if (!DirectoryExists(path))
  {
    foundFiles.PushBack(path);
    return;
  }

  // Otherwise, iterate over all files and folders in the directory. The
  // recursion will take care of adding all the files
  FileRange fileRange(path);
  for (; !fileRange.Empty(); fileRange.PopFront())
  {
    FileEntry entry = fileRange.FrontEntry();
    String filePath = entry.GetFullPath();
    FindFilesRecursively(filePath, foundFiles);
  }
}

void FindFilesRecursively(StringParam path, Array<String>& foundFiles, FileFilter* filter)
{
  // If this is a file then just add this to the results and return
  if (!DirectoryExists(path))
  {
    if (filter->Filter(path) == FilterResult::Include)
      foundFiles.PushBack(path);
    return;
  }

  // Otherwise, iterate over all files and folders in the directory. The
  // recursion will take care of adding all the files
  FileRange fileRange(path);
  for (; !fileRange.Empty(); fileRange.PopFront())
  {
    FileEntry entry = fileRange.FrontEntry();
    String filePath = entry.GetFullPath();
    FindFilesRecursively(filePath, foundFiles, filter);
  }
}

FilterResult::Enum FilterFileRegex::Filter(StringParam filename)
{
  if (!mAccept.Empty())
  {
    Regex regex(mAccept);
    Matches matches;
    regex.Search(filename, matches);
    if (matches.Empty())
      return FilterResult::Ignore;
  }

  if (!mIgnore.Empty())
  {
    Regex regex(mIgnore);
    Matches matches;
    regex.Search(filename, matches);
    if (!matches.Empty())
      return FilterResult::Ignore;
  }

  return FilterResult::Include;
}

FilterResult::Enum ExtensionFilterFile::Filter(StringParam filename)
{
  String extension = FilePath::GetExtension(filename);
  if (!mCaseSensative)
  {
    extension = extension.ToLower();
  }

  if (extension == mExtension)
    return FilterResult::Include;
  return FilterResult::Ignore;
}

String GetTimeAndDateStamp()
{
  // get time now
  TimeType t = Time::GetTime();
  CalendarDateTime now = Time::GetLocalTime(t);
  // struct tm * now = localtime(&t);
  return String::Format("%d-%d-%d--%d-%02d-%02d", now.Year, now.Month + 1, now.Day, now.Hour, now.Minutes, now.Seconds);
}

String GetDate()
{
  TimeType t = Time::GetTime();
  CalendarDateTime now = Time::GetLocalTime(t);
  return String::Format("%d/%d/%d", now.Month + 1, now.Day, now.Year);
}

// Make a time stamp backup of the file.
void BackUpFile(StringParam backupPath, StringParam fileName)
{
  CreateDirectoryAndParents(backupPath);

  String timeStamp = BuildString("--", GetTimeAndDateStamp());

  String extension = BuildString(".", FilePath::GetExtension(fileName));

  String baseFileName = FilePath::GetFileNameWithoutExtension(fileName);

  String fileNameWithExt = BuildString(baseFileName, timeStamp, extension);
  String backupFile = FilePath::Combine(backupPath, fileNameWithExt);

  CopyFile(backupFile, fileName);
}

DataBlock AllocateBlock(size_t size)
{
  DataBlock block;
  block.Data = (byte*)zAllocate(size);
  block.Size = size;
  return block;
}

void FreeBlock(DataBlock& block)
{
  zDeallocate(block.Data);
  block.Data = nullptr;
}

void CloneBlock(DataBlock& destBlock, const DataBlock& source)
{
  destBlock.Data = (byte*)zAllocate(source.Size);
  destBlock.Size = source.Size;
  memcpy(destBlock.Data, source.Data, source.Size);
}

u64 ComputeFolderSizeRecursive(StringParam folder)
{
  u64 totalSize = 0;
  FileRange range(folder);
  for (; !range.Empty(); range.PopFront())
  {
    FileEntry entry = range.FrontEntry();
    String filePath = FilePath::Combine(entry.mPath, entry.mFileName);
    // If this is a directory then recurse
    if (DirectoryExists(filePath))
      totalSize += ComputeFolderSizeRecursive(filePath);
    else
      totalSize += entry.mSize;
  }
  return totalSize;
}

String HumanReadableFileSize(u64 bytes)
{
  const float cConversion = 1024.0f;

  cstr formats[] = {"%0.0f bytes", "%.0f KiB", "%.0f MiB", "%.2f GiB", "%.2f TiB", "%.2f PiB"};
  size_t count = sizeof(formats) / sizeof(formats[0]);

  float current = (float)bytes;

  for (uint i = 0; i < count; ++i)
  {
    if (current < cConversion)
      return String::Format(formats[i], current);
    current = current / cConversion;
  }

  return "File too large";
}

void PopulateVirtualFileSystemWithZip(void* userData)
{
  ProfileScopeFunction();
  ByteBufferBlock block(VirtualFileSystemData, VirtualFileSystemSize, false);

  // Now open the file as an archive
  Archive archive(ArchiveMode::Decompressing);
  archive.ReadBuffer(ArchiveReadFlags::All, block);

  // Populate file system entries based on what's in the archive
  forRange (ArchiveEntry& archiveEntry, archive.GetEntries())
  {
    // Create our entries for our files based on name, data, and modified time
    String absolutePath = BuildString("/", archiveEntry.Name);
    AddVirtualFileSystemEntry(absolutePath, &archiveEntry.Full, archiveEntry.ModifiedTime);
  }
}

void Download(StringParam fileName, const Array<byte>& binaryData)
{
  Os::DownloadFile(fileName.c_str(), DataBlock((byte*)binaryData.Data(), binaryData.Size()));
}

void Download(StringParam fileName, const DataBlock& binaryData)
{
  Os::DownloadFile(fileName.c_str(), DataBlock(binaryData.Data, binaryData.Size));
}

void Download(StringParam fileName, StringParam binaryData)
{
  Os::DownloadFile(fileName.c_str(), DataBlock((byte*)binaryData.Data(), binaryData.SizeInBytes()));
}

void Download(StringParam fileName, const ByteBufferBlock& binaryData)
{
  Os::DownloadFile(fileName.c_str(), binaryData.GetBlock());
}

void Download(StringParam filePath)
{
  Download(FilePath::GetFileNameWithoutExtension(filePath),
           FilePath::GetDirectoryPath(filePath),
           Array<String>(ZeroInit, filePath));
}

void Download(StringParam suggestedNameWithoutExtension, StringParam workingDirectory, const Array<String>& filePaths)
{
  if (!Os::SupportsDownloadingFiles() || filePaths.Empty())
    return;

  if (filePaths.Size() == 1 && FileExists(filePaths.Front()))
  {
    String filePath = filePaths.Front();
    ZPrint("Downloading single file %s\n", filePath.c_str());
    ByteBufferBlock block = ReadFileIntoByteBufferBlock(filePath.c_str());
    String fileName = FilePath::GetFileName(filePath);
    Os::DownloadFile(fileName.c_str(), block.GetBlock());
  }
  else
  {
    String workingDirectoryNormalized = FilePath::Normalize(workingDirectory);
    Status status;
    Archive archive(ArchiveMode::Compressing);
    forRange (StringParam filePath, filePaths)
    {
      String normalized = FilePath::Normalize(filePath);
      if (normalized.StartsWith(workingDirectoryNormalized))
      {
        String relativePath =
            normalized.SubStringFromByteIndices(workingDirectoryNormalized.SizeInBytes(), normalized.SizeInBytes());

        if (FileExists(filePath))
        {
          ZPrint("Adding file to download archive (in working directory) %s\n", filePath.c_str());
          archive.AddFile(filePath, relativePath);
        }
        else
        {
          ZPrint("Adding directory to download archive (in working directory) %s\n", filePath.c_str());
          archive.ArchiveDirectory(status, filePath, relativePath);
        }
      }
      else
      {
        if (FileExists(filePath))
        {
          ZPrint("Adding file to download archive (out of working directory) %s\n", filePath.c_str());
          archive.AddFile(filePath, filePath);
        }
        else
        {
          ZPrint("Adding directory to download archive (out of working directory) %s\n", filePath.c_str());
          archive.ArchiveDirectory(status, filePath, filePath);
        }
      }
    }

    ByteBufferBlock block(archive.ComputeZipSize());
    archive.WriteBuffer(block);

    String fileName = suggestedNameWithoutExtension + ".zip";
    Os::DownloadFile(fileName.c_str(), block.GetBlock());
  }
}

} // namespace Zero
