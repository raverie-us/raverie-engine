// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

//#define DEBUG_FILE_TIMES

namespace Raverie
{
const Rune cDirectorySeparatorRune = Rune('/');
const char cDirectorySeparatorCstr[] = "/";
bool cFileSystemCaseSensitive = false;

static const String cRoot(cDirectorySeparatorCstr);

DeclareEnum2(EntryType, File, Directory);

class SystemEntry : public LinkBase
{
public:
  SystemEntry();
  ~SystemEntry();

  SystemEntry* FindChild(StringParam name);
  SystemEntry* FindOrAddChild(StringParam name, EntryType::Enum type);
  bool DeleteChild(StringParam name);
  void CopyTo(SystemEntry* copyTo);
  SystemEntry* CloneUnattached();
  void AttachTo(SystemEntry* parent);
  bool Delete();

  SystemEntry* mParent;
  EntryType::Enum mType;
  String mName;
  typedef BaseInList<LinkBase, SystemEntry> EntryList;
  EntryList mChildren;
  Array<byte> mFileData;
  TimeType mModifiedTime;
};

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

struct FileRangePrivateData
{
  SystemEntry::EntryList::range mRange;
};

FileRange::FileRange(StringParam path) : mPath(path)
{
  RaverieConstructPrivateData(FileRangePrivateData);
  SystemEntry* entry = FileSystem::GetInstance()->FindEntry(path);
  if (entry && entry->mType == EntryType::Directory)
    self->mRange = entry->mChildren.All();
}

FileRange::~FileRange()
{
  RaverieDestructPrivateData(FileRangePrivateData);
}

bool FileRange::Empty()
{
  RaverieGetPrivateData(FileRangePrivateData);
  return self->mRange.Empty();
}

String FileRange::Front()
{
  RaverieGetPrivateData(FileRangePrivateData);
  SystemEntry& entry = self->mRange.Front();
  return entry.mName;
}

FileEntry FileRange::FrontEntry()
{
  RaverieGetPrivateData(FileRangePrivateData);
  SystemEntry& frontEntry = self->mRange.Front();
  FileEntry result;
  result.mPath = mPath;
  result.mFileName = frontEntry.mName;
  result.mSize = frontEntry.mFileData.Size();
  return result;
}

void FileRange::PopFront()
{
  RaverieGetPrivateData(FileRangePrivateData);
  self->mRange.PopFront();
}

const int File::PlatformMaxPath = 4096;

struct FilePrivateData
{
  SystemEntry* mEntry;
  size_t mPosition;
};

File::File()
{
  RaverieConstructPrivateData(FilePrivateData);
  self->mEntry = nullptr;
  self->mPosition = 0;
}

File::~File()
{
  Close();
  RaverieDestructPrivateData(FilePrivateData);
}

size_t File::Size()
{
  RaverieGetPrivateData(FilePrivateData);
  if (self->mEntry)
    return self->mEntry->mFileData.Size();
  return 0;
}

long long File::CurrentFileSize()
{
  return Size();
}

bool File::Open(StringParam filePath, FileMode::Enum mode, FileAccessPattern::Enum accessPattern, FileShare::Enum share, Status* status)
{
  RaverieGetPrivateData(FilePrivateData);

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
    self->mPosition = entry->mFileData.Size();
  }

  // Currently don't handle/set locked

  mFileMode = mode;
  mFilePath = filePath;
  self->mEntry = entry;
  return true;
}

void File::Open(OsHandle handle, FileMode::Enum mode)
{
  Error("Virtual File System does not support opening files via OsHandle");
}

void File::Open(Status& status, FILE* file, FileMode::Enum mode)
{
  RaverieGetPrivateData(FilePrivateData);
  // We could support this by offering a dual interface where we also store a
  // FILE*
  Error("Memory File does not support opening files via FILE");
}

bool File::IsOpen()
{
  RaverieGetPrivateData(FilePrivateData);
  return self->mEntry != nullptr;
}

void File::Close()
{
  RaverieGetPrivateData(FilePrivateData);

  // Guard against closing more than once.
  if (!self->mEntry)
    return;

  self->mEntry = nullptr;
  if (mFileMode != FileMode::Read)
    FileModifiedState::EndFileModified(mFilePath);
}

FilePosition File::Tell()
{
  RaverieGetPrivateData(FilePrivateData);
  return self->mPosition;
}

bool File::Seek(FilePosition pos, SeekOrigin::Enum rel)
{
  RaverieGetPrivateData(FilePrivateData);
  if (!self->mEntry)
    return false;

  switch (rel)
  {
  case SeekOrigin::Begin:
    self->mPosition = (size_t)pos;
    break;
  case SeekOrigin::Current:
    self->mPosition += (size_t)pos;
    break;
  case SeekOrigin::End:
    self->mPosition = self->mEntry->mFileData.Size() + (size_t)pos;
    break;
  default:
    return false;
  }

  return true;
}

size_t File::Write(byte* data, size_t sizeInBytes)
{
  RaverieGetPrivateData(FilePrivateData);
  SystemEntry* entry = self->mEntry;
  if (!entry)
    return 0;

  size_t newSize = Math::Max(self->mPosition + sizeInBytes, entry->mFileData.Size());
  entry->mFileData.Resize(newSize);

  memcpy(entry->mFileData.Data() + self->mPosition, data, sizeInBytes);
  self->mPosition += sizeInBytes;
  return sizeInBytes;
}

size_t File::Read(Status& status, byte* data, size_t sizeInBytes)
{
  RaverieGetPrivateData(FilePrivateData);
  if (!self->mEntry)
  {
    status.SetFailed("No file was open");
    return 0;
  }

  if (self->mPosition >= self->mEntry->mFileData.Size())
    return 0;

  size_t dataLeft = self->mEntry->mFileData.Size() - self->mPosition;
  sizeInBytes = Math::Min(sizeInBytes, dataLeft);

  memcpy(data, self->mEntry->mFileData.Data() + self->mPosition, sizeInBytes);
  self->mPosition += sizeInBytes;
  return sizeInBytes;
}

bool File::HasData(Status& status)
{
  RaverieGetPrivateData(FilePrivateData);
  if (!self->mEntry)
  {
    status.SetFailed("No file was open");
    return false;
  }

  return self->mPosition < self->mEntry->mFileData.Size();
}

void File::Flush()
{
  // Memory files don't need to be flushed
}

void File::Duplicate(Status& status, File& destinationFile)
{
  RaverieGetPrivateData(FilePrivateData);
  RaverieGetObjectPrivateData(FilePrivateData, &destinationFile, other);

  if (!self->mEntry || !other->mEntry)
  {
    status.SetFailed("File is not valid. Open a valid file before attempting "
                     "file operations.");
    return;
  }

  self->mEntry->CopyTo(other->mEntry);
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
