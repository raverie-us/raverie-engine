// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
DefineEvent(CommandStateChange);
DefineEvent(CommandCaptureContext);
DefineEvent(CommandExecute);
DefineEvent(CommandAdded);
DefineEvent(CommandRemoved);
DefineEvent(CommandUpdated);
} // namespace Events

namespace Tags
{
DefineTag(Command);
} // namespace Tags

String SeperateWords(StringParam sourceString)
{
  if (sourceString.Empty())
    return sourceString;

  StringBuilder output;
  StringRange str = sourceString.All();
  // Always take the start of the string
  output.Append(str.Front());
  str.PopFront();
  while (!str.Empty())
  {
    if (str.IsCurrentRuneUpper())
      output.Append(' ');
    output.Append(str.Front());
    str.PopFront();
  }
  return output.ToString();
}

RaverieDefineType(CommandUpdateEvent, builder, type)
{
}

CommandUpdateEvent::CommandUpdateEvent(Command* command) : mCommand(command)
{
}

RaverieDefineType(CommandEvent, builder, type)
{
  RaverieBindDocumented();

  RaverieBindGetterProperty(Space);
}

CommandEvent::CommandEvent(Object* source, CommandManager* manager) : ObjectEvent(source), mManager(manager)
{
}

Space* CommandEvent::GetSpace()
{
  return mManager->GetContext()->Get<Space>();
}

RaverieDefineType(CommandExecuter, builder, type)
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
  BoundType* metaType = RaverieVirtualTypeId(systemObject);
  Function* function = metaType->GetFunction(functionName);

  ReturnIf(function == nullptr || !function->FunctionType->Parameters.Empty(),
           nullptr,
           "Failed to find method or invalid number of parameters '%s' on '%s'",
           functionName.c_str(),
           metaType->Name.c_str());

  MetaCommandExecuter* executer = new MetaCommandExecuter();
  executer->mDelegate.ThisHandle = systemObject;
  executer->mDelegate.BoundFunction = function;
  return executer;
}

RaverieDefineType(Command, builder, type)
{
}

Command::Command()
{
  mExecuter = nullptr;
  DevOnly = false;
  Active = false;
  ReadOnly = false;
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
  SerializeNameDefault(ReadOnly, false);
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

  if (mExecuter)
    return mExecuter->IsEnabled(this, commandManager);

  return true;
}

void Command::Execute()
{
  CommandManager* commandManager = CommandManager::GetInstance();

  CommandEvent eventToSend(this, commandManager);
  this->GetDispatcher()->Dispatch(Events::CommandExecute, &eventToSend);

  if (mExecuter)
    mExecuter->Execute(this, commandManager);
}

StringParam Command::GetDisplayName()
{
  return DisplayName;
}

void Command::SetDisplayName(StringParam name)
{
  DisplayName = SeperateWords(name);
}

void Command::FillOutToolTip()
{
  // Assumes all commands must have names.
  if (Name.Empty())
    return;

  if (!Description.Empty())
  {
    if (!Shortcut.Empty())
      ToolTip = String::Format("%s - %s (%s)", Name.c_str(), Description.c_str(), Shortcut.c_str());
    else
      ToolTip = String::Format("%s - %s", Name.c_str(), Description.c_str());
  }
  else if (!Shortcut.Empty())
  {
    ToolTip = String::Format("%s (%s)", Name.c_str(), Shortcut.c_str());
  }
  else // !Name.Empty()
  {
    ToolTip = String::Format("%s", Name.c_str());
  }
}

void Command::ChangeState()
{
  ObjectEvent toSend = ObjectEvent(this);
  this->GetDispatcher()->Dispatch(Events::CommandStateChange, &toSend);
}

void Command::ExecuteCommand()
{
  if (Z::gEngine->IsReadOnly() && !ReadOnly)
  {
    DoNotifyWarning("Command", BuildString("Cannot execute command ", Name, " because we are in read-only mode"));
    return;
  }

  Execute();
}

CommandSearchProvider::CommandSearchProvider() : SearchProvider("Command")
{
}

void CommandSearchProvider::Search(SearchData& search)
{
  DeveloperConfig* devConfig = Z::gEngine->GetConfigCog()->has(DeveloperConfig);

  String filterString = search.SearchString;
  forRange (Command* command, mCommandSet->mCommands.All())
  {
    // Only show dev commands if dev config is present
    if (command->DevOnly && devConfig == nullptr)
      continue;

    FilterAddCommand(search, command);
  }
}

String CommandSearchProvider::GetElementType(SearchViewResult& element)
{
  const String CommandName = "Command";
  return CommandName;
}

void CommandSearchProvider::RunCommand(SearchView* searchView, SearchViewResult& element)
{
  Command* command = (Command*)element.Data;
  command->ExecuteCommand();
}

