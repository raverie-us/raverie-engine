// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

//#define DEBUG_FILE_TIMES

namespace Raverie
{
const Rune cDirectorySeparatorRune = Rune('/');
const char cDirectorySeparatorCstr[] = "/";
bool cFileSystemCaseSensitive = false;

static const String cRoot(cDirectorySeparatorCstr);

class FileSystem : public ExplicitSingleton<FileSystem>
{
public:
  FileSystem();
  SystemEntry* FindEntry(StringParam path);
  SystemEntry* CreateEntry(StringParam path, EntryType::Enum type);
  bool DeleteEntry(StringParam path, EntryType::Enum type);
  bool DeleteEntry(StringParam path);
  String GetFullPath(StringParam path);

  SystemEntry mRoot;
  String mWorkingDirectory;
};

SystemEntry::SystemEntry() : mParent(nullptr), mType(EntryType::Directory), mModifiedTime(Time::GetTime())
{
}

SystemEntry::~SystemEntry()
{
  if (mParent)
    EntryList::Unlink(this);

  while (!mChildren.Empty())
    mChildren.Front().Delete();
}

SystemEntry* SystemEntry::FindChild(StringParam name)
{
  // If we have double slashes with nothing in between, or the starting root /
  // then there will be an empty entry before it (just return itself always).
  if (name.Empty())
    return this;

  forRange (SystemEntry& child, mChildren)
  {
    if (child.mName == name)
      return &child;
  }

  return nullptr;
}

SystemEntry* SystemEntry::FindOrAddChild(StringParam name, EntryType::Enum type)
{
  // If we have double slashes with nothing in between, or the starting root /
  // then there will be an empty entry before it (just return itself always).
  if (name.Empty())
    return this;

  forRange (SystemEntry& child, mChildren)
  {
    if (child.mName == name)
      return &child;
  }

  SystemEntry* newChild = new SystemEntry();
  newChild->mParent = this;
  newChild->mName = name;
  newChild->mType = type;
  this->mChildren.PushBack(newChild);
  return newChild;
}

bool SystemEntry::DeleteChild(StringParam name)
{
  // We may have attempted to delete /test/a/
  // Split will return an empty space after a/
  if (name.Empty())
    return Delete();

  forRange (SystemEntry& child, mChildren)
  {
    if (child.mName == name)
      return child.Delete();
  }

  return false;
}

void SystemEntry::CopyTo(SystemEntry* copyTo)
{
  // Handles both directories and files
  copyTo->mType = mType;
  copyTo->mFileData = mFileData;

  while (!copyTo->mChildren.Empty())
    copyTo->mChildren.Front().Delete();

  forRange (SystemEntry& child, mChildren)
  {
    SystemEntry* clonedChild = child.CloneUnattached();
    clonedChild->mParent = copyTo;
    copyTo->mChildren.PushBack(clonedChild);
  }
}

SystemEntry* SystemEntry::CloneUnattached()
{
  SystemEntry* clone = new SystemEntry();
  clone->mName = mName;
  clone->mType = mType;
  clone->mFileData = mFileData;

  forRange (SystemEntry& child, mChildren)
  {
    SystemEntry* clonedChild = child.CloneUnattached();
    clonedChild->mParent = clone;
    clone->mChildren.PushBack(clonedChild);
  }
  return clone;
}

void SystemEntry::AttachTo(SystemEntry* parent)
{
  ReturnIf(mParent == nullptr, , "Cannot move the root directory");

  EntryList::Unlink(this);

  mParent = parent;
  parent->mChildren.PushBack(this);
}

bool SystemEntry::Delete()
{
  ReturnIf(mParent == nullptr, false, "Attempting to delete the root directory");
  delete this;
  return true;
}

FileSystem::FileSystem() : mWorkingDirectory(cRoot)
{
  CreateEntry(GetUserLocalDirectory(), EntryType::Directory);
  CreateEntry(GetUserDocumentsDirectory(), EntryType::Directory);
  CreateEntry(GetTemporaryDirectory(), EntryType::Directory);
}

SystemEntry* FileSystem::FindEntry(StringParam path)
{
  String fullPath = GetFullPath(path);
  SystemEntry* entry = &mRoot;

  forRange (String part, fullPath.Split(cDirectorySeparatorCstr))
  {
    entry = entry->FindChild(part);
    if (!entry)
      return nullptr;
  }
  return entry;
}

SystemEntry* FileSystem::CreateEntry(StringParam path, EntryType::Enum type)
{
  String fullPath = GetFullPath(path);

  SystemEntry* entry = &mRoot;

  forRange (String part, fullPath.Split(cDirectorySeparatorCstr))
    entry = entry->FindOrAddChild(part, EntryType::Directory);

  entry->mType = type;
  return entry;
}

bool FileSystem::DeleteEntry(StringParam path, EntryType::Enum type)
{
  SystemEntry* entry = FindEntry(path);
  if (entry && entry->mType == type)
    return entry->Delete();

  return false;
}

bool FileSystem::DeleteEntry(StringParam path)
{
  SystemEntry* entry = FindEntry(path);
  if (entry)
    return entry->Delete();

  return false;
}

String FileSystem::GetFullPath(StringParam path)
{
  String normalizedPath = FilePath::Normalize(path);
  if (PathIsRooted(normalizedPath))
    return normalizedPath;

  return FilePath::Combine(mWorkingDirectory, normalizedPath);
}

FileSystemInitializer::FileSystemInitializer(PopulateVirtualFileSystem callback, void* userData)
{
  FileSystem::Initialize();

  // Calling this will allow the outside product to populate the
  // virtual file system by calling 'AddVirtualFileSystemEntry'.
  if (callback)
    callback(userData);
}

FileSystemInitializer::~FileSystemInitializer()
{
  FileSystem::Destroy();
}

void AddVirtualFileSystemEntry(StringParam absolutePath, DataBlock* stealData, TimeType modifiedTime)
{
  ErrorIf(!PathIsRooted(absolutePath), "The given path should have been an absolute/rooted path");

  SystemEntry* entry = nullptr;

  // Create our entries for our files based on name, data, and modified time
  // If the size is 0, then it's a directory
  if (stealData == nullptr || stealData->Data == nullptr || stealData->Size == 0)
  {
    entry = FileSystem::GetInstance()->CreateEntry(absolutePath, EntryType::Directory);
  }
  else
  {
    entry = FileSystem::GetInstance()->CreateEntry(absolutePath, EntryType::File);

    // Steal the data from the whoever passed it in
    entry->mFileData.SetData(stealData->Data, stealData->Size);
    stealData->Data = nullptr;
    stealData->Size = 0;
  }

  entry->mModifiedTime = modifiedTime;
#ifdef DEBUG_FILE_TIMES
  ZPrint("AddVirtualFileSystemEntry: '%s' (%lld)\n", absolutePath.c_str(), (long long)modifiedTime);
#endif
}

bool CopyFileInternal(StringParam dest, StringParam source)
{
  SystemEntry* sourceEntry = FileSystem::GetInstance()->FindEntry(source);
  if (!sourceEntry || sourceEntry->mType != EntryType::File)
    return false;

  SystemEntry* destEntry = FileSystem::GetInstance()->FindEntry(dest);
  if (destEntry)
  {
    // We can't copy a file over the top of a directory
    if (destEntry->mType == EntryType::Directory)
    {
      return false;
    }
  }
  else
  {
    destEntry = FileSystem::GetInstance()->CreateEntry(dest, EntryType::File);
  }

  sourceEntry->CopyTo(destEntry);
  return true;
}

bool MoveFileInternal(StringParam dest, StringParam source)
{
  SystemEntry* sourceEntry = FileSystem::GetInstance()->FindEntry(source);
  if (!sourceEntry || sourceEntry->mType != EntryType::File)
    return false;

  SystemEntry* destEntry = FileSystem::GetInstance()->FindEntry(dest);

  if (destEntry)
  {
    // We can't move a file over the top of a directory
    if (destEntry->mType == EntryType::Directory)
    {
      return false;
    }

    // It must be a file that exists there, so delete the file
    destEntry->Delete();
  }

  String dirDest = FilePath::GetDirectoryPath(dest);
  SystemEntry* dirEntry = FileSystem::GetInstance()->CreateEntry(dirDest, EntryType::Directory);

  sourceEntry->mName = FilePath::GetFileName(dest);
  sourceEntry->AttachTo(dirEntry);
  return true;
}

bool DeleteFileInternal(StringParam file)
{
  return FileSystem::GetInstance()->DeleteEntry(file, EntryType::File);
}

bool DeleteDirectory(StringParam directory)
{
  return FileSystem::GetInstance()->DeleteEntry(directory, EntryType::Directory);
}

bool PathIsRooted(StringParam directoryPath)
{
  return directoryPath.StartsWith(cRoot);
}

void CreateDirectory(StringParam dest)
{
  String dirDest = FilePath::GetDirectoryPath(dest);
  SystemEntry* entry = FileSystem::GetInstance()->FindEntry(dirDest);
  if (!entry)
    return;

  entry->FindOrAddChild(FilePath::GetDirectoryName(dest), EntryType::Directory);
}

void CreateDirectoryAndParents(StringParam directory)
{
  FileSystem::GetInstance()->CreateEntry(directory, EntryType::Directory);
}

bool SetFileToCurrentTime(StringParam filename)
{
  SystemEntry* entry = FileSystem::GetInstance()->FindEntry(filename);
  if (entry)
  {
    entry->mModifiedTime = Time::GetTime();
    return true;
  }
  return false;
}

TimeType GetFileModifiedTime(StringParam fileName)
{
  SystemEntry* entry = FileSystem::GetInstance()->FindEntry(fileName);
  if (entry)
    return entry->mModifiedTime;
  return 0;
}

u64 GetFileSize(StringParam fileName)
{
  SystemEntry* entry = FileSystem::GetInstance()->FindEntry(fileName);
  if (entry)
    entry->mFileData.Size();
  return 0;
}

bool FileExists(StringParam filePath)
{
  SystemEntry* entry = FileSystem::GetInstance()->FindEntry(filePath);
  return entry != nullptr && entry->mType == EntryType::File;
}

bool FileWritable(StringParam filePath)
{
  SystemEntry* entry = FileSystem::GetInstance()->FindEntry(filePath);
  return entry == nullptr || (entry->mType == EntryType::File);
}

bool DirectoryExists(StringParam directoryPath)
{
  SystemEntry* entry = FileSystem::GetInstance()->FindEntry(directoryPath);
  return entry != nullptr && entry->mType == EntryType::Directory;
}

String CanonicalizePath(StringParam directoryPath)
{
  return FilePath::Normalize(directoryPath);
}

String GetWorkingDirectory()
{
  return FileSystem::GetInstance()->mWorkingDirectory;
}

void SetWorkingDirectory(StringParam path)
{
  FileSystem::GetInstance()->mWorkingDirectory = path;
}

String GetUserLocalDirectory()
{
  return "/Local/";
}

String GetUserDocumentsDirectory()
{
  return "/User/";
}

String GetApplication()
{
  return "/App/Main";
}

String GetTemporaryDirectory()
{
  return "/Temp/";
}

String UniqueFileId(StringParam fullpath)
{
  return FilePath::Normalize(fullpath);
}

FileRange::FileRange(StringParam path) : mPath(path)
{
  SystemEntry* entry = FileSystem::GetInstance()->FindEntry(path);
  if (entry && entry->mType == EntryType::Directory)
    mRange = entry->mChildren.All();
}

FileRange::~FileRange()
{
}

bool FileRange::Empty()
{
  return mRange.Empty();
}

String FileRange::Front()
{
  SystemEntry& entry = mRange.Front();
  return entry.mName;
}

FileEntry FileRange::FrontEntry()
{
  SystemEntry& frontEntry = mRange.Front();
  FileEntry result;
  result.mPath = mPath;
  result.mFileName = frontEntry.mName;
  result.mSize = frontEntry.mFileData.Size();
  return result;
}

void FileRange::PopFront()
{
  mRange.PopFront();
}

const int File::PlatformMaxPath = 4096;

File::File()
{
}

File::~File()
{
  Close();
}

size_t File::Size()
{
  if (mEntry)
    return mEntry->mFileData.Size();
  return 0;
}

long long File::CurrentFileSize()
{
  return Size();
}

bool File::Open(StringParam filePath, FileMode::Enum mode, FileAccessPattern::Enum accessPattern, FileShare::Enum share, Status* status)
{

  SystemEntry* entry = nullptr;
  if (mode == FileMode::Read)
  {
    entry = FileSystem::GetInstance()->FindEntry(filePath);
  }
  else
  {
    FileModifiedState::BeginFileModified(filePath);
    entry = FileSystem::GetInstance()->CreateEntry(filePath, EntryType::File);
  }

  if (!entry)
  {
    if (status)
      status->SetFailed("The file did not exist or could not be opened");
    return false;
  }

  if (mode == FileMode::Write)
  {
    entry->mFileData.Clear();
  }
  else if (mode == FileMode::Append)
  {
    mPosition = entry->mFileData.Size();
  }

  // Currently don't handle/set locked

  mFileMode = mode;
  mFilePath = filePath;
  mEntry = entry;
  return true;
}

bool File::IsOpen()
{
  return mEntry != nullptr;
}

void File::Close()
{

  // Guard against closing more than once.
  if (!mEntry)
    return;

  mEntry = nullptr;
  if (mFileMode != FileMode::Read)
    FileModifiedState::EndFileModified(mFilePath);
}

FilePosition File::Tell()
{
  return mPosition;
}

bool File::Seek(FilePosition pos, SeekOrigin::Enum rel)
{
  if (!mEntry)
    return false;

  switch (rel)
  {
  case SeekOrigin::Begin:
    mPosition = (size_t)pos;
    break;
  case SeekOrigin::Current:
    mPosition += (size_t)pos;
    break;
  case SeekOrigin::End:
    mPosition = mEntry->mFileData.Size() + (size_t)pos;
    break;
  default:
    return false;
  }

  return true;
}

size_t File::Write(byte* data, size_t sizeInBytes)
{
  SystemEntry* entry = mEntry;
  if (!entry)
    return 0;

  size_t newSize = Math::Max(mPosition + sizeInBytes, entry->mFileData.Size());
  entry->mFileData.Resize(newSize);

  memcpy(entry->mFileData.Data() + mPosition, data, sizeInBytes);
  mPosition += sizeInBytes;
  return sizeInBytes;
}

size_t File::Read(Status& status, byte* data, size_t sizeInBytes)
{
  if (!mEntry)
  {
    status.SetFailed("No file was open");
    return 0;
  }

  if (mPosition >= mEntry->mFileData.Size())
    return 0;

  size_t dataLeft = mEntry->mFileData.Size() - mPosition;
  sizeInBytes = Math::Min(sizeInBytes, dataLeft);

  memcpy(data, mEntry->mFileData.Data() + mPosition, sizeInBytes);
  mPosition += sizeInBytes;
  return sizeInBytes;
}

bool File::HasData(Status& status)
{
  if (!mEntry)
  {
    status.SetFailed("No file was open");
    return false;
  }

  return mPosition < mEntry->mFileData.Size();
}

void File::Flush()
{
  // Memory files don't need to be flushed
}

void File::Duplicate(Status& status, File& destinationFile)
{
  if (!mEntry || !destinationFile.mEntry)
  {
    status.SetFailed("File is not valid. Open a valid file before attempting "
                     "file operations.");
    return;
  }

  mEntry->CopyTo(destinationFile.mEntry);
}

void RaverieExportNamed(ExportFileCreate)(const char* filePath, byte* dataSteal, size_t dataLength)
{
  // The data will be "stolen" by AddVirtualFileSystemEntry (no need to de-allocate)
  DataBlock block;
  block.Data = dataSteal;
  block.Size = dataLength;
  AddVirtualFileSystemEntry(filePath, &block, time(0));
}

void RaverieExportNamed(ExportFileDelete)(const char* filePath)
{
  FileSystem::GetInstance()->DeleteEntry(filePath);
}

} // namespace Raverie
