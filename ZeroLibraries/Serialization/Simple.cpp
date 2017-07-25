///////////////////////////////////////////////////////////////////////////////
///
/// \file Simple.cpp
/// Implementation of the simple serialization functions.
///
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

String GetTypeInFile(StringParam fileName)
{
  Status status;
  File file;
  bool loaded = file.Open(fileName.c_str(), FileMode::Read, FileAccessPattern::Sequential);
  if(!loaded)
  {
    ErrorIf(true, "Failed to load file. Could not open file %s", 
      fileName.c_str());
    return String();
  }
  
  const uint bufferSize = 512;
  char buffer[bufferSize];
  file.Read(status, (byte*)buffer, bufferSize);

  //Eat leading whitespace
  uint startPos = 0;
  while(startPos < bufferSize && IsSpace(Rune(buffer[startPos])))
    ++startPos;

  // Skip file version
  if(startPos < bufferSize && buffer[startPos] == '[')
  {
    // Eat until we finish the attribute
    while(startPos < bufferSize)
    {
      if(buffer[startPos] == ']')
      {
        // Skip the new line
        startPos += 2;
        break;
      }
      ++startPos;
    }
  }

  //Find end of text
  uint pos = startPos;
  while(pos < bufferSize && !IsSpace(Rune(buffer[pos])))
    ++pos;

  //Null terminate
  buffer[pos] = '\0';

  return String(buffer+startPos);
}

Serializer* GetLoaderStreamDataBlock(Status& status, DataBlock block, DataFileFormat::Enum format)
{
  if(format == DataFileFormat::Text)
  {
    DataTreeLoader* loader = new DataTreeLoader();
    StringRange range( (char*)block.Data, (char*)block.Data, (char*)block.Data + block.Size);
    loader->OpenBuffer(status, range);
    return loader;
  }
  else
  {
    //Binary Data File load using BinaryFileLoader
    BinaryBufferLoader* loader = new BinaryBufferLoader();
    loader->SetBuffer(block.Data, block.Size);
    return loader;
  }
}

Serializer* GetLoaderStreamFile(Status& status, StringParam fileName, DataFileFormat::Enum format)
{
  if(FileExists(fileName))
  {
    if(format == DataFileFormat::Text)
    {
      //Text Data File Load using Data Tree
      //ObjectLoader* loader = new ObjectLoader();
      DataTreeLoader* loader = new DataTreeLoader();
      loader->OpenFile(status, fileName);
      if(status.Failed())
      {
        delete loader;
        return NULL;
      }
      return loader;
    }
    else
    {
      //Binary Data File load using BinaryFileLoader
      BinaryFileLoader* loader = new BinaryFileLoader();
      loader->OpenFile(status, fileName.c_str());
      return loader;
    }
  }
  else
  {
    status.SetFailed( BuildString("File Not Found ", fileName), FileSystemErrors::FileNotFound );
  }

  return NULL;
}


Serializer* GetSaverStreamFile(Status& status, StringParam fileName, DataFileFormat::Enum format)
{
  if(format == DataFileFormat::Text)
  {
    //Save with Text Saver
    TextSaver* saver = new TextSaver();
    saver->Open(status, fileName.c_str());
    return saver;
  }
  else
  {
    //Save with Binary File Saver
    BinaryFileSaver* saver = new BinaryFileSaver();
    saver->Open(status, fileName.c_str());
    return saver;
  }
}

Serializer* GetSaverStreamBlock(Status& status, DataFileFormat::Enum format)
{
  if(format == DataFileFormat::Text)
  {
    //Save with Text Saver
    TextSaver* saver = new TextSaver();
    saver->OpenBuffer();
    return saver;
  }
  else
  {
    //Save with Binary File Saver
    BinaryBufferSaver* saver = new BinaryBufferSaver();
    saver->Open();
    return saver;
  }
}

Serializer* GetDefaultStream()
{
  return new DefaultSerializer();
}

}//namespace Zero
