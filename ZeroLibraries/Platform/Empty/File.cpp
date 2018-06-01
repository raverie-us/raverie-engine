////////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2018, DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
//----------------------------------------------------------- File Functions
byte* ReadFileIntoMemory(cstr path, size_t& fileSize, size_t extra)
{
  fileSize = 0;
  return nullptr;
}

size_t WriteToFile(cstr filePath, const byte* data, size_t bufferSize)
{
  return 0;
}

//----------------------------------------------------------- File
struct FilePrivateData
{
};

File::File()
{
}

File::~File()
{
}

bool File::Open(StringParam filePath, FileMode::Enum mode, FileAccessPattern::Enum accessPattern, FileShare::Enum share, Status* status)
{
  if (status)
    status->SetFailed("Not implemented");
  return false;
}

void File::Open(OsHandle handle, FileMode::Enum mode)
{
}

void File::Open(Status& status, FILE* file, FileMode::Enum mode)
{
  status.SetFailed("Not implemented");
}

void File::Close()
{
}

FilePosition File::Tell()
{
  return 0;
}

bool File::Seek(FilePosition filePosition, FileOrigin::Enum origin)
{
  return false;
}

size_t File::Write(byte* data, size_t sizeInBytes)
{
  return 0;
}

size_t File::Read(Status& status, byte* data, size_t sizeInBytes)
{
  status.SetFailed("Not implemented");
  return 0;
}

bool File::HasData(Status& status)
{
  status.SetFailed("Not implemented");
  return false;
}

void File::Flush()
{
}

size_t File::Size()
{
  return 0;
}

long long File::CurrentFileSize()
{
  return 0;
}

bool File::IsOpen()
{
  return false;
}

void File::Duplicate(Status& status, File& destinationFile)
{
  status.SetFailed("Not implemented");
}

}//namespace Zero
