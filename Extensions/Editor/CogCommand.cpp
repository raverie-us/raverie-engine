///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// The attribute parameter name
const String cAutoRegisterParameterName = "autoRegister";

//---------------------------------------------------------------------- Command
ZilchDefineType(CogCommand, builder, type)
{
}

//******************************************************************************
CogCommand::CogCommand(Archetype* archetype) :
  mArchetype(archetype),
  mScriptComponentType(nullptr)
{
  DisplayName = archetype->Name;
  Name = archetype->Name;
}

//******************************************************************************
CogCommand::CogCommand(BoundType* componentType) :
  mScriptComponentType(componentType)
{
  DisplayName = componentType->Name;
  Name = componentType->Name;
}

//******************************************************************************
void CogCommand::Execute()
{
  if(Cog* cog = mCog)
  {
    CommandManager* commandManager = CommandManager::GetInstance();

    CommandEvent eventToSend(this, commandManager);
    cog->DispatchEvent(Events::CommandExecute, &eventToSend);
  }
}

//---------------------------------------------------------- Cog Command Manager
ZilchDefineType(CogCommandManager, builder, type)
{
}

//******************************************************************************
CogCommandManager::CogCommandManager() :
  EditorScriptObjects<CogCommand>(Tags::Command)
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

Space* CogCommandManager::GetSpace(CogCommand*)
{
  return GetSpace();
}

Space* CogCommandManager::GetSpace()
{
  if(mCommandSpace.IsNull())
  {
    GameSession* gameSession = Z::gEditor->GetEditGameSession();

    if(gameSession)
    {
      Archetype* spaceArchetype = ArchetypeManager::Find(CoreArchetypes::DefaultSpace);
      mCommandSpace = gameSession->CreateSpace(spaceArchetype);
    }
    else
    {
      mCommandSpace = Z::gFactory->CreateSpace(CoreArchetypes::DefaultSpace,
        CreationFlags::Default, nullptr);
    }
  }

  return mCommandSpace;
}

}//namespace Zero
