////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Ryan Edgemon
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"


namespace Zero
{


//--------------------------------------------------------------- Shortcut Entry
/******************************************************************************/
void ShortcutEntry::Serialize(Serializer& stream)
{
  SerializeName(mIndex);
  SerializeName(mName);
  SerializeName(mShortcut);
  SerializeName(mDescription);
}

//----------------------------------------------------------- Shortcut Set Entry
const ShortcutSetEntry ShortcutSetEntry::cZero;

/******************************************************************************/
void ShortcutSetEntry::Serialize(Serializer& stream)
{
  SerializeName(mName);
  SerializeName(mShortcutSet);
}

//-------------------------------------------------------------------- Shortcuts
ZilchDefineType(Shortcuts, builder, type)
{
}

/******************************************************************************/
Shortcuts::~Shortcuts()
{
}

/******************************************************************************/
void Shortcuts::Serialize(Serializer& stream)
{
  PolymorphicNode node;
  while(stream.GetPolymorphic(node))
  {
    ShortcutSetEntry* entry = new ShortcutSetEntry();
    entry->Serialize(stream);

    stream.EndPolymorphic();

    mShorcutSets.Insert(entry->mName, entry);
  }

}

/******************************************************************************/
void Shortcuts::Load(StringParam filename)
{
  // will throw if unable to load file
  LoadFromDataFile(*this, filename);

  // save the global documentation
  Z::gShortcutsDoc = this;
}

/******************************************************************************/
const ShortcutSet* Shortcuts::FindSet(StringParam className)
{
  ShortcutSetEntry* set = mShorcutSets.FindValue(className, nullptr);

  if(set == nullptr)
    return nullptr;
  else
    return &set->mShortcutSet;
}

namespace Z
{
  Shortcuts* gShortcutsDoc = nullptr;
}


}  // namespace Zero