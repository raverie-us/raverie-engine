///////////////////////////////////////////////////////////////////////////////
///
/// \file Simple.hpp
/// Declaration of the simple serialization functions.
///
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma  once

namespace Zero
{

DeclareEnum3(DataFileFormat,
  // Binary Data Format. 
  Binary,
  // Text Data Format.
  Text,
  // Determine from file
  Auto
);

// Simple serialization helper functions for data file serialization.

/// Create loading serializer for DataBlock with given format.
Serializer* GetLoaderStreamDataBlock(Status& status, DataBlock block, DataFileFormat::Enum format);

/// Create loading serializer for file with given format.
Serializer* GetLoaderStreamFile(Status& status, StringParam fileName, DataFileFormat::Enum format=DataFileFormat::Text);

/// Create saving serializer for file with given format.
Serializer* GetSaverStreamFile(Status& status, StringParam fileName, DataFileFormat::Enum format=DataFileFormat::Text);

/// Create saving serialize
Serializer* GetSaverStreamBlock(Status& status, DataFileFormat::Enum format=DataFileFormat::Text);

/// Get serialized that always returns defaults
Serializer* GetDefaultStream();

//Get what type is in the file
String GetTypeInFile(StringParam fileName);

//------------------------------------------------------------ Simple Save and Load

// Save an object to a data file.
template<typename type>
bool SaveToDataFile(type& object, StringParam fileName, DataFileFormat::Enum format=DataFileFormat::Text)
{
  Status status;
  UniquePointer<Serializer> stream(GetSaverStreamFile(status, fileName, format));
  if(stream)
  {
    stream->SerializePolymorphic(object);
    stream->Close();
    return true;
  }
  else
  {
    ErrorIf(!stream, "Failed to open file %s for writing data stream.", fileName.c_str());
    return false;
  }
}

// Load an object to a data file.
template<typename type>
bool LoadFromDataFile(type& object, StringParam fileName, DataFileFormat::Enum format = DataFileFormat::Text,  bool checkTypename = true)
{
  Status status;
  UniquePointer<Serializer> stream(GetLoaderStreamFile(status, fileName, format));
  if(stream)
  {
    PolymorphicNode node;
    stream->GetPolymorphic(node);
    cstr objectTypeName = ZilchVirtualTypeId(&object)->Name.c_str();
    if(!checkTypename || node.TypeName == objectTypeName)
    {
      object.Serialize(*stream);
      stream->EndPolymorphic();
      return true;
    }
    else
    {
      ErrorIf(true, "Object in file %s not the same as object. "
        "Object's type '%s' Type In file '%s'. ", 
        fileName.c_str(), objectTypeName, node.TypeName.Data());
      return false;
    }
  }
  else
  {
    ErrorIf(!stream, "Failed to load file %s into data stream. %s", 
            fileName.c_str(), status.Message.c_str());
    return false;
  }
}


// Load an object from a data block.
template<typename type>
bool LoadFromDataBlock(type& object, DataBlock block, DataFileFormat::Enum format)
{
  Status status;
  UniquePointer<Serializer> stream(GetLoaderStreamDataBlock(status, block, format));
  if(stream)
  {
    PolymorphicNode node;
    stream->GetPolymorphic(node);
    object.Serialize(*stream);
    stream->EndPolymorphic();
    return true;
  }
  return false;
}

// Save a object to a data block
template<typename type>
DataBlock SaveToDataBlock(type& object, DataFileFormat::Enum format)
{
  Status status;
  UniquePointer<Serializer> stream(GetSaverStreamBlock(status, format));
  if(stream)
  {
    stream->SerializePolymorphic(object);
    return stream->ExtractAsDataBlock();
  }
  return DataBlock();
}

// Serialize an object to default values
template<typename type>
void SerializeDefaults(type* instance)
{
  UniquePointer<Serializer> stream(GetDefaultStream());
  instance->Serialize(*stream);
}

//------------------------------------------------------------ Polymorphic Serialization

template<typename containerType>
void SavePolymorphicSerialize(cstr Name, cstr FieldName, Serializer& serializer,
                              containerType& container)
{
  serializer.Start(Name, FieldName, StructureType::Object);

  uint containerSize = container.Size();
  serializer.ArraySize(containerSize);

  typename containerType::range elements = container.All();
  while(!elements.Empty())
  {
    serializer.SerializePolymorphic(elements.Front()->GetSerializeInfo(), *elements.Front());
    elements.PopFront();
  }

  serializer.End(Name, StructureType::Object);
}

template<typename containerType, typename factoryType>
void LoadPolymorphicSerialize(cstr Name, cstr FieldName, Serializer& stream, 
                              containerType& container, factoryType* factory)
{
  stream.Start(Name, FieldName, StructureType::Object);

  uint containerSize = 0;
  stream.ArraySize(containerSize);

  for(uint i = 0; i < containerSize; ++i)
  {
    PolymorphicNode node;
    stream.GetPolymorphic(node);

    typename containerType::value_type objectPtr = factory->CreateFromName(node.TypeName);
    if(objectPtr)
    {
      objectPtr->Serialize(stream);
      container.PushBack(objectPtr);
      stream.EndPolymorphic();
    }
  }
  stream.End(Name, StructureType::Object);
}


template<typename containerType, typename factoryType>
void PolymorphicSerialize(cstr Name, cstr FieldName, Serializer& stream, 
                          containerType& container, factoryType* factory)
{
  if(stream.GetMode() == SerializerMode::Saving)
  {
    SavePolymorphicSerialize(Name, FieldName, stream, container);
  }
  else
  {
    LoadPolymorphicSerialize(Name, FieldName, stream, container, factory);
  }
}


}//namespace Zero
