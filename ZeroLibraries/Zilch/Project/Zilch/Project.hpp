/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_PROJECT_HPP
#define ZILCH_PROJECT_HPP

namespace Zilch
{
  namespace Events
  {
    // Sent before we even begin parsing (the library builder should be generally empty)
    // This is an ideal place to add new types to this builder that our parsed Zilch code may depend upon
    ZilchDeclareEvent(PreParser, ParseEvent);

    // Sent when the parsing engine parses a type
    // This event occurs before any members or functions are parsed, which makes it an ideal place
    // to dynamically add members (such as component properties like .RigidBody or .Transform to a composition)
    ZilchDeclareEvent(TypeParsed, ParseEvent);

    // Sent when the engine finishes syntax checking on the abstract syntax tree (but before code generation)
    // Note: Any types added in this phase will NOT be able to be used by the Zilch scripts
    // Note: None of the code from the library should be executed at this time
    ZilchDeclareEvent(PostSyntaxer, ParseEvent);
  }

  // An even that it sent out from parsing (such as when a type is parsed)
  class ZeroShared ParseEvent : public EventData
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    ParseEvent();
    LibraryBuilder* Builder;
    BoundType* Type;
    CodeLocation* Location;
    Project* BuildingProject;
  };

  // A completion is an entry in the auto complete list
  class ZeroShared CompletionEntry
  {
  public:
    CompletionEntry();

    // The comparison operator allows us to easily sort entries by name, type, longest description length, and finally description
    bool operator<(const CompletionEntry& rhs) const;

    // The name to show in the auto-complete list
    String Name;

    // A description provided by user comments or loaded documentation (for overloads, this is the first description)
    String Description;

    // The whole type name or delegate signature
    String Type;

    // The shorter version of the type name (limited to AutoCompleteInfo::ShortTypeNameMaxLength, usually around 20)
    String ShortType;

    // When this completion represents a property or function, we pull this CodeUserData
    // Note that if we remove duplicates upon request, then this
    // may only be the first one sorted by the 'operator<' above
    const void* CodeUserData;
  };

  // A parameter in a function overload
  class ZeroShared CompletionParameter
  {
  public:
    // Constructor
    CompletionParameter();

    // The name of the parameter
    String Name;

    // The parameter's description (or empty if it doesn't have one, which is very common)
    String Description;

    // The type of the parameter stringified
    String Type;

    // The shorter version of the parameter's type name (limited to AutoCompleteInfo::ShortTypeNameMaxLength, usually around 20)
    String ShortType;
  };

  // When showing all the overloads for a function call, this is basically the signature and description
  // Note that we don't sort overloads because their order is actually important to Zilch
  class ZeroShared CompletionOverload
  {
  public:
    // All the parameters in the overload (with types and optional names/descriptions)
    Array<CompletionParameter> Parameters;

    // The description for this overload
    String Description;

    // The return type of the overload (or empty for Void)
    String ReturnType;

    // A shortened version of the return type (or empty for Void, limited to AutoCompleteInfo::ShortTypeNameMaxLength, usually around 20)
    String ReturnShortType;

    // The entire stringified signature of the overload in standard delegate format (with parameter names if applicable)
    String Signature;
  };

  // Returned when we perform an auto-complete query on the Project
  class ZeroShared AutoCompleteInfo
  {
  public:
    // The maximum length for generated short types
    static const size_t ShortTypeNameMaxLength = 20;

    // Constructor
    AutoCompleteInfo();

    // Generates a short type name from a full type name by taking the first word out of the type
    // If the name is still too long, then it truncates it with a trailing elipsis '...'
    static String GetShortTypeName(StringParam fullTypeName);

    // By default, we remove all duplicate completion entries (we sort by name/type and we keep ones with the longest description)
    bool RemoveDuplicateNameEntries;

    // Whether the completion query was successful at finding anything
    bool Success;
    
    // The nearest type we found to the left of the cursor (can be null if we were unable to find it)
    Type* NearestType;

    // If the value we're accessing is a literal value (such as "hello", 5, 3.3, true, false, etc)
    // If the NearestType is Integer and its a literal and the user is pressing '.', most IDEs will ignore this and not show auto-complete
    // The reason is that the user may be typing a Real value in after the '.' (eg 5.678)
    bool IsLiteral;

    // Whether or not we were accessing statics or instance members of the type
    bool IsStatic;

    // A convenient array of completion names and descriptions to show in any text editor (sorted by name)
    Array<CompletionEntry> CompletionEntries;

    // The name of the function when overloads are involved
    // If the overloads are generated from a delegate, the name will be "delegate"
    String FunctionName;

    // When performing a function call these are all the possible overloads that we should show (also works on single delegates)
    // This includes descriptions, parameter names, and the types
    Array<CompletionOverload> CompletionOverloads;

    // If any text editor is incapable of showing an overload resolution list (with classic up down arrows to change between overloads)
    // then the editor is recommended to show this the overload at this index
    // This will attempt to choose the first overload with a description, or it will choose the first overload (will be -1 if there are no overloads)
    int BestCompletionOverload;

    // In the case that we were accessing overloaded functions, these are our options
    FunctionArray FunctionOverloads;

    // We build an incomplete library to keep references to types alive
    LibraryRef IncompleteLibrary;

    // Turns the auto complete information to a Json format (typically used for reading by other external applications)
    String GetJson();
  };

  // Returned when we perform an definition query on the Project
  class ZeroShared CodeDefinition
  {
  public:
    CodeDefinition();

    // The location of the entire element that we found: 'var test = 5;'
    CodeLocation ElementLocation;

    // The location of just the name of an element if applicable: var 'test' = 5;
    CodeLocation NameLocation;

    // All the pertinant definitions will be filled out where applicable
    Variable*         DefinedVariable;
    Function*         DefinedFunction;
    GetterSetter*     DefinedGetterSetter;
    Field*            DefinedField;
    Property*         DefinedProperty;
    Member*           DefinedMember;
    ReflectionObject* DefinedObject;

    // The name is only ever set for members or variables
    // This is effectively used in cases where we would have a ':' to specify a type
    String Name;

    // The type of a member, variable, class/struct definition, or of any expression
    Type* ResolvedType;

    // An automatically generated tooltip to show to users
    String ToolTip;

    // We build an incomplete library to keep references to types alive
    LibraryRef IncompleteLibrary;
  };

  class ZeroShared PluginEntry
  {
  public:
    PluginEntry();
    PluginEntry(StringParam path, void* userData = nullptr);

    // The file or directory that the plugin exists at
    String Path;

    // Any user data we wisht to attach
    // This gets attached to the plugin, and possibly even the library depending on the plugin's implementation
    void* UserData;
  };

  // The project Contains all the files that are being compiled together
  class ZeroShared Project : public CompilationErrors
  {
  public:
    friend class Debugger;

    // Constructor
    Project();

    // Adds a code to the project
    // The origin is the display name (typically the file name)
    // Any time any error occurs with compilation, or anything that references
    // this particular block of code will be linked up to the code user-data
    void AddCodeFromString(StringParam code, StringParam origin = CodeString, void* codeUserData = nullptr);

    // Adds code from a file (see AddCode)
    // Returns true if it succeeded, false otherwise
    bool AddCodeFromFile(StringParam fileName, void* codeUserData = nullptr);

    // Clears out the project (removes all code strings/files, plugin directories, plugin files, etc)
    void Clear();

    // Reads a text file into a string, returns true on success, false on failure
    static String ReadTextFile(Status& status, StringParam fileName);

    // Tokenizes all files into a token stream
    bool Tokenize(Array<UserToken>& tokensOut, Array<UserToken>& commentsOut);

    // Attach all the parsed comments to the syntax tree nodes that are nearby
    void AttachCommentsToNodes(SyntaxTree& syntaxTree, Array<UserToken>& comments);

    // Compiles the project into an unchecked syntax tree (only parsed)
    bool CompileUncheckedSyntaxTree(SyntaxTree& syntaxTreeOut, Array<UserToken>& tokensOut, EvaluationMode::Enum evaluation);

    // Compiles the project into a checked syntax tree
    bool CompileCheckedSyntaxTree
    (
      SyntaxTree& syntaxTreeOut,
      LibraryBuilder& builder,
      Array<UserToken>& tokensOut,
      const Module& dependencies,
      EvaluationMode::Enum evaluation
    );

    // Compiles the project into a single library and also returns the syntax tree
    LibraryRef Compile(StringParam libraryName, Module& dependencies, EvaluationMode::Enum evaluation, SyntaxTree& treeOut, BuildReason::Enum reason);

    // Compiles the project into a single library
    LibraryRef Compile(StringParam libraryName, Module& dependencies, EvaluationMode::Enum evaluation);

    // Attempt to compile the code in tolerant mode, and return the type nearest to the left hand side of a given cursor
    // This function is generally used for auto-completion lists and will attempt to return the type left of the cursor
    // The minimum code you can provide to the project is a single class (the one that the cursor is inside of, typically the whole file being edited)
    // The old library can be a nullptr, however it is generally recommended to provide it if it has previously been compiled
    // since it will allow the auto-completer to resolve local types too
    void GetAutoCompleteInfo(Module& dependencies, size_t cursorPosition, StringParam cursorOrigin, AutoCompleteInfo& resultOut);

    // For every usage of an identifier there is a location where we defined that identifier (variable definiton, member, type, etc)
    // This gets the definition location (and the actual resulting definition object) of whatever is under the cursor
    void GetDefinitionInfo(Module& dependencies, size_t cursorPosition, StringParam cursorOrigin, CodeDefinition& resultOut);

  public:

    // A pointer to any data the user wants to attach
    mutable const void* UserData;

    // Any user data that cant simply be represented by a pointer
    // Data can be written to the buffer and will be properly destructed
    // when this object is destroyed (must be read in the order it's written)
    mutable DestructibleBuffer ComplexUserData;

    // If a variable needs to generate a unique name that will be guaranteed to never conflict with
    // any other local variables within the function, then we use this counter as a unique id
    size_t VariableUniqueIdCounter;

    // We will automatically attempt to load plugins from these directories
    Array<PluginEntry> PluginDirectories;

    // We will also attempt to load these specific plugin files
    Array<PluginEntry> PluginFiles;

    // Setup the location and the name for a found definition
    void InitializeDefinitionInfo(CodeDefinition& resultOut, ReflectionObject* object);

  private:

    // Returns the name of the type, but prepends 'class' or 'struct' for BoundTypes
    static String GetFriendlyTypeName(Type* type);

    // Internal function called by the above 'GetDefinitionInfo'
    void GetDefinitionInfoInternal(Module& dependencies, size_t cursorPosition, StringParam cursorOrigin, CodeDefinition& resultOut);

    // Loads all the plugins into the given library builder
    void LoadPlugins(LibraryBuilder& builder, Module& dependencies, BuildReason::Enum reason);

    // Get auto complete information (but doesn't parse it into completions)
    void GetAutoCompleteInfoInternal(Module& dependencies, size_t cursorPosition, StringParam cursorOrigin, AutoCompleteInfo& resultOut);

    // Creates a completion for an overload using only the delegate type (generally used when performing a call)
    CompletionOverload& AddAutoCompleteOverload(AutoCompleteInfo& info, DelegateType* delegateType);

  private:

    // All the code that makes up this project
    Array<CodeEntry> Entries;

    // A special constant that means we don't have a cursor
    static const size_t NoCursor = (size_t)-1;

    // When attempting to generate code-completion, this is the cursor position for the user
    String CursorOrigin;
    size_t CursorPosition;

    // Not copyable
    ZilchNoCopy(Project);
  };
}

#endif
