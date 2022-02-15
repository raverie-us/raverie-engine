// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
String GetDocumentationLocation()
{
  String sourceDir = Z::gEngine->GetConfigCog()->has(MainConfig)->SourceDirectory;
  String documentationLocation = FilePath::Combine(sourceDir.All(), "BuildOutput", "Documentation");

  CreateDirectoryAndParents(documentationLocation);
  return documentationLocation;
}

/// This exists just in case you want to get all unbound meta types with the
/// documentation tool
void SaveNonZilchMetaDocumentation()
{
  // obviously, this command is only meant to be ran by developers since this
  // saves in source directory
  String documentationLocation = GetDocumentationLocation();

  String fileName = FilePath::Combine(documentationLocation.All(), "UnboundMetaTypesDocumentationSkeleton.data");
  SaveInfoFromMetaToFile(fileName, true);
  Z::gEngine->Terminate();
}
/// Saves the list of all non dev-only commands to file
void SaveListOfCommandsToDataFile()
{
  String documentationLocation = GetDocumentationLocation();

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

    forRange (StringParam tag, currCommand->TagList.All())
    {
      currDoc->mTags.PushBack(tag);
    }
  }
  commandDocs.Sort();

  Status status;

  TextSaver saver;
  saver.Open(status, fileName.c_str());

  if (status.Failed())
  {
    Error("Unable to save command list file: %s\n", fileName.c_str());
    return;
  }

  saver.StartPolymorphic("CommandList");

  saver.SerializeField("Commands", commandDocs.mCommands);

  saver.EndPolymorphic();

  saver.Close();
}

void SaveListOfCommands()
{
  SaveListOfCommandsToDataFile();

  Z::gEngine->Terminate();
}

/// saves list of all events in the MetaDataBase to file
void SaveEventListToDataFile()
{
  String documentationLocation = GetDocumentationLocation();

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

  Status status;
  TextSaver saver;

  saver.Open(status, fileName.c_str());

  if (status.Failed())
  {
    Error("Unable to save command list file: %s\n", fileName.c_str());
    return;
  }

  saver.StartPolymorphic("EventList");

  saver.SerializeField("Events", allTheEvents.mEvents);

  saver.EndPolymorphic();

  saver.Close();
}

void SaveEventList()
{
  SaveEventListToDataFile();

  Z::gEngine->Terminate();
}

void SaveUserAttributeListToFile()
{
  // getting list of user attributes
  AttributeExtensions* userExtensions = AttributeExtensions::GetInstance();

  AttributeDocList attribDocumentation;

  typedef Pair<String, AttributeExtension*> AttribPairType;
  // object attributes
  forRange (AttribPairType& attrib, userExtensions->mClassExtensions.All())
  {
    attribDocumentation.mObjectAttributes.PushBack(new AttributeDoc(attrib.second));
  }

  // function attributes
  forRange (AttribPairType& attrib, userExtensions->mFunctionExtensions.All())
  {
    attribDocumentation.mFunctionAttributes.PushBack(new AttributeDoc(attrib.second));
  }

  // property attributes
  forRange (AttribPairType& attrib, userExtensions->mPropertyExtensions.All())
  {
    attribDocumentation.mPropertyAttributes.PushBack(new AttributeDoc(attrib.second));
  }

  // file creation and opening
  String documentationLocation = GetDocumentationLocation();

  String fileName = FilePath::Combine(documentationLocation.All(), "UserAttributeList.data");

  attribDocumentation.SaveToFile(fileName);
}

/// saves all bound classes(with methods + properties) to file, then does same
/// for commands and events
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

  SaveUserAttributeListToFile();

  Z::gEngine->Terminate();
}

/// binds all of the documentation commands as dev only as they all require
/// source directory
void BindDocumentationCommands(Cog* config, CommandManager* commands)
{
  if (Z::gEngine->GetConfigCog()->has(Zero::DeveloperConfig))
  {
    commands->AddCommand("SaveDocumentation", BindCommandFunction(SaveDocumentation))->DevOnly = true;
    commands->AddCommand("SaveNonZilchDocumentation", BindCommandFunction(SaveNonZilchMetaDocumentation))->DevOnly =
        true;
    commands->AddCommand("SaveEventList", BindCommandFunction(SaveEventList))->DevOnly = true;
    commands->AddCommand("SaveCommandList", BindCommandFunction(SaveListOfCommands))->DevOnly = true;
  }
}

} // namespace Zero
