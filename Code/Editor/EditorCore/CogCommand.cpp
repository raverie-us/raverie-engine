// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// The attribute parameter name
const String cAutoRegisterParameterName = "autoRegister";

ZilchDefineType(CogCommand, builder, type)
{
}

CogCommand::CogCommand(Archetype* archetype) : mArchetype(archetype), mScriptComponentType(nullptr)
{
  SetDisplayName(archetype->Name);
  Name = archetype->Name;
}

CogCommand::CogCommand(BoundType* componentType) : mScriptComponentType(componentType)
{
  SetDisplayName(componentType->Name);
  Name = componentType->Name;

  Description = componentType->Description;

  MetaScriptTagAttribute* tagAttribute = componentType->HasInherited<MetaScriptTagAttribute>();

  if (tagAttribute == nullptr)
    return;

  Tags = tagAttribute->mTags;

  MetaScriptShortcutAttribute* shortcut = componentType->HasInherited<MetaScriptShortcutAttribute>();

  if (shortcut == nullptr)
    return;

  CommandManager* commandManager = CommandManager::GetInstance();
  Shortcut = commandManager->BuildShortcutString(shortcut->mCtrl, shortcut->mAlt, shortcut->mShift, shortcut->mKey);
}

void CogCommand::Execute()
{
  if (Cog* cog = mCog)
  {
    CommandManager* commandManager = CommandManager::GetInstance();

    CommandEvent eventToSend(this, commandManager);
    cog->DispatchEvent(Events::CommandExecute, &eventToSend);
  }
}

ZilchDefineType(CogCommandManager, builder, type)
{
}

CogCommandManager::CogCommandManager() : EditorScriptObjects<CogCommand>(ObjectAttributes::cCommand)
{
  mCommands = CommandManager::GetInstance();
}

void CogCommandManager::AddObject(CogCommand* object)
{
  mCommands->AddCommand(object);
}

void CogCommandManager::RemoveObject(CogCommand* object)
{
  mCommands->RemoveCommand(object);
}

CogCommand* CogCommandManager::GetObject(StringParam objectName)
{
  Command* command = mCommands->GetCommand(objectName);
  return Type::DynamicCast<CogCommand*>(command);
}

uint CogCommandManager::GetObjectCount()
{
  return mCommands->mCommands.Size();
}

CogCommand* CogCommandManager::GetObject(uint index)
{
  Command* command = mCommands->mCommands[index];
  return Type::DynamicCast<CogCommand*>(command);
}

CogCommand* CogCommandManager::UpdateData(StringParam objectName)
{
  CogCommand* command = (CogCommand*)mCommands->GetCommand(objectName);

  if (command == nullptr)
    return command;

  BoundType* componentType = MetaDatabase::GetInstance()->FindType(objectName);
  command->Description = componentType->Description;

  bool commandModified = false;

  // Update command tags, if possible.
  MetaScriptTagAttribute* tagAttribute = componentType->HasInherited<MetaScriptTagAttribute>();
  if (tagAttribute != nullptr)
    commandModified |= mCommands->UpdateCommandTags(command, tagAttribute->mTags);
  else
    commandModified |= mCommands->UpdateCommandTags(command, ""); // Tag attribute removed.

  // Update command shortcut, if possible.
  MetaScriptShortcutAttribute* sc = componentType->HasInherited<MetaScriptShortcutAttribute>();
  if (sc != nullptr)
    commandModified |= mCommands->UpdateCommandShortcut(command, sc->mCtrl, sc->mAlt, sc->mShift, sc->mKey);
  else
    commandModified |= mCommands->ClearCommandShortcut(command);

  if (commandModified)
  {
    CommandUpdateEvent eventToSend(command);
    mCommands->DispatchEvent(Events::CommandUpdated, &eventToSend);
  }

  return command;
}

Space* CogCommandManager::GetSpace(CogCommand*)
{
  return GetSpace();
}

Space* CogCommandManager::GetSpace()
{
  if (mCommandSpace.IsNull())
  {
    if (Z::gEditor->mEditGame != nullptr)
    {
      GameSession* gameSession = Z::gEditor->GetEditGameSession();

      Archetype* spaceArchetype = ArchetypeManager::Find(CoreArchetypes::DefaultSpace);
      mCommandSpace = gameSession->CreateSpace(spaceArchetype);
    }
    else
    {
      mCommandSpace = Z::gFactory->CreateSpace(CoreArchetypes::DefaultSpace, CreationFlags::Default, nullptr);
    }
  }

  return mCommandSpace;
}

} // namespace Zero
