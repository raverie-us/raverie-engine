///////////////////////////////////////////////////////////////////////////////
///
/// \file ScriptEditor.cpp
/// Implementation of the ScriptEditor Widget.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(AutoCompletePopUp, builder, type)
{
}

ZilchDefineType(CallTipPopUp, builder, type)
{
}

const int cAutoCompleteFailed = -2;

const cstr cLocation = "EditorUi/ScriptEditor";
Tweakable(Vec4, AutoCompleteConfident, Vec4(0.3f, 1.0f, 0.3f, 1.0f), cLocation);
Tweakable(Vec4, AutoCompleteNonConfident, Vec4(0.9f, 0.3f, 0.3f, 1.0f), cLocation);
Tweakable(Vec2, CallTipTextSize, Vec2(400.0f, 400.0f), cLocation);
Tweakable(Vec2, AutoCompleteSize, Vec2(300.0f, 200.0f), cLocation);
Tweakable(Vec2, AutoCompleteTextSize, Vec2(300.0f, 200.0f), cLocation);

namespace Events
{
  DefineEvent(AutoCompleteItemDoubleClicked);
}

CompletionDataSource::CompletionDataSource()
{
}

uint CompletionDataSource::GetCount()
{
  return mFiltered.Size();
}

void CompletionDataSource::SetCompletions(Array<Completion>& completions)
{
  mAll = completions;
  Modified();
}

int CompletionDataSource::Modified()
{
  if (mPartialText.Empty())
  {
    mFiltered.Clear();

    for (size_t i = 0; i < mAll.Size(); ++i)
    {
      mFiltered.PushBack(&mAll[i]);
    }
    
    ListSource::Modified();
    return -1;
  }

  auto filterResults = FilterStrings(mAll.All(), mPartialText, [](Completion& completion) -> String
  {
    return completion.Name;
  });

  if (!filterResults.mFiltered.Empty())
  {
    mFiltered.Clear();
    for (size_t i = 0 ; i < filterResults.mFiltered.Size(); ++i)
    {
      auto completion = filterResults.mFiltered[i];

      mFiltered.PushBack(completion);
    }
    
    ListSource::Modified();
    return filterResults.mPrimaryFilteredIndex;
  }

  return cAutoCompleteFailed;
}

String CompletionDataSource::GetStringValueAt(DataIndex index)
{
  return mFiltered[(size_t)index.Id]->Name;
}

AutoCompletePopUp::AutoCompletePopUp(Widget* source)
  : PopUp(source, PopUpCloseMode::MouseDistance, PopUpLight)
{
  mConfidence = CompletionConfidence::Unsure;
  mManualSelection = false;
  mSearchFailed = true;
  mLastCompletion = nullptr;
  mToolTip = nullptr;
  mList = new ListBox(this);
  mList->SetDataSource(&mSource);

  ConnectThisTo(mList, Events::ItemSelected, OnItemSelected);
  ConnectThisTo(mList, Events::ItemDoubleClicked, OnItemDoubleClicked);

  this->SetSize(AutoCompleteSize);
  mList->SetSize(AutoCompleteSize);
}

void AutoCompletePopUp::OnDestroy()
{
  PopUp::OnDestroy();
  mToolTip.SafeDestroy();
}

void AutoCompletePopUp::SetCompletions(Array<Completion>& completions)
{
  // Let the list-box know that we set the completion data
  mSource.SetCompletions(completions);

  Vec2 size = mList->GetSizeWithItems(AutoCompleteSize.mValue.x, mList->GetCount());
  if (size.y > AutoCompleteSize.mValue.y)
    size.y = AutoCompleteSize.mValue.y;

  this->SetSize(size);
  mList->SetSize(size);
}

void AutoCompletePopUp::PartialTextChanged(StringParam partial)
{
  if (mSource.mPartialText != partial)
  {
    mSource.mPartialText = partial;
    int index = mSource.Modified();

    if (index == cAutoCompleteFailed)
    {
      // We need to gray out the entry of the list
      mSearchFailed = true;
      mList->SetSelectionColor(Vec4(0.5f));
    }
    else
    {
      mList->SetSelectedItem(index, true);
      mSearchFailed = false;
      mList->SetSelectionColor(Vec4(1.0f));
    }
    // This MUST come after 'SetSelectedItem' above, because in the selection event we set it to true
    mManualSelection = false;
  }
}

void AutoCompletePopUp::NextCompletion()
{
  int count = (int)mList->GetCount();
  int index = mList->GetSelectedItem();

  ++index;

  if (index >= count)
    index = 0;

  mList->SetSelectedItem(index, true);
}

void AutoCompletePopUp::PreviousCompletion()
{
  int count = (int)mList->GetCount();
  int index = mList->GetSelectedItem();

  --index;

  if (index < 0)
    index = count - 1;

  mList->SetSelectedItem(index, true);
}

void AppendResourcePreviewToToolTip(Composite* tooltip, Resource* resource, float topPadding)
{
  if (!resource)
    return;

  Vec2 size = tooltip->GetSize();

  // We specify high importance because there is no need to show an icon
  // or any preview for resources that don't support it
  PreviewWidget* preview = ResourcePreview::CreatePreviewWidget(
    tooltip,
    resource->Name,
    resource,
    PreviewImportance::High);
  
  if (preview)
  {
    preview->SetTranslation(Vec3(0, size.y, 0));
    preview->SetSize(Pixels(150, 150));

    Vec2 previewSize = preview->GetSize();
    size.y += previewSize.y + topPadding;
    size.x = Math::Max(size.x, previewSize.x);

    tooltip->SetSize(size);
  }
}

