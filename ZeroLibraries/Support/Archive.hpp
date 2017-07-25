///////////////////////////////////////////////////////////////////////////////
///
/// \file Archive.hpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
//#include "Containers/Array.hpp"
//#include "Platform/File.hpp"
//#include "Utility/EnumDeclaration.hpp"
//#include "Utility/Status.hpp"

namespace Zero
{

class ByteBuffer;
class ByteBufferBlock;
class FileFilter;

int RawDeflate(byte* outputData, uint outsize, byte* inputData, uint inSize, int level);
int RawInflate(byte* outputData, uint outSize, byte* inputData, uint inSize);

struct ArchiveEntry
{
  // Name of Entry or Path to file.
  String Name;

  // Data of the entry.
  DataBlock Full;
  DataBlock Compressed;

  // Last Modified Time
  TimeType ModifiedTime;

  // Offset Of File in Data
  size_t Offset;
  // Compression level for entry
  uint CompressionLevel;
  // Crc Code
  u32 Crc;
};

//Should the files be stored in a compressed or decompressed state?
DeclareEnum2(ArchiveMode, Compressing, Decompressing);

DeclareBitField3(ArchiveReadFlags,
  // Read entries name, size, and offset
  Entries,
  // Read entry data into allocated memory (Entries need to be set)
  Data,
  // Decompress entries (Data needs to be set)
  Decompress
)

DeclareEnum2(ArchiveExportMode, Overwrite, OverwriteIfNewer);

namespace ArchiveReadFlags
{
  static const ArchiveReadFlags::Enum All = ArchiveReadFlags::Enum(Entries | Data | Decompress);
}

namespace CompressionLevel
{
const uint NoCompression = 0;
const uint DefaultCompression = 6;
const uint MaxCompression = 9;
}

/// Archive is a collection files in one large file. Used
/// for writing / reading zip files.
class Archive
{
public:
  Archive(ArchiveMode::Enum mode, uint compressionLevel=CompressionLevel::DefaultCompression);
  ~Archive();

  // Adding Files in Compressing mode

  //Directory Logic
  void ArchiveDirectory(Status& status, StringParam path,
    StringParam parentPath = String(), FileFilter* test = nullptr);

  //Export
  void ExportToDirectory(ArchiveExportMode::Enum exportMode, StringParam path);

  //Add a file to the archive. Full path is file path to file 
  //and relativeName is name stored in the archive.
  void AddFile(StringParam fullpath, StringParam relativeName);

  //Add file DataBlock. Adds data block data to relative path name.
  void AddFileBlock(StringParam relativeName, DataBlock sourceBlock);

  //Add a file to the archive. Full path will be basePath + relativeName
  //relativeName is the name it will be stored in the archive.
  void AddFileRelative(StringParam basePath, StringParam relativeName);

  // Per file extract
  void Extract(File& file, StringParam name, StringParam destfile);

  // General Read/Write

  //Write or Read from a file object.
  void WriteZip(File& file);
  void ReadZip(ArchiveReadFlags::Enum readFlags, File& file);
  void ReadZip(ArchiveReadFlags::Enum readFlags, DataBlock block);

  //Write or Read from a buffer object.
  void WriteBuffer(ByteBuffer& buffer);
  void WriteBuffer(ByteBufferBlock& buffer);
  void ReadBuffer(ArchiveReadFlags::Enum readFlags, ByteBufferBlock& buffer);
  // Given an entry read by an archive and the same file object, decompress just\
  // that entry so that "entry.Full" contains that entry's data.
  void DecompressEntry(ArchiveEntry& entry, File& file);

  //Write or Read to a file.
  void WriteZipFile(StringParam filename);
  void ReadZipFile(ArchiveReadFlags::Enum readFlags, StringParam filename);
  uint ComputeZipSize();
  //Entry Access

  //Clear all entries
  void Clear();
  uint EntryCount(){return Entries.Size();}
  typedef Array<ArchiveEntry>::range range;
  range GetEntries(){return Entries.All();}

private:
  uint mCompressionLevel;
  ArchiveMode::Enum mMode;
  Array<ArchiveEntry> Entries;

  void DecompressEntries();
  void DecompressEntry(ArchiveEntry& entry);
  void ComputeOffsets();

  template<typename Stream>
  void ReadZipFileInternal(ArchiveReadFlags::Enum readFlags, Stream& file);

  // Helper to decompress an entry. Assumes that the entry wasn't already
  // decompressed and that the entry is owned by the archive.
  template<typename Stream>
  void DecompressEntryInternal(ArchiveEntry& entry, Stream& file);

  template<typename Stream>
  void WriteZipInternal(Stream& file);
};

}
