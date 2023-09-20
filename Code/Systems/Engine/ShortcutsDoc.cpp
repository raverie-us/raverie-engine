// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

void ShortcutEntry::Serialize(Serializer& stream)
{
  SerializeName(mIndex);
  SerializeName(mName);
  SerializeName(mShortcut);
  SerializeName(mDescription);
}

const ShortcutSetEntry ShortcutSetEntry::cZero;

void ShortcutSetEntry::Serialize(Serializer& stream)
{
  SerializeName(mName);
  SerializeName(mShortcutSet);
}

RaverieDefineType(Shortcuts, builder, type)
{
}

Shortcuts::~Shortcuts()
{
  DeleteObjectsInContainer(mShorcutSets);
}

void Shortcuts::Serialize(Serializer& stream)
{
  PolymorphicNode node;
  while (stream.GetPolymorphic(node))
  {
    ShortcutSetEntry* entry = new ShortcutSetEntry();
    entry->Serialize(stream);

    stream.EndPolymorphic();

    mShorcutSets.Insert(entry->mName, entry);
  }
}

void Shortcuts::Load(StringParam filename)
{
  // will throw if unable to load file
  LoadFromDataFile(*this, filename);

  // save the global documentation
  Z::gShortcutsDoc = this;
}

const ShortcutSet* Shortcuts::FindSet(StringParam className)
{
  ShortcutSetEntry* set = mShorcutSets.FindValue(className, nullptr);

  if (set == nullptr)
    return nullptr;
  else
    return &set->mShortcutSet;
}

namespace Z
{
Shortcuts* gShortcutsDoc = nullptr;
}

#define IF_ROOT(r)                                                                                                     \
  DataEntry* root = &mSet;                                                                                             \
  if (dataEntry == root)                                                                                               \
    return r;

ShortcutSource::ShortcutSource()
{
}

DataEntry* ShortcutSource::GetRoot()
{
  return &mSet;
}

DataEntry* ShortcutSource::ToEntry(DataIndex index)
{
  DataEntry* root = &mSet;
  if (((DataEntry*)index.Id) == root)
    return ((DataEntry*)index.Id);
  else if (index.Id >= mSet.Size())
    return NULL;

  return &mSet[(uint)index.Id];
}

DataIndex ShortcutSource::ToIndex(DataEntry* dataEntry)
{
  IF_ROOT((DataIndex)((u64)dataEntry));

  return DataIndex(((ShortcutEntry*)dataEntry)->mIndex);
}

DataEntry* ShortcutSource::Parent(DataEntry* dataEntry)
{
  // Everyone but the root has the parent of the root.
  IF_ROOT(NULL);

  return root;
}

DataEntry* ShortcutSource::GetChild(DataEntry* dataEntry, uint index, DataEntry* prev)
{
  if (index < mSet.Size())
    return &mSet[index];

  IF_ROOT(&mSet.Front());

  return NULL;
}

uint ShortcutSource::ChildCount(DataEntry* dataEntry)
{
  // Only the root has children, no one else does.
  IF_ROOT(mSet.Size());

  return 0;
}

bool ShortcutSource::IsExpandable(DataEntry* dataEntry)
{
  // Only the root is expandable.
  IF_ROOT(true);

  return false;
}

void ShortcutSource::GetData(DataEntry* dataEntry, Any& variant, StringParam column)
{
  ShortcutEntry* entry = ((ShortcutEntry*)dataEntry);

  if (column == "Name")
    variant = entry->mName;
  else if (column == "Shortcut")
    variant = entry->mShortcut;
  else if (column == "Description")
    variant = entry->mDescription;
}

bool ShortcutSource::SetData(DataEntry* dataEntry, AnyParam variant, StringParam column)
{
  return false;
}

} // namespace Raverie
