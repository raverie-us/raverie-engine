///////////////////////////////////////////////////////////////////////////////
///
/// \file HotKeyEditor.cpp
/// Definition of the HotKey Editor class.
///
/// Authors: Ryan Edgemon
/// Copyright 2015-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(CommandRenamed);
  DefineEvent(BindingOverwrite);
}

/// HotKeyBinding ref when a binding conflict has occurred
class BindingConflictEvent : public Event
{
public:
  BindingConflictEvent(CommandEntry* newCommand, CommandEntry* existingComnnad, HotKeyBinding* newBinding)
    : mNewCommand(newCommand), mExistingCommad(existingComnnad), mNewBinding(newBinding) {}

  CommandEntry* mNewCommand;
  CommandEntry* mExistingCommad;
  HotKeyBinding* mNewBinding;
};

//----------------------------------------------------------- Binding Editor ---
class BindingEditor : public InPlaceTextEditor
{
public:
  typedef BindingEditor ZilchSelf;

    // <Keys::Enum>
  unsigned mModifier1;
  unsigned mModifier2;
  unsigned mMainKey;

  String mBindingStr;

  BindingEditor(Composite* parent) : InPlaceTextEditor(parent, 0)//InPlaceTextEditorFlags::EditOnDoubleClick)
  {
    mModifier1 = Keys::Unknown;
    mModifier2 = Keys::Unknown;
    mMainKey = Keys::Unknown;

    ConnectThisTo(this, Events::KeyDown, OnKeyDown);
    ConnectThisTo(this, Events::KeyUp, OnKeyUp);
  }

  void GetVariant(Any& variant)
  {
    //HotKeyBinding binding(mModifier1, mModifier2, mMainKey);
    //binding.mString = mBindingStr;
    variant = new HotKeyBinding(mModifier1, mModifier2, mMainKey, mBindingStr);
  }

  void SetVariant(AnyParam variant) override
  {
    mText->SetText(variant.ToString());
  }

  void OnKeyDown(KeyboardEvent* event)
  {
    return;

    //if(mainKey == Keys::Unknown && event->Key >= Keys::A && event->Key <= Keys::Backslash)
    //  mainKey = event->Key;
    //if(modifier1 == Keys::Unknown && event->Key >= Keys::Up && event->Key <= Keys::NumPad9)
    //  modifier1 = event->Key;
    //if(modifier2 == Keys::Unknown && event->Key >= Keys::Up && event->Key <= Keys::NumPad9)
    //  modifier2 = event->Key;

    if(!mEdit || mEdit->mEditTextField->mTakeFocusMode != FocusMode::Hard)
      return;

    if(event->Key != Keys::Control && event->Key != Keys::Alt && event->Key != Keys::Shift)
    {
      if(mMainKey == Keys::Unknown && event->Key >= Keys::A && event->Key <= Keys::NumPad9)
      {
        mMainKey = event->Key;

        Edit();
        mText->SetText(mBindingStr = mEdit->GetText());

        //HotKeyBindingEvent e(modifier1, modifier2, mainKey);
        //DispatchBubble(Events::CommitHotKeyBinding, &e);

        ObjectEvent objectEvent(this);
        DispatchEvent(Events::ValueChanged, &objectEvent);
        

        mModifier1 = Keys::Unknown;
        mModifier2 = Keys::Unknown;
        mMainKey = Keys::Unknown;

        mEdit->Destroy();
        LoseFocus();
        return;
      }

    }
    else if(event->Key == Keys::Control || event->Key == Keys::Alt || event->Key == Keys::Shift)
    {
      if(mModifier1 == Keys::Unknown)
        mModifier1 = event->Key;
      else if(mModifier2 == Keys::Unknown)
        mModifier2 = event->Key;
    }

    if(mModifier2 > mModifier1)
      Math::Swap(mModifier1, mModifier2);

    Edit();
  }

  void OnKeyUp(KeyboardEvent* event)
  {
    return;

    if(!event->mKeyboard->KeyIsDown(Keys::Control) && !event->mKeyboard->KeyIsDown(Keys::Shift) && !event->mKeyboard->KeyIsDown(Keys::Alt))
    {
      mModifier1 = Keys::Unknown;
      mModifier2 = Keys::Unknown;

      if(mEdit)
        mEdit->Destroy();

      return;
    }
    else if(mModifier1 == event->Key)
    {
      mModifier1 = Keys::Unknown;
      if(event->mKeyboard->KeyIsDown(Keys::Control) && mModifier2 != Keys::Control) mModifier1 = Keys::Control;
      if(event->mKeyboard->KeyIsDown(Keys::Alt) && mModifier2 != Keys::Alt) mModifier1 = Keys::Alt;
      if(event->mKeyboard->KeyIsDown(Keys::Shift) && mModifier2 != Keys::Shift) mModifier1 = Keys::Shift;
    }
    else if(mModifier2 == event->Key)
    {
      mModifier2 = Keys::Unknown;
      if(event->mKeyboard->KeyIsDown(Keys::Control) && mModifier1 != Keys::Control) mModifier2 = Keys::Control;
      if(event->mKeyboard->KeyIsDown(Keys::Alt) && mModifier1 != Keys::Alt) mModifier2 = Keys::Alt;
      if(event->mKeyboard->KeyIsDown(Keys::Shift) && mModifier1 != Keys::Shift) mModifier2 = Keys::Shift;
    }

    if(mModifier2 > mModifier1)
      Math::Swap(mModifier1, mModifier2);

    Edit();
  }

