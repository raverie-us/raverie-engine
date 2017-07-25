///////////////////////////////////////////////////////////////////////////////
///
/// \file Archive.cpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "zlib.h"

#if ZeroRelease
#pragma comment(lib, "zlib.lib")
#else
#pragma comment(lib, "zlibd.lib")
#endif

//------------------------------------------------------------
namespace Zero
{

  class Deflater
  {
  public:
    z_stream stream;
    int written;
    Deflater(int level)
    {
      //allocate inflate state
      stream.zalloc = Z_NULL;
      stream.zfree = Z_NULL;
      stream.opaque = Z_NULL;
      stream.avail_in = 0;
      stream.next_in = Z_NULL;
      deflateInit2(&stream, level, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
    }

    ~Deflater()
    {
      deflateEnd(&stream);
    }

    void Deflate(byte* input, byte* output, int availableIn, int availableOut, int finished)
    {
      int flushStatus = finished ? Z_FINISH : Z_NO_FLUSH;
      stream.avail_in = availableIn;
      stream.next_in = input;
      stream.avail_out = availableOut;
      stream.next_out = output;
      deflate(&stream, flushStatus);
      written = availableOut - stream.avail_out;
    }
  };

  class Inflater
  {
  public:
    z_stream stream;
    int written;
    bool done;

    Inflater()
    {
      //allocate inflate state
      stream.zalloc = Z_NULL;
      stream.zfree = Z_NULL;
      stream.opaque = Z_NULL;
      stream.avail_in = 0;
      stream.next_in = Z_NULL;
      inflateInit2(&stream, -MAX_WBITS);
    }

    ~Inflater()
    {
      inflateEnd(&stream);
    }

    void Inflate(byte* input, byte* output, int availableIn, int availableOut)
    {
      stream.avail_in = availableIn;
      stream.next_in = input;
      stream.avail_out = availableOut;
      stream.next_out = output;
      int zstatus = inflate(&stream, Z_SYNC_FLUSH);
      written = availableOut - stream.avail_out;

      if(zstatus==Z_STREAM_END)
        done = true;
    }
  };

  int RawDeflate(byte* outputData, uint outsize, byte* inputData, uint inSize, int level)
  {
    Deflater deflater(level);
    deflater.Deflate(inputData, outputData, inSize, outsize, true);
    return deflater.written;
  }

