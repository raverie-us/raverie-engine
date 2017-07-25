///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "FileSupport.hpp"
#include "Platform/FileSystem.hpp"
#include "String/StringBuilder.hpp"
#include "Regex/Regex.hpp"
#include "Platform/FilePath.hpp"
#include "Common/Time.hpp"

namespace Zero
{
void WriteStringRangeToFile(StringParam path, StringRange range)
{
  WriteToFile(path.c_str(), (byte*)range.Data(), range.SizeInBytes());
}

void MoveFolderContents(StringParam dest, StringParam source, FileFilter* filter)
{
  ReturnIf(source.Empty(), , "Cannot copy from an empty directory/path string (working directory as empty string not supported)");
  ReturnIf(dest.Empty(), , "Cannot copy to an empty directory/path string (working directory as empty string not supported)");

  CreateDirectoryAndParents(dest);

  FileRange files(source);
  for(; !files.Empty(); files.PopFront())
  {
    String sourceFile = FilePath::Combine(source, files.Front());
    String destFile = FilePath::Combine(dest, files.Front());

    //Filter
    FilterResult::Enum filterResult = FilterResult::Include;
    if(filter)
      filterResult = filter->Filter(sourceFile);

    //Skip
    if(filterResult == FilterResult::Ignore)
      continue;

    if(IsDirectory(sourceFile))
    {
      CreateDirectory(dest);

      MoveFolderContents(destFile, sourceFile);
    }
    else
    {
      MoveFile(destFile, sourceFile);
    }

  }
}

void CopyFolderContents(StringParam dest, StringParam source, FileFilter* filter)
{
  CreateDirectory(dest);

  FileRange files(source);
  for(;!files.Empty();files.PopFront())
  {
    //If the path is a directory enumerate it
    String sourceFile = FilePath::Combine(source, files.Front());
    String destFile = FilePath::Combine(dest, files.Front());

    //Filter
    FilterResult::Enum filterResult = FilterResult::Include;
    if(filter) filterResult = filter->Filter(sourceFile);

    //Skip the file
    if(filterResult == FilterResult::Ignore)
      continue;

    if(IsDirectory(sourceFile))
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
  if(!IsDirectory(path))
  {
    foundFiles.PushBack(path);
    return;
  }

  // Otherwise, iterate over all files and folders in the directory. The recursion will take care of adding all the files
  FileRange fileRange(path);
  for(; !fileRange.Empty(); fileRange.PopFront())
  {
    FileEntry entry = fileRange.frontEntry();
    String filePath = entry.GetFullPath();
    FindFilesRecursively(filePath, foundFiles);
  }
}

void FindFilesRecursively(StringParam path, Array<String>& foundFiles, FileFilter* filter)
{
  // If this is a file then just add this to the results and return
  if(!IsDirectory(path))
  {
    if(filter->Filter(path) == FilterResult::Include)
      foundFiles.PushBack(path);
    return;
  }

  // Otherwise, iterate over all files and folders in the directory. The recursion will take care of adding all the files
  FileRange fileRange(path);
  for(; !fileRange.Empty(); fileRange.PopFront())
  {
    FileEntry entry = fileRange.frontEntry();
    String filePath = entry.GetFullPath();
    FindFilesRecursively(filePath, foundFiles, filter);
  }
}

FilterResult::Enum FilterFileRegex::Filter(StringParam filename)
{
  if(!mAccept.Empty())
  {
    Regex regex(mAccept);
    Matches matches;
    regex.Search(filename, matches);
    if(matches.Empty())
      return FilterResult::Ignore;
  }

  if(!mIgnore.Empty())
  {
    Regex regex(mIgnore);
    Matches matches;
    regex.Search(filename, matches);
    if(!matches.Empty())
      return FilterResult::Ignore;
  }

  return FilterResult::Include;
}

FilterResult::Enum ExtensionFilterFile::Filter(StringParam filename)
{
  String extension = FilePath::GetExtension(filename);
  if(!mCaseSensative)
  {
    extension = extension.ToLower();
  }

  if(extension == mExtension)
    return FilterResult::Include;
  return FilterResult::Ignore;
}

String GetTimeAndDateStamp()
{ 
  // get time now
  TimeType t = Time::GetTime();
  CalendarDateTime now = Time::GetLocalTime(t);
  //struct tm * now = localtime(&t);
  return String::Format("%d-%d-%d--%d-%02d-%02d", now.Year, now.Month + 1, 
    now.Day, now.Hour, now.Minutes, now.Seconds);
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
  for(; !range.Empty(); range.PopFront())
  {
    FileEntry entry = range.frontEntry();
    String filePath = FilePath::Combine(entry.mPath, entry.mFileName);
    // If this is a directory then recurse
    if(IsDirectory(filePath))
      totalSize += ComputeFolderSizeRecursive(filePath);
    else
      totalSize += entry.mSize;
  }
  return totalSize;
}

String HumanReadableFileSize(u64 bytes)
{
  const float cConversion = 1024.0f;

  cstr formats[] = {"%0.0f bytes", "%.0f KiB", "%.0f MiB", "%.2f GiB",
    "%.2f TiB",    "%.2f PiB"};
  size_t count = sizeof(formats) / sizeof(formats[0]);

  float current = (float)bytes;

  for(uint i = 0; i < count; ++i)
  {
    if(current < cConversion)
      return String::Format(formats[i], current);
    current = current / cConversion;
  }

  return "File too large";
}

}//namespace Zero