  void BuildBindingString(String& out)
  {
    char m0[16];  const char *pm0 = m0;
    char m1[16];  const char *pm1 = m1;
    char m2[16];  const char *pm2 = m2;

    pm0 = (mMainKey == Keys::Unknown) ? '\0' : HotKeyEditor::sKeyMap[mMainKey].c_str();
    pm1 = (mModifier1 == Keys::Unknown) ? '\0' : HotKeyEditor::sKeyMap[mModifier1].c_str();
    pm2 = (mModifier2 == Keys::Unknown) ? '\0' : HotKeyEditor::sKeyMap[mModifier2].c_str();

    out = BuildString(pm1, (pm1 && (pm2 || pm0)) ? " + " : '\0', pm2, (pm2 && pm0) ? " + " : '\0', pm0);
  }

  void Edit( )
  {
    String text;
    BuildBindingString(text);

    TextBox* edit = new TextBox(this);
    edit->SetTranslation(Vec3(0, 0, 0));
    edit->SetText(text);
    edit->SetSize(this->GetSize());
    edit->SetEditable(true);
    edit->TakeFocus();

    ConnectThisTo(edit->mEditTextField, Events::TextBoxChanged, OnTextBoxChanged);
    ConnectThisTo(edit->mEditTextField, Events::FocusLost, OnTextLostFocus);

    mEdit = edit;
  }

  void OnTextBoxChanged(ObjectEvent* event)
  {
    return;

    InPlaceTextEditor::OnTextBoxChanged(event);
  }

  void OnTextLostFocus(FocusEvent* event)
  {
    //mModifier1 = Keys::Unknown;
    //mModifier2 = Keys::Unknown;

    InPlaceTextEditor::OnTextLostFocus(event);
  }

};

ValueEditor* CreateBindingEditor(Composite* parent, AnyParam data, u32 flags)
{
  return new BindingEditor(parent);
}

ValueEditor* CreateCommandEditor(Composite* parent, AnyParam data, u32 flags)
{
  return new InPlaceTextEditor(parent, 0);//, InPlaceTextEditorFlags::EditOnDoubleClick);
}

ValueEditor* CreateDescriptionEditor(Composite* parent, AnyParam data, u32 flags)
{
  return new InPlaceTextEditor(parent, 0);//InPlaceTextEditorFlags::EditOnDoubleClick);
}

ValueEditor* CreateTagEditor(Composite* parent, AnyParam data, u32 flags)
{
  return new InPlaceTextEditor(parent, 0);//InPlaceTextEditorFlags::EditOnDoubleClick);
}

//------------------------------------------------------------- HotKeyFilter ---
DeclareEnum3(FilterSearchMode,
  CommandName,
  BindingString,
  CommandTag);

class HotKeyFilter : public DataSourceFilter
{
public:
  StringRange SubFilterString;
  FilterSearchMode::Enum mSearchMode;

  HotKeyFilter()
  {
    mSearchMode = FilterSearchMode::CommandName;
  }

  void Filter(StringParam filterString) override
  {
    mFilteredList.Clear();

    if(filterString.Empty())
      return;

    DataEntry* root = mSource->GetRoot();

    SubFilterString = filterString.All();

    FilterSearchMode::Enum searchMode = mSearchMode;

    Rune rune = filterString.Front();
    if(rune == '$')
    {
      ++SubFilterString.Begin();
      searchMode = FilterSearchMode::BindingString;
    }
    else if(rune == '.')
    {
      ++SubFilterString.Begin();
      searchMode = FilterSearchMode::CommandTag;
    }

    if(SubFilterString.Empty())
      return;

    if(searchMode == FilterSearchMode::CommandName)
    {
      BindMethodPtr<HotKeyFilter, &HotKeyFilter::FilterCommand> filter(this);
      FilterNodes(filter, root);
      return;
    }
    else if(searchMode == FilterSearchMode::BindingString)
    {
      BindMethodPtr<HotKeyFilter, &HotKeyFilter::FilterBinding> filter(this);
      FilterNodes(filter, root);
      return;
    }
    else if(searchMode == FilterSearchMode::CommandTag)
    {
      BindMethodPtr<HotKeyFilter, &HotKeyFilter::FilterTag> filter(this);
      FilterNodes(filter, root);
      return;
    }

  }

  bool FilterCommand(DataEntry* dataEntry)
  {
    CommandEntry *row = ((CommandEntry *)dataEntry);

    int priority = PartialMatch(SubFilterString, row->mName.All(), CaseInsensitiveCompare);
    return priority != cNoMatch;
  }

