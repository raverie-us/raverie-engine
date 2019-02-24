// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class Document;
class ResourceDocument;
class WindowTabEvent;

namespace Events
{
DeclareEvent(AutoCompleteItemDoubleClicked);
}

class DocumentEditor : public TextEditor
{
public:
  ZilchDeclareType(DocumentEditor, TypeCopyMode::ReferenceType);

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
  virtual void OnTextModified(Event* event);
  void OnResourceModified(ResourceEvent* event);

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
  void FilterHidden();

  // Internal
  Array<Completion> mAll;
  Array<Completion*> mFiltered;
  String mPartialText;
};

class AutoCompletePopUp : public PopUp
{
public:
  ZilchDeclareType(AutoCompletePopUp, TypeCopyMode::ReferenceType);

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
  ZilchDeclareType(CallTipPopUp, TypeCopyMode::ReferenceType);

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
  typedef DocumentEditor ZilchBase;
  ScriptEditor(Composite* parent);
  ~ScriptEditor();

  void Save() override;
  void OnKeyDown(KeyboardEvent* event) override;
  void OnKeyUp(KeyboardEvent* event);
  void OnCharacterAdded(TextEditorEvent* event);
  void OnRightClick(MouseEvent* event);
  void OnMouseDown(MouseEvent* event);
  void OnMouseScroll(MouseEvent* event);
  void OnMouseMove(MouseEvent* event);
  void OnAutoCompleteItemDoubleClicked(ObjectEvent* event);
  void OnTextModified(Event* event) override;
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
  void ShowCallTips(Array<CallTip>& tips, StringParam functionName, size_t parameterIndex) override;
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
  String GetOrigin() override;

  // TextEditor Interface
  void OnFocusIn() override;
  void OnFocusOut() override;
  ICodeEditor* GetCodeEditor() override
  {
    return this;
  }
  void BreakpointsClicked(int line, int position) override;
  void ScriptError(ScriptEvent* event) override;

  /// Formats the arrow line that points at the given
  /// character offset for an error message
  String FormatErrorMessage(StringParam message, int offset);

  /// Get the previous world on line.
  HandleOf<CallTipPopUp> mCallTip;
  int mCallTipLine;
  int mCallTipStart;
  HandleOf<AutoCompletePopUp> mAutoComplete;
  int mAutoCompleteLine;
  int mAutoCompleteStart;
  bool mCharacterWasAdded;
  HandleOf<ToolTip> mToolTip;
  CodeLocation mLastLocation;
};

ScriptEditor* CreateScriptEditor(Composite* parent, ResourceDocument* scriptDocument);
DocumentEditor* CreateDocumentEditor(Composite* parent, Document* document);

} // namespace Zero