  int RawInflate(byte* outputData, uint outSize, byte* inputData, uint inSize)
  {
    Inflater inflater;
    inflater.Inflate(inputData, outputData, inSize, outSize);
    return inflater.written;
  }
  


//  ------------------ Archive

Archive::Archive(ArchiveMode::Enum mode, uint compressionLevel)
  :mMode(mode), mCompressionLevel(compressionLevel)
{

}


Archive::~Archive()
{
  Clear();
}

void Archive::Clear()
{
  forRange(ArchiveEntry& entry, Entries.All())
  {
    FreeBlock(entry.Full);
    FreeBlock(entry.Compressed);


  }

  Entries.Clear();
}

void Archive::ArchiveDirectory(Status& status, StringParam path, StringParam parentPath, FileFilter* filter)
{
  FileRange files(path);
  for(;!files.Empty();files.PopFront())
  {
    //If the path is a directory enumerate it
    String localPath = files.Front();
    String fullPath = FilePath::Combine(path, files.Front());
    String relativePath = FilePath::Combine(parentPath, localPath);

    FilterResult::Enum filterResult = FilterResult::Include;
    if(filter) filterResult = filter->Filter(fullPath);
    
    if(filterResult == FilterResult::Ignore)
      continue;

    if(IsDirectory(fullPath))
    {
      ArchiveDirectory(status, FilePath::Combine(path, localPath), relativePath, filter);
    }
    else
    {
      AddFile(fullPath, relativePath);
    }
  }
}

void Archive::AddFileRelative(StringParam basePath, StringParam relativeName)
{
  AddFile(FilePath::Combine(basePath, relativeName), relativeName);
}

void Archive::AddFileBlock(StringParam relativeName, DataBlock sourceBlock)
{
  byte* destBuffer = 0;
  uLong compressedSize = 0;

  //Compute Crc
  uLong crcTemp = crc32(0L, Z_NULL, 0);
  uLong crc = crc32(crcTemp, sourceBlock.Data, sourceBlock.Size);

  if(mCompressionLevel==0)
  {
    //No compression, just copy data into buffer
    compressedSize = sourceBlock.Size;
    destBuffer = (byte*)zAllocate(sourceBlock.Size);
    memcpy(destBuffer, sourceBlock.Data, sourceBlock.Size);
  }
  else
  {
    //Get the upper bound for the compressed data
    uint maxCompressedSize = compressBound(sourceBlock.Size);

    //Allocate the output buffer
    destBuffer = (byte*)zAllocate(maxCompressedSize);

    //Deflate the buffer to the compressed size
    compressedSize = RawDeflate(destBuffer, maxCompressedSize,  sourceBlock.Data, sourceBlock.Size, mCompressionLevel);

    //Optionally shrink the buffer to the compresses size
    //this costs another memory copy bug the compresses size can be significantly smaller
    const bool shrinkToActualCompressedSize = true;
    if(shrinkToActualCompressedSize)
    {
      byte* compressed = (byte*)zAllocate(compressedSize);
      memcpy(compressed, destBuffer, compressedSize);
      zDeallocate(destBuffer);
      destBuffer = compressed;
    }
  }

  // Add the entry
  ArchiveEntry entry;
  entry.Name = relativeName;
  entry.Full.Data = nullptr;
  entry.Full.Size = sourceBlock.Size;
  entry.Compressed.Size = compressedSize;
  entry.Compressed.Data = destBuffer;
  entry.Crc = crc;
  entry.CompressionLevel = mCompressionLevel;
  // Use current time
  entry.ModifiedTime = Time::GetTime();
  Entries.PushBack(entry);

}

void Archive::AddFile(StringParam fullpath, StringParam relativeName)
{
  DataBlock dataBlock = ReadFileIntoDataBlock(fullpath.c_str());

  //Could not open file.
  if(!dataBlock)
    return;

  AddFileBlock(relativeName, dataBlock);

  FreeBlock(dataBlock);
}

void Archive::ExportToDirectory(ArchiveExportMode::Enum exportMode, StringParam path)
{

  CreateDirectoryAndParents(path);
  forRange(ArchiveEntry& entry, Entries.All())
  {
    //some zippers place empty entries for folders
    if(entry.Full.Size == 0)
      continue;

    String outputFile = FilePath::Combine(path, FilePath::Normalize(entry.Name));
    String newpath = FilePath::GetDirectoryPath(outputFile);
    CreateDirectoryAndParents(newpath);

    if(exportMode == ArchiveExportMode::OverwriteIfNewer && FileExists(outputFile))
    {
      TimeType fileTime = GetFileModifiedTime(outputFile);
      if(fileTime >= entry.ModifiedTime)
        continue;
    }

    //Decompress the entry if it has not been done.
    if(entry.Full.Data == nullptr)
      DecompressEntry(entry);

    WriteToFile(outputFile.c_str(), entry.Full.Data, entry.Full.Size);
  }
}



void Archive::DecompressEntry(ArchiveEntry& entry)
{
  if(entry.Full.Data == nullptr)
  {
    if(entry.CompressionLevel == CompressionLevel::NoCompression)
    {
      // Not compressed copy data
      entry.Full.Data = entry.Compressed.Data;
      entry.Compressed.Data = nullptr;
    }
    else
    {
      // Inflate Data
      entry.Full.Data = (byte*)zAllocate(entry.Full.Size);
      RawInflate(entry.Full.Data, entry.Full.Size, entry.Compressed.Data, entry.Compressed.Size);

      // Free compressed data
      zDeallocate(entry.Compressed.Data);
      entry.Compressed.Data = nullptr;
    }
  }
}

void Archive::DecompressEntries()
{
  forRange(ArchiveEntry& entry, Entries.All())
  {
    DecompressEntry(entry);
  }
}


//-------------------------Zip------------------------------------------------

const u32 LocalHeader = 0x04034b50;
const u32 CentralHeader = 0x02014b50;
const u32 EndCentralHeader = 0x06054b50;

#pragma pack(2)
struct FileInfo
{
  //Version needed to extract (minimum)
  u16 MinVersion;
  //General purpose bit flag
  u16 GeneralFlags;
  //Compression Method
  u16 CompressionMethod;
  //Last Modified Time
  u16 LastModifiedTime;
  //Last Modified Date
  u16 LastModifiedDate;
  //Cyclic redundancy check 32
  u32 Crc;
  //Compressed size in bytes.
  u32 CompressedSize;
  //Uncompressed size
  u32 UncompressedSize;
  //File name length
  u16 FileNameLength;
  //Extra field length
  u16 ExtraFieldLength;
};


struct ZipLocalFileHeader
{
  //Signature
  u32 Signature;
  FileInfo Info;
};

struct ZipCentralFileHeader
{
  //Signature
  uint Signature;
  //Version Made By
  u16 VersionMadeBy;
  FileInfo Info;
  //File Comment length
  u16 FileCommentLength;
  //Disk number
  u16 DiskNumber;
  //Internal File Attributes
  u16 InternalAttributes;
  //External File Attributes
  u32 ExternalAttributes;
  //RelativeOffset
  u32 Offset;
};

struct EndCentral
{ 
  //Signature
  uint Signature;
  //Disk number
  u16 DiskNumber;
  //Disk number Start
  u16 DiskNumberStart;
  //Disk number
  u16 NumberOnDisk;
  //Number of Records
  u16 NmberOfRecords;