  bool FilterBinding(DataEntry* dataEntry)
  {
    CommandEntry *row = ((CommandEntry *)dataEntry);

    int priority = PartialMatch(SubFilterString, row->mBindingStr.All(), CaseInsensitiveCompare);
    return priority != cNoMatch;
  }

  bool FilterTag(DataEntry* dataEntry)
  {
    CommandEntry *row = ((CommandEntry *)dataEntry);

    int priority = PartialMatch(SubFilterString, row->mTags.All(), CaseInsensitiveCompare);
    return priority != cNoMatch;
  }

};

//---------------------------------------------------- TreeViewSearchHotKeys ---
class TreeViewSearchHotKeys : public TreeViewSearch
{
public:
  typedef TreeViewSearchHotKeys ZilchSelf;
  HotKeyFilter* mHotKeyFilter;

  TreeViewSearchHotKeys(Composite* parent)
    :TreeViewSearch(parent, NULL, NULL)
  {
    mHotKeyFilter = new HotKeyFilter();
    mFiltered = mHotKeyFilter;
    mSearchField->SetHintText("Search by command name...");
    ConnectThisTo(mIcon, Events::LeftClick, OnMouseClick);
    mIcon->SetColor(Vec4(1, 1, 1, 1));
  }

  void UpdateTransform() override
  {
    mIcon->SetColor(Vec4(1, 1, 1, 1));
    TreeViewSearch::UpdateTransform();
  }

  void OnMouseClick(Event* event)
  {
    ContextMenu* menu = new ContextMenu(this);
    Mouse* mouse = Z::gMouse;
    menu->SetBelowMouse(mouse, Pixels(0, 0));

    ConnectMenu(menu, "By Command", OnCommand);
    ConnectMenu(menu, "By Binding ($)", OnBinding);
    ConnectMenu(menu, "By Tag (.)", OnTag);
  }

  void OnCommand(Event* event)
  {
    mSearchField->SetText(String());
    mSearchField->SetHintText("Search by command name...");
    mHotKeyFilter->mSearchMode = FilterSearchMode::CommandName;
  }

  void OnBinding(Event* event)
  {
    mSearchField->SetText(String());
    mSearchField->SetHintText("Search by binding...");
    mHotKeyFilter->mSearchMode = FilterSearchMode::BindingString;
  }

  void OnTag(Event* event)
  {
    mSearchField->SetText(String());
    mSearchField->SetHintText("Search tag...");
    mHotKeyFilter->mSearchMode = FilterSearchMode::CommandTag;
  }

};

//----------------------------------------------------------- HotKeyCommands ---

/******************************************************************************/
void HotKeySortHelper(bool updateIndexes, HotKeyDataSet* set)
{
  HotKeySortHelper(updateIndexes, set->mCommand);
}

void HotKeySortHelper(bool updateIndexes, CommandSet& set)
{
  int i = 0;
  CommandSet::range sortRange = set.All();
  while(sortRange[0].mName == cDefaultCommandString || sortRange[0].mBindingStr == cDefaultBindingString)
  {
    sortRange.PopFront();
    ++i;
  }

  Sort(sortRange);

  if(!updateIndexes)
    return;

  int size = (int)set.Size();
  for(; i < size; ++i)
    set[i].mIndex = i;
}

void RegisterHotKeyEditors()
{
  ValueEditorFactory* factory = ValueEditorFactory::GetInstance();
  factory->RegisterEditor("CommandEditor", CreateCommandEditor);
  factory->RegisterEditor("BindingEditor", CreateBindingEditor);
  factory->RegisterEditor("TagEditor", CreateTagEditor);
  factory->RegisterEditor("DescriptionEditor", CreateDescriptionEditor);
}


static const String cCommandColumn = "Command";
static const String cBindingColumn = "Binding";
static const String cTagColumn = "Tag";
static const String cDescriptionColumn = "Description";

ZilchDefineType(HotKeyCommands, builder, type)
{
}

/******************************************************************************/
HotKeyCommands::HotKeyCommands()
{
}

/******************************************************************************/
DataEntry* HotKeyCommands::GetRoot()
{
  return mSet;
}

/******************************************************************************/
DataEntry* HotKeyCommands::ToEntry(DataIndex index)
{
  DataEntry* root = mSet;
  if(((DataEntry *)index.Id) == root)
    return ((DataEntry *)index.Id);
  else if(index.Id >= mSet->mCommand.Size())
    return NULL;

  return &mSet->mCommand[(unsigned)index.Id];
}

/******************************************************************************/
DataIndex HotKeyCommands::ToIndex(DataEntry *dataEntry)
{
  DataEntry* root = mSet;
  if(dataEntry == root)
    return (DataIndex)((u64)dataEntry);

  return DataIndex(((CommandEntry *)dataEntry)->mIndex);
}