Composite* CommandSearchProvider::CreatePreview(Composite* parent, SearchViewResult& element)
{
  Command* command = (Command*)element.Data;
  return CreateTextPreview(parent, command->Description);
}

void CommandSearchProvider::FilterAddCommand(SearchData& search, Command* command)
{
  if (!CheckAndAddTags(search, command->TagList))
    return;

  int priority = PartialMatch(search.SearchString.All(), command->Name.All(), CaseInsensitiveCompare);
  if (priority != cNoMatch)
  {
    SearchViewResult& result = search.Results.PushBack();
    result.Data = command;
    result.Interface = this;
    result.Name = command->Name;
    result.Priority = priority + SearchViewResultPriority::CommandBegin;
  }
}

// MenuDefinition
void MenuDefinition::Serialize(Serializer& stream)
{
  SerializeName(Name);
  SerializeNameDefault(Description, String());
  SerializeNameDefault(Icon, String());
  SerializeName(Entries);
}

// Manager
RaverieDefineType(CommandManager, builder, type)
{
  RaverieBindEvent(Events::CommandExecute, CommandEvent);
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
  if (status)
  {
    Array<Command*> commands;
    LoadPolymorphicSerialize("Commands", "commands", *stream, commands, this);
    forRange (Command* command, commands.All())
    {
      AddCommand(command);
    }
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
  if (status)
    stream->SerializeValue(menus);

  forRange (MenuDefinition* menuDef, menus.All())
    mMenus[menuDef->Name] = menuDef;
}

Command* CommandManager::CreateFromName(StringParam name)
{
  return new Command();
}

Command* CommandManager::AddCommand(StringParam commandName, CommandExecuter* executer, bool readOnly)
{
  Command* existingCommand = mNamedCommands.FindValue(commandName, nullptr);
  if (existingCommand)
  {
    // Command already loaded, except the binding executer.
    existingCommand->mExecuter = executer;
    existingCommand->ReadOnly = readOnly;
    if (existingCommand->Description.Empty())
      existingCommand->Description = executer->GetDescription();

    return existingCommand;
  }

  // Make a command object
  Command* command = new Command();
  command->Name = commandName;
  command->mExecuter = executer;
  command->ReadOnly = readOnly;

  if (command->Description.Empty())
    command->Description = executer->GetDescription();

  AddCommand(command);
  return command;
}

void CommandManager::AddCommand(Command* command)
{
  mNamedCommands[command->Name] = command;
  mCommands.PushBack(command);

  command->SetDisplayName(command->Name);
  command->FillOutToolTip();

  BuildTagList(command);

  if (!command->Shortcut.Empty() && !IsShortcutReserved(command->Shortcut))
    mShortcuts.Insert(command->Shortcut, command);

  // Build command executer from meta, if applicable.
  if (!command->Function.Empty())
    command->mExecuter = BuildMetaCommandExecuter(command->Function);

  CommandUpdateEvent eventToSend(command);
  DispatchEvent(Events::CommandAdded, &eventToSend);
}

Command* CommandManager::GetCommand(StringParam name)
{
  return mNamedCommands.FindValue(name, nullptr);
}

void CommandManager::RemoveCommand(Command* command)
{
  mNamedCommands.Erase(command->Name);
  mShortcuts.Erase(command->Name);
  mCommands.EraseValueError(command);

  CommandUpdateEvent eventToSend(command);
  DispatchEvent(Events::CommandRemoved, &eventToSend);

  delete command;
}

void CommandManager::RunParsedCommandsDelayed()
{
  if (Z::gWidgetManager->RootWidgets.Empty())
  {
    RunParsedCommandsImmediately();
  }
  else
  {
    // We delay running commands by one frame so that lazy tasks have had a frame to complete
    ActionSequence* seq = new ActionSequence(&Z::gWidgetManager->RootWidgets.Front());
    seq->Add(new CallAction<CommandManager, &CommandManager::RunParsedCommandsImmediately>(this));
  }
}

void CommandManager::RunParsedCommandsImmediately()
{
  Environment* environment = Environment::GetInstance();
  StringMap& arguments = environment->mParsedCommandLineArguments;
  for (StringMap::Range range = arguments.All(); !range.Empty(); range.PopFront())
  {
    String commandName = range.Front().first;
    Command* command = GetCommand(commandName);
    if (command)
    {
      ZPrint("Executing command: %s\n", commandName.c_str());
      command->ExecuteCommand();
    }
  }
}

String CommandManager::BuildShortcutString(bool ctrl, bool alt, bool shift, StringParam key)
{
  StringBuilder builder;

  if (ctrl)
    builder << "Ctrl+";
  if (alt)
    builder << "Alt+";
  if (shift)
    builder << "Shift+";

  builder << key;
  return builder.ToString();
}

bool CommandManager::TestCommandKeyboardShortcuts(KeyboardEvent* event)
{
  // Do not process handled keyboard events
  if (event->Handled || event->Key == Keys::Unknown)
    return false;

  String shortcut = BuildShortcutString(
      event->CtrlPressed, event->AltPressed, event->ShiftPressed, event->mKeyboard->ToSymbol(event->Key));

  // ZPrint("Shortcut %s\n", shortcutString.c_str());
  Command* command = mShortcuts.FindValue(shortcut, nullptr);
  if (command == nullptr)
    return false;

  event->Handled = true;
  command->ExecuteCommand();

  return true;
}

bool CommandManager::TestCommandCopyPasteShortcuts(ClipboardEvent* event)
{
  if (event->mHandled)
    return false;

  // The "Cut", "Copy", and "Paste" events are special.
  Command* command = mShortcuts.FindValue(event->EventId, nullptr);
  if (command == nullptr)
    return false;

  event->mHandled = true;

  GetContext()->Add(event);
  command->ExecuteCommand();
  GetContext()->Remove(RaverieVirtualTypeId(event));

  return true;
}

bool CommandManager::IsShortcutReserved(StringParam validShortcut)
{
  return mShortcuts.FindValue(validShortcut, nullptr) != nullptr;
}

bool CommandManager::IsShortcutReserved(bool ctrl, bool alt, bool shift, StringParam validKey, Command** out)
{
  String shortcut = BuildShortcutString(ctrl, alt, shift, validKey);

  *out = mShortcuts.FindValue(shortcut, nullptr);
  return (*out != nullptr);
}

bool CommandManager::ClearCommandShortcut(Command* command, bool sendEvents)
{
  if (command->Shortcut.Empty())
    return false;

  mShortcuts.Erase(command->Shortcut);
  command->Shortcut.Clear();

  if (sendEvents)
  {
    CommandUpdateEvent eventToSend(command);
    DispatchEvent(Events::CommandUpdated, &eventToSend);
  }

  return true;
}

bool CommandManager::UpdateCommandShortcut(
    StringParam commandName, bool ctrl, bool alt, bool shift, StringParam key, bool sendEvents)
{
  if (Command* command = mNamedCommands.FindValue(commandName, nullptr))
    return UpdateCommandShortcut(command, ctrl, alt, shift, key, sendEvents);

  return false;
}

bool CommandManager::UpdateCommandShortcut(
    Command* command, bool ctrl, bool alt, bool shift, StringParam key, bool sendEvents)
{
  String shortcut = BuildShortcutString(ctrl, alt, shift, key);

  // Supplied shortcut is already set on this command, do nothing.
  if (command->Shortcut == shortcut)
    return false;

  mShortcuts.Erase(command->Shortcut);

  command->Shortcut = shortcut;
  mShortcuts.Insert(shortcut, command);

  if (sendEvents)
  {
    CommandUpdateEvent eventToSend(command);
    DispatchEvent(Events::CommandUpdated, &eventToSend);
  }

  return true;
}

bool CommandManager::UpdateCommandTags(StringParam commandName, StringParam tags, bool sendEvents)
{
  if (Command* command = mNamedCommands.FindValue(commandName, nullptr))
    return UpdateCommandTags(command, tags);

  return false;
}

bool CommandManager::UpdateCommandTags(Command* command, StringParam tags, bool sendEvents)
{
  if (command->Tags == tags)
    return false;

  command->Tags = tags;
  command->TagList.Clear();

  // Build all tags for this command
  StringTokenRange tokens(command->Tags.c_str(), ' ');
  for (; !tokens.Empty(); tokens.PopFront())
  {
    String tag = tokens.Front();
    command->TagList.Insert(tag);
  }

  if (sendEvents)
  {
    CommandUpdateEvent eventToSend(command);
    DispatchEvent(Events::CommandUpdated, &eventToSend);
  }

  return true;
}

SearchProvider* CommandManager::GetCommandSearchProvider()
{
  CommandSearchProvider* commandCompletion = new CommandSearchProvider();
  commandCompletion->mCommandSet = this;
  return commandCompletion;
}

void CommandManager::BuildTagList(Command* command)
{
  // Build all tags for this command
  StringTokenRange tokens(command->Tags.c_str(), ' ');
  for (; !tokens.Empty(); tokens.PopFront())
  {
    String tag = tokens.Front();
    command->TagList.Insert(tag);
  }
}

void CommandManager::ValidateCommands()
{
  forRange (Command* command, mCommands.All())
  {
    if (command->mExecuter)
      continue;

    if (command->GetDispatcher()->IsAnyConnected(Events::CommandExecute))
      continue;

    ErrorIf(true, "Command not valid %s", command->Name.c_str());
  }
}

Context* CommandManager::GetContext()
{
  return &mContext;
}

RaverieDefineType(CommandCaptureContextEvent, builder, type)
{
}

} // namespace Raverie
