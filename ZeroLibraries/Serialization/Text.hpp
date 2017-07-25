///////////////////////////////////////////////////////////////////////////////
///
/// \file Text.hpp
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------------- Text Saver
/// Saving Serializer for simple test file format.
class TextSaver : public SerializerBuilder<TextSaver>
{
public:
  /// Constructor / destructor.
  TextSaver();
  ~TextSaver();

  /// Open a text file for saving.
  void Open(Status& status, cstr file, DataVersion::Enum version = DataVersion::Current);
  bool OpenBuffer(DataVersion::Enum version = DataVersion::Current);
  uint GetBufferSize();
  String GetString();

  SerializerClass::Enum GetClass() override;

  void ExtractInto(byte* data, uint size);
  DataBlock ExtractAsDataBlock();
  void Close();

  //Polymorphic Serialization
  void StartPolymorphicInternal(const PolymorphicInfo& info) override;
  bool GetPolymorphic(PolymorphicNode& node) override;
  void EndPolymorphic()  override;
  void AddSubtractivePolymorphicNode(cstr typeName, Guid nodeId = 0) override;

  //Standard Serialization
  bool InnerStart(cstr typeName, cstr fieldName, StructType structType, bool ignoreTabs = false);
  void InnerEnd(cstr typeName, StructType structType);

  //Fundamental Serialization
  template<typename type>
  bool FundamentalType(type& value);

  bool SimpleField(cstr typeName, cstr fieldName, StringRange& stringRange) override;

  bool StringField(cstr typeName, cstr fieldName, StringRange& stringRange) override;

  //Array Serialization
  bool ArrayField(cstr typeName, cstr fieldName, byte* data, ArrayType arrayType,
                           uint numberOfElements, uint sizeOftype) override;

  void ArraySize(uint& arraySize){};

  //Enum Serialization
  bool EnumField(cstr enumTypeName, cstr fieldName, uint& enumValue, BoundType* type) override;

  void SaveAttribute(StringParam name, StringParam value = "", bool stringValue = false);

//private:
  StringBuilder mStream;
  String mFilename;
  uint mDepth;

  /// The version being saved
  DataVersion::Enum mVersion;

  void Tabs();
  void SetFlags();
};

}//namespace Zero