/******************************************************************************/
DataEntry* HotKeyCommands::Parent(DataEntry* dataEntry)
{
    // everyone but the root has the parent of the root
  DataEntry* root = mSet;
  if(dataEntry == root)
    return NULL;

  return root;
}

/******************************************************************************/
uint HotKeyCommands::ChildCount(DataEntry* dataEntry)
{
    // only the root has children, no one else does
  DataEntry* root = mSet;
  if(dataEntry == root)
  {
    //unsigned count = 0;

    //CommandList::iterator itr = mSet->mCommand.Begin();
    //CommandList::iterator end = mSet->mCommand.End();

    //for(; itr != end; ++itr)
    //  ++count;

    //return count;

    return mSet->mCommand.Size();
  }

  return 0;
}

/******************************************************************************/
DataEntry* HotKeyCommands::GetChild(DataEntry* dataEntry, uint index, DataEntry* prev)
{
  if(index >= 0 && index < mSet->mCommand.Size())
  {
    //mSet->mCommand[index].mIndex = index;
    return &mSet->mCommand[index];
  }

  DataEntry* root = mSet;
  if(dataEntry == root)
  {
    //mSet->mCommand.Front().mIndex = 0;
    return &mSet->mCommand.Front();
  }

  //CommandListElement *prevCmd = (CommandListElement *)prev;
  //if(prevCmd)
  //  return prevCmd->Next;

  //CommandList::iterator itr = mSet->mCommand.Begin();
  //CommandList::iterator end = mSet->mCommand.End();

  //for(int i = 0; itr != end; ++itr, ++i)
  //{
  //  if(i == index)
  //    return itr;
  //}

  //return &mSet->mCommand.Front();

  return NULL;
}

/******************************************************************************/
bool HotKeyCommands::IsExpandable(DataEntry* dataEntry)
{
  //  //only the root is expandable
  //DataEntry* root = mSet;
  //if(dataEntry == root)
  //  return true;

  return false;
}

/******************************************************************************/
void HotKeyCommands::GetData(DataEntry* dataEntry, Any& variant, StringParam column)
{
  CommandEntry *row = ((CommandEntry *)dataEntry);

  if(column == cCommandColumn)
  {
    variant = row->mName;
  }
  else if(column == cBindingColumn)
  {
    //StringBuilder binding;
    //for(int i = 0; i < row->mBinding.Size(); ++i)
    //{
    //  if(row->mBinding[0] < Keys::Backslash)
    //    binding += (unsigned char)row->mBinding[0];
    //  else
    //    binding += HotKeyEditor::sKeyMap[row->mBinding[0]];

    //  if(i != row->mBinding.Size()-1)
    //    binding += " + ";
    //}

    //variant = binding.ToString();

    variant = row->mBindingStr;
  }
  else if(column == cTagColumn)
  {
    variant = row->mTags;
  }
  else if(column == cDescriptionColumn)
  {
    variant = row->mDescription;
  }

}

/******************************************************************************/
bool HotKeyCommands::SetData(DataEntry* dataEntry, AnyParam variant, StringParam column)
{
  CommandEntry* row = ((CommandEntry *)dataEntry);

  if(column == cCommandColumn)
  {
    row->mName = variant.ToString();

    HotKeySortHelper(true, mSet);

    ObjectEvent objectEvent(this);
    DispatchEvent(Events::CommandRenamed, &objectEvent);
    //mTreeView->ClearAllRows();
    //mTreeView->SetDataSource(&mHotKeys);
  }
  else if(column == cBindingColumn)
  {
    HotKeyBinding *binding = variant.Get<HotKeyBinding*>();

    int size = (int)mSet->mCommand.Size();
    for(int i = 0; i < size; ++i)
    {
      if(mSet->mCommand[i].mBindingStr == binding->mString)
      {
        if(mSet->mCommand[i].mName == row->mName)
          return false;

        DispatchEvent(Events::BindingOverwrite, new BindingConflictEvent(row, &mSet->mCommand[i], binding));
        return false;
      }

    }

    row->mBindingStr = binding->mString;
    row->mModifier1 = binding->mModifier1;
    row->mModifier2 = binding->mModifier2;
    row->mMainKey = binding->mMainKey;
    
    delete binding;
  }
  else if(column == cTagColumn)
  {
    row->mTags = variant.ToString();
  }
  else if(column == cDescriptionColumn)
  {
    row->mDescription = variant.ToString();
  }

  MetaOperations::NotifyObjectModified(mSet);
  return true;
}

/******************************************************************************/
bool HotKeyCommands::Remove(DataEntry* dataEntry)
{
  CommandEntry *row = ((CommandEntry *)dataEntry);

  DataEvent e;
  e.Index = row->mIndex;
  this->DispatchEvent(Events::DataRemoved, &e);

  return true;
}

//------------------------------------------------------------- HotKeyEditor ---
ZilchDefineType(HotKeyEditor, builder, type)
{
  
}

HashMap<unsigned, String> HotKeyEditor::sKeyMap;

