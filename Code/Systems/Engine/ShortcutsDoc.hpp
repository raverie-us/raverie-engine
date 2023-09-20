// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class ShortcutEntry
{
public:
  void Serialize(Serializer& stream);

  uint mIndex;
  String mName;
  String mShortcut;
  String mDescription;
};

typedef Array<ShortcutEntry> ShortcutSet;

class ShortcutSetEntry
{
public:
  void Serialize(Serializer& stream);

  static const ShortcutSetEntry cZero;

  String mName;
  ShortcutSet mShortcutSet;
};

class Shortcuts : public LazySingleton<Shortcuts, EventObject>
{
public:
  RaverieDeclareType(Shortcuts, TypeCopyMode::ReferenceType);

  ~Shortcuts();

  void Serialize(Serializer& stream);
  void Load(StringParam filename);

  const ShortcutSet* FindSet(StringParam className);

public:
  HashMap<String, ShortcutSetEntry*> mShorcutSets;
};

namespace Z
{
extern Shortcuts* gShortcutsDoc;
}

class ShortcutSource : public DataSource
{
public:
  ShortcutSource();

  DataEntry* GetRoot() override;

  DataEntry* ToEntry(DataIndex index) override;
  DataIndex ToIndex(DataEntry* dataEntry) override;

  DataEntry* Parent(DataEntry* dataEntry) override;
  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override;

  uint ChildCount(DataEntry* dataEntry) override;

  bool IsExpandable(DataEntry* dataEntry) override;

  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override;
  bool SetData(DataEntry* dataEntry, AnyParam variant, StringParam column) override;

public:
  ShortcutSet mSet;
};

} // namespace Raverie