void AutoCompletePopUp::ShowToolTip()
{
  auto index = mList->GetSelectedItem();

  if (index < 0)
  {
    mToolTip.SafeDestroy();
    mLastCompletion = nullptr;
    return;
  }

  Completion* completion = mSource.mFiltered[index];

  if (completion == mLastCompletion && mToolTip)
    return;

  mToolTip.SafeDestroy();

  if (completion->SignaturePathType.Empty() && completion->Name.Empty())
  {
    mLastCompletion = completion;
    return;
  }

  StringBuilder tipBuilder;
  
  if (!completion->SignaturePathType.Empty() && !completion->Description.Empty())
  {
    tipBuilder.Append(completion->SignaturePathType);
    tipBuilder.Append("\n\n");
    tipBuilder.Append(completion->Description);
  }
  else if (!completion->SignaturePathType.Empty())
  {
    tipBuilder.Append(completion->SignaturePathType);
  }
  else
  {
    tipBuilder.Append(completion->Description);
  }

  auto config = Z::gEditor->mConfig->has(TextEditorConfig);

  bool allowSymbolCompletion = mConfidence == CompletionConfidence::Perfect && config->ConfidentAutoCompleteOnSymbols;

  if (allowSymbolCompletion)
  {
    if (config->AutoCompleteOnEnter)
    {
      tipBuilder.Append("\n(Tab/Enter/Symbols to complete)");
    }
    else
    {
      tipBuilder.Append("\n(Tab/Symbols to complete)");
    }
  }
  else
  {
    if (config->AutoCompleteOnEnter)
    {
      tipBuilder.Append("\n(Tab/Enter to complete)");
    }
    else
    {
      tipBuilder.Append("\n(Tab to complete)");
    }
  }

  String tipText = tipBuilder.ToString();

  if (tipText.Empty())
    return;

  auto tooltip = new PopUp(this, PopUpCloseMode::MouseDistance, PopUpLight);
  mToolTip = tooltip;
  
  tooltip->FadeIn();
  tooltip->SetTranslation(this->GetTranslation() + Vec3(this->GetSize().x, 0, 0));

  Thickness borderThickness = mBackground->GetBorderThickness();

  borderThickness = borderThickness + Thickness::All(5);

  Text* text = new Text(tooltip, cText);
  text->SetMultiLine(true);
  text->SetText(tipText);

  // The 2.0f is a little padding to the edge of the screen
  float distanceToRightEdgeOfScreen = GetRootWidget()->mSize.x - GetScreenRect().Right() - 2.0f;

  float width = Math::Min(AutoCompleteTextSize.mValue.x, distanceToRightEdgeOfScreen);

  text->FitToWidth(width, 1000000.0f);
  Vec2 size = text->GetSize();
  size = ExpandSizeByThickness(borderThickness, size);

  text->SetTranslation(Vec3(borderThickness.TopLeft()));

  tooltip->SetSize(size);

  Resource* resource = completion->AssociatedResource;
  AppendResourcePreviewToToolTip(tooltip, resource, borderThickness.Top);

  mLastCompletion = completion;
}

String AutoCompletePopUp::GetCurrentSelection()
{
  if (mList->GetSelectedItem() < 0)
  {
    return String();
  }
  else
  {
    return mSource.mFiltered[mList->GetSelectedItem()]->Name;
  }
}

void AutoCompletePopUp::SetConfidence(CompletionConfidence::Enum confidence)
{
  mConfidence = confidence;

  this->mList->mCustomBorderColor = true;
  if (confidence == CompletionConfidence::Perfect)
    this->mList->SetBorderColor(AutoCompleteConfident);
  else
    this->mList->SetBorderColor(AutoCompleteNonConfident);
}

void AutoCompletePopUp::PlaceOnScreen(Vec3 cursorOffset, float lineHeight)
{
  Vec2 screenSize = this->GetParent()->GetSize();
  Vec2 thisSize = this->GetSize();
  cursorOffset.y += lineHeight;

  if (cursorOffset.y + thisSize.y > screenSize.y)
    cursorOffset.y -= thisSize.y + lineHeight;

  if (cursorOffset.x + thisSize.x > screenSize.x)
    cursorOffset.x -= (cursorOffset.x + thisSize.x) - screenSize.x;

  this->SetTranslation(cursorOffset);
}

void AutoCompletePopUp::OnItemSelected(ObjectEvent* event)
{
  mList->ScrollToView();
  this->ShowToolTip();
  mManualSelection = true;
}

void AutoCompletePopUp::OnItemDoubleClicked(ObjectEvent* event)
{
  ObjectEvent objectEvent(this);
  this->DispatchEvent(Events::AutoCompleteItemDoubleClicked, &objectEvent);
}

CallTipPopUp::CallTipPopUp(Widget* source)
  :PopUp( source, PopUpCloseMode::MouseDistance, PopUpLight)
{
  mIndex = 0;
  mParameterIndex = 0;
  mOverloadIndex = new Text(this, cText);
  mText = new Text(this, cText);
  mText->SetMultiLine(true);

  mPreviousTip = CreateAttached<Element>("ArrowUp");
  mNextTip = CreateAttached<Element>("ArrowDown");

  this->FadeIn();

  ConnectThisTo(mPreviousTip, Events::LeftMouseDown, OnPrevious);
  ConnectThisTo(mNextTip, Events::LeftMouseDown, OnNext);
}

void CallTipPopUp::OnPrevious(MouseEvent* event)
{
  PreviousTip();
}

void CallTipPopUp::OnNext(MouseEvent* event)
{
  NextTip();
}

void CallTipPopUp::SetTips(Array<CallTip>& tips, StringParam functionName)
{
  mTips = tips;
  mIndex = 0;
  mFunctionName = functionName;
  UpdateTip();
}

void CallTipPopUp::NextTip()
{
  ++mIndex;

  if (mIndex == mTips.Size())
  {
    mIndex = 0;
  }

  UpdateTip();
}

void CallTipPopUp::PreviousTip()
{
  if (mIndex == 0)
  {
    mIndex = mTips.Size() - 1;
  }
  else
  {
    --mIndex;
  }

  UpdateTip();
}

void CallTipPopUp::SetParameterIndex(size_t index)
{
  ErrorIf(mTips.Empty(), "Attempting to set a parameter index when we have no call tips");

  if (mParameterIndex != index)
  {
    mParameterIndex = index;
    UpdateTip();
  }
}