/******************************************************************************/
HotKeyEditor::HotKeyEditor(Composite* parent) : Composite(parent)
{
  //String userHotKeyFile = FilePath::Combine(GetUserDocumentsDirectory(), "ZeroEditor", "Default.HotKeyDataSet.data");

  //if(!FileExists(userHotKeyFile))
  //{
  //  String coreHotKeyFile = HotKeyManager::GetDefault()->mContentItem->GetFullPath();

  //  CopyFileInternal(userHotKeyFile, coreHotKeyFile);
  //}
  //else
  //{
  //  Status status;
  //  UniquePointer<Serializer> stream(GetLoaderStreamFile(status, userHotKeyFile));
  //  if(status)
  //  {
  //    //LoadPolymorphicSerialize("Commands", "commands", *stream, commands, this);
  //  }

  //}

  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 2), Thickness::cZero));
  
  //ConnectThisTo(&mHotKeys, Events::CommandRenamed, OnRenamedCommand);

  TreeViewSearch* search = new TreeViewSearchHotKeys(this);
  search->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(20));

  mTreeView = new TreeView(this);
  search->mTreeView = mTreeView;

  //ConnectThisTo(mTreeView, Events::TreeRightClick, OnCommandRightClick);
  //ConnectThisTo(mTreeView, Events::KeyDown, OnKeyDown);

  TreeFormatting formatting;
  BuildFormat(formatting);
  mTreeView->SetFormat(formatting);

  float sizeX = 0.0f;  int columnCount = (int)formatting.Columns.Size();
  for(int x = 0; x < columnCount; ++x)
    sizeX += formatting.Columns[x].MinWidth * 4.0f;

  mTreeView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);
  mTreeView->SetSizing(SizeAxis::X, SizePolicy::Flex, sizeX);

  //mAddCommand = new TextButton(this);
  //mAddCommand->SetText("Add Command");
  //mAddCommand->SetSizing(SizePolicy::Fixed, mAddCommand->GetMinSize() + mAddCommand->GetPadding().Size());
  //mAddCommand->mHorizontalAlignment = HorizontalAlignment::Right;

  //ConnectThisTo(mAddCommand, Events::LeftMouseUp, OnAddCommand);

  //Vec3 startPosition(Pixels(150), yStart + groupCount * (size.y + buffer), 0);
  //mHotKeySetDropdown = new ComboBox(this);
  //mHotKeySetDropdown->SetTranslation(Vec3(-5, -20, 0));
  //mHotKeySetDropdown->mHorizontalAlignment = HorizontalAlignment::Right;
  //mHotKeySetDropdown->SetSizing(SizePolicy::Fixed, Pixels(150, 20));

  //ConnectThisTo(mHotKeySetDropdown, Events::ItemSelected, OnCommandSetSelected);

  //ConnectThisTo(&mHotKeys, Events::BindingOverwrite, OnConfirmBindingOverwrite);

  //TextButton* button2 = new TextButton(this);
  //button2->SetText("Butt");
  //button2->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1.0f);

  //ConnectThisTo(button1, Events::ButtonPressed, OnButton1Pressed);
  //ConnectThisTo(button2, Events::ButtonPressed, OnButton2Pressed);

    // letters 
  sKeyMap[Keys::A] = "A";
  sKeyMap[Keys::B] = "B";
  sKeyMap[Keys::C] = "C";
  sKeyMap[Keys::D] = "D";
  sKeyMap[Keys::E] = "E";
  sKeyMap[Keys::F] = "F";
  sKeyMap[Keys::G] = "G";
  sKeyMap[Keys::H] = "H";
  sKeyMap[Keys::I] = "I";
  sKeyMap[Keys::J] = "J";
  sKeyMap[Keys::K] = "K";
  sKeyMap[Keys::L] = "L";
  sKeyMap[Keys::M] = "M";
  sKeyMap[Keys::N] = "N";
  sKeyMap[Keys::O] = "O";
  sKeyMap[Keys::P] = "P";
  sKeyMap[Keys::Q] = "Q";
  sKeyMap[Keys::R] = "R";
  sKeyMap[Keys::S] = "S";
  sKeyMap[Keys::T] = "T";
  sKeyMap[Keys::U] = "U";
  sKeyMap[Keys::V] = "V";
  sKeyMap[Keys::W] = "W";
  sKeyMap[Keys::Y] = "Y";
  sKeyMap[Keys::X] = "X";
  sKeyMap[Keys::Z] = "Z";

  sKeyMap[Keys::Space] = "Space";

    // numbers
  sKeyMap[Keys::Num0] = "0";
  sKeyMap[Keys::Num1] = "1";
  sKeyMap[Keys::Num2] = "2";
  sKeyMap[Keys::Num3] = "3";
  sKeyMap[Keys::Num4] = "4";
  sKeyMap[Keys::Num5] = "5";
  sKeyMap[Keys::Num6] = "6";
  sKeyMap[Keys::Num7] = "7";
  sKeyMap[Keys::Num8] = "8";
  sKeyMap[Keys::Num9] = "9";

    // symbols
  sKeyMap[Keys::LeftBracket] = "[";
  sKeyMap[Keys::RightBracket] = "]";
  sKeyMap[Keys::Comma] = ";";
  sKeyMap[Keys::Period] = ".";
  sKeyMap[Keys::Semicolon] = ";";
  sKeyMap[Keys::Minus] = "-";
  sKeyMap[Keys::Apostrophe] = "\"";
  sKeyMap[Keys::Slash] = "/";
  sKeyMap[Keys::Backslash] = "\\";

    // arrow keys
  sKeyMap[Keys::Up] = "Up";
  sKeyMap[Keys::Down] = "Down";
  sKeyMap[Keys::Left] = "Left";
  sKeyMap[Keys::Right] = "Right";

    // fn keys
  sKeyMap[Keys::F1] = "F1";
  sKeyMap[Keys::F2] = "F2";
  sKeyMap[Keys::F3] = "F3";
  sKeyMap[Keys::F4] = "F4";
  sKeyMap[Keys::F5] = "F5";
  sKeyMap[Keys::F6] = "F6";
  sKeyMap[Keys::F7] = "F7";
  sKeyMap[Keys::F8] = "F8";
  sKeyMap[Keys::F9] = "F9";
  sKeyMap[Keys::F10] = "F10";
  sKeyMap[Keys::F11] = "F11";
  sKeyMap[Keys::F12] = "F12";

    // special keys
  sKeyMap[Keys::Delete] = "Delete";
  sKeyMap[Keys::Back] = "Backspace";
  sKeyMap[Keys::Home] = "Home";
  sKeyMap[Keys::End] = "End";
  sKeyMap[Keys::Tilde] = "~";
  sKeyMap[Keys::Tab] = "Tab";
  sKeyMap[Keys::Shift] = "Shift";
  sKeyMap[Keys::Alt] = "Alt";
  sKeyMap[Keys::Control] = "Ctrl";
  sKeyMap[Keys::Capital] = "Caps Lock";
  sKeyMap[Keys::Enter] = "Enter";
  sKeyMap[Keys::Escape] = "Esc";
  sKeyMap[Keys::PageUp] = "PageUp";
  sKeyMap[Keys::PageDown] = "PageDown";
  sKeyMap[Keys::Equal] = "=";

    // numpad
  sKeyMap[Keys::NumPad0] = "NumPad0";
  sKeyMap[Keys::NumPad1] = "NumPad1";
  sKeyMap[Keys::NumPad2] = "NumPad2";
  sKeyMap[Keys::NumPad3] = "NumPad3";
  sKeyMap[Keys::NumPad4] = "NumPad4";
  sKeyMap[Keys::NumPad5] = "NumPad5";
  sKeyMap[Keys::NumPad6] = "NumPad6";
  sKeyMap[Keys::NumPad7] = "NumPad7";
  sKeyMap[Keys::NumPad8] = "NumPad8";
  sKeyMap[Keys::NumPad9] = "NumPad9";
}

