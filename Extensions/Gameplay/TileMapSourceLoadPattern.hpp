///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Nathan Carlson
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct TileMapSourceLoadPattern
{
  template <typename DataType, typename WriterType>
  static void SaveArray(Array<DataType>& array, int chunkId, WriterType& file)
  {
    u32 startPos = file.StartChunk(chunkId);
    file.Write(array.Data(), array.Size());
    file.EndChunk(startPos);
  }

  template <typename DataType, typename ReaderType>
  static void LoadArray(Array<DataType>& array, u32 chunkSize, ReaderType& file)
  {
    u32 size = chunkSize - 8;
    u32 itemCount = size / sizeof(DataType);
    array.Resize(itemCount);
    file.ReadArraySize(array.Data(), size);
  }

  template <typename SourceType, typename ReaderType>
  static void Load(SourceType* source, ReaderType& file)
  {
    uint version = 0;
    file.Read(version);
    source->mVersion = version;

    FileChunk fileChunk = file.ReadChunkHeader();
    if (fileChunk.Type == 'full')
    {
      Array<IntVec2> position;
      Array<Guid> archetype;
      Array<Guid> sprite;
      Array<Guid> collision;
      Array<bool> merge;

      bool eof = false;
      while (!eof)
      {
        FileChunk chunk = file.ReadChunkHeader();
        switch (chunk.Type)
        {
          case 0:
            eof = true;
            break;

          case 'posi':
            LoadArray(position, chunk.Size, file);
            break;

          case 'arch':
            LoadArray(archetype, chunk.Size, file);
            break;

          case 'spri':
            LoadArray(sprite, chunk.Size, file);
            break;

          case 'coll':
            LoadArray(collision, chunk.Size, file);
            break;

          case 'merg':
            LoadArray(merge, chunk.Size, file);
            break;

          default:
            file.SkipChunk(chunk);
            break;
        }
      }

      for(uint i = 0; i < position.Size(); ++i)
      {
        IntVec2 gridPos = position[i];
        TileMap::Tile tile;
        tile.ArchetypeResource = archetype[i];
        tile.SpriteResource = sprite[i];
        tile.CollisionResource = collision[i];
        tile.Merge = merge[i];

        source->mData[gridPos] = tile;
      }

    }

  }

  template <typename SourceType, typename WriterType>
  static void Save(SourceType* source, WriterType& file)
  {
    file.Write(source->mVersion);

    u32 mapStart = file.StartChunk('full');

    uint size = source->mData.Size();

    Array<IntVec2> position;
    Array<Guid> archetype;
    Array<Guid> sprite;
    Array<Guid> collision;
    Array<bool> merge;

    forRange(typename SourceType::value_type pair, source->mData.All())
    {
      IntVec2 gridPos = pair.first;
      TileMap::Tile tile = pair.second;

      position.PushBack(gridPos);
      archetype.PushBack(tile.ArchetypeResource);
      sprite.PushBack(tile.SpriteResource);
      collision.PushBack(tile.CollisionResource);
      merge.PushBack(tile.Merge);
    }

    SaveArray(position, 'posi', file);
    SaveArray(archetype, 'arch', file);
    SaveArray(sprite, 'spri', file);
    SaveArray(collision, 'coll', file);
    SaveArray(merge, 'merg', file);

    file.EndChunk(mapStart);
  }

};

} // namespace Zero
