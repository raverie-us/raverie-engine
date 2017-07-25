///////////////////////////////////////////////////////////////////////////////
///
/// \file DocumentationCommands.cpp
///
/// Authors: Joshua Shlemmer
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
  /// This exists just in case you want to get all unbound meta types with the documentation tool
  void SaveNonZilchMetaDocumentation()
  {
    // obviously, this command is only meant to be ran by developers since this saves in source directory
    String sourceDir = Z::gEngine->GetConfigCog()->has(MainConfig)->SourceDirectory;
    String documentationLocation = FilePath::Combine(sourceDir.All(), "Projects", "Editor");

    if (!FileExists(documentationLocation))
      CreateDirectoryAndParents(documentationLocation);

    String fileName = FilePath::Combine(documentationLocation.All(), "UnboundMetaTypesDocumentationSkeleton.data");
    SaveInfoFromMetaToFile(fileName, true);
    Z::gEngine->Terminate();
  }
  /// Saves the list of all non dev-only commands to file
  void SaveListOfCommandsToDataFile()
  {
    String sourceDir = Z::gEngine->GetConfigCog()->has(MainConfig)->SourceDirectory;
    String documentationLocation = FilePath::Combine(sourceDir.All(), "Projects", "Editor");


    if (!FileExists(documentationLocation))
      CreateDirectoryAndParents(documentationLocation);

    String fileName = FilePath::Combine(documentationLocation.All(), "CommandList.data");

    Array<Command*>& commandList = CommandManager::GetInstance()->mCommands;

    CommandDocList commandDocs;

    Array<CommandDoc*>& commandListDoc = commandDocs.mCommands;

    for (uint i = 0; i < commandList.Size(); ++i)
    {
      Command* currCommand = commandList[i];

      if (currCommand->DevOnly)
        continue;

      CommandDoc* currDoc = new CommandDoc();
      commandListDoc.PushBack(currDoc);

      currDoc->mName = currCommand->Name;
      currDoc->mDescription = currCommand->Description;
      currDoc->mShortcut = currCommand->Shortcut;

      forRange(StringParam tag, currCommand->TagList.All())
      {
        currDoc->mTags.PushBack(tag);
      }
    }
    commandDocs.Sort();

    SaveToDataFile(commandDocs, fileName);
  }

  void SaveListOfCommands()
  {
    SaveListOfCommandsToDataFile();

    Z::gEngine->Terminate();
  }

  /// saves list of all events in the MetaDataBase to file
  void SaveEventListToDataFile()
  {
    String sourceDir = Z::gEngine->GetConfigCog()->has(MainConfig)->SourceDirectory;
    String documentationLocation = FilePath::Combine(sourceDir.All(), "Projects", "Editor");


    if (!FileExists(documentationLocation))
      CreateDirectoryAndParents(documentationLocation);

    String fileName = FilePath::Combine(documentationLocation.All(), "EventList.data");

    MetaDatabase::StringToTypeMap::range eventRange = MetaDatabase::GetInstance()->mEventMap.All();
    EventDocList allTheEvents;
    while (!eventRange.Empty())
    {
      String eventName = eventRange.Front().first;
      BoundType* eventType = eventRange.Front().second;
      allTheEvents.mEvents.PushBack(new EventDoc(eventType->Name, eventName));
      eventRange.PopFront();
    }

    allTheEvents.Sort();

    SaveToDataFile(allTheEvents, fileName);
  }

  void SaveEventList()
  {
    SaveEventListToDataFile();

    Z::gEngine->Terminate();
  }

  /// saves all bound classes(with methods + properties) to file, then does same for commands and events
  void SaveDocumentation()
  {
    String sourceDir = Z::gEngine->GetConfigCog()->has(MainConfig)->SourceDirectory;
    String documentationLocation = FilePath::Combine(sourceDir.All(), "Projects", "Editor");

    if (!FileExists(documentationLocation))
      CreateDirectoryAndParents(documentationLocation);

    String fileName = FilePath::Combine(documentationLocation.All(), "DocumentationSkeleton.data");
    SaveInfoFromMetaToFile(fileName, false);

    SaveListOfCommandsToDataFile();

    SaveEventListToDataFile();

    Z::gEngine->Terminate();
  }

  /// binds all of the documentation commands as dev only as they all require source directory
  void BindDocumentationCommands(Cog* config, CommandManager* commands)
  {
    if (Z::gEngine->GetConfigCog()->has(Zero::DeveloperConfig))
    {
      commands->AddCommand("SaveDocumentation", BindCommandFunction(SaveDocumentation))
        ->DevOnly = true;
      commands->AddCommand("SaveNonZilchDocumentation", BindCommandFunction(SaveNonZilchMetaDocumentation))
        ->DevOnly = true;
      commands->AddCommand("SaveEventList", BindCommandFunction(SaveEventList))
        ->DevOnly = true;
      commands->AddCommand("SaveCommandList", BindCommandFunction(SaveListOfCommands))
        ->DevOnly = true;
    }
  }

}