/******************************************************************************/

void HotKeyEditor::BuildFormat(TreeFormatting& formatting)
{
  formatting.Flags.SetFlag(FormatFlags::ShowHeaders);

    // command column
  ColumnFormat* format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = cCommandColumn;
  format->HeaderName = "Command";
  format->ColumnType = ColumnType::Flex;
  format->FlexSize = 3;
  format->MinWidth = Pixels(120.0f);
  format->Editable = true;
  format->CustomEditor = "CommandEditor";

    // binding column
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = cBindingColumn;
  format->HeaderName = "Binding";
  format->ColumnType = ColumnType::Flex;
  format->FlexSize = 2;
  format->MinWidth = Pixels(100.0f);
  format->Editable = true;
  format->CustomEditor = "BindingEditor";

    // tag column
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = cTagColumn;
  format->HeaderName = "Tag";
  format->ColumnType = ColumnType::Flex;
  format->FlexSize = 1;
  format->MinWidth = Pixels(80.0f);
  format->Editable = true;
  format->CustomEditor = "TagEditor";

    // description column
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = cDescriptionColumn;
  format->HeaderName = "Description";
  format->ColumnType = ColumnType::Flex;
  format->FlexSize = 5;
  //format->FixedSize = Pixels(20, 400);
  format->MinWidth = Pixels(600.0f);
  format->Editable = true;
  format->CustomEditor = "DescriptionEditor";
}

/******************************************************************************/
void HotKeyEditor::UpdateTransform( )
{
  Composite::UpdateTransform();
}

/******************************************************************************/
void HotKeyEditor::EditResource(HotKeyDataSet* set, HotKeyManager *hkManager)
{
  mHotKeys.mSet = set;
  mTreeView->SetDataSource(&mHotKeys);

  mHotKeyManager = hkManager;
  //ResourceManager::ResourceRange rRange = ;
  forRange(auto resource, mHotKeyManager->ResourceIdMap.All())
  {
      // ghetto, set names should NOT be added every time, it should be done once
      // and it should be done elsewhere [ie, in an initialization phase]
    if(mSetNames.Strings.FindIndex(resource.second->Name) == Array<String>::InvalidIndex)
      mSetNames.Strings.PushBack(resource.second->Name);
  }

  //mHotKeySetDropdown->SetListSource(&mSetNames);
  //mHotKeySetDropdown->SetText(set->Name);
}

