// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Declarations
class CogInitializer;
class CogCreationContext;

const u32 cInvalidObjectRawId = 0xFFFFFFFD;
const u32 cDeletedObjectRawId = 0xFFFFDEAD;

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
  bool operator<(const CogId& rhs) const
  {
    return Id < rhs.Id || (Id == rhs.Id && Slot < rhs.Slot);
  }
  bool operator==(const CogId& rhs) const
  {
    return Id == rhs.Id && Slot == rhs.Slot;
  }
  bool operator!=(const CogId& rhs) const
  {
    return Id != rhs.Id || Slot != rhs.Slot;
  }

  /// Convert id to cog may return null
  Cog* ToCog() const;

  /// Is this referring to an valid/alive object?
  bool IsValid();

  /// Destroy the object if it is valid
  void SafeDestroy();

  /// Utility Functions
  operator Cog*() const
  {
    return ToCog();
  }
  u64 ToUint64() const;
  size_t Hash() const;

  /// Look up component
  template <typename type>
  type* Has();

  /// Look up component by type id
  Component* QueryComponentId(BoundType* typeId);

  /// Use to Restore links in serialization
  Cog* OnAllObjectsCreated(CogInitializer& initializer);

  u32& GetId()
  {
    return Id;
  }
  u32& GetSlot()
  {
    return Slot;
  }
};

template <typename type>
inline type* CogId::Has()
{
  return (type*)QueryComponentId(RaverieTypeId(type));
}

namespace Serialization
{
template <>
struct Policy<CogId>
{
  static bool Serialize(Serializer& stream, cstr fieldName, CogId& value);
};
} // namespace Serialization

const CogId cInvalidCogId = CogId(cInvalidObjectRawId, cInvalidObjectRawId);

Cog* RestoreLink(CogId* id,
                 CogCreationContext* context,
                 Component* component = NULL,
                 StringParam propertyName = String(),
                 bool isError = true);

} // namespace Raverie
