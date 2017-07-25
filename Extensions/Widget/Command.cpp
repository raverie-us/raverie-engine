///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//----------------------------------------------------------------------- Events
namespace Events
{
  DefineEvent(CommandStateChange);
  DefineEvent(CommandCaptureContext);
  DefineEvent(CommandExecute);
}//namespace Events

//------------------------------------------------------------------------- Tags
namespace Tags
{
  DefineTag(Command);
}//namespace Tags

//---------------------------------------------------------------- Command Event
//******************************************************************************
ZilchDefineType(CommandEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterProperty(Space);
}

//******************************************************************************
CommandEvent::CommandEvent(Object* source, CommandManager* manager) :
  ObjectEvent(source),
  mManager(manager)
{
  
}

//******************************************************************************
Space* CommandEvent::GetSpace()
{
  return mManager->GetContext<Space>();
}

//-------------------------------------------------------------------CommandExecuter
ZilchDefineType(CommandExecuter, builder, type)
{
}

CommandExecuter* BuildMetaCommandExecuter(StringParam executionFunction)
{
  // Parse the function string
  StringTokenRange tokens(executionFunction.All(), '.');
  ReturnIf(tokens.Empty(), nullptr, "Bad execution function format '%s'", executionFunction.c_str());

  String object = tokens.Front();
  tokens.PopFront();

  ReturnIf(tokens.Empty(), nullptr, "Bad execution function format '%s'", executionFunction.c_str());

  String functionName = tokens.Front();

  // Find object
  Object* systemObject = Z::gSystemObjects->FindObject(object);
  ReturnIf(systemObject == nullptr, nullptr, "Failed to find system object '%s'", object.c_str());

  // Find Function
  BoundType* metaType = ZilchVirtualTypeId(systemObject);
  Function* function = metaType->GetFunction(functionName);

  ReturnIf(function == nullptr || !function->FunctionType->Parameters.Empty(),
    nullptr, "Failed to find method or invalid number of parameters '%s' on '%s'",
    functionName.c_str(), metaType->Name.c_str());

  MetaCommandExecuter* executer = new MetaCommandExecuter();
  executer->mDelegate.ThisHandle = systemObject;
  executer->mDelegate.BoundFunction = function;
  return executer;
}

//---------------------------------------------------------------------- Command
ZilchDefineType(Command, builder, type)
{
}

Command::Command()
{
  mExecuter = nullptr;
  DevOnly = false;
  Active = false;
}

Command::~Command()
{
  SafeDelete(mExecuter);
}

void Command::Serialize(Serializer& stream)
{
  SerializeName(Name);
  SerializeNameDefault(Description, String());
  SerializeNameDefault(IconName, String());
  SerializeNameDefault(Shortcut, String());
  SerializeNameDefault(Function, String());
  SerializeName(Tags);
  SerializeNameDefault(DevOnly, false);
}

void Command::SetActive(bool active)
{
  Active = active;
  ChangeState();
}

bool Command::IsActive()
{
  return Active;
}

bool Command::IsEnabled()
{
  CommandManager* commandManager = CommandManager::GetInstance();

  if(mExecuter)
    return mExecuter->IsEnabled(this, commandManager);

  return true;
}

void Command::Execute()
{
  CommandManager* commandManager = CommandManager::GetInstance();

  CommandEvent eventToSend(this, commandManager);
  this->GetDispatcher()->Dispatch(Events::CommandExecute, &eventToSend);

  if(mExecuter)
    mExecuter->Execute(this, commandManager);
}

void Command::Format()
{
  ToolTip = String::Format("%s - %s (%s)", Name.c_str(), Description.c_str(), Shortcut.c_str());
}

void Command::ChangeState()
{
  ObjectEvent toSend = ObjectEvent(this);
  this->GetDispatcher()->Dispatch(Events::CommandStateChange, &toSend);
}

String SeperateWords(StringParam sourceString)
{
  StringBuilder output;
  StringRange str = sourceString.All();
  // Always take the start of the string
  output.Append(str.Front());
  str.PopFront();
  while(!str.Empty())
  {
    if(str.IsCurrentRuneUpper())
      output.Append(' ');
    output.Append(str.Front());
    str.PopFront();
  }
  return output.ToString();
}

//-------------------------------------------------------------------CommandSearchProvider
void CommandSearchProvider::Search(SearchData& search)
{
  DeveloperConfig* devConfig = Z::gEngine->GetConfigCog()->has(DeveloperConfig);

  String filterString = search.SearchString;
  forRange(Command* command, mCommandSet->mCommands.All())
  {
    // Only show dev commands if dev config is present
    if(command->DevOnly && devConfig == nullptr)
      continue;

    FilterAddCommand(search, command);
  }
}

String CommandSearchProvider::GetType(SearchViewResult& element)
{
  const String CommandName = "Command";
  return CommandName;
}

void CommandSearchProvider::RunCommand(SearchView* searchView, SearchViewResult& element)
{
  Command* command = (Command*)element.Data;
  command->Execute();
}

Composite* CommandSearchProvider::CreatePreview(Composite* parent, SearchViewResult& element)
{
  Command* command = (Command*)element.Data;
  return CreateTextPreview(parent, command->Description);
}

void CommandSearchProvider::FilterAddCommand(SearchData& search, Command* command)
{
  if(!CheckAndAddTags(search, command->TagList))
    return;

  int priority = PartialMatch(search.SearchString.All(), command->Name.All(), CaseInsensitiveCompare);
  if(priority != cNoMatch)
  {
    SearchViewResult& result = search.Results.PushBack();
    result.Data = command;
    result.Interface = this;
    result.Name = command->Name;
    result.Priority = priority + sCommandPriorityBoost;
  }
}

