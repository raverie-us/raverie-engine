///////////////////////////////////////////////////////////////////////////////
///
/// \file ScriptEditor.hpp
/// Declaration of the ScriptEditor Widget.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Document;
class ScriptDebugEngine;
class DebugEngineEvent;
class ResourceDocument;
class WindowTabEvent;

namespace Events
{
  DeclareEvent(AutoCompleteItemDoubleClicked);
}

class DocumentEditor : public TextEditor
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  DocumentEditor(Composite* parent);
  ~DocumentEditor();
  
  void FocusWindow();
  void CloseWindow();

  virtual void Save();
  void SetDocument(Document* doc);
  void SaveDocument();
  void OnFocusIn() override;

  DocumentResource* GetResource();
  Document* GetDocument();

  void OnClearAllAnnotations(Event* event);
  void OnDocumentRemoved(Event* event);
  void OnDocumentReload(Event* event);
  void OnTextModified(Event* event);

  void OnSave(SavingEvent* event);
  void OnSaveCheck(SavingEvent* event);
  void OnTabFind(WindowTabEvent* event);

  void OnQueryModified(QueryModifiedSaveEvent* event);
  void OnSaveConfirmed(Event* event);

protected:
  void ReleaseDocument();
  Document* mDocument;
};

class CompletionDataSource : public ListSource
{
public:
  CompletionDataSource();

  void SetCompletions(Array<Completion>& completions);

  // DataSource Interface
  uint GetCount() override;
  String GetStringValueAt(DataIndex index) override;
  int Modified();

  // Internal
  Array<Completion> mAll;
  Array<Completion*> mFiltered;
  String mPartialText;
};

class AutoCompletePopUp : public PopUp
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  AutoCompletePopUp(Widget* source);
  
  void SetCompletions(Array<Completion>& completions);
  void PartialTextChanged(StringParam partial);

  void NextCompletion();
  void PreviousCompletion();
  String GetCurrentSelection();
  void SetConfidence(CompletionConfidence::Enum confidence);
  void PlaceOnScreen(Vec3 cursorOffset, float lineHeight);

  // Internal
  void OnItemSelected(ObjectEvent* event);
  void OnItemDoubleClicked(ObjectEvent* event);
  void OnDestroy() override;
  void ShowToolTip();
  CompletionDataSource mSource;
  ListBox* mList;
  HandleOf<PopUp> mToolTip;
  Completion* mLastCompletion;
  bool mSearchFailed;
  bool mManualSelection;
  CompletionConfidence::Enum mConfidence;
};

class CallTipPopUp : public PopUp
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CallTipPopUp(Widget* source);

  void SetTips(Array<CallTip>& tips, StringParam functionName);
  void NextTip();
  void PreviousTip();
  void SetParameterIndex(size_t index);
  bool HasOverloads();

  // Internal
  void UpdateTip();
  void OnPrevious(MouseEvent* event);
  void OnNext(MouseEvent* event);
  
  Element* mPreviousTip;
  Text* mOverloadIndex;
  Element* mNextTip;
  Text* mText;
  Array<CallTip> mTips;
  size_t mIndex;
  size_t mParameterIndex;
  String mFunctionName;
};

DeclareEnum2(UserCompletion, ExplitilyRequested, OnlyIfMatching);

class ScriptEditor : public DocumentEditor, public ICodeEditor
{
public:
  typedef ScriptEditor ZilchSelf;
  ScriptEditor(Composite* parent);
  ~ScriptEditor();
  void SetDebugEngine(ScriptDebugEngine* debugEngine);

  void Save() override;
  void OnKeyDown(KeyboardEvent* event) override;
  void OnKeyUp(KeyboardEvent* event);
  void OnDebugBreak(DebugEngineEvent* event);
  void OnDebugException(DebugEngineEvent* event);
  void OnCharacterAdded(TextEditorEvent* event);
  void OnRightClick(MouseEvent* event);
  void OnMouseDown(MouseEvent* event);
  void OnMouseScroll(MouseEvent* event);
  void OnMouseMove(MouseEvent* event);
  void OnAutoCompleteItemDoubleClicked(ObjectEvent* event);
  void CheckPopups();
  CallTipPopUp* GetCallTip();
  AutoCompletePopUp* GetAutoComplete();
  ICodeInspector* GetCodeInspector();
  bool AttemptFinishAutoComplete(UserCompletion::Enum mode);
  void ShowAutoComplete(Array<Completion>& tips, int cursorOffset, CompletionConfidence::Enum confidence);
  void AttemptAddLocalWordCompletions(Array<Completion>& completionsOut);
  void AttemptAddKeywordAndTypeCompletions(Array<Completion>& completionsOut);
  String ReplaceTabRuneWithOurTabStyle(StringParam text, StringParam perLineIndent = String());
  ToolTip* ShowToolTip(StringParam text);
  ToolTip* ShowToolTip(StringParam text, Vec3Param screenPos);
  void HideToolTip();
  bool GetCompleteZeroConnectInfo(String& eventNameOut, String& indentOut, int& functionPositionOut);
  bool CanCompleteZeroConnect();
  bool AutoCompleteZeroConnect();

  // ICodeEditor interface
  void ShowAutoComplete(Array<Completion>& tips, CompletionConfidence::Enum confidence) override;
  void HideAutoComplete() override;
  void ShowCallTips(Array<CallTip>& tips, StringParam functionName) override;
  void HideCallTips() override;
  StringRange GetAllText() override;
  DocumentResource* GetDocumentResource() override;
  size_t GetCaretLine() override;
  size_t GetLineCount() override;
  String GetLineText(size_t line) override;
  size_t GetLinePosition(size_t line) override;
  size_t GetCaretPosition() override;
  void Indent(size_t line) override;
  void Unindent(size_t line) override;
  String GetDocumentDisplayName() override;

  // TextEditor Interface
  void OnFocusIn() override;
  void OnFocusOut() override;
  ICodeEditor* GetCodeEditor() override { return this; }

  /// Formats the arrow line that points at the given
  /// character offset for an error message
  String FormatErrorMessage(StringParam message, int offset);

  /// Get the previous world on line.
  ScriptDebugEngine* mDebugEngine;
  HandleOf<CallTipPopUp> mCallTip;
  int mCallTipLine;
  int mCallTipStart;
  HandleOf<AutoCompletePopUp> mAutoComplete;
  int mAutoCompleteLine;
  int mAutoCompleteStart;
  bool mCharacterWasAdded;
  HandleOf<ToolTip> mToolTip;
};

ScriptEditor* CreateScriptEditor(Composite* parent, ResourceDocument* scriptDocument);
DocumentEditor* CreateDocumentEditor(Composite* parent, Document* document);

}
