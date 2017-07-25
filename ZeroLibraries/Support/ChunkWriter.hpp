///////////////////////////////////////////////////////////////////////////////
///
/// \file ChunkWriter.hpp
/// Simple binary chunk based file writer.
///
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Platform/File.hpp"

namespace Zero
{

template<typename streamType>
class ChunkWriter
{
public:
  static const uint HeaderSize = sizeof(u32)*2;
  typedef unsigned short StringLengthType;

  ChunkWriter(){};

  void Open(StringParam filename)
  {
    file.Open(filename.c_str(), FileMode::Write, FileAccessPattern::Random);
  }

  void Close()
  {
    file.Close();
  }

  u32 StartChunk(u32 chunkType)
  {
    u32 chunkStartPos = (u32)file.Tell();
    Write(chunkType);
    Write(chunkType);
    return chunkStartPos;
  }

  void EndChunk(u32 chunkStartPos)
  {
    u32 curPos =  (u32)file.Tell();
    u32 chunksize = curPos - chunkStartPos;

    // Move pass the header
    file.Seek(chunkStartPos + sizeof(u32));
    file.Write((byte*)&chunksize , sizeof(uint));
    file.Seek(curPos);
  }

  // Writing Functions
  template<typename type>
  void Write(const type& data)
  {
    file.Write((byte*)&data , sizeof(type));
  }

  template<typename type>
  void Write(type* data, uint count)
  {
    file.Write((byte*)data , sizeof(type) * count);
  }

  void Write(String& str)
  {
    Write(StringLengthType(str.SizeInBytes()));
    Write(str.Data(), str.SizeInBytes());
  }

  void WriteChunk(u32 chunkType , uint chunkSize)
  {
    Write(chunkType);
    Write(chunkSize);
  };

  //File output
  streamType file;
};

typedef ChunkWriter<File> ChunkFileWriter;

}
