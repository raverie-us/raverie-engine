///////////////////////////////////////////////////////////////////////////////
///
/// \file BroadPhaseProxy.hpp
/// Declaration of BroadPhaseProxy and ProxyPair class'.
/// 
/// Authors: Josh Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

typedef u64 PairId;
typedef u64 ClientPairId;

///Used by BroadPhase to identify an object. The proxy is
///specific to each type of BroadPhase and Contains whatever
///the BroadPhase needs to find the object quickly. An object 
///should never do anything to this proxy apart from hold it.
struct BroadPhaseProxy
{
  ///Defaulted for constructing an empty proxy to eventually fill out.
  explicit BroadPhaseProxy(void* proxy = nullptr) { mProxy = proxy; }
  explicit BroadPhaseProxy(u32 proxy) { mIntProxy = proxy; }

  void* ToVoidPointer() const { return mProxy; }
  u32 ToU32() const { return mIntProxy; }

private:
  ///Any type that is useful to a BroadPhase should be in this union.
  union
  {
    void* mProxy;
    u32 mIntProxy;
  };
};

///If the client data is a generic type, don't default
///it to anything, we don't know how.
template <typename ClientDataType>
inline void GetDefaultClientDataValue(ClientDataType& data)
{

}

///If the client data is a void*, we can default it to null.
template <>
inline void GetDefaultClientDataValue(void*& data)
{
  data = nullptr;
}

///Used to tell a BroadPhase where an object resides.
template <typename ClientDataType>
struct BaseBroadPhaseData
{
  BaseBroadPhaseData() { GetDefaultClientDataValue(mClientData); }

  ///The aabb for the client data. To be used for any Aabb BroadPhase.
  Aabb mAabb;
  ///The sphere for the client data. To be used for any Sphere BroadPhase.
  Sphere mBoundingSphere;
  ///The actual client data. Whatever the user wants to receive from a query.
  ClientDataType mClientData;
};

///Used in BroadPhases that want to store a non-shrinking array
///of BroadPhaseData's (mValid deals with the non shrinking aspect).
template <typename ClientDataType>
struct BaseDataArrayObject
{
  typedef BaseBroadPhaseData<ClientDataType> DataType;

  DataType mData;
  bool mValid;
};

template <typename ClientDataType>
struct BaseBroadPhaseObject
{
  typedef BaseBroadPhaseData<ClientDataType> DataType;
  BaseBroadPhaseObject() {};

  BaseBroadPhaseObject(BroadPhaseProxy* proxy, DataType& data)
    : mProxy(proxy), mData(data)
  {

  }

  ///We need to be able to change the proxy that our client owns
  //if we move change it so we store a pointer.
  BroadPhaseProxy* mProxy;
  DataType mData;
};

///Used for storing a pair of BroadPhaseData in a HashMap.
template <typename ClientDataType>
struct BaseBroadPhaseDataPair
{
  typedef BaseBroadPhaseData<ClientDataType> DataType;
  BaseBroadPhaseDataPair() {}
  BaseBroadPhaseDataPair(DataType& data1, DataType& data2)
  {
    mData[0] = data1;
    mData[1] = data2;
  }

  DataType mData[2];
};

///Stores a pair of client data types for a BroadPhase to return. This is
///the generic versions that a BroadPhase will use internally. Anything
///that is at the IBroadPhase level will use void* for the client data type.
template <typename ClientDataType>
struct BaseClientPair
{
  typedef BaseBroadPhaseData<ClientDataType> DataType;

  BaseClientPair()
  {
    GetDefaultClientDataValue(mClientData[0]);
    GetDefaultClientDataValue(mClientData[1]);
  }

  BaseClientPair(DataType& data1, DataType& data2)
  {
    mClientData[0] = data1.mClientData;
    mClientData[1] = data2.mClientData;
  }

  BaseClientPair(ClientDataType& data1, ClientDataType& data2)
  {
    mClientData[0] = data1;
    mClientData[1] = data2;
  }

  ClientDataType mClientData[2];
};

///A structure to store and hash a pair of pointers. Used in a BroadPhase
///that needs to prevent duplicates and uses something similar to nodes
///for hashing.
struct NodePointerPair
{
  NodePointerPair() {}
  NodePointerPair(void* node1, void* node2)
  {
    if(node1 < node2)
    {
      mNodes[0] = node1;
      mNodes[1] = node2;
    }
    else
    {
      mNodes[1] = node1;
      mNodes[0] = node2;
    }
  }

  ///Helper to convert the internal type to the desired node pointer type.
  template <typename NodeType>
  void Convert(NodeType*& node1, NodeType*& node2)
  {
    node1 = static_cast<NodeType*>(mNodes[0]);
    node2 = static_cast<NodeType*>(mNodes[1]);
  }

  ///Attempt to hash this pair by hashing each void* and then xoring them.
  size_t Hash() const
  {
    HashPolicy<void*> policy;
    size_t hash1 = policy(mNodes[0]);
    size_t hash2 = policy(mNodes[1]);
    return hash1 ^ hash2;
  }

  bool operator==(const NodePointerPair& rhs) const
  {
    return mNodes[0] == rhs.mNodes[0] && mNodes[1] == rhs.mNodes[1];
  }

  void* mNodes[2];
};

typedef BaseBroadPhaseData<void*> BroadPhaseData;
typedef BaseBroadPhaseObject<void*> BroadPhaseObject;
typedef BaseClientPair<void*> ClientPair;
typedef BaseBroadPhaseDataPair<void*> BroadPhaseDataPair;

typedef Array<BroadPhaseData> BroadPhaseDataArray;
typedef Array<BroadPhaseProxy*> ProxyHandleArray;
typedef Array<BroadPhaseObject> BroadPhaseObjectArray;
typedef Array<ClientPair> ClientPairArray;
typedef Array<BroadPhaseDataPair> DataPairArray;

}//namespace Zero