void CallTipPopUp::UpdateTip()
{
  ReturnIf(mTips.Empty(),, "Attempting to update the call tips dialog when no tips were added");

  auto& tip = mTips[mIndex];

  auto overloadText = String::Format("%d of %d", mIndex + 1, mTips.Size());

  StringBuilder builder;

  builder.Append(mFunctionName);
  builder.Append("(");
  
  String currentParamDescription;
  String currentParamName;

  auto paramCount = tip.Parameters.Size();
  for(size_t i = 0; i < paramCount; ++i)
  {
    auto& param = tip.Parameters[i];
    bool isNotLast = (i != (paramCount - 1));

    auto paramName = param.Name;
    
    if (paramName.Empty())
    {
      if (paramCount == 1)
      {
        paramName = "value";
      }
      else
      {
        paramName = String::Format("p%d", i);
      }
    }

    if(mParameterIndex == i)
    {
      builder.Append("[");
      builder.Append(paramName);
      builder.Append("]");
      currentParamDescription = param.Description;
      currentParamName = paramName;
    }
    else
    {
      builder.Append(paramName);
    }

    if (!param.Type.Empty())
    {
      builder.Append(" : ");
      builder.Append(param.Type);
    }

    if (isNotLast)
    {
      builder.Append(", ");
    }
  }

  builder.Append(")");

  if (!tip.Return.Empty())
  {
    builder.Append(" : ");
    builder.Append(tip.Return);
  }

  if(!tip.Description.Empty())
  {
    builder.Append("\n\n");
    builder.Append(tip.Description);
  }

  if(!currentParamDescription.Empty())
  {
    builder.Append("\n\n");
    builder.Append(currentParamName);
    builder.Append(": ");
    builder.Append(currentParamDescription);
  }

  auto text = builder.ToString();
  mText->SetText(text);
  
  Thickness borderThickness = mBackground->GetBorderThickness();
  borderThickness = borderThickness + Thickness::All(5);

  float x = borderThickness.Left;
  float y = borderThickness.Top;

  if (HasOverloads())
  {
    mPreviousTip->SetActive(true);
    mOverloadIndex->SetActive(true);
    mNextTip->SetActive(true);
      
    mPreviousTip->SetTranslation(Vec3(x, y, 0));
    x += mPreviousTip->GetSize().x;

    mOverloadIndex->SetText(overloadText);
    Vec2 size = mOverloadIndex->GetMinSize();
    mOverloadIndex->SetTranslation(Vec3(x, y, 0));
    mOverloadIndex->SetSize(size);
    x += size.x;
    mNextTip->SetTranslation(Vec3(x, y, 0));
    x += mNextTip->GetSize().x + Pixels(5);
  }
  else
  {
    mPreviousTip->SetActive(false);
    mOverloadIndex->SetActive(false);
    mNextTip->SetActive(false);
  }
 
  mText->SetTranslation(Vec3(x, y, 0));
  mText->FitToWidth(CallTipTextSize.mValue.x, CallTipTextSize.mValue.y);

  Vec2 textMinSize = mText->GetSize();
  Vec2 popupSize = textMinSize;
  popupSize.x += x;
  popupSize = ExpandSizeByThickness(borderThickness, popupSize);
  this->SetSize(popupSize);
}

bool CallTipPopUp::HasOverloads()
{
  return mTips.Size() > 1;
}

ZilchDefineType(DocumentEditor, builder, type)
{
}

DocumentEditor::DocumentEditor(Composite* composite)
  :TextEditor(composite)
{
  DocumentManager* docManager = DocumentManager::GetInstance();
  mDocument = nullptr;
  docManager->Instances.PushBack(this);
  ConnectThisTo(this, Events::TextEditorModified, OnTextModified);
  ConnectThisTo(this, Events::TabFind, OnTabFind);
  ConnectThisTo(this, Events::QueryModifiedSave, OnQueryModified);
  ConnectThisTo(this, Events::ConfirmModifiedSave, OnSaveConfirmed);

  SetBackspaceUnindents(true);
  EnableScrollPastEnd(true);
}

DocumentEditor::~DocumentEditor()
{
  ReleaseDocument();
  DocumentManager* docManager = DocumentManager::GetInstance();
  docManager->Instances.EraseValueError(this);
  if(docManager->CurrentEditor == this)
    docManager->CurrentEditor = nullptr;
}

void DocumentEditor::FocusWindow()
{
  ShowWidget(this);
  this->TakeFocus();
}

void DocumentEditor::CloseWindow()
{
  CloseTabContaining(this);
}

void DocumentEditor::OnClearAllAnnotations(Event* event)
{
  ClearAnnotations();
}

void DocumentEditor::OnDocumentRemoved(Event* event)
{
  ReleaseDocument();
  SetAllText(StringRange(), false);
  ClearUndo();
  CloseWindow();
}

void DocumentEditor::ReleaseDocument()
{
  if(mDocument)
  {
    DocumentResource* resource = mDocument->GetResource();
    if(resource != nullptr)
      resource->GetDispatcher()->Disconnect(this);

    mDocument->mEditor = nullptr;
    --mDocument->mEditCounter;
    mDocument->GetDispatcher()->Disconnect(this);
    mDocument = nullptr;
  }
}

void DocumentEditor::SetDocument(Document* doc)
{
  ReleaseDocument();
  
  ConnectThisTo(doc, Events::DocumentRemoved, OnDocumentRemoved);
  ConnectThisTo(doc, Events::DocumentReload, OnDocumentReload);

  DocumentResource* resource = doc->GetResource();
  if(resource != nullptr)
    ConnectThisTo(resource->GetManager(), Events::ClearAllAnnotations, OnClearAllAnnotations);

  mDocument = doc;
  ++mDocument->mEditCounter;
  mDocument->mEditor = this;

  OnDocumentReload(nullptr);

  DocumentManager* manager = DocumentManager::GetInstance();

  // We just set the document, make us into the current document
  manager->CurrentEditor = this;
}

void DocumentEditor::OnTextModified(Event* event)
{
  FindTextDialog::Instance->ClearContext();

  TabModifiedEvent e(IsModified());
  GetDispatcher()->Dispatch(Events::TabModified, &e);
}

void DocumentEditor::OnDocumentReload(Event* event)
{
  String alltext = mDocument->GetTextData();
  SetAllText(alltext, false);
  ClearUndo();
}

DocumentResource* DocumentEditor::GetResource()
{
  if(mDocument)
    return mDocument->GetResource();
  else
    return nullptr;
}

Document* DocumentEditor::GetDocument()
{
  return mDocument;
}

void DocumentEditor::SaveDocument()
{
  if(mDocument)
  {
    StringRange text = GetAllText();
    mDocument->Save(text);
    SetSavePoint();

    TabModifiedEvent e(false);
    GetDispatcher()->Dispatch(Events::TabModified, &e);
  }
}

void DocumentEditor::Save()
{
  SaveDocument();
}

void DocumentEditor::OnSave(SavingEvent* event)
{
  Save();
}

void DocumentEditor::OnSaveCheck(SavingEvent* event)
{
  if(this->IsModified())
    event->NeedSaving = true;
}

