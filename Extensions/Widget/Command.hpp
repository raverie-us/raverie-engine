///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class CommandManager;
class Command;
class CogCommand;

//----------------------------------------------------------------------- Events
namespace Events
{
  DeclareEvent(CommandStateChange);
  DeclareEvent(CommandCaptureContext);
  DeclareEvent(CommandExecute);
}//namespace Events

//------------------------------------------------------------------------- Tags
namespace Tags
{
  DeclareTag(Command);
}//namespace Tags

//---------------------------------------------------------------- Command Event
class CommandEvent : public ObjectEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CommandEvent(Object* source, CommandManager* manager);

  /// Gives context to where the command was executed.
  Space* GetSpace();

  CommandManager* mManager;
};

//------------------------------------------------------------- Command Executer
/// Runs when a command is Executed.
class CommandExecuter : public Object
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  virtual ~CommandExecuter() {}

  /// Run the command
  virtual void Execute(Command* command, CommandManager* manager) = 0;

  /// Is the command available? If you disable a command make sure the description describes why
  virtual bool IsEnabled(Command* command, CommandManager* manager) { return true; }

  virtual String GetDescription() { return String(); }
};

CommandExecuter* BuildMetaCommandExecuter(StringParam executionFunction);

//---------------------------------------------------------------------- Command
// Command that is available in the search list and tool bars.
class Command : public EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Command();
  ~Command();

  /// Serializes the command to the given stream.
  void Serialize(Serializer& stream);

  /// Set a command's active status.
  void SetActive(bool active);
  /// Is this command currently active?
  bool IsActive();
  /// Is this command currently able to run? Checks the command's executer if it exists.
  bool IsEnabled();
  

  virtual void Execute();

  /// Sets the ToolTip member value.
  void Format();
  
  /// Sends out that the state of this command changed
  void ChangeState();

  /// Name of the command.
  String Name;
  /// Display Name
  String DisplayName;
  /// Description of what the command will do.
  String Description;
  /// Icon to be used on toolbars and menus.
  String IconName;
  /// Keyboard shortcut.
  String Shortcut;
  /// Execution function
  String Function;
  /// Tags strings
  String Tags;
  /// Will only be available if DeveloperConfig is present.
  bool DevOnly;
  /// Is the command active used for tools and togglable commands
  bool Active;

  //-------------------------------------------------------------------Generated Data
  // Tool tip or short description.
  String ToolTip;
  // Tags on this object
  HashSet<String> TagList;
  CommandExecuter* mExecuter;
};

//-------------------------------------------------------------------CommandSearchProvider
class CommandSearchProvider : public SearchProvider
{
public:
  /// Boost priority to make commands appear above results with the same
  /// name (like script commands over script files).
  static const int sCommandPriorityBoost = 10;

  // Search Provider Interface
  void Search(SearchData& search) override;
  String GetType(SearchViewResult& element) override;
  void RunCommand(SearchView* searchView, SearchViewResult& element) override;
  Composite* CreatePreview(Composite* parent, SearchViewResult& element) override;

  void FilterAddCommand(SearchData& search, Command* command);

  CommandManager* mCommandSet;
};

//-------------------------------------------------------------- Menu Definition
/// A MenuDefinition Contains list of commands for context menus and toolbars.
class MenuDefinition
{
public:
  void Serialize(Serializer& stream);

  String Name;
  String Description;
  String Icon;
  Array<String> Entries;
};

//------------------------------------------------------------------ Command Set
class CommandManager : public ExplicitSingleton<CommandManager, EventObject>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CommandManager();
  ~CommandManager();

  /// Load commands from a file
  void LoadCommands(StringParam filename);
  /// Load menu definitions from a file path
  void LoadMenu(StringParam filename);
  /// Used for polymorphic serialization.
  Command* CreateFromName(StringParam name);
  
  /// Creates a command by name with a executer (callback).
  Command* AddCommand(StringParam commandName, CommandExecuter* executer);
  void AddCommand(Command* command);
  /// Finds a command by name.
  Command* GetCommand(StringParam name);
  /// Deletes the given command
  void RemoveCommand(Command* command);
  /// Checks the command-line arguments to see if any match a command.
  void RunParsedCommands();

  // Context functions
  template<typename ContextType>
  ContextType* GetContext();
  Handle GetContextFromTypeName(StringParam typeName);
  void SetContext(HandleParam object, BoundType* overrideType = nullptr);
  void ClearContext(BoundType* boundType);
  
  /// Check to see if a command's shortcut hot-key has been pressed.
  bool TestCommandKeyboardShortcuts(KeyboardEvent* event);

  SearchProvider* GetCommandSearchProvider();
  
  /// Process all loaded commands to find tags, shortcuts, etc...
  void ScanCommands();
  void ValidateCommands();

  typedef HashMap<String, Handle> ContextMapType;
  ContextMapType ContextMap;

  typedef HashMap<String, Command*> CommandMapType;
  Array<Command*> mCommands;

  HashMap<String, MenuDefinition*> mMenus;
  CommandMapType NamedCommands;
  CommandMapType ShortCuts;
};

template<typename ContextType>
ContextType* CommandManager::GetContext()
{
  return GetContextFromTypeName(ZilchTypeId(ContextType)->Name).Get<ContextType*>();
}

//------------------------------------------------ Command Capture Context Event
class CommandCaptureContextEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  CommandManager* ActiveSet;
};

}//namespace Zero
