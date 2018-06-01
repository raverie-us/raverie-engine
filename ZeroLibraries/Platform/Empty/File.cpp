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
  Error("Not implemented");
  return nullptr;
}

size_t WriteToFile(cstr filePath, const byte* data, size_t bufferSize)
{
  Error("Not implemented");
  return 0;
}

//----------------------------------------------------------- File
struct FilePrivateData
{
};

File::File()
{
  Error("Not implemented");
}

File::~File()
{
  Error("Not implemented");
}

bool File::Open(StringParam filePath, FileMode::Enum mode, FileAccessPattern::Enum accessPattern, FileShare::Enum share, Status* status)
{
  Error("Not implemented");
  return false;
}

void File::Open(OsHandle handle, FileMode::Enum mode)
{
  Error("Not implemented");
}

void File::Open(Status& status, FILE* file, FileMode::Enum mode)
{
  Error("Not implemented");
}

void File::Close()
{
  Error("Not implemented");
}

FilePosition File::Tell()
{
  Error("Not implemented");
  return 0;
}

bool File::Seek(FilePosition filePosition, FileOrigin::Enum origin)
{
  Error("Not implemented");
  return false;
}

size_t File::Write(byte* data, size_t sizeInBytes)
{
  Error("Not implemented");
  return 0;
}

size_t File::Read(Status& status, byte* data, size_t sizeInBytes)
{
  Error("Not implemented");
  return 0;
}

bool File::HasData(Status& status)
{
  Error("Not implemented");
  return false;
}

void File::Flush()
{
  Error("Not implemented");
}

size_t File::Size()
{
  Error("Not implemented");
  return 0;
}

long long File::CurrentFileSize()
{
  Error("Not implemented");
  return 0;
}

bool File::IsOpen()
{
  Error("Not implemented");
  return false;
}

void File::Duplicate(Status& status, File& destinationFile)
{
  Error("Not implemented");
}

}//namespace Zero
