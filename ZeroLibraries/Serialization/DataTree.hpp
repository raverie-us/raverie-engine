///////////////////////////////////////////////////////////////////////////////
///
/// \file DataTree.hpp
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class DataNode;
class DataObject;

//------------------------------------------------------------- Data Tree Loader
class DataTreeLoader : public SerializerBuilder<DataTreeLoader>
{
public:
  /// Constructor / Destructor.
  DataTreeLoader();
  ~DataTreeLoader();

  /// Serializer Interface.
  SerializerClass::Enum GetClass() override;

  /// Load a data tree from a file
  bool OpenFile(Status& status, StringParam file);

  /// Load a data tree from a StringRange. The 'source' parameter is used
  /// to make error messages more helpful.
  bool OpenBuffer(Status& status, StringRange data, StringRange source = "text buffer");

  /// Release the data tree and file memory.
  void Close() override;
  
  /// Reset the tree so it can serialize to cogs again
  void Reset();

  /// Polymorphic Serialization
  bool GetPolymorphic(PolymorphicNode& node) override;
  void EndPolymorphic() override;

  /// Standard Serialization
  bool InnerStart(cstr typeName, cstr fieldName, StructType structType);
  void InnerEnd(cstr typeName, StructType structType);

  DataNode* GetCurrent();
  DataNode* GetNext();
  void SetNext(DataNode* node);
  void SetRoot(DataNode* node);

  /// Takes ownership of the data tree. Primarily used in data tree patching.
  DataNode* TakeOwnershipOfRoot();

  String DebugLocation() override;

  bool StringField(cstr typeName, cstr fieldName, StringRange& stringRange) override;

  /// Array Serialization
  bool ArrayField(cstr typeName, cstr fieldName, byte* data, ArrayType simpleTypeId,
                  uint numberOfElements, uint sizeOftype) override;
  void ArraySize(uint& arraySize) override;

  /// Enum Serialization
  bool EnumField(cstr enumTypeName, cstr fieldName, uint& enumValue, BoundType* type) override;

  /// Fundamental Serialization
  template<typename type>
  bool FundamentalType(type& value);

  virtual PatchResolveMethod::Enum ResolveInheritedData(StringRange inheritId,
                                                        DataNode*& result);
  /// Returns whether or not the 
  virtual DependencyAction::Enum ResolveDependencies(DataNode* parent,
                                                     DataNode* newChild,
                                                     DataNode** toReplace,
                                                     Status& status);

  uint mLoadedFileVersion;

  DataAttributes mRootAttributes;

  /// If set to false, data inheritance will be ignored. This was added because
  /// Archetypes need to load the file just to look at the root node's inheritance id,
  /// the rest of the tree doesn't matter.
  bool mIgnoreDataInheritance;

protected:
  Array<DataNode*> mNodeStack;
  String mFileName;
  DataNode* mRoot;
  DataNode* mNext;
  void PushChildOnStack();
  void PopStack();

};

}//namespace Zero
