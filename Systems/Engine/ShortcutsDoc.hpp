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
  ZilchDeclareType(TypeCopyMode::ReferenceType);

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

}  // namespace Zero