void DocumentEditor::OnTabFind(WindowTabEvent* event)
{
  // Skip checking if there is no valid object
  if(event->SearchObject.IsNull())
    return;

  Resource* resource = GetResource();
  if(event->SearchObject == resource)
    event->TabWidgetFound = this;
  if(event->SearchObject == mDocument)
    event->TabWidgetFound = this;
}

void DocumentEditor::OnQueryModified(QueryModifiedSaveEvent* event)
{
  event->Modified = this->IsModified();
  event->Title = String("Save and Close");
  event->Message = String("Save changes to the current document before closing it?");
}

void DocumentEditor::OnSaveConfirmed(Event* event)
{
  SaveDocument();
}

void DocumentEditor::OnFocusIn()
{
  if (this->mDocument != nullptr)
  {
    DocumentManager* manger = DocumentManager::GetInstance();
    manger->CurrentEditor = this;
  }
}

ScriptEditor::ScriptEditor(Composite* parent)
  :DocumentEditor(parent)
{
  mDebugEngine = nullptr;
  mCharacterWasAdded = false;

  mCallTipLine = 0;
  mCallTipStart = 0;
  mAutoCompleteLine = 0;
  mAutoCompleteStart = 0;

  ConnectThisTo(this, Events::KeyUp, OnKeyUp);
  ConnectThisTo(this, Events::CharacterAdded, OnCharacterAdded);
  ConnectThisTo(this, Events::LeftMouseDown, OnMouseDown);
  ConnectThisTo(this, Events::MouseScroll, OnMouseScroll);
  ConnectThisTo(this, Events::RightClick, OnRightClick);
  ConnectThisTo(this, Events::MouseMove, OnMouseMove);
}

ScriptEditor::~ScriptEditor()
{
}

const int InstructionMarker = 2;
const int BreakPointMarker = 1;

void ScriptEditor::OnDebugBreak(DebugEngineEvent* event)
{
  ClearMarker(-1, InstructionMarker);
  // METAREFACTOR Confirm this should subtract 1 from PrimaryLine (PrimaryLine starts at 1, 0 is invalid)
  GoToLine(event->Location.PrimaryLine - 1);
  SetMarker(event->Location.PrimaryLine - 1, InstructionMarker);
}

void ScriptEditor::OnFocusOut()
{
  DocumentEditor::OnFocusOut();
}

void ScriptEditor::OnFocusIn()
{
  DocumentEditor::OnFocusIn();
}

void ScriptEditor::OnDebugException(DebugEngineEvent* event)
{
  if( mDocument->GetPath() == event->Location.Origin )
  {
    String errMsg = FormatErrorMessage(event->Message,event->Location.PrimaryCharacter);
    this->SetAnnotation(event->Location.PrimaryLine - 1, errMsg);

    // If the error is a runtime clear Focus to prevent any keys
    // held down from typing in the editor
    if(event->EventId != Events::SyntaxError)
      this->LoseFocus();
  }
}

CodeLocation GetCurrentLocation(ScriptEditor* editor)
{
  CodeLocation location;
  location.Origin = editor->GetDocument()->GetPath();
  // METAREFACTOR - Confirm this should be + 1
  location.PrimaryLine = editor->GetCurrentLine() + 1;
  return location;
}

void ScriptEditor::SetDebugEngine(ScriptDebugEngine* debugEngine)
{
  mDebugEngine = debugEngine;
  ///ConnectThisTo(mDebugEngine, Events::DebugBreak, OnDebugBreak);
  //ConnectThisTo(mDebugEngine, Events::DebugException, OnDebugException);
}

void ScriptEditor::OnKeyUp(KeyboardEvent* event)
{
  CheckPopups();
}

String ScriptEditor::ReplaceTabRuneWithOurTabStyle(StringParam text, StringParam perLineIndent)
{
  String newTab = GetTabStyleAsString();

  StringBuilder builder;

  StringRange textTemp = text;
  for (; !textTemp.Empty(); textTemp.PopFront())
  {
    Rune r = textTemp.Front();

    // Append indents for every new line
    if (r == '\f')
    {
      builder.Append(perLineIndent);
    }
    // If we encounter the actual tab character, replace it with our tab style
    else if (r == '\t')
    {
      builder.Append(newTab);
    }
    // Otherwise just add the character as is
    else
    {
      builder.Append(r);
    }
  }

  return builder.ToString();
}

void ScriptEditor::OnKeyDown(KeyboardEvent* event)
{
  this->HideToolTip();

  // Quick escape to clear errors
  if(event->Key == Keys::Escape)
  {
    this->ClearAnnotations();
    this->HideAutoComplete();
    this->HideCallTips();
  }

  // Command processing
  if(event->CtrlPressed)
  {
    ICodeInspector* code = this->GetCodeInspector();
    if (code && (event->Key == 'K'))
    {
      this->BlockComment(code->GetSingleLineCommentToken().c_str());
    }

    // Debugger commands
    if(mDebugEngine)
    {
      if(event->Key == Keys::Up)
        mDebugEngine->SetBreakMode(BreakMode::BreakNormal, 0);
      if(event->Key == Keys::Down)
        mDebugEngine->SetBreakMode(BreakMode::BreakOnStack, 0);
      if(event->Key == Keys::Right)
        mDebugEngine->SetBreakMode(BreakMode::BreakOnStack, 1);
      if(event->Key == Keys::Left)
        mDebugEngine->SetBreakMode(BreakMode::BreakOnStack, -1);

      ClearMarker(-1, InstructionMarker);
    }
  }
  
  auto autoCompletion = GetAutoComplete();
  if(autoCompletion)
  {
    // If we have more than one tip...
    if (event->Key == Keys::Down)
    {
      autoCompletion->NextCompletion();
      return;
    }
    else if (event->Key == Keys::Up)
    {
      autoCompletion->PreviousCompletion();
      return;
    }
    // Always complete on tab!
    else if (event->Key == Keys::Tab)
    {
      if (this->AttemptFinishAutoComplete(UserCompletion::ExplitilyRequested))
        return;
    }
    // If we have the option enabled to auto-complete on pressing enter...
    else if (event->Key == Keys::Enter)
    {
      if (GetConfig()->AutoCompleteOnEnter)
      {
        if (this->AttemptFinishAutoComplete(UserCompletion::ExplitilyRequested))
          return;
      }
    }
  }

  if (event->Key == Keys::Tab)
  {
    if (this->AutoCompleteZeroConnect())
      return;
  }
  
  auto tip = GetCallTip();
  if(tip && (event->Key == Keys::Down || event->Key == Keys::Up))
  {
    // If we have more than one tip...
    if(tip->mTips.Size() > 1)
    {
      if (event->Key == Keys::Down)
      {
        tip->NextTip();
      }
      else
      {
        tip->PreviousTip();
      }

      // We handled this message, so don't pass it on to Scintilla
      return;
    }
  }

  DocumentEditor::OnKeyDown(event);

  //if(event->Key == KeyCodes::F6)
  //{
  //  DebugValue value;
  //  mDebugEngine->EvaluateIdentifier(value, "self");
  //}

  //if(event->Key == KeyCodes::F5)
  //{
  //  CodeLocation l = GetCurrentLocation(this);
  //  mDebugEngine->SetBreakPoint(l);
  //  this->SetMarker(l.LineNumber, BreakPointMarker);
  //}
}

