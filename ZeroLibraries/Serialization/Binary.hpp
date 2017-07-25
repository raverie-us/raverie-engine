///////////////////////////////////////////////////////////////////////////////
///
/// \file Binay.hpp
/// Declaration of the Binary Serializers.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

BoundType* const BinaryEndSignature = (BoundType*)0xFFFFDEAD;

//----------------------------------------------------------------- Binary Saver
template<typename binaryType>
class BinarySaver : public SerializerBuilder< BinarySaver<binaryType> >
{
public:
  typedef binaryType sub_type;
  typedef SerializerBuilder< BinarySaver<binaryType> > base_type;
  typedef Serializer::ArrayType ArrayType;
  typedef Serializer::StructType StructType;

  BinarySaver()
  {
    base_type::mMode = SerializerMode::Saving;
    base_type::mSerializerType = SerializerType::Binary;
  }
  SerializerClass::Enum GetClass() override { return SerializerClass::BinarySaver; }

  inline sub_type *BinaryType() { return static_cast<sub_type*>(this); }
 
  //------------------------------------------------------------ Polymorphic Serialization

  void StartPolymorphicInternal(const PolymorphicInfo& info) override
  {
    ErrorIf(info.mFlags.U32Field != 0, "Patching not supported in binary serialization");
    BinaryType()->Data((byte*)&info.mRuntimeType, sizeof(info.mRuntimeType));
  }

  bool GetPolymorphic(PolymorphicNode& node) override
  {
    //Invalid for Binary saver
    ErrorIf(true, cPolymorphicSerializationError);
    return false;
  }

  void EndPolymorphic() override
  {
    BoundType* end = BinaryEndSignature;
    BinaryType()->Data((byte*)&end, sizeof(end));
  }

  //-------------------------------------------------------  Fundamental serialization
  template<typename type>
  bool FundamentalType(type& value)
  {
    BinaryType()->Data((byte*)&value, sizeof(value));
    return true;
  }

  //------------------------------------------------------------ String Serialization
  bool StringField(cstr typeName, cstr fieldName, StringRange& stringRange) override
  {
    size_t sizeInBytes = stringRange.SizeInBytes();
    BinaryType()->Data((byte*)&sizeInBytes, sizeof(sizeInBytes));
    BinaryType()->Data((byte*)stringRange.Data(), sizeInBytes *sizeof(byte));
    return true;
  }

  //------------------------------------------------------------ Enum Serialization
  bool EnumField(cstr enumTypeName, cstr fieldName, uint& enumValue, BoundType*) override
  {
    //Serialize the enum as a simple integer
    FundamentalType(enumValue);
    return true;
  }

  //------------------------------------------------------------ Array Serialization
  bool ArrayField(cstr typeName, cstr fieldName, byte* data, 
                  ArrayType arrayType, uint numberOfElements, uint sizeOftype) override
  {
    BinaryType()->Data((byte*)data, sizeOftype*numberOfElements);
    return true;
  }

  void ArraySize(uint& arraySize) override
  {
    BinaryType()->Data((byte*)&arraySize, sizeof(arraySize));
  }

  //Serialization decorators does nothing for binary serializers
  bool InnerStart(cstr typeName, cstr fieldname, StructType structType) { return true; }
  void InnerEnd(cstr typeName, StructType structType) {}

#define FUNDAMENTAL(type) bool FundamentalField(cstr fieldName, type& value) override { return FundamentalType(value); }
#include "FundamentalTypes.hpp"
#undef FUNDAMENTAL

};

//---------------------------------------------------------------- Binary Loader
template<typename binaryType>
class BinaryLoader : public SerializerBuilder< BinaryLoader<binaryType> >
{
public:
  typedef SerializerBuilder< BinaryLoader<binaryType> > base_type;
  typedef Serializer::ArrayType ArrayType;
  typedef Serializer::StructType StructType;

  BinaryLoader()
  {
    base_type::mMode = SerializerMode::Loading;
    base_type::mSerializerType = SerializerType::Binary;
  }
  SerializerClass::Enum GetClass() override { return SerializerClass::BinaryLoader; }

  typedef binaryType sub_type;
  inline sub_type *BinaryType() { return static_cast<sub_type*>(this); }

