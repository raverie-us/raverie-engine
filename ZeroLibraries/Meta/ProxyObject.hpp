///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum2(ProxyReason, TypeDidntExist, AllocationException);

// This file being in the Meta library is subject to discussion. For now, you
// have to know about Serialization (for DataNode) to use it.
class DataNode;

//---------------------------------------------------------------------------------- Proxy Component
template <typename ComponentType>
class ProxyObject : public ComponentType
{
public:
  ZilchDeclareInheritableType(TypeCopyMode::ReferenceType);

  // Creates a type 
  static BoundType* CreateProxyMetaFromFile(StringParam typeName, ProxyReason::Enum reason);
   
  ProxyObject();
  ~ProxyObject();
  void Serialize(Serializer& stream) override;

  DataNode* mProxiedData;
};

//**************************************************************************************************
template <typename ComponentType>
BoundType* ProxyObject<ComponentType>::CreateProxyMetaFromFile(StringParam typeName, ProxyReason::Enum reason)
{
  // Build the new type
  LibraryBuilder builder(typeName);
  BoundType* type = builder.AddBoundType(typeName, TypeCopyMode::ReferenceType, sizeof(ProxyObject<ComponentType>));

  type->BaseType = ZilchTypeId(ComponentType);

  // Assign the same handle manager as the base type we're proxying
  type->HandleManager = type->BaseType->HandleManager;
  type->AddAttribute(ObjectAttributes::cProxy);
  if(reason == ProxyReason::AllocationException)
    type->AddAttribute(ObjectAttributes::cExceptionProxy);
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  LibraryRef library = builder.CreateLibrary();
  MetaDatabase::GetInstance()->AddLibrary(library);

  return type;
}

//**************************************************************************************************
template <typename ComponentType>
ProxyObject<ComponentType>::ProxyObject()
  : mProxiedData(nullptr)
{

}

template <typename ComponentType>
ProxyObject<ComponentType>::~ProxyObject()
{
  SafeDelete(mProxiedData);
}

//**************************************************************************************************
template <typename ComponentType>
void ProxyObject<ComponentType>::Serialize(Serializer& stream)
{
  ErrorIf(stream.GetType() != SerializerType::Text, "Proxies serialized to Binary is not supported.");

  if(stream.GetMode() == SerializerMode::Loading)
  {
    // Copy the data tree from the top of the stack
    DataTreeLoader& loader = *(DataTreeLoader*)(&stream);
    mProxiedData = loader.GetCurrent()->Clone();
  }
  else
  {
    if (mProxiedData == nullptr)
      return;

    // mProxiedData stored the Component node, and it should have already been opened in the 
    // serializer once this function has been called, so we just want to save out the child nodes
    forRange(DataNode& child, mProxiedData->GetChildren())
      child.SaveToStream(stream);
  }
}

}//namespace Zero
