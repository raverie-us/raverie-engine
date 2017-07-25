///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Ryan Edgemon
/// Copyright 2015-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once


namespace Zero
{

class Command;


//class CommandEntry
//{
//public:
//  String mCommand;
//  Array<Keys::Enum> mBinding;
//
//  Link<CommandEntry> link;
//};

//typedef Link< Pair< String, Array<unsigned> > > CommandListElement;
//typedef InList<CommandEntry> CommandList;

struct CommandEntry
{
  bool mDevOnly;

  unsigned mIndex;

  String mName;
  String mDescription;

  String mIconName;
  String mFunction;

  String mTags;

  String mBindingStr;
  unsigned mModifier1;
  unsigned mModifier2;
  unsigned mMainKey;
  

  void Serialize(Serializer& stream)
  {
    SerializeName(mName);
    SerializeNameDefault(mDescription, String());
    SerializeNameDefault(mIconName, String());
    //SerializeNameDefault(mShortcut, String());  BindingStr
    SerializeNameDefault(mFunction, String());

    SerializeNameDefault(mTags, String());
    SerializeNameDefault(mDevOnly, false);

    SerializeNameDefault(mBindingStr, String());

    SerializeNameDefault(mModifier1, (unsigned)Keys::Unknown);
    SerializeNameDefault(mModifier2, (unsigned)Keys::Unknown);
    SerializeNameDefault(mMainKey, (unsigned)Keys::Unknown);
  }

  bool operator<(const CommandEntry& rhs) const
  {
    return mName < rhs.mName;
  }

};

typedef Array<CommandEntry> CommandSet;

//------------------------------------------------------------ HotKeyDataSet ---
class HotKeyDataSet : public DataResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  HotKeyDataSet( );
  void Serialize(Serializer& stream);

public:
  CommandSet mCommand;
};

//------------------------------------------------------------ HotKeyManager ---
class HotKeyManager : public ResourceManager
{
public:
  DeclareResourceManager(HotKeyManager, HotKeyDataSet);

  HotKeyManager(BoundType* resourceType);

  HotKeyDataSet* CreateNewResourceInternal(StringParam name) override;
  //HotKeyDataSet* GetResource(StringParam name, ResourceNotFound::Enum notFound);

  void RegisterSystemCommands(Array<Command*> & coreCommands);

public:

};

const String cDefaultCommandString = "\'NEW COMMAND\'";
const String cDefaultBindingString = "\'NEW BINDING\'";

}//namespace Zero