void ScriptEditor::ShowAutoComplete(Array<Completion>& tips, CompletionConfidence::Enum confidence)
{
  ShowAutoComplete(tips, 0, confidence);
}

void ScriptEditor::OnAutoCompleteItemDoubleClicked(ObjectEvent* event)
{
  this->AttemptFinishAutoComplete(UserCompletion::ExplitilyRequested);
}

void ScriptEditor::ShowAutoComplete(Array<Completion>& completions, int cursorOffset, CompletionConfidence::Enum confidence)
{
  if (completions.Empty())
    return;

  ICodeEditor::SortAndFilterCompletions(completions);

  AutoCompletePopUp* popup = new AutoCompletePopUp(this->GetParent());

  popup->SetConfidence(confidence);

  ConnectThisTo(popup, Events::AutoCompleteItemDoubleClicked, OnAutoCompleteItemDoubleClicked);

  mAutoComplete.SafeDestroy();

  mAutoComplete = popup;
  mAutoCompleteLine = GetCurrentLine();
  mAutoCompleteStart = GetCurrentPosition() + cursorOffset;

  popup->SetCompletions(completions);

  Vec3 cursorScreenPos = GetScreenPositionOfCursor();
  popup->PlaceOnScreen(cursorScreenPos, (float)GetLineHeight());
  popup->UpdateTransformExternal();
}

bool ScriptEditor::AttemptFinishAutoComplete(UserCompletion::Enum mode)
{
  auto autoComplete = GetAutoComplete();

  if (autoComplete)
  {
    if (autoComplete->mSearchFailed && mode == UserCompletion::OnlyIfMatching)
    {
      this->HideAutoComplete();
      return false;
    }

    String text = autoComplete->GetCurrentSelection();

    if (text.Empty())
    {
      this->HideAutoComplete();
      return false;
    }

    // If the completion is NOT confident, and the partial text they typed in is LONGER than the
    // current completion selection, then just let the text they typed be there
    String& partialText = autoComplete->mSource.mPartialText;
    if (!partialText.Empty() && autoComplete->mConfidence == CompletionConfidence::Unsure)
    {
      if (partialText.SizeInBytes() > text.SizeInBytes())
      {
        this->HideAutoComplete();
        return false;
      }
    }

    int position = GetCurrentPosition();
    int added = 0;

    if (mCharacterWasAdded)
    {
      added = 1;
    }

    position -= added;

    this->InsertAutoCompleteText(text.c_str(), text.SizeInBytes(), position - mAutoCompleteStart, added);
    //this->RemoveRange(mAutoCompleteStart, position - mAutoCompleteStart);
    //this->InsertText(mAutoCompleteStart, text.c_str());
    //this->SetSelectionStartAndLength(mAutoCompleteStart + text.Size() + added, 0);

    this->HideAutoComplete();
    return true;
  }

  return false;
}

void ScriptEditor::HideAutoComplete()
{
  mAutoComplete.SafeDestroy();
}

void ScriptEditor::HideToolTip()
{
  mToolTip.SafeDestroy();
}

ToolTip* ScriptEditor::ShowToolTip(StringParam text)
{
  Vec3 cursorScreenPos = GetScreenPositionOfCursor();
  cursorScreenPos += Pixels(4, 10, 0);

  return ShowToolTip(text, cursorScreenPos);
}

ToolTip* ScriptEditor::ShowToolTip(StringParam text, Vec3Param screenPos)
{
  ToolTip* tip = new ToolTip(this->GetParent());
  tip->SetText(text);

  mToolTip.SafeDestroy();

  mToolTip = tip;

  tip->SetArrowTipTranslation(screenPos);
  return tip;
}

void ScriptEditor::ShowCallTips(Array<CallTip>& tips, StringParam functionName)
{
  if (tips.Empty())
    return;

  ICodeEditor::SortCallTips(tips);

  CallTipPopUp* popup = new CallTipPopUp(this->GetParent());

  mCallTip.SafeDestroy();

  mCallTip = popup;
  mCallTipLine = GetCurrentLine();
  mCallTipStart = GetCurrentPosition();

  popup->SetTips(tips, functionName);

  Vec3 cursorScreenPos = GetScreenPositionOfCursor();
  cursorScreenPos.y += GetLineHeight();
  popup->SetTranslation(cursorScreenPos);
}

void ScriptEditor::HideCallTips()
{
  mCallTip.SafeDestroy();
}

StringRange ScriptEditor::GetAllText()
{
  return DocumentEditor::GetAllText();
}

DocumentResource* ScriptEditor::GetDocumentResource()
{
  return GetResource();
}

size_t ScriptEditor::GetCaretLine()
{
  return (size_t)GetCurrentLine();
}
size_t ScriptEditor::GetLineCount()
{
  return TextEditor::GetLineCount();
}

String ScriptEditor::GetLineText(size_t line)
{
  return DocumentEditor::GetLineText(line);
}

size_t ScriptEditor::GetLinePosition(size_t line)
{
  return DocumentEditor::GetPositionFromLine(line);
}

size_t ScriptEditor::GetCaretPosition()
{
  return GetCurrentPosition();
}

void ScriptEditor::Indent(size_t line)
{
  String newTab = GetTabStyleAsString();

  int linePosition = this->GetPositionFromLine(line);

  InsertText(linePosition, newTab.c_str());
}

