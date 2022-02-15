// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// TODO PLATFORM find and define the max path through C, if not possible move
// this variable to where it will be possible for other platforms
const int File::PlatformMaxPath = 1024;

struct FilePrivateData
{
  FilePrivateData()
  {
    mFileData = nullptr;
  }

  bool IsValidFile()
  {
    return mFileData != nullptr;
  }

  FILE* mFileData;
};

String FileModeToString(FileMode::Enum fileMode)
{
  if (fileMode == FileMode::Read)
    return String("rb");

  if (fileMode == FileMode::Write)
    return String("wb");

  if (fileMode == FileMode::Append)
    return String("ab");

  if (fileMode == FileMode::ReadWrite)
    return String("r+b");

  return String();
}

File::File()
{
  ZeroConstructPrivateData(FilePrivateData);
  mFileMode = FileMode::Read;
}

File::~File()
{
  Close();
  ZeroDestructPrivateData(FilePrivateData);
}

size_t File::Size()
{
  return (size_t)CurrentFileSize();
}

long long File::CurrentFileSize()
{
  ZeroGetPrivateData(FilePrivateData);
  if (!self->IsValidFile())
    return 0;

  auto originalPosition = ftell(self->mFileData);
  fseek(self->mFileData, 0, SEEK_END);
  long long size = (long long)ftell(self->mFileData);
  fseek(self->mFileData, originalPosition, SEEK_SET);
  return size;
}

bool File::Open(StringParam filePath,
                FileMode::Enum mode,
                FileAccessPattern::Enum accessPattern,
                FileShare::Enum share,
                Status* status)
{
  ZeroGetPrivateData(FilePrivateData);

  String fileMode = FileModeToString(mode);
  self->mFileData = fopen(filePath.c_str(), fileMode.c_str());

  if (self->mFileData == nullptr)
  {
    Warn("Failed to open file '%s'.", filePath.c_str());
    return false;
  }

  if (mode != FileMode::Read)
    FileModifiedState::BeginFileModified(mFilePath);

  return true;
}

void File::Open(OsHandle handle, FileMode::Enum mode)
{
  Error("STL does not support opening files via OSHandles");
}

void File::Open(Status& status, FILE* file, FileMode::Enum mode)
{
  ZeroGetPrivateData(FilePrivateData);
  self->mFileData = file;

  if (self->mFileData == nullptr)
  {
    status.SetFailed("Failed to open file");
    return;
  }

  mFileMode = mode;
}

bool File::IsOpen()
{
  ZeroGetPrivateData(FilePrivateData);
  return self->mFileData != nullptr;
}

void File::Close()
{
  ZeroGetPrivateData(FilePrivateData);
  if (self->mFileData)
  {
    fclose(self->mFileData);
    self->mFileData = nullptr;

    // Must come after closing the file because it may need access to the
    // modified date.
    if (mFileMode != FileMode::Read)
      FileModifiedState::EndFileModified(mFilePath);
  }
}

FilePosition File::Tell()
{
  ZeroGetPrivateData(FilePrivateData);
  if (!self->IsValidFile())
    return 0;

  return (FilePosition)ftell(self->mFileData);
}

bool File::Seek(FilePosition pos, SeekOrigin::Enum rel)
{
  ZeroGetPrivateData(FilePrivateData);
  if (!self->IsValidFile())
    return false;

  int origin = SEEK_SET;
  switch (rel)
  {
  case SeekOrigin::Begin:
    origin = SEEK_SET;
    break;
  case SeekOrigin::Current:
    origin = SEEK_CUR;
    break;
  case SeekOrigin::End:
    origin = SEEK_END;
    break;
  default:
    return false;
  }

  int ret = fseek(self->mFileData, pos, origin);
  return (ret == 0);
}

size_t File::Write(byte* data, size_t sizeInBytes)
{
  ZeroGetPrivateData(FilePrivateData);
  if (!self->IsValidFile())
    return 0;

  return fwrite(data, 1, sizeInBytes, self->mFileData);
}

size_t File::Read(Status& status, byte* data, size_t sizeInBytes)
{
  ZeroGetPrivateData(FilePrivateData);
  if (!self->IsValidFile())
  {
    status.SetFailed("No file was open");
    return 0;
  }

  return fread(data, 1, sizeInBytes, self->mFileData);
}

bool File::HasData(Status& status)
{
  ZeroGetPrivateData(FilePrivateData);
  if (!self->IsValidFile())
  {
    status.SetFailed("No file was open");
    return false;
  }

  return Tell() != CurrentFileSize();
}

void File::Flush()
{
  ZeroGetPrivateData(FilePrivateData);
  if (!self->IsValidFile())
    return;

  fflush(self->mFileData);
}

void File::Duplicate(Status& status, File& destinationFile)
{
  ZeroGetPrivateData(FilePrivateData);
  ZeroGetObjectPrivateData(FilePrivateData, &destinationFile, other);

  if (!self->IsValidFile() || !other->IsValidFile())
  {
    status.SetFailed("File is not valid. Open a valid file before attempting "
                     "file operations.");
    return;
  }

  // Get this files data size in bytes
  size_t fileSizeInBytes = (size_t)CurrentFileSize();
  byte* buffer = new byte[fileSizeInBytes];

  // Read this files data
  size_t ret = Read(status, buffer, fileSizeInBytes);
  WarnIf(ret != fileSizeInBytes, "Failed to duplicate original file");
  if (status.Failed())
  {
    delete[] buffer;
    return;
  }

  // Write the duplicate data into the other file
  ret = destinationFile.Write(buffer, fileSizeInBytes);
  delete[] buffer;
  WarnIf(ret != fileSizeInBytes, "Failed to duplicate original file");
}

} // namespace Zero
