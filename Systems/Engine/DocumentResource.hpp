
///////////////////////////////////////////////////////////////////////////////
///
/// \file Resource.hpp
/// Declaration of the Resource base class.
///
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
namespace Events
{
  // Sent on the document to inform any editors that they should clear annotations (such as errors)
  // This is primarily used when Zilch fully compiles and informs all script editors to clear their errors
  DeclareEvent(ClearAllAnnotations);
}

class MethodDoc;
class DocumentResource;

struct Completion
{
  Completion();
  Completion(StringParam name);
  Completion(StringParam name, StringParam description);

  String Name;
  String Description;
  String SignaturePathType;
  Resource* AssociatedResource;
};

struct ParameterTip
{
  String Name;
  String Description;
  String Type;
};

struct CallTip
{
  String Description;
  Array<ParameterTip> Parameters;
  String Return;
};

DeclareEnum2(CompletionConfidence, Unsure, Perfect);

/// Implemented by the script editor to provide functionality to the code inspector
class ICodeEditor
{
public:

  /// Tells the text editor to show an auto-complete dialog at the current cursor position
  virtual void ShowAutoComplete(Array<Completion>& completions, CompletionConfidence::Enum confidence) = 0;

  /// If the option is enabled, we will add local word completions from the current document (or do nothing)
  virtual void AttemptAddLocalWordCompletions(Array<Completion>& completions) = 0;
  
  /// Tells the text editor to hide any auto-complete dialogs
  virtual void HideAutoComplete() = 0;

  /// Tells the text editor to show a call-tip at the current cursor position (supports overloads)
  virtual void ShowCallTips(Array<CallTip>& tips, StringParam functionName) = 0;
  
  /// Tells the text editor to hide any call-tips
  virtual void HideCallTips() = 0;

  /// Get all the text in the document
  virtual StringRange GetAllText() = 0;

  /// Get the associated document resource with this code editor
  virtual DocumentResource* GetDocumentResource() = 0;

  /// Get the line that the caret is on
  virtual size_t GetCaretLine() = 0;

  /// Get the number of lines
  virtual size_t GetLineCount() = 0;

  /// Get the text of the given line
  virtual String GetLineText(size_t line) = 0;

  /// Get the starting character position of the given line
  virtual size_t GetLinePosition(size_t line) = 0;

  /// Get the current caret position
  /// In the case that this is called when a key was typed, this will be the position after the added character
  virtual size_t GetCaretPosition() = 0;

  /// Tells the code editor to increase its indentation for the current line
  virtual void Indent(size_t line) = 0;

  /// Tells the code editor to decrease its indentation for the current line
  virtual void Unindent(size_t line) = 0;

  /// Get's the name of the document we're editing. Often times this can be used in place of a class name for dynamic languages
  virtual String GetDocumentDisplayName() = 0;

  /// Sorts call-tips from least to most parameters, and then alphabetically by type
  static void SortCallTips(Array<CallTip>& tips);

  /// Sorts completions alphabetically and removes duplicates (uses the first description it finds)
  static void SortAndFilterCompletions(Array<Completion>& completions);
};

DeclareBitField1(ArgumentOptions, Typeless);

/// Implemented by each of the languages to provide auto-complete and code assistance
class ICodeInspector
{
public:

  /// Occurs every time a key is typed
  virtual void OnCharacterAdded(ICodeEditor* editor, Rune rune) = 0;

  /// Occurs when the user manually toggles auto-complete
  virtual void OnShowAutoComplete(ICodeEditor* editor) = 0;

  /// Gets the single line comment token
  virtual String GetSingleLineCommentToken() = 0;

  /// Gets a list of keywords used in the language
  virtual void GetKeywords(Array<Completion>& keywordsOut) = 0;

  /// If the editor would like to start local word completion, we can tell it not to
  /// For example, after writing 'class _____' we don't want it to complete the class name
  virtual bool CanStartLocalWordCompletion(ICodeEditor* editor);

  /// Gets whether or not we support completion of Zero.Connect(...
  virtual bool SupportsZeroConnect();

  /// Attempts to go to the definition at the cursor position (type, member, variable, etc)
  virtual void AttemptGetDefinition(ICodeEditor* editor, size_t cursorPosition, CodeDefinition& definition);

  /// When generating the Zero.Connect function, where should we put it?
  /// Set positionOut to -1 if you are unable to find a position
  virtual void FindPositionToGenerateFunction(ICodeEditor* editor, int& positionOut, String& indent);

  /// Generate the definition of a function
  virtual String GenerateConnectCallEnd(StringParam functionName);

  /// Generate the first part of a definition of a function
  /// The cursor will be placed directly after this part
  /// Use the tab character to indicate indenting (will be replaced with the editor's settings)
  virtual String GenerateConnectFunctionStart(StringParam functionName, StringParam eventType);

  /// Generate the last part of a definition of a function
  /// The cursor will be placed directly before this part
  // Use the tab character to indicate indenting (will be replaced with the editor's settings)
  virtual String GenerateConnectFunctionEnd();

  /// Get the keyword right before our cursor (a keyword is [a-zA-Z][a-zA-Z0-9]*)
  /// This is useful for implementing 'CanStartLocalWordCompletion', to prevent completion after keywords like 'class'
  static StringRange GetPreviousKeyword(ICodeEditor* editor);

  /// Indents if the last character matches
  /// Returns true if it's handled, false otherwise
  static bool IndentOnNewlineWhenLastCharacterIs(ICodeEditor* editor, Rune added, Rune lookFor);

  /// Unindents when we type a certain character
  /// Returns true if it's handled, false otherwise
  static bool UnindentOnCharacter(ICodeEditor* editor, Rune added, Rune lookFor);

  /// Takes a meta method and populates a call tip (always attempts to look for a DocMethod first)
  static void AddCallTipFromMetaMethod(Array<CallTip>& tips, BoundType* owner, Function* method);
  
  /// Takes a meta method and populates a call tip (always attempts to look for a DocMethod first)
  static void AddCompletionsFromMetaType(Array<Completion>& completions, BoundType* type);

  /// Parses an argument list string into types and parameter names
  /// An argument list string takes the form of '(int lives, float speed)'
  static void AddCallTipParametersFromArgumentString(CallTip& tip, StringRange arguments, ArgumentOptions::Type options);
};

//------------------------------------------------------------ Document Resource
/// Resource that can be edited as text and is searchable.
class DocumentResource : public Resource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  DocumentResource();
  ~DocumentResource();

  //Resource interface
  ResourceEditType::Type GetEditType() override { return ResourceEditType::Text; }
  void UpdateContentItem(ContentItem* contentItem) override;
  void SetAndSaveData(StringRange data);

  /// What syntax editor is used for this text
  virtual String GetFormat() = 0;
  /// Gets a code inspector which is used for code completion and other code editing features
  virtual ICodeInspector* GetCodeInspector() { return nullptr; }
  /// Setup the Resource for searching and editing.
  void DocumentSetup(ResourceEntry& entry, bool searchable=true);
  /// Path this resource should use to load or save.
  String LoadPath;
  /// We need to only include a document once in a crash dump and to avoid allocations we
  /// store this on the document itself (it's only for crashes so whatever)
  bool IncludedInCrash;

  /// Get all the text
  virtual StringRange LoadTextData() = 0;
  /// Save the text
  virtual void ReloadData(StringRange data) = 0;
};

namespace Events
{
  DeclareEvent(Save);
  DeclareEvent(SaveCheck);
}

class SavingEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  SavingEvent() : NeedSaving(false){}
  bool NeedSaving;
  Status SaveStatus;
};

}//namespace Zero