void ScriptEditor::Unindent(size_t line)
{
  int tabWidth = GetTabWidth();

  int lineIndentation = GetLineIndentation(line);
  
  // Don't unindent if we have no indenting to do...
  if(lineIndentation < tabWidth)
    return;

  int linePosition = this->GetPositionFromLine(line);
  this->RemoveRange(linePosition, tabWidth);
}

// Gets the name of the document we're editing. Often times this can be used in place of a class name for dynamic languages
String ScriptEditor::GetDocumentDisplayName()
{
  if (mDocument)
  {
    return mDocument->GetDisplayName();
  }

  return String();
}

CallTipPopUp* ScriptEditor::GetCallTip()
{
  return mCallTip;
}

AutoCompletePopUp* ScriptEditor::GetAutoComplete()
{
  return mAutoComplete;
}

ICodeInspector* ScriptEditor::GetCodeInspector()
{
  if (mDocument)
  {
    return mDocument->GetCodeInspector();
  }
  return nullptr;
}

void ScriptEditor::CheckPopups()
{
  CallTipPopUp* tip = GetCallTip();
  if(tip)
  {
    size_t cursorPosition = GetCurrentPosition();
    size_t currentLine = GetCurrentLine();

    bool lessThanStart = (int)cursorPosition < mCallTipStart;
    bool cursorMovedOffLine = currentLine != mCallTipLine;
    
    if(lessThanStart || cursorMovedOffLine)
    {
      HideCallTips();
    }
    else
    {
      size_t linePosition = GetPositionFromLine(currentLine);
      size_t cursorColumn = cursorPosition - linePosition;
      StringRange lineText = GetLineText(currentLine);

      // Just for safety we make sure the cursor column is within the line text size
      // The -1 is because the cursor is because indexing the line at that
      // value gets the character to the right instead of the left
      cursorColumn = Math::Min(cursorColumn - 1, lineText.ComputeRuneCount() - 1);


      // Closers are any type of symbol such as [] {} ()
      int closerCount = 0;
      size_t parameterIndex = 0;

      // Walk backwards through the text looking for commas
      // We have to be careful because of nested function calls / types
      for (;!lineText.Empty(); lineText.PopBack())
      {
        Rune r = lineText.Back();

        if (r == ',' && closerCount == 0)
        {
          ++parameterIndex;
        }
        else if (r == ']' || r == '}' || r == ')')
        {
          ++closerCount;
        }
        else if (r == '[' || r == '{' || r == '(')
        {
          --closerCount;

          if (closerCount == -1)
          {
            break;
          }
        }
      }

      tip->SetParameterIndex(parameterIndex);
    }
  }

  auto autoComplete = GetAutoComplete();
  if(autoComplete)
  {
    size_t cursorPosition = GetCurrentPosition();
    size_t currentLine = GetCurrentLine();

    bool hideAutoComplete;
    // the cursor must be before the auto complete start to hide the box during perfect confidence
    if(autoComplete->mConfidence == CompletionConfidence::Perfect)
      hideAutoComplete = (int)cursorPosition < mAutoCompleteStart;
    // not perfect completion should be cleared when returning to the start otherwise starting letter
    // changes won't pop up a properly populated local auto complete
    else
      hideAutoComplete = (int)cursorPosition <= mAutoCompleteStart;
    
    bool cursorMovedOffLine = currentLine != mAutoCompleteLine;

    if(hideAutoComplete || cursorMovedOffLine)
    {
      HideAutoComplete();
    }
    else
    {
      auto partialText = this->GetText(mAutoCompleteStart, (int)cursorPosition);
      autoComplete->PartialTextChanged(partialText);
    }
  }
}


void ScriptEditor::OnMouseDown(MouseEvent* event)
{
  this->CheckPopups();
  this->HideToolTip();
}

void ScriptEditor::OnMouseScroll(MouseEvent* event)
{
  this->HideCallTips();
  this->HideAutoComplete();
  this->HideToolTip();
}

void ScriptEditor::OnMouseMove(MouseEvent* event)
{
  DocumentResource* resource = GetResource();
  
  if (resource == nullptr)
    return;
  
  ICodeInspector* codeInspector = resource->GetCodeInspector();
  
  if (codeInspector == nullptr)
    return;
  
  int cursor = GetCursorFromScreenPosition(event->Position);

  if (cursor == -1)
  {
    mToolTip.SafeDestroy();
    return;
  }
  
  CodeDefinition definition;
  codeInspector->AttemptGetDefinition(this, cursor, definition);
  
  if (!definition.ToolTip.Empty())
  {
    ToolTip* tooltip = ShowToolTip(definition.ToolTip, Vec3(event->Position));

    const void* codeUserData = definition.NameLocation.CodeUserData;
    if (codeUserData)
      AppendResourcePreviewToToolTip(tooltip, (Resource*)resource, 4);

    ToolTipPlacement placement;
    placement.mHotSpot = Math::ToVector2(GetScreenPositionFromCursor(cursor)) + Vec2(0, (float)GetLineHeight() * 0.5f);
    placement.mScreenRect = Rect::CenterAndSize(placement.mHotSpot, Vec2(200, (float)GetLineHeight() + 2));
    placement.SetPriority(IndicatorSide::Bottom, IndicatorSide::Top, IndicatorSide::Right, IndicatorSide::Left);
    tooltip->SetArrowTipTranslation(placement);

    tooltip->UpdateTransformExternal();
  }
}

void ScriptEditor::OnRightClick(MouseEvent* event)
{
  ContextMenu* contextMenu = new ContextMenu(this->GetParent());
  contextMenu->AddCommandByName("GoToDefinition");
  contextMenu->SizeToContents();
  contextMenu->SetBelowMouse(event->GetMouse(), Pixels(-1, -1));
  event->GetMouse()->SetCursor(Cursor::Arrow);
}

