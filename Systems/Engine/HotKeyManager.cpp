///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Ryan Edgemon
/// Copyright 2015-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------ HotKeyDataSet ---
ZilchDefineType(HotKeyDataSet, builder, type)
{
  ZeroBindSetup(SetupMode::DefaultSerialization);
}

/******************************************************************************/
HotKeyDataSet::HotKeyDataSet()
{
}

/******************************************************************************/
void HotKeyDataSet::Serialize(Serializer& stream)
{
  if(stream.GetMode() == SerializerMode::Saving)
  {
    CommandSet::range culledRange = mCommand.All();
    while(culledRange[0].mName == cDefaultCommandString || culledRange[0].mBindingStr == cDefaultBindingString)
      culledRange.PopFront();

    CommandSet culledSet;
    forRange(CommandEntry& command, culledRange)
    {
      culledSet.PushBack(command);
    }

    Sort(culledSet.All());

    int size = (int)culledSet.Size();
    for(int i = 0; i < size; ++i)
      culledSet[i].mIndex = i;

    stream.SerializeField("mCommand", culledSet);
  }
  else if(stream.GetMode() == SerializerMode::Loading)
  {
    SerializeNameDefault(mCommand, CommandSet());

    Sort(mCommand.All());

    int size = (int)mCommand.Size();
    for(int i = 0; i < size; ++i)
      mCommand[i].mIndex = i;
  }

}

//------------------------------------------------------------ HotKeyManager ---

ImplementResourceManager(HotKeyManager, HotKeyDataSet);

/******************************************************************************/
HotKeyManager::HotKeyManager(BoundType* resourceType) : ResourceManager(resourceType)
{
  AddLoader("HotKeyDataSet", new TextDataFileLoader<HotKeyManager>());
  DefaultResourceName = "Core.HotKeys";
  //mCanReload = true;
  mCanCreateNew = false;  // for now, no ... blocked by new UI and editor content being unfinished
  mExtension = DataResourceExtension;
}

/******************************************************************************/
HotKeyDataSet* HotKeyManager::CreateNewResourceInternal(StringParam name)
{
  HotKeyDataSet* hotKeys;

  hotKeys = (HotKeyDataSet*)GetResource(name, ResourceNotFound::ReturnNull);

  if(hotKeys == NULL)
    hotKeys = new HotKeyDataSet();

  //hotKeys->SetDefaults();
  return hotKeys;
}

/******************************************************************************/
//HotKeyDataSet* HotKeyManager::GetResource(StringParam name, ResourceNotFound::Enum notFound)
//{
//  return ResourceManager::GetResource(name);
//}

/******************************************************************************/
void HotKeyManager::RegisterSystemCommands(Array<Command*> & coreCommands)
{
  forRange(Command *command, coreCommands.All())
  {
    //command->TagList;
  }

}

}//namespace Zero