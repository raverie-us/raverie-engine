// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Saving Serializer for simple test file format.
class TextSaver : public SerializerBuilder<TextSaver>
{
public:
  /// Constructor / destructor.
  TextSaver();
  ~TextSaver();

  /// Open a text file for saving. FileMode must be either Write or Append.
  void Open(Status& status,
            cstr file,
            DataVersion::Enum version = DataVersion::Current,
            FileMode::Enum fileMode = FileMode::Write);
  bool OpenBuffer(DataVersion::Enum version = DataVersion::Current, FileMode::Enum fileMode = FileMode::Write);
  uint GetBufferSize();
  String GetString();

  SerializerClass::Enum GetClass() override;

  void ExtractInto(byte* data, uint size);
  DataBlock ExtractAsDataBlock();
  void Close();

  // Polymorphic Serialization
  void StartPolymorphicInternal(const PolymorphicInfo& info) override;
  bool GetPolymorphic(PolymorphicNode& node) override;
  void EndPolymorphic() override;
  void AddSubtractivePolymorphicNode(cstr typeName, Guid nodeId = 0) override;

  // Standard Serialization
  bool InnerStart(cstr typeName, cstr fieldName, StructType structType, bool ignoreTabs = false);
  void InnerEnd(cstr typeName, StructType structType);

  // Fundamental Serialization
  template <typename type>
  bool FundamentalType(type& value)
  {
    // Make sure its not NAN/IND/INF
    bool result = CorrectNonFiniteValues(value);
    mStream << value;
    return result;
  }

  bool SimpleField(cstr typeName, cstr fieldName, StringRange& stringRange) override;

  bool StringField(cstr typeName, cstr fieldName, StringRange& stringRange) override;

  // Array Serialization
  bool ArrayField(
      cstr typeName, cstr fieldName, byte* data, ArrayType arrayType, uint numberOfElements, uint sizeOftype) override;

  void ArraySize(uint& arraySize){};

  // Enum Serialization
  bool EnumField(cstr enumTypeName, cstr fieldName, uint& enumValue, BoundType* type) override;

  void SaveAttribute(StringParam name, StringParam value = "", bool stringValue = false);

  void SaveFileVersion();

  // private:
  StringBuilder mStream;
  String mFilename;
  uint mDepth;
  FileMode::Enum mWriteMode;

  /// The version being saved
  DataVersion::Enum mVersion;

  void Tabs();
  void SetFlags();
};

} // namespace Raverie
