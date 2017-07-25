///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "Containers/ArrayMap.hpp"
#include "Utility/IdStore.hpp"

namespace Zero
{

/// Maps unique IDs to unique items (where 0 is an invalid ID)
/// Provides quick look up for a given ID or item
/// Can be operated bi-directionally, but primarily built to be used in a single direction
/// (either outgoing mapping from item to ID, or incoming mapping from ID to item)
/// Designed for use with UintN IDs
template <typename Item, typename Id>
class ItemCache
{
  /// Typedefs
  typedef typename IdStore<Id>        IdStore;
  typedef typename ArrayMap<Item, Id> ItemIdMap;
  typedef typename ArrayMap<Id, Item> IdItemMap;

public:
  /// Typedefs
  typedef typename Id                 id_type;
  typedef typename Item               item_type;
  typedef typename IdStore            id_store_type;
  typedef typename ItemIdMap          item_id_map_type;
  typedef typename IdItemMap          id_item_map_type;
  typedef typename ItemIdMap::pointer item_id_pointer;
  typedef typename IdItemMap::pointer id_item_pointer;
  typedef typename ItemIdMap::range   item_id_range;
  typedef typename IdItemMap::range   id_item_range;

  /// Constructor
  ItemCache()
    : mIdStore(),
      mItemIdMap(),
      mIdItemMap()
  {
  }
  /// Copy Constructor
  ItemCache(const ItemCache& rhs)
    : mIdStore(rhs.mIdStore),
      mItemIdMap(rhs.mItemIdMap),
      mIdItemMap(rhs.mIdItemMap)
  {
  }
  /// Move Constructor
  ItemCache(MoveReference<ItemCache> rhs)
    : mIdStore(ZeroMove(rhs->mIdStore)),
      mItemIdMap(ZeroMove(rhs->mItemIdMap)),
      mIdItemMap(ZeroMove(rhs->mIdItemMap))
  {
  }

  /// Destructor
  ~ItemCache()
  {
    Reset();
  }

  /// Copy Assignment Operator
  ItemCache& operator =(const ItemCache& rhs)
  {
    Reset();

    mIdStore   = rhs.mIdStore;
    mItemIdMap = rhs.mItemIdMap;
    mIdItemMap = rhs.mIdItemMap;

    return *this;
  }
  /// Move Assignment Operator
  ItemCache& operator =(MoveReference<ItemCache> rhs)
  {
    Reset();

    mIdStore   = ZeroMove(rhs->mIdStore);
    mItemIdMap = ZeroMove(rhs->mItemIdMap);
    mIdItemMap = ZeroMove(rhs->mIdItemMap);

    return *this;
  }

  /// Resets the item cache to it's default state
  void Reset()
  {
    mIdStore.Reset();
    mItemIdMap.Clear();
    mIdItemMap.Clear();
  }

  /// Returns the ID store
  const IdStore& GetIdStore() const
  {
    return mIdStore;
  }

  /// Returns the map of items to IDs
  const ItemIdMap& GetItemIdMap() const
  {
    return mItemIdMap;
  }

  /// Returns the map of IDs to items
  const IdItemMap& GetIdItemMap() const
  {
    return mIdItemMap;
  }

  //
  // Outgoing Operations
  //

  /// Maps a unique item to a generated ID
  /// Returns a pointer to the item-id-pair if successful, else nullptr
  item_id_pointer MapItem(const Item& item)
  {
    // Get next ID (don't actually acquire it yet)
    Id id = mIdStore.GetNextId();
    if(id == 0) // Unable?
    {
      Error("Unable to map item - There are no free IDs available");
      return nullptr;
    }

    // Map unique item to ID
    ItemIdMap::pointer_bool_pair result = mItemIdMap.Insert(item, id);
    if(result.second) // Insertion succeeded?
    {
      // Actually acquire the ID we used
      Id acquiredId = mIdStore.AcquireId();
      Assert(acquiredId == id); // (Acquired ID should match the ID we used)
    }
    else // Insertion failed?
      return nullptr;

    // Success
    return result.first;
  }
  /// Maps a unique item to a generated ID or returns the existing mapping (if the mapping already exists)
  /// Returns a pointer to the item-id-pair if successful, else nullptr
  item_id_pointer GetOrMapItem(const Item& item)
  {
    // Get next ID (don't actually acquire it yet)
    Id id = mIdStore.GetNextId();
    if(id == 0) // Unable?
    {
      Error("Unable to map item - There are no free IDs available");
      return nullptr;
    }

    // Map unique item to ID or get current mapping
    ItemIdMap::pointer_bool_pair result = mItemIdMap.FindOrInsert(item, id);
    if(result.second) // Insertion succeeded?
    {
      // Actually acquire the ID we used
      Id acquiredId = mIdStore.AcquireId();
      Assert(acquiredId == id); // (Acquired ID should match the ID we used)
    }

    // Success
    return result.first;
  }

  /// Returns true if the item is mapped to an ID, else false
  bool IsItemMapped(const Item& item) const
  {
    return mItemIdMap.Contains(item);
  }

  /// Returns the ID the item is mapped to, else 0 (invalid ID)
  Id GetMappedItemId(const Item& item) const
  {
    return mItemIdMap.FindValue(item, 0);
  }

  //
  // Incoming Operations
  //

  /// Maps a unique ID to a given item
  /// Returns true if successful, else false
  bool MapId(Id id, const Item& item)
  {
    // Map unique ID to item
    IdItemMap::pointer_bool_pair result = mIdItemMap.Insert(id, item);
    if(!result.second) // Unable?
      return false;

    // Success
    return true;
  }
  /// Maps a unique ID to a given item, overwriting the equivalent entry if one already exists
  /// Returns true if insertion took place, else false if assignment took place
  bool MapIdOverwrite(Id id, const Item& item)
  {
    // Map unique ID to item
    IdItemMap::pointer_bool_pair result = mIdItemMap.InsertOrAssign(id, item);

    // Success
    return result.second;
  }

  /// Returns true if the ID is mapped to an item, else false
  bool IsIdMapped(Id id) const
  {
    return mIdItemMap.Contains(id);
  }

  /// Returns the item the ID is mapped to, else error
  const Item& GetMappedIdItem(Id id) const
  {
    // Get item
    const Item* result = GetMappedIdItemPointer(id);
    if(!result) // Unable?
      Error("Unable to get mapped item by ID - Mapped item was not found");

    // Success
    return *result;
  }
  /// Returns the item the ID is mapped to, else itemIfNotFound
  const Item& GetMappedIdItemValue(Id id, const Item& itemIfNotFound) const
  {
    return mIdItemMap.FindValue(id, itemIfNotFound);
  }
  /// Returns the item the ID is mapped to, else nullptr
  const Item* GetMappedIdItemPointer(Id id) const
  {
    return mIdItemMap.FindPointer(id);
  }

private:
  /// Data
  IdStore   mIdStore;   /// [Outgoing] Id store, used to acquire/release IDs
  ItemIdMap mItemIdMap; /// [Outgoing] Map from unique item to ID
  IdItemMap mIdItemMap; /// [Incoming] Map from unique ID to item
};

/// ItemCache Move-Without-Destruction Operator
template <typename Item, typename Id>
struct MoveWithoutDestructionOperator< ItemCache<Item, Id> >
{
  static inline void MoveWithoutDestruction(ItemCache<Item, Id>* dest, ItemCache<Item, Id>* source)
  {
    new(dest) ItemCache<Item, Id>(ZeroMove(*source));
  }
};

} // namespace Zero
