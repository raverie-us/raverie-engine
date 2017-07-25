///////////////////////////////////////////////////////////////////////////////
///
/// \file CogId.hpp
/// Declaration of CogId class.
///
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------ Forward Declarations
class CogInitializer;
class CogCreationContext;

//--------------------------------------------------------- Cog Id Handle System
const u32 cInvalidObjectRawId = 0xFFFFFFFD;
const u32 cDeletedObjectRawId = 0xFFFFDEAD;

//----------------------------------------------------------------------- CogId

/// Cog Id is a handle / weak reference to a Cog
struct CogId
{
public:
  /// Constructors
  CogId();
  CogId(u32 id, u32 slot);
  CogId(u64 idAndSlot);
  CogId(Cog* object);

  /// Id of the game object
  u32 Id;
  /// Slot for the handle map.
  u32 Slot;

  /// Comparison
  bool operator<(const CogId& rhs) const {return Id < rhs.Id || (Id == rhs.Id && Slot < rhs.Slot); }
  bool operator==(const CogId& rhs) const { return Id == rhs.Id && Slot == rhs.Slot; }
  bool operator!=(const CogId& rhs) const { return Id != rhs.Id || Slot != rhs.Slot; }

  /// Convert id to cog may return null
  Cog* ToCog() const;

  /// Is this referring to an valid/alive object?
  bool IsValid();

  /// Destroy the object if it is valid
  void SafeDestroy();

  /// Utility Functions
  operator Cog*() const{return ToCog();}
  u64 ToUint64() const;
  u32 Hash() const;

  /// Look up component
  template<typename type>
  type* Has();

  /// Look up component by type id
  Component* QueryComponentId(BoundType* typeId);

  /// Use to Restore links in serialization
  Cog* OnAllObjectsCreated(CogInitializer& initializer);

  u32& GetId(){ return Id; }
  u32& GetSlot(){ return Slot; }
};

template<typename type>
inline type* CogId::Has()
{
  return (type*)QueryComponentId(ZilchTypeId(type));
}

namespace Serialization
{
  template<>
  struct Policy<CogId>
  {
    static bool Serialize(Serializer& stream, cstr fieldName, CogId& value);
  };
}

const CogId cInvalidCogId = CogId(cInvalidObjectRawId, cInvalidObjectRawId);

Cog* RestoreLink(CogId* id, CogCreationContext* context, Component* component = NULL,
                 StringParam propertyName = String(), bool isError = true);

}
