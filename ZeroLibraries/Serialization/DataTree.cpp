///////////////////////////////////////////////////////////////////////////////
///
/// \file DataTree.cpp
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Platform/FileSystem.hpp"
#include "Platform/File.hpp"
#include "LegacyDataTreeParser.hpp"

namespace Zero
{

//------------------------------------------------------------- Data Tree Loader
HashMap<String, String> ValidTypeNameConversions;

//******************************************************************************
DataTreeLoader::DataTreeLoader()
{
  mMode = SerializerMode::Loading;
  mSerializerType = SerializerType::Text;
  mRoot = nullptr;
  mNext = nullptr;
  mIgnoreDataInheritance = false;
  mLoadedFileVersion = (uint)-1;
}

//******************************************************************************
DataTreeLoader::~DataTreeLoader()
{
  Close();
}

//******************************************************************************
SerializerClass::Enum DataTreeLoader::GetClass()
{
  return SerializerClass::DataTreeLoader;
}

//void RegisterPasers()
//{
//  RegisterParser(1, 5, grammar);
//  RegisterParser(6, 15, grammar);
//  RegisterParser(16, 18, grammar);
//}
//
//DataNode* ConvertToNewFormat(DataNode* tree)
//{
//  int version = tree->GetAttribute("DataVersion").As<int>();
//
//  Parser* parser = GetParser(version);

//  DataNode* correctVersion =
//}

//******************************************************************************
bool DataTreeLoader::OpenFile(Status& status, StringParam fileName)
{
  // First check if the file exists
  if(!FileExists(fileName))
  {
    status.SetFailed(String::Format("File not found '%s'", fileName.c_str()), FileSystemErrors::FileNotFound);
    return false;
  }

  // Attempt to open the file
  String fileRange = ReadFileIntoString(fileName);

  // Notify if we couldn't open the file
  if(fileRange.Empty())
  {
    status.SetFailed(String::Format("Can not open '%s'", fileName.c_str()), FileSystemErrors::FileNotAccessible);
    return false;
  }

  return OpenBuffer(status, fileRange, fileName);
}

//******************************************************************************
bool DataTreeLoader::OpenBuffer(Status& status, StringRange data, StringRange source)
{
  DataNode* root = ReadDataSet(status, data, source, this, &mLoadedFileVersion);

  if(root)
  {
    SetRoot(root);
    return true;
  }
  
  return false;
}

//******************************************************************************
void DataTreeLoader::Close()
{
  SafeDelete(mRoot);
}

//******************************************************************************
void DataTreeLoader::Reset()
{
  mNext = mRoot;
}

//******************************************************************************
bool DataTreeLoader::GetPolymorphic(PolymorphicNode& node)
{
  if(mNext)
  {
    PushChildOnStack();

    DataNode* current = GetCurrent();
    node.Name = current->mPropertyName.All();
    node.TypeName = current->mTypeName.All();
    node.RuntimeType = nullptr;
    node.UniqueNodeId = PolymorphicNode::cInvalidUniqueNodeId;
    node.Flags.Clear();
    node.mAttributes = &current->mAttributes;
    node.mInheritId = current->mInheritedFromId;

    // Subtractive flag
    if(current->mPatchState == PatchState::ShouldRemove)
      node.Flags.SetFlag(PolymorphicFlags::Subtractive);

    // DataSet settings
    if(current->mNodeType == DataNodeType::Object)
    {
      node.UniqueNodeId = current->mUniqueNodeId;

      // Check if this node was inherited from something else
      if(!current->mInheritedFromId.Empty())
        node.Flags.SetFlag(PolymorphicFlags::Inherited);
    }

    // Child order override
    if(current->mFlags.IsSet(DataNodeFlags::ChildOrderOverride))
      node.Flags.SetFlag(PolymorphicFlags::ChildOrderOverride);

    // Store whether or not it was patched
    if(current->IsPatched())
      node.Flags.SetFlag(PolymorphicFlags::Patched);

    node.ChildCount = current->GetNumberOfChildren();
    return true;
  }
  return false;
}

//******************************************************************************
void DataTreeLoader::EndPolymorphic()
{
  End((cstr)nullptr, StructureType::Object);
}

//******************************************************************************
bool CheckNode(DataNode* node, cstr typeName, DataTreeLoader::StructType structType)
{
  // Don't allow the incorrect node type (when we expect a value node)
  if(structType == StructureType::Value)
  {
    if(node->mNodeType != DataNodeType::Value)
      return false;
  }
  else
  {
    if(node->mNodeType != DataNodeType::Object)
      return false;
  }

  return true;
}

//******************************************************************************
bool DataTreeLoader::InnerStart(cstr typeName, cstr fieldName, StructType structType)
{
  //The current node that will be parent if successful
  DataNode* parent = GetCurrent();
  //The child node that will be the current if successful
  DataNode* current = mNext;

  if(fieldName != NULL)
  {
    if(*fieldName == 'm')
      ++fieldName;
  }

  if(fieldName)
  {
    if(current == NULL || current->mPropertyName != fieldName)
    {
      //No name just type mean old enum (deprecated)
      if(current && typeName && current->mTypeName.All() == typeName && current->mPropertyName.Empty())
      {
        PushChildOnStack();
        return true;
      }

      ErrorIf(current != NULL && current->mParent != parent,
              "Invalid parent!");

      //Try to find the node on the parent
      DataNode* findNode = parent->FindChildWithName(fieldName);
      if(findNode == NULL)
      {
        return false;
      }
      else if(CheckNode(findNode, typeName, structType))
      {
        mNodeStack.PushBack(findNode);
        mNext = findNode->GetFirstChild();
        return true;
      }
      else
      {
        return false;
      }
    }
  }

  if(CheckNode(current, typeName, structType))
  {
    PushChildOnStack();
    return true;
  }

  return false;
}

//******************************************************************************
void DataTreeLoader::InnerEnd(cstr typeName, StructType structType)
{
  PopStack();
}

//******************************************************************************
DataNode* DataTreeLoader::GetCurrent()
{
  return mNodeStack.Back();
}

//******************************************************************************
DataNode* DataTreeLoader::GetNext()
{
  return mNext;
}

//******************************************************************************
void DataTreeLoader::SetNext(DataNode* node)
{
  ErrorIf(node->mParent != mNodeStack.Back(), "Must be child of current node");
  mNext = node;
}

//******************************************************************************
void DataTreeLoader::SetRoot(DataNode* node)
{
  SafeDelete(mRoot);
  mNodeStack.Clear();

  mRoot = node;
  mNext = node;
  mNodeStack.PushBack(mRoot);
}

//******************************************************************************
DataNode* DataTreeLoader::TakeOwnershipOfRoot()
{
  DataNode* root = mRoot;
  mRoot = nullptr;
  mNext = nullptr;
  return root;
}

//******************************************************************************
String DataTreeLoader::DebugLocation()
{
  DataNode* node = GetCurrent();

  if(node == NULL)
    return "No location";

  return String::Format(
    "Node '%s %s' in file '%s'",
    node->mTypeName.c_str(), node->mPropertyName.c_str(),
    mFileName.c_str());
}

//******************************************************************************
PatchResolveMethod::Enum DataTreeLoader::ResolveInheritedData(StringRange inheritId,
                                                              DataNode*& result)
{
  Error("Data inheritance was found in data file, but not loaded with appropriate loader");
  return PatchResolveMethod::Error;
}

//******************************************************************************
DependencyAction::Enum DataTreeLoader::ResolveDependencies(DataNode* parent,
                                                           DataNode* newChild,
                                                           DataNode** toReplace,
                                                           Status& status)
{
  return DependencyAction::Add;
}

//---------------------------------------------------- Fundamental Serialization
//******************************************************************************
template<typename type>
bool DataTreeLoader::FundamentalType(type& value)
{
  if(GetCurrent() != NULL)
  {
    DataNode* treeValue = GetCurrent();
    ToValue(treeValue->mTextValue.All(), value);
  }
  return true;
}

//******************************************************************************
template bool DataTreeLoader::FundamentalType<float>(float&);
template bool DataTreeLoader::FundamentalType<int>(int&);
template bool DataTreeLoader::FundamentalType<double>(double&);
template bool DataTreeLoader::FundamentalType<unsigned int>(unsigned int&);
template bool DataTreeLoader::FundamentalType<bool>(bool&);
template bool DataTreeLoader::FundamentalType<Guid>(Guid&);

//******************************************************************************
bool DataTreeLoader::StringField(cstr typeName, cstr fieldName,
                                 StringRange& stringRange)
{
  if(InnerStart(typeName, fieldName, StructureType::Value))
  {
    DataNode* current = GetCurrent();
    stringRange = current->mTextValue.All();
    InnerEnd(typeName, StructureType::Value);
    return true;
  }
  return false;
}

//---------------------------------------------------- Polymorphic Serialization 
//******************************************************************************
void DataTreeLoader::PopStack()
{
  //Move the stack back up
  mNext = mNodeStack.Back();
  mNodeStack.PopBack();

  //Move to the next node
  mNext = mNext->NextSibling();
}

//******************************************************************************
void DataTreeLoader::PushChildOnStack()
{
  ErrorIf(mNext==NULL, "Child is not valid serialization error.");
  mNodeStack.PushBack(mNext);
  mNext = mNext->GetFirstChild();
}

//---------------------------------------------------------- Array Serialization 
//******************************************************************************
void DataTreeLoader::ArraySize(uint& arraySize)
{
  arraySize = GetCurrent()->GetNumberOfChildren();
}

//******************************************************************************
template<typename type>
void ReadArray(DataNode* arrayNode, type* data, uint numberOfElements)
{
  ErrorIf(arrayNode->mNodeType != DataNodeType::Object, "Node is not an array type. "
          "Name:'%s'", arrayNode->mPropertyName.c_str());
  ErrorIf(arrayNode->GetNumberOfChildren() != numberOfElements, "Array size "
          "does not match. Expected %u got %u", numberOfElements, 
          arrayNode->GetNumberOfChildren());

  DataNode* curElem = &arrayNode->GetChildren().Front();
  for(uint i = 0; i < numberOfElements && curElem; ++i)
  {
    DataNode* lit = curElem;
    ToValue(lit->mTextValue.All(), data[i]);
    curElem = curElem->link.Next;
  }
}

//******************************************************************************
bool DataTreeLoader::ArrayField(cstr typeName, cstr fieldName, byte* data, ArrayType arrayType, 
                                uint numberOfElements, uint sizeOftype)
{
  if(InnerStart(typeName, fieldName, StructureType::BasicArray))
  {
    //Array Size Safety Check
    if(GetCurrent()->GetNumberOfChildren() != numberOfElements)
    {
      End(typeName, StructureType::BasicArray);
      return false;
    }

    DataNode* arrayNode = GetCurrent();
    switch(arrayType)
    {
    case BasicArrayType::Float:
      ReadArray<float>(arrayNode, (float*)data, numberOfElements);
      break;

    case BasicArrayType::Integer:
      ReadArray<int>(arrayNode, (int*)data, numberOfElements);
      break;

    default:
      ErrorIf(true, "Can not serialize type."); 
      break;
    }
    InnerEnd(typeName, StructureType::BasicArray);
    return true;
  }

  return false;
}

//----------------------------------------------------------- Enum Serialization
//******************************************************************************
bool DataTreeLoader::EnumField(cstr enumTypeName, cstr fieldName, uint& enumValue, BoundType* type)
{
  if(InnerStart(enumTypeName, fieldName, StructureType::Value))
  {
    DataNode* treeValue = GetCurrent();
    Integer* foundEnumValue = type->StringToEnumValue.FindPointer(treeValue->mTextValue);

    if (foundEnumValue)
    {
      enumValue = *foundEnumValue;
    }
    else
    {
      // METAREFACTOR we should return false if this doesn't parse a value
      ToValue(treeValue->mTextValue, (Integer&)enumValue);
    }

    InnerEnd(enumTypeName, StructureType::Value);
    return true;
  }
  else
  {
    enumValue = 0;
    return false;
  }
}

}//namespace Zero
