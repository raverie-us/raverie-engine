////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Ryan Edgemon
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{


//--------------------------------------------------------------- Shortcut Entry
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

//----------------------------------------------------------- Shortcut Set Entry
class ShortcutSetEntry
{
public:
  void Serialize(Serializer& stream);

  static const ShortcutSetEntry cZero;

  String mName;
  ShortcutSet mShortcutSet;
};

//-------------------------------------------------------------------- Shortcuts
class Shortcuts : public LazySingleton<Shortcuts, EventObject>
{
public:
  ZilchDeclareType(Shortcuts, TypeCopyMode::ReferenceType);

  ~Shortcuts( );

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

//------------------------------------------------------------- ShortcutSource
class ShortcutSource : public DataSource
{
public:
  ShortcutSource( );

  DataEntry* GetRoot( ) override;

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

}  // namespace Zero