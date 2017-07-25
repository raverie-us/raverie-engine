///////////////////////////////////////////////////////////////////////////////
///
/// \file Serialization.cpp
/// Implementation of the Serializer interface.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//--------------------------------------------------------------- Data Attributes
namespace SerializationAttributes
{
DefineStringConstant(Id);
DefineStringConstant(InheritId);
DefineStringConstant(ChildOrderOverride);
DefineStringConstant(LocallyAdded);
DefineStringConstant(LocallyRemoved);
}

//******************************************************************************
DataAttribute::DataAttribute(StringParam name, StringParam value) :
  mName(name),
  mValue(value)
{

}

const Guid PolymorphicNode::cInvalidUniqueNodeId = (Guid)-1;

//------------------------------------------------------------- Polymorphic Info
//******************************************************************************
PolymorphicInfo::PolymorphicInfo() : 
  mTypeName(nullptr),
  mFieldName(nullptr),
  mFlags(0),
  mRuntimeType(nullptr),
  mUniqueNodeId(PolymorphicNode::cInvalidUniqueNodeId)
{

}

//------------------------------------------------------------------- Serializer

//******************************************************************************
Serializer::Serializer()
{
  mSerializationContext = NULL;
  mPatching = false;
}

//******************************************************************************
Serializer::~Serializer()
{

}

//******************************************************************************
void* Serializer::GetSerializationContext()
{
  return mSerializationContext;
}

//******************************************************************************
void Serializer::SetSerializationContext(void* context)
{
  mSerializationContext = context;
}

//******************************************************************************
SerializerMode::Enum Serializer::GetMode()
{
  return mMode;
}

//******************************************************************************
SerializerType::Enum Serializer::GetType()
{
  return mSerializerType;
}

//******************************************************************************
bool Serializer::Start(BoundType* type, cstr fieldName, StructType structType)
{
  return Start(type->Name.c_str(), fieldName, structType);
}

//******************************************************************************
void Serializer::End(BoundType* type, StructType structType)
{
  End(type->Name.c_str(), structType);
}

//******************************************************************************
void Serializer::StartPolymorphic(cstr typeName)
{
  PolymorphicInfo info;
  info.mTypeName = typeName;
  StartPolymorphicInternal(info);
}

//******************************************************************************
void Serializer::StartPolymorphic(cstr typeName, PolymorphicSaveFlags::Enum flags)
{
  PolymorphicInfo info;
  info.mTypeName = typeName;
  info.mFlags.SetFlag(flags);
  StartPolymorphicInternal(info);
}

//******************************************************************************
void Serializer::StartPolymorphic(BoundType* runtimeType)
{
  PolymorphicInfo info;
  info.mTypeName = runtimeType->Name.c_str();
  info.mRuntimeType = runtimeType;
  StartPolymorphicInternal(info);
}

//******************************************************************************
void Serializer::StartPolymorphicInheritence(cstr typeName, cstr dataInheritanceId)
{
  PolymorphicInfo info;
  info.mTypeName = typeName;
  info.mInheritanceId = dataInheritanceId;
  StartPolymorphicInternal(info);
}

//******************************************************************************
void Serializer::StartPolymorphicInheritence(cstr typeName, cstr dataInheritanceId,
                                             PolymorphicSaveFlags::Enum flags)
{
  PolymorphicInfo info;
  info.mTypeName = typeName;
  info.mInheritanceId = dataInheritanceId;
  info.mFlags.SetFlag(flags);
  StartPolymorphicInternal(info);
}

//******************************************************************************
void Serializer::StartPolymorphicInternal(const PolymorphicInfo& info)
{
  Error(cPolymorphicSerializationError);
}

//******************************************************************************
void Serializer::AddSubtractivePolymorphicNode(BoundType* boundType, Guid nodeId)
{
  AddSubtractivePolymorphicNode(boundType->Name.c_str(), nodeId);
}

//******************************************************************************
bool Serializer::SimpleField(cstr typeName, cstr fieldName, StringRange& stringRange)
{
  DoNotifyError("Unimplemented function", "SimpleField not implemented");
  return true;
}

//******************************************************************************
DataBlock Serializer::ExtractAsDataBlock()
{
  return DataBlock();
}

//******************************************************************************
String Serializer::DebugLocation()
{
  return "Unknown";
}

//******************************************************************************
void EncodeBinary(ByteBufferBlock& buffer, String& encodedOut)
{
  // We should replace this with a base64 encoding of zipped data

  // Just hexify the code right now (+1 for a version number character)
  StringNode* node = String::AllocateNode(buffer.Size() * 2 + 1);
  node->Data[0] = '0';

  for (size_t i = 0; i < buffer.Size(); ++i)
  {
    byte c = buffer.GetBegin()[i];
    byte high = c / 16;
    byte low  = c % 16;

    high += (high < 10) ? '0' : 'A';
    low  += (low  < 10) ? '0' : 'A';

    node->Data[i * 2 + 1] = high;
    node->Data[i * 2 + 2] = low;
  }

  encodedOut = String(node);
}

//******************************************************************************
bool DecodeBinary(ByteBufferBlock& buffer, const String& encoded)
{
  // If there's no data to decode, then early out
  if (encoded.Empty())
    return false;

  byte version = *encoded.Data();

  if (version == '0')
  {
    size_t bufferSize = (encoded.SizeInBytes() - 1) / 2;
    buffer.SetData(new byte[bufferSize], bufferSize, true);

    for (size_t i = 0; i < bufferSize; ++i)
    {
      byte high = encoded.Data()[i * 2 + 1];
      byte low  = encoded.Data()[i * 2 + 2];

      high -= (high < 'A') ? '0' : 'A';
      low  -= (low  < 'A') ? '0' : 'A';

      buffer.GetBegin()[i] = high * 16 + low;
    }
    return true;
  }

  Error("Unexpected binary version");
  return false;
}

}//namespace Zero