  u32 SizeOfCentralDirectory;
  u32 OffsetOfCentralDirectory;
  u16 CommentLength;
};

//Convert time to Zip time format (MS-DOS zip format)
//Thanks http://proger.i-forge.net/MS-DOS_date_and_time_format/OFz
void TimeToZipTime(TimeType time, u16* mdate, u16* mtime)
{
  //Format of MSDOSTIME
  //date:   YYYYYYYM MMMDDDDD
  //time:   HHHHHMMM MMMSSSSS
  //Y - Year from 1980
  //M - Month 1-12
  //D - Day 1-31
  //H - Hour 0 - 23
  //M - Minute 0-59
  //S = 0 -29 (every 2 seconds)
  CalendarDateTime lt = Time::GetLocalTime(time);
  //Rebase from 0 to 1980
  u16 year =   lt.Year - 1980;
  u16 month =  lt.Month + 1;
  u16 day =    lt.Day;
  u16 hour =   lt.Hour;
  u16 minute = lt.Minutes;
  //Every other second
  u16 second = lt.Seconds / 2;
  *mdate = (year << 9) | (month << 5) | (day);
  *mtime = (hour << 11) | (minute << 5) | (second);
}

TimeType ZipTimeToTime(u16 mdate, u16 mtime)
{
  CalendarDateTime newTime;
  newTime.Year = ((mdate & 0xFE00) >> 9) + 1980;;
  newTime.Month =  ((mdate & 0x01E0) >> 5) - 1;
  newTime.Day = (mdate & 0x001F) >> 0;
  newTime.Hour = (mtime & 0xF800) >> 11;
  newTime.Minutes =  (mtime & 0x07E0) >> 5;
  newTime.Seconds =  ((mtime & 0x001F) >> 0) * 2;
  return Time::CalendarDateTimeToTimeType(newTime);
}

void FillEntry(FileInfo& info, ArchiveEntry& entry, uint compressionMethod)
{
  u16 currentDate = 0;
  u16 currentTime = 0;
  TimeToZipTime(entry.ModifiedTime, &currentDate, &currentTime);
  info.Crc = entry.Crc;
  info.CompressedSize = entry.Compressed.Size;
  info.UncompressedSize = entry.Full.Size;
  info.CompressionMethod = compressionMethod;
  info.ExtraFieldLength = 0;
  info.FileNameLength = (u16)entry.Name.SizeInBytes();
  info.GeneralFlags = 0;
  info.LastModifiedDate = currentDate;
  info.LastModifiedTime = currentTime;
  info.MinVersion = 20;
}

uint Archive::ComputeZipSize()
{
  uint sizeOfallNames = 0;
  uint sizeOfAllData = 0;
  
  //Sum the size of all file name strings
  forRange(ArchiveEntry& entry, Entries.All())
  {
    sizeOfallNames += entry.Name.SizeInBytes();
    sizeOfAllData += entry.Compressed.Size;
  }

  uint entryCount = Entries.Size();
  uint size = 0;

  //For each entry there is a ZipLocalFileHeader followed by the name and the data
  size += sizeof(ZipLocalFileHeader) * entryCount + sizeOfallNames + sizeOfAllData;

  //At the end of the file is the central header with an entry for each archive entry
  size += sizeof(ZipCentralFileHeader) * entryCount + sizeOfallNames;

  //Add in the End footer
  size += sizeof(EndCentral);

  return size;
}

const uint DeflateMethod = 8;
const uint NoCompressionMethod = 0;

template<typename Stream>
void Archive::WriteZipInternal(Stream& file)
{
  uint compressionMethod = mCompressionLevel == 0 ? NoCompressionMethod : DeflateMethod;

  forRange(ArchiveEntry& entry, Entries.All())
  {
    entry.Offset = (size_t)file.Tell();
    ZipLocalFileHeader header;
    header.Signature = LocalHeader;
    FillEntry(header.Info, entry, compressionMethod);
    Write(file, header);
    file.Write((byte*)entry.Name.Data(), entry.Name.SizeInBytes()); 
    file.Write((byte*)entry.Compressed.Data, entry.Compressed.Size);
  }

  u32 centralOffset = (u32)file.Tell();
  forRange(ArchiveEntry& entry, Entries.All())
  {
    ZipCentralFileHeader header;
    header.Signature = CentralHeader;
    header.DiskNumber = 0;
    header.ExternalAttributes = 0;
    header.InternalAttributes = 0;
    header.VersionMadeBy = 20;
    header.FileCommentLength = 0;
    header.Offset = entry.Offset;
    FillEntry(header.Info, entry, compressionMethod);
    Write(file, header);
    file.Write((byte*)entry.Name.Data(), entry.Name.SizeInBytes());
  }

  EndCentral endCentral;
  endCentral.Signature = EndCentralHeader;
  endCentral.DiskNumber = 0;
  endCentral.DiskNumberStart = 0;
  endCentral.NmberOfRecords = (u16)Entries.Size();
  endCentral.NumberOnDisk = (u16)Entries.Size();
  endCentral.OffsetOfCentralDirectory = centralOffset;
  endCentral.SizeOfCentralDirectory = (u32)file.Tell() - centralOffset;
  endCentral.CommentLength = 0;

  Write(file, endCentral);
}

template<typename Stream>
void Archive::ReadZipFileInternal(ArchiveReadFlags::Enum readFlags, Stream& file)
{
  u32 signature = 0;
  Status status;
  for(;;)
  {
    //Look at what is next.
    PeekType(file, signature);
    if(signature == LocalHeader)
    {
      //Local file header.
      ZipLocalFileHeader localFile;
      Read(file,  localFile);

      ArchiveEntry& entry = Entries.PushBack();

      entry.ModifiedTime =  ZipTimeToTime(localFile.Info.LastModifiedDate, localFile.Info.LastModifiedTime);
      entry.Full.Size = localFile.Info.UncompressedSize;
      entry.Full.Data = nullptr;
      entry.Compressed.Size = localFile.Info.CompressedSize;
      entry.Compressed.Data = nullptr;

      // Do not know the original compression level so 
      // mark with DefaultCompression so entry will be decompressed properly
      // in decompress entry
      if(localFile.Info.CompressionMethod == DeflateMethod)
        entry.CompressionLevel = CompressionLevel::DefaultCompression;
      else
        entry.CompressionLevel = CompressionLevel::NoCompression;

      // Read file name
      String extra;
      ReadString(file, localFile.Info.FileNameLength, entry.Name);
      ReadString(file, localFile.Info.ExtraFieldLength, extra);

      entry.Offset = (uint)file.Tell();
      entry.Crc = localFile.Info.Crc;

      if(readFlags & ArchiveReadFlags::Data)
      {
        // Read the compressed data into a heap allocated block
        byte* buffer = (byte*)zAllocate(localFile.Info.CompressedSize);
        file.Read(status, buffer, localFile.Info.CompressedSize);
        entry.Compressed.Data = buffer;

        if(readFlags & ArchiveReadFlags::Decompress)
        {
          DecompressEntry(entry);
        }
      }
      else
      {
        //Seek past the entry
        file.Seek( localFile.Info.CompressedSize, FileOrigin::Current);
      }
    }
    else if(signature == CentralHeader)
    {
      ZipCentralFileHeader centralHeader;
      Read(file,  centralHeader);
      String fileName;
      String extra;
      String comment;
      ReadString(file, centralHeader.Info.FileNameLength, fileName);

      ReadString(file, centralHeader.Info.ExtraFieldLength, extra);

      ReadString(file, centralHeader.FileCommentLength, comment);
    }
    else
    {
      if(signature!=EndCentralHeader)
        return;

      EndCentral endCentral;
      Read(file, endCentral);

      String comment;

      ReadString(file, endCentral.CommentLength, comment);
      return;
    }
  }
}

void Archive::Extract(File& file, StringParam name, StringParam destfile)
{
  forRange(ArchiveEntry& entry, Entries.All())
  {
    if(entry.Name == name)
    {
      Status status;
      byte* buffer = (byte*)zAllocate(entry.Compressed.Size);
      file.Seek(entry.Offset, FileOrigin::Begin);
      file.Read(status, buffer, entry.Compressed.Size);
      entry.Compressed.Data = buffer;
      DecompressEntry(entry);
      WriteToFile(destfile.c_str(), entry.Full.Data, entry.Full.Size);
      FreeBlock(entry.Full);
    }
  }
}

void Archive::WriteZip(File& file)
{
  WriteZipInternal(file);
}

void Archive::WriteZipFile(StringParam name)
{
  File file;
  file.Open(name.c_str(), FileMode::Write, FileAccessPattern::Sequential);
  WriteZip(file);
}

void Archive::ReadZip(ArchiveReadFlags::Enum readFlags, DataBlock block)
{
  ByteBufferBlock buffer;
  buffer.SetBlock(block);
  ReadBuffer(readFlags, buffer);
}

void Archive::ReadZip(ArchiveReadFlags::Enum readFlags, File& file)
{
  ReadZipFileInternal(readFlags, file);
}

void Archive::ReadZipFile(ArchiveReadFlags::Enum readFlags, StringParam name)
{
  File file;
  file.Open(name.c_str(), FileMode::Read, FileAccessPattern::Sequential);
  ReadZip(readFlags, file);
}

void Archive::ReadBuffer(ArchiveReadFlags::Enum readFlags, ByteBufferBlock& buffer)
{
  ReadZipFileInternal(readFlags, buffer);
}

template<typename Stream>
void Archive::DecompressEntryInternal(ArchiveEntry& entry, Stream& file)
{
  // Seek to the start of the entry
  file.Seek(entry.Offset, FileOrigin::Begin);

  // Read the compressed data into a heap allocated block
  byte* buffer = (byte*)zAllocate(entry.Compressed.Size);
  Status status;
  file.Read(status, buffer, entry.Compressed.Size);
  entry.Compressed.Data = buffer;
  DecompressEntry(entry);
}

void Archive::DecompressEntry(ArchiveEntry& entry, File& file)
{
  DecompressEntryInternal(entry, file);
}

void Archive::WriteBuffer(ByteBuffer& buffer)
{
  WriteZipInternal(buffer);
}

void Archive::WriteBuffer(ByteBufferBlock& buffer)
{
  WriteZipInternal(buffer);
}

}