/******************************************************************************/
bool HotKeyEditor::TakeFocusOverride( )
{
  this->HardTakeFocus();
  //MetaSelection* selection = Z::gEditor->GetActiveSelection();
  //MetaObjectInstance primary = selection->GetPrimary();
  //ObjectSelected(primary.As<Cog>(NULL));
  return true;
}

/******************************************************************************/
void HotKeyEditor::AutoClose( )
{
  if(HasFocus())
    LoseFocus();

  CloseTabContaining(this);
}

/******************************************************************************/
void HotKeyEditor::OnCancel(SearchViewEvent* event)
{
  AutoClose();
}

/******************************************************************************/
void HotKeyEditor::OnCommandRename(ObjectEvent* event)
{
  TreeRow* row = mTreeView->FindRowByIndex(mRowIndex);
  row->Edit(cCommandColumn);

  HotKeySortHelper(true, mHotKeys.mSet);

  mTreeView->ClearAllRows();
  mTreeView->SetDataSource(&mHotKeys);

  MetaOperations::NotifyObjectModified(mHotKeys.mSet);
}

/******************************************************************************/
void HotKeyEditor::OnCommandRebind(ObjectEvent* event)
{
  TreeRow* row = mTreeView->FindRowByIndex(mRowIndex);

  row->Edit(cBindingColumn);
}

/******************************************************************************/
void HotKeyEditor::OnCommandDelete(ObjectEvent* event)
{
  //TreeRow* row = mTreeView->FindRowByIndex(mRowIndex);
  //row->Remove();  // dispatch event
  //
  //mHotKeys.mSet->mCommand.EraseAt((unsigned)mRowIndex.Id);
  //mTreeView->mRows.EraseAt((unsigned)mRowIndex.Id);

  //HotKeySortHelper(false, mHotKeys.mSet);

  //int size = (int)mHotKeys.mSet->mCommand.Size();
  //for(int i = 0; i < size; ++i)
  //{
  //  mHotKeys.mSet->mCommand[i].mIndex = i;

  //    // +1 as the first row is the header
  //  mTreeView->mRows[i+1]->mIndex = i;
  //  mTreeView->mRows[i+1]->mVisibleRowIndex = i;
  //}

  Array<unsigned> toErase;

  HashDataSelection* data = (HashDataSelection*)mTreeView->GetSelection();

  if(data->mSelection.Empty())
    return;

  forRange(u64 selection, data->mSelection.All())
  {
    //mHotKeys.mSet->mCommand.EraseAt((unsigned)selection);
    toErase.PushBack((unsigned)selection);
  }

  Sort(toErase.All());
  mHotKeys.mSet->mCommand.Erase(mHotKeys.mSet->mCommand.SubRange(toErase[0], toErase.Size()));

  int size = (int)mHotKeys.mSet->mCommand.Size();
  for(int i = 0; i < size; ++i)
    mHotKeys.mSet->mCommand[i].mIndex = i;

    // deselect UI elements
  data->mSelection.Clear();

  mTreeView->ClearAllRows();
  mTreeView->SetDataSource(&mHotKeys);

  MetaOperations::NotifyObjectModified(mHotKeys.mSet);
}

/******************************************************************************/
void HotKeyEditor::OnCommandRightClick(TreeEvent* event)
{
  return;

  ContextMenu* menu = new ContextMenu(event->Row);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0, 0));
  mRowIndex = event->Row->mIndex;
  ConnectMenu(menu, "Rename", OnCommandRename);
  ConnectMenu(menu, "Rebind", OnCommandRebind);
  ConnectMenu(menu, "Delete", OnCommandDelete);
}

/******************************************************************************/
void HotKeyEditor::OnKeyDown(KeyboardEvent* event)
{
   // only delete if not editing a command, as 'delete' is a valid hotkey
  if(event->Handled)
    return;

    // only key with functionality currently (if not editing a command/binding)
  if(event->Key == Keys::Delete)
  {
    //HashDataSelection* data = (HashDataSelection*)mTreeView->GetSelection();
    //forRange(u64 selection, data->mSelection.All())
    //{
    //  mHotKeys.mSet->mCommand.EraseAt((unsigned)selection);
    //}

    //mRowIndex = event->Row->mIndex;
    OnCommandDelete(NULL);
  }

}

/******************************************************************************/
void HotKeyEditor::OnSelectionChanged(Event* event)
{
  mTreeView->MarkAsNeedsUpdate();

  // Do not show row when focus change is 
  // caused by the object view itself
  if(this->HasFocus())
    return;
}

/******************************************************************************/
void HotKeyEditor::OnRenamedCommand(ObjectEvent* event)
{
  mTreeView->ClearAllRows();
  mTreeView->SetDataSource(&mHotKeys);
}