void ScriptEditor::OnCharacterAdded(TextEditorEvent* event)
{
  this->HideToolTip();
  this->ClearAnnotations();

  mCharacterWasAdded = true;
  // Finish off any auto-complete dialogs when the user types an invalid key
  AutoCompletePopUp* autoComplete = GetAutoComplete();
  static const String doNotCompleteCharacters = "_[]\r\n";
  Rune addedRune = Rune(event->Added);
  if (autoComplete && !IsAlphaNumeric(addedRune) && doNotCompleteCharacters.FindFirstOf(addedRune).Empty())
  {
    // If we allow confident completion on other symbols, and we have a confident auto-complete
    bool shouldFinish = GetConfig()->ConfidentAutoCompleteOnSymbols &&
      autoComplete->mConfidence == CompletionConfidence::Perfect;

    if (shouldFinish)
    {
      this->AttemptFinishAutoComplete(UserCompletion::OnlyIfMatching);
    }
    else
    {
      this->HideAutoComplete();
    }
  }

  
  bool isInComment = false;
  bool isInString = false;
  bool isInCommentOrString = false;

  ICodeInspector* code = this->GetCodeInspector();
  if (code)
  {
    int curPos = GetCurrentPosition();
    int linePos = GetLinePosition(GetCurrentLine());

    String comment = code->GetSingleLineCommentToken();
    String currentLine = GetCurrentLineText();
    StringIterator start = currentLine.Begin();
    StringIterator end = start + (curPos - linePos);
    StringRange currentLineRange = currentLine.SubString(start, end);
    String line = currentLineRange;

    bool previousWasEscape = false;

    for (;!currentLineRange.Empty(); currentLineRange.PopFront())
    {
      Rune r = currentLineRange.Front();

      if (r == '\\')
      {
        previousWasEscape = !previousWasEscape;
      }
      else
      {
        if (!previousWasEscape && (r == '"' || r == '`'))
          isInString = !isInString;

        previousWasEscape = false;
      }

      if (!isInString)
      {
        previousWasEscape = false;
      }
    }

    if (line.Contains(comment))
    {
      isInComment = true;
    }

    isInCommentOrString = isInComment || isInString;

    if (!isInCommentOrString)
    {
      code->OnCharacterAdded(this, addedRune);
    }
  }
  mCharacterWasAdded = false;

  // We could have made a new dialog above...
  autoComplete = GetAutoComplete();
  if (autoComplete == nullptr && !isInCommentOrString)
  {
    // Caret positions of word being edited
    int curPos = GetCurrentPosition();
    int startPos = GetWordStartPosition(curPos);
    int delimiter = GetRuneAt(curPos - 1);

    // Word completion
    if (IsAlpha(addedRune) || addedRune == '_')
    {
      bool doWordCompletion = true;

      if(code)
      {
        doWordCompletion = code->CanStartLocalWordCompletion(this);
      }
      if(doWordCompletion)
      {
        Array<Completion> completions;
        AttemptAddLocalWordCompletions(completions);
        AttemptAddKeywordAndTypeCompletions(completions);

        // Show the auto-complete, but since the character was just added we need to backup by one
        this->ShowAutoComplete(completions, startPos - curPos, CompletionConfidence::Unsure);
      }
    }
  }

  if (code && (addedRune == ' ' || addedRune == ','))
  {
    if (this->CanCompleteZeroConnect())
    {
      this->ShowToolTip("Press Tab to add event handler");
    }
  }

  //Auto indent feature
  if (addedRune == '\n')
  {
    Array<int> caretPositions;
    GetAllCaretPositions(caretPositions);
    Sort(caretPositions.All());

    int insertedCharCount = 0;
    for (size_t i = 0; i < caretPositions.Size(); ++i)
    {
      int caret = caretPositions[i] + insertedCharCount;
      int line = GetLineFromPosition(caret);
      int length = GetLineLength(line);

      if (line > 0)
      {
        StringRange prevLine = GetLineText(line - 1);
        StringIterator prevlineBegin = prevLine.Begin();

        String initialSpace;

        for (;!prevLine.Empty(); prevLine.PopFront())
        {
          Rune r = prevLine.Front();

          if (!UTF8::IsWhiteSpace(r) || r == '\n' || r == '\r')
          {
            initialSpace = StringRange(prevlineBegin, prevLine.Begin());
            break;
          }
        }
        
        InsertText(caret, initialSpace.Data());
        insertedCharCount += initialSpace.ComputeRuneCount();
      }
    }
  }

  CheckPopups();
}

String AlphaNumericScrub(StringRange input)
{
  StringBuilder builder;
  while (!input.Empty())
  {
    Rune r = input.Front();
    if (IsAlphaNumeric(r))
    {
      builder.Append(r);
    }
    input.PopFront();
  }

  return builder.ToString();
}

bool ScriptEditor::GetCompleteZeroConnectInfo(String& eventNameOut, String& indentOut, int& functionPositionOut)
{
  functionPositionOut = -1;

  ICodeInspector* code = this->GetCodeInspector();
  if (code && code->SupportsZeroConnect())
  {
    String line = GetCurrentLineText();
    Regex zeroConnect("Zero.Connect\\(.+,\\s*Events\\.([a-zA-Z0-9]+)\\s*,\\s*$");
    Matches matches;
    zeroConnect.Search(line, matches);

    if (matches.Size() != 2)
    {
      Regex zeroConnectString("Zero.Connect\\(.+,\\s*\"([a-zA-Z0-9]+)\"\\s*,\\s*$");
      zeroConnectString.Search(line, matches);

      if (matches.Size() != 2)
      {
        Regex zeroConnectString("Zero.Connect\\(.+,\\s*.*\\.([a-zA-Z0-9]+)\\s*,\\s*$");
        zeroConnectString.Search(line, matches);

        if (matches.Size() != 2)
        {
          Regex zeroConnectString("Zero.Connect\\(.+,\\s*([^,;]*)\\s*,\\s*$");
          zeroConnectString.Search(line, matches);
        }
      }
    }

    if (matches.Size() == 2)
    {
      eventNameOut = AlphaNumericScrub(matches[1]);

      if (eventNameOut.Empty())
      {
        return false;
      }
      
      code->FindPositionToGenerateFunction(this, functionPositionOut, indentOut);

      ReturnIf(functionPositionOut <= GetCurrentPosition(), false,
        "The function must be inserted after the cursor (below)");

      if (functionPositionOut != -1)
      {
        return true;
      }
    }
  }

  return false;
}

bool ScriptEditor::CanCompleteZeroConnect()
{
  String eventName;
  String indent;
  int functionPosition;
  return GetCompleteZeroConnectInfo(eventName, indent, functionPosition);
}

