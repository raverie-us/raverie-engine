///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Platform/File.hpp"

namespace Zero
{
bool CompareFile(Status& status, StringParam filePath1, StringParam filePath2)
{
  File file1;
  File file2;

  file1.Open(filePath1, FileMode::Read, FileAccessPattern::Sequential, FileShare::Read, &status);
  if (status.Failed())
    return false;

  file2.Open(filePath2, FileMode::Read, FileAccessPattern::Sequential, FileShare::Read, &status);
  if (status.Failed())
    return false;

  if (file1.Size() != file2.Size())
    return false;

  const size_t ReadSize = 1024 * 1024;
  
  ByteBufferBlock file1BufferBlock(ReadSize);
  ByteBufferBlock file2BufferBlock(ReadSize);
  byte* file1Buffer = file1BufferBlock.GetBegin();
  byte* file2Buffer = file2BufferBlock.GetBegin();

  for(;;)
  {
    size_t file1Read = file1.Read(status, file1Buffer, ReadSize);
    size_t file2Read = file2.Read(status, file2Buffer, ReadSize);

    // If we ever read a different amount (it either means eof or error for either file)
    if (file1Read != file2Read)
      return false;

    // We already compared the size, so we can just compare up to the point that both read
    if (memcmp(file1Buffer, file2Buffer, file1Read) != 0)
      return false;

    // If we hit the end of both files (or magically an error in the exact same spot of both files...)
    if (file1Read != ReadSize)
      break;
  }

  return true;
}

bool CompareFileAndString(Status& status, StringParam filePath, StringParam string)
{
  File file;
  file.Open(filePath, FileMode::Read, FileAccessPattern::Sequential, FileShare::Read, &status);
  if (status.Failed())
    return false;

  if (file.Size() != string.SizeInBytes())
    return false;

  const size_t ReadSize = 65536;
  
  ByteBufferBlock fileBufferBlock(ReadSize);
  byte* fileBuffer = fileBufferBlock.GetBegin();

  size_t index = 0;

  for(;;)
  {
    size_t fileRead = file.Read(status, fileBuffer, ReadSize);

    // This should never happen unless somehow the file has an error and reads more than it should
    if (index + fileRead > string.SizeInBytes())
      return false;

    if (memcmp(fileBuffer, string.c_str() + index, fileRead) != 0)
      return false;

    index += fileRead;

    // If we hit the end of the file
    if (fileRead != ReadSize)
    {
      // Make sure we read everything up to the end of the string
      if (index != string.SizeInBytes())
        return false;

      // Otherwise break out here (we could just return true, but control flow paths...)
      break;
    }
  }

  return true;
}

DataBlock ReadFileIntoDataBlock(cstr path)
{
  DataBlock block;
  block.Data = ReadFileIntoMemory(path, block.Size, 0);
  return block;
}

String ReadFileIntoString(StringParam path)
{
  Zero::DataBlock block = Zero::ReadFileIntoDataBlock(path.c_str());
  if (block.Data == nullptr)
    return String();

  Zero::String dataFormat((char*)block.Data, block.Size);
  delete block.Data;
  return dataFormat;
}

}//namespace Zero