//------------------------------------------------------------------ MenuDefinition
void MenuDefinition::Serialize(Serializer& stream)
{
  SerializeName(Name);
  SerializeNameDefault(Description, String());
  SerializeNameDefault(Icon, String());
  SerializeName(Entries);
}

//------------------------------------------------------------------ Command Manager
ZilchDefineType(CommandManager, builder, type)
{
  ZeroBindEvent(Events::CommandExecute, CommandEvent);
}

CommandManager::CommandManager()
{
  
}

CommandManager::~CommandManager()
{
  DeleteObjectsInContainer(mCommands);
  DeleteObjectsInContainer(mMenus);
}

void CommandManager::LoadCommands(StringParam filename)
{
  Status status;
  UniquePointer<Serializer> stream(GetLoaderStreamFile(status, filename));
  if(status)
  {
    Array<Command*> commands;
    LoadPolymorphicSerialize("Commands", "commands", *stream, commands, this);
    forRange(Command* command, commands.All())
    {
      command->Format();
      AddCommand(command);
    }
    ScanCommands();
  }
  else
  {
    DoNotifyStatus(status);
  }
}

void CommandManager::LoadMenu(StringParam filename)
{
  Status status;

  Array<MenuDefinition*> menus;

  UniquePointer<Serializer> stream(GetLoaderStreamFile(status, filename));
  if(status)
    stream->SerializeValue(menus);

  forRange(MenuDefinition* menuDef, menus.All())
    mMenus[menuDef->Name] = menuDef;
}

Command* CommandManager::CreateFromName(StringParam name)
{
  return new Command();
}

Command* CommandManager::AddCommand(StringParam commandName, CommandExecuter* executer)
{
  Command* existingCommand = NamedCommands.FindValue(commandName, nullptr);
  if(existingCommand)
  {
    // Command already loaded but binding executer
    existingCommand->mExecuter = executer;
    if(existingCommand->Description.Empty())
      existingCommand->Description = executer->GetDescription();
    return existingCommand;
  }

  // Make a command object
  Command* command = new Command();
  command->Name = commandName;
  command->mExecuter = executer;
  command->DisplayName = SeperateWords(commandName);

  if(command->Description.Empty())
    command->Description = executer->GetDescription();

  AddCommand(command);
  return command;
}

void CommandManager::AddCommand(Command* command)
{
  NamedCommands[command->Name] = command;
  mCommands.PushBack(command); 
}

Command* CommandManager::GetCommand(StringParam name)
{
  return NamedCommands.FindValue(name, nullptr);
}

void CommandManager::RemoveCommand(Command* command)
{
  NamedCommands.Erase(command->Name);
  ShortCuts.Erase(command->Name);
  mCommands.EraseValueError(command);

  delete command;
}

void CommandManager::RunParsedCommands()
{
  Environment* environment = Environment::GetInstance();
  StringMap& arguments = environment->mParsedCommandLineArguments;
  for(StringMap::range range = arguments.All(); !range.Empty(); range.PopFront())
  {
    String commandName = range.Front().first;
    Command* command = GetCommand(commandName);
    if(command)
      command->Execute();
  }
}

Handle CommandManager::GetContextFromTypeName(StringParam typeName)
{
  return ContextMap.FindValue(typeName, Handle());
}

void CommandManager::SetContext(HandleParam context, BoundType* overrideType)
{
  if(overrideType)
    ContextMap[overrideType->Name] = context;
  else
    ContextMap[context.StoredType->Name] = context;
}

void CommandManager::ClearContext(BoundType* boundType)
{
  ContextMap.Erase(boundType->Name);
}

bool CommandManager::TestCommandKeyboardShortcuts(KeyboardEvent* event)
{
  // Do not process handled keyboard events
  if(event->Handled || event->Key == Keys::Unknown)
    return false;

  StringBuilder builder;
  if(event->CtrlPressed)
    builder << "Ctrl+";
  if(event->AltPressed)
    builder << "Alt+";
  if(event->ShiftPressed)
    builder << "Shift+";
  builder << event->mKeyboard->GetKeyName(event->Key);
  String shortcutString = builder.ToString();

  // ZPrint("Shortcut %s\n", shortcutString.c_str());
  Command* command = ShortCuts.FindValue(shortcutString, nullptr);
  if(command == nullptr)
    return false;

  event->Handled = true;
  command->Execute();
  return true;
}

SearchProvider* CommandManager::GetCommandSearchProvider()
{
  CommandSearchProvider* commandCompletion = new CommandSearchProvider();
  commandCompletion->mCommandSet = this;
  return commandCompletion;
}

void CommandManager::ScanCommands()
{
  forRange(Command* command, mCommands.All())
  {
    // Build all tags for this command
    StringTokenRange tokens(command->Tags.c_str(), ' ');
    for(; !tokens.Empty(); tokens.PopFront())
    {
      String tag = tokens.Front();
      command->TagList.Insert(tag);
    }

    // Add shortcut if provided
    if(!command->Shortcut.Empty())
      ShortCuts.Insert(command->Shortcut, command);

    // Build command executer from meta
    if(!command->Function.Empty())
      command->mExecuter = BuildMetaCommandExecuter(command->Function);

    command->DisplayName = SeperateWords(command->Name);
  }
}

void CommandManager::ValidateCommands()
{
  forRange(Command* command, mCommands.All())
  {
    if(command->mExecuter)
      continue;

    if(command->GetDispatcher()->IsAnyConnected(Events::CommandExecute))
      continue;

    ErrorIf(true, "Command not valid %s", command->Name.c_str());
  }
}

//-------------------------------------------------------------------CommandCaptureContextEvent
ZilchDefineType(CommandCaptureContextEvent, builder, type)
{
}

}//namespace Zero