/******************************************************************************/
void HotKeyEditor::OnAddCommand(MouseEvent* event)
{
  mTreeView->ClearAllRows();

  mHotKeys.mSet->mCommand.InsertAt(0, CommandEntry());

  mHotKeys.mSet->mCommand[0].mName = cDefaultCommandString;
  mHotKeys.mSet->mCommand[0].mBindingStr = cDefaultBindingString;
  
  //Sort(mHotKeys.mSet->mCommand.All());
  int size = (int)mHotKeys.mSet->mCommand.Size();
  for(int i = 0; i < size; ++i)
    mHotKeys.mSet->mCommand[i].mIndex = i;

  mTreeView->SetDataSource(&mHotKeys);

  //MetaOperations::NotifyObjectModified(mHotKeys.mSet);
}

/******************************************************************************/
void HotKeyEditor::OnCommandSetSelected(ObjectEvent* event)
{
  int index = mHotKeySetDropdown->GetSelectedItem();
  if(index < 0 || index >= int(mSetNames.Strings.Size()))
    return;

  //mTreeView->ClearAllRows();
  //mTreeView->mRows[0]->mChildren.Clear();
  //mTreeView->mRows.Clear();
  //mTreeView->mRowMap.Clear();
  //mTreeView->Refresh();

  mHotKeys.mSet = (HotKeyDataSet *)mHotKeyManager->GetResource(mSetNames.Strings[index], ResourceNotFound::ReturnNull);
  mTreeView->SetDataSource(&mHotKeys);
}

/******************************************************************************/
void HotKeyEditor::OnConfirmBindingOverwrite(BindingConflictEvent* event)
{
  String prompt(BuildString("Overwrite Command: \'", event->mExistingCommad->mName, "\'?"));
  ModalConfirmAction* modal = new ModalConfirmAction(this, prompt);
  modal->mUserData = event;

  ConnectThisTo(modal, Events::ModalConfirmResult, OnModalOption);
  ConnectThisTo(modal, Events::ModalClosed, OnModalClosed);
}

/******************************************************************************/
void HotKeyEditor::OnModalOption(ModalConfirmEvent* event)
{
  BindingConflictEvent* effectedData = (BindingConflictEvent*)event->mUserData;

  if(event->mConfirmed)
  {
    effectedData->mExistingCommad->mBindingStr.Clear();
    effectedData->mExistingCommad->mModifier1 = Keys::Unknown;
    effectedData->mExistingCommad->mModifier2 = Keys::Unknown;
    effectedData->mExistingCommad->mMainKey = Keys::Unknown;

    effectedData->mNewCommand->mBindingStr = effectedData->mNewBinding->mString;
    effectedData->mNewCommand->mModifier1 = effectedData->mNewBinding->mModifier1;
    effectedData->mNewCommand->mModifier2 = effectedData->mNewBinding->mModifier2;
    effectedData->mNewCommand->mMainKey = effectedData->mNewBinding->mMainKey;
  }
  else
  {
    effectedData->mNewCommand->mBindingStr.Clear();
    effectedData->mNewCommand->mModifier1 = Keys::Unknown;
    effectedData->mNewCommand->mModifier2 = Keys::Unknown;
    effectedData->mNewCommand->mMainKey = Keys::Unknown;
  }

  delete effectedData->mNewBinding;
  delete effectedData;
  effectedData = NULL;

  mTreeView->ClearAllRows();
  mTreeView->SetDataSource(&mHotKeys);

  MetaOperations::NotifyObjectModified(mHotKeys.mSet);
}

/******************************************************************************/
void HotKeyEditor::OnModalClosed(ModalConfirmEvent* event)  // unused, not firing
{
  //BindingConflictEvent* effectedData = (BindingConflictEvent*)event->mUserData;

  //if(effectedData == NULL)
  //  return;

  //effectedData->mNewCommand->mBindingStr.Clear();
  //effectedData->mNewCommand->mModifier1 = Keys::Unknown;
  //effectedData->mNewCommand->mModifier2 = Keys::Unknown;
  //effectedData->mNewCommand->mMainKey = Keys::Unknown;

  //delete effectedData->mNewBinding;
  //delete effectedData;

  //MetaOperations::NotifyObjectModified(mHotKeys.mSet);
}

/******************************************************************************/
void HotKeyEditor::OnButton2Pressed(ObjectEvent* event)
{
  mHotKeys.mSet = (HotKeyDataSet *)mHotKeyManager->GetResource("Pleb", ResourceNotFound::ReturnNull);
  mTreeView->SetDataSource(&mHotKeys);

  //mTreeView->SetName("Pleb");
  //mTreeView->MarkAsNeedsUpdate();
}

ZilchDefineType(HotKeyBinding, builder, type)
{
  ZilchBindFieldProperty(mModifier1);
  ZilchBindFieldProperty(mModifier2);
  ZilchBindFieldProperty(mMainKey);
}

}//namespace Zero