bool ScriptEditor::AutoCompleteZeroConnect()
{
  String eventName;
  String indent;
  int functionPosition;
  if (GetCompleteZeroConnectInfo(eventName, indent, functionPosition))
  {
    this->StartUndo();

    // We know this is always valid since the
    // 'GetCompleteZeroConnectInfo' wouldn't return true if it wasn't
    ICodeInspector* code = this->GetCodeInspector();

    // Get the type of the event
    BoundType* eventType = MetaDatabase::GetInstance()->mEventMap.FindValue(eventName, nullptr);

    // If we could not find an event type, assume it's just event
    if(eventType == nullptr)
      eventType = ZilchTypeId(Event);

    //HACK for gameplay
    String eventTypeName = eventType->Name;
    if(eventType == ZilchTypeId(MouseEvent))
      eventTypeName = "ViewportMouseEvent";

    String functionName = BuildString("On", eventName);
  
    String connectCallEnd = code->GenerateConnectCallEnd(functionName);

    int cursorPosition = GetCurrentPosition();

    String start = code->GenerateConnectFunctionStart(functionName, eventTypeName);
    String end = code->GenerateConnectFunctionEnd();
    
    start = ReplaceTabRuneWithOurTabStyle(start, indent);
    end = ReplaceTabRuneWithOurTabStyle(end, indent);

    // Check if we need a leading space...
    int r = GetRuneAt(cursorPosition - 1);
    if (r != ' ')
    {
      connectCallEnd = BuildString(" ", connectCallEnd);
    }
    
    InsertText(cursorPosition, connectCallEnd.c_str());

    int startPosition = functionPosition + connectCallEnd.SizeInBytes();
    int positionAfterStart = startPosition + start.SizeInBytes();

    InsertText(startPosition, start.c_str());
    InsertText(positionAfterStart, end.c_str());
    Select(positionAfterStart, positionAfterStart);

    this->EndUndo();
    return true;
  }

  return false;
}

void ScriptEditor::AttemptAddKeywordAndTypeCompletions(Array<Completion>& completionsOut)
{
  auto config = Z::gEditor->mConfig->has(TextEditorConfig);

  if(!config->KeywordAndTypeCompletion)
    return;

  auto code = GetCodeInspector();
  if(code)
  {
    code->GetKeywords(completionsOut);
  }
}

void ScriptEditor::AttemptAddLocalWordCompletions(Array<Completion>& completionsOut)
{
  auto config = Z::gEditor->mConfig->has(TextEditorConfig);

  if(!config->LocalWordCompletion)
    return;
  
  // Caret positions of word being edited
  int curPos = GetCurrentPosition();
  int startPos = GetWordStartPosition(curPos);

  // Leave as char, treat text as bytes, parse UTF8
  char curText[1024] = {0};
  GetText(startPos, curPos, curText, 1024);
  String curWord(curText);
  // Get entire file text
  int length = GetLength();
  char* buffer = new char[length + 1];
  GetText(0, length, buffer, length + 1);
  String text(buffer);
  delete [] buffer;

  HashSet<String> words;

  Array<int> startPositions;
  GetAllWordStartPositions(startPositions);

  // Tokenize all words in the text
  bool inWord = false;
  bool inComment = false;
  StringIterator start = text.Begin();
  StringIterator current = text.Begin();
  /*for (int s = 0, e = 0; text[e]; ++e)*/
  for (; current.ReadCurrentRune() != '\0'; ++current)
  {
    Rune r = current.ReadCurrentRune();

    if (IsAlphaNumeric(r) || r == '_')
    {
      if (!inWord)
      {
        inWord = true;
        start = current;
      }
    }
    else
    {
      if (inWord)
      {
        inWord = false;

        // Skip if not a valid identifier
        Rune s = start.ReadCurrentRune();
        if (!IsAlpha(s) && s != '_')
          continue;

        // Skip if it's the word being edited
        if (startPositions.Contains(start.Data() - text.Data()))
        {
          if (inComment)
          {
            words.Clear();
            break;
          }
          continue;
        }

        String word(text.SubString(start, current));
        // Only keep words that match up to the caret in size
        bool couldMatch = word.SizeInBytes() >= curWord.SizeInBytes();
        if (couldMatch)
        {
          StringRange curWordRange = curWord;
          StringRange wordRange = word;
          for (; !curWordRange.Empty(); curWordRange.PopFront(), wordRange.PopFront())
          {
            Rune a = word.Front();
            Rune b = curWord.Front();
            if (!(a == b || a == (b.value + ' ') || (a.value + ' ') == b))
            {
              couldMatch = false;
              break;
            }
          }
        }

        if (couldMatch && !inComment)
        {
          // Using HashSet to prevent duplicate entries
          words.Insert(word);
        }
      }

      ICodeInspector* code = this->GetCodeInspector();
      if (code)
      {
        String comment = code->GetSingleLineCommentToken();

        StringRange remainingText = text.SubString(current, text.End());
        if (remainingText.FindFirstOf(comment).Begin() == current)
          inComment = true;
      }
    }

    if (inComment)
    {
      if (r == '\r' || r == '\n')
        inComment = false;
    }
  }

  // Put words into an array to sort it
  forRange(String& str, words.All())
  {
    auto& completion = completionsOut.PushBack();
    completion.Name = str;
  }
}

String ScriptEditor::FormatErrorMessage(StringParam message, int offset)
{
  if(offset != 0)
  {
    //build up a line with an arrow to the
    Array<char> whiteSpace;
    whiteSpace.Resize(offset + 2,' ');
    whiteSpace[offset - 1] = '^';
    whiteSpace[offset + 0] = '\n';
    whiteSpace[offset + 1] = '\0';
    String arrowLine(whiteSpace.Data());
    return BuildString(arrowLine,message);
  }

  return message;
}

void ScriptEditor::Save()
{
  this->ClearAnnotations();

  if(IsModified())
  {
    SaveDocument();
  }
}

ScriptEditor* CreateScriptEditor(Composite* parent, ResourceDocument* doc)
{
  ScriptEditor* editor = new ScriptEditor(parent);

  editor->SetDocument(doc);

  String format = doc->mResource->GetFormat();

  if(format == "Python")
    editor->SetLexer(Lexer::Python);
  else if(format == "Zilch")
    editor->SetLexer(Lexer::Zilch);
  else if(format == "Shader")
    editor->SetLexer(Lexer::Shader);
  else if(format == "C++")
    editor->SetLexer(Lexer::Cpp);
  else
    editor->SetLexer(Lexer::Text);

  editor->UseTextEditorConfig();

  return editor;
}

DocumentEditor* CreateDocumentEditor(Composite* parent, Document* doc)
{
  DocumentEditor* editor = new DocumentEditor(parent);
  editor->SetDocument(doc);
  editor->SetLexer(Lexer::Text);
  return editor;
}

}
