// MIT Licensed (see LICENSE.md).
#pragma once
#include "Utility/Standard.hpp"
#include "Platform/File.hpp"

namespace Zero
{

#pragma pack(push, 4)
struct FileChunk
{
  // Header
  u32 Type;
  u32 Size;

  // Computed Values
  u32 StartPos;
  u32 EndPos;
};
#pragma pack(pop)

template <typename streamType>
class ChunkReader
{
public:
  static const uint HeaderSize = sizeof(u32) * 2;
  typedef unsigned short StringLengthType;

  ChunkReader(){};

  void Open(StringParam filename)
  {
    file.Open(filename.c_str(), FileMode::Read, FileAccessPattern::Sequential);
  }

  void Open(DataBlock dataBlock)
  {
    file.SetBlock(dataBlock);
  }

  void Close()
  {
    file.Close();
  }

  // Read the chunk and then move back
  FileChunk PeekChunk()
  {
    FileChunk chunk = ReadChunkHeader();
    int headerSize = (int)HeaderSize;
    file.Seek(-headerSize, SeekOrigin::Current);
    return chunk;
  }

  FileChunk ReadChunkHeader()
  {
    // Read the chunk header from the file
    Status status;
    FileChunk chunk;
    chunk.Type = 0;
    chunk.Size = 0;

    if ((size_t)file.Tell() + HeaderSize <= file.Size())
    {
      file.Read(status, (byte*)&chunk.Type, HeaderSize);

      // Compute offsets
      chunk.StartPos = (uint)file.Tell();
      chunk.EndPos = chunk.StartPos + chunk.Size;
    }

    return chunk;
  }

  // Scan through the file until the chunk type is found.
  FileChunk ReadUntilChunk(uint chunktype)
  {
    FileChunk Chunk = ReadChunkHeader();
    while (Chunk.Type != chunktype && Chunk.Type != 0)
    {
      SkipChunk(Chunk);
      Chunk = ReadChunkHeader();
    }
    return Chunk;
  }

  // Skip to the end of this chunk
  void SkipChunk(FileChunk& chunk)
  {
    file.Seek(chunk.EndPos, SeekOrigin::Begin);
  }

  // Move to the beginning of the chunk
  void MoveToChunk(FileChunk& chunk)
  {
    file.Seek(chunk.StartPos, SeekOrigin::Begin);
  }

  template <typename type>
  void Read(type& data)
  {
    Status status;
    file.Read(status, (byte*)&data, sizeof(type));
  }

  template <typename type>
  void ReadArray(type* data, uint count)
  {
    Status status;
    file.Read(status, (byte*)data, sizeof(type) * count);
  }

  template <typename type>
  void ReadArraySize(type* data, uint size)
  {
    Status status;
    file.Read(status, (byte*)data, size);
  }

  template <typename StringType>
  void ReadString(StringType& stringValue)
  {
    StringLengthType strSize = 0;
    Read(strSize);
    byte* data = (byte*)alloca(strSize + 1);
    ReadArray(data, strSize);
    data[strSize] = '\0';
    stringValue = (char*)data;
  }

  streamType file;
};

typedef ChunkReader<File> ChunkFileReader;
typedef ChunkReader<ByteBufferBlock> ChunkBufferReader;

} // namespace Zero