  //-------------------------------------------------------  Fundamental serialization
  template<typename type>
  bool FundamentalType(type& value)
  {
    BinaryType()->Data((byte*)&value, sizeof(value));
    return true;
  }

  //------------------------------------------------------------ String Serialization

  ///Each binary loader must handle string separately

  //------------------------------------------------------------ Polymorphic Serialization
  bool GetPolymorphic(PolymorphicNode& node) override
  {
    //Read the integer based type id from the file
    node.Name = StringRange();
    node.TypeName = StringRange();
    node.ChildCount = 0;

    //Does this polymorphic node have more children?
    BoundType* nextBlockRuntimeType = nullptr;
    if(BinaryType()->TestForObjectEnd(&nextBlockRuntimeType))
    {
      //There is another polymorphic block in this element
      //Store the id
      node.RuntimeType = nextBlockRuntimeType;
      return true;
    }
    return false;
  }

  void EndPolymorphic() override
  {
    BoundType* end = nullptr;
    BinaryType()->Data((byte*)&end, sizeof(end));
    ErrorIf(end != BinaryEndSignature, "Binary buffer serialization error did "
            "not read the end element. A different number of bytes was "
            "serialized inside this polymorphic node.");
  }

  //------------------------------------------------------------ Array Serialization
  bool ArrayField(cstr typeName, cstr fieldName, byte* data, ArrayType arrayType, 
                 uint numberOfElements, uint sizeOftype) override
  {
    BinaryType()->Data((byte*)data, sizeOftype*numberOfElements);
    return true;
  }

  void ArraySize(uint& arraySize) override
  {
    BinaryType()->Data((byte*)&arraySize, sizeof(arraySize));
  }

  bool EnumField(cstr enumTypeName, cstr fieldName, uint& enumValue, BoundType*) override
  {
    //Serialize the enum as a simple integer
    FundamentalType(enumValue);
    return true;
  }

  //Serialization decorators does nothing for binary serializers
  bool InnerStart(cstr typeName, cstr fieldName, StructType structType) { return true; }
  void InnerEnd(cstr typeName, StructType structType) {}

  #define FUNDAMENTAL(type) bool FundamentalField(cstr fieldName, type& value) override { return FundamentalType(value); }
  #include "FundamentalTypes.hpp"
  #undef FUNDAMENTAL
};

//----------------------------------------------------------- Binary File Loader
class BinaryFileLoader : public BinaryLoader<BinaryFileLoader>
{
public:
  bool StringField(cstr typeName, cstr fieldName, StringRange& stringRange) override;
  bool OpenFile(Status& status, cstr filename);
  void Close();
  void Data(byte* data, uint size);
  bool TestForObjectEnd(BoundType** runtimeType);

private:
  char mTempSpace[512];
  File mFile;
};

//------------------------------------------------------------ Binary File Saver
class BinaryFileSaver : public BinarySaver<BinaryFileSaver>
{
public:
  bool Open(Status& status, cstr filename);
  void Close();
  void Data(byte* data, uint size);

private:
  File mFile;
};

//---------------------------------------------------------- Binary Buffer Saver
class BinaryBufferSaver  : public BinarySaver<BinaryBufferSaver>
{
public:
  BinaryBufferSaver();
  ~BinaryBufferSaver();

  void Open();
  uint GetSize();
  void Deallocate();

  void ExtractInto(byte* data, uint size);
  void ExtractInto(DataBlock& block);
  DataBlock ExtractAsDataBlock() override;

  void Data(byte* data, uint size);
private:
  ByteBuffer mBuffer;
};

//--------------------------------------------------------- Binary Buffer Loader
class BinaryBufferLoader: public BinaryLoader<BinaryBufferLoader>
{
public:
  bool StringField(cstr typeName, cstr fieldName, StringRange& stringRange) override;
  void SetBuffer(byte* data, uint size);
  void SetBlock(DataBlock block);

  void Data(byte* data, uint size);
  bool TestForObjectEnd(BoundType** runtimeType);

private:
  uint mBufferSize;
  byte* mCurrentPosition;
  byte* mBuffer;
};

}//namespace Zero
