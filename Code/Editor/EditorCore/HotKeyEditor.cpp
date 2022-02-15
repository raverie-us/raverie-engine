// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(CommandRenamed);
DefineEvent(BindingOverwrite);
} // namespace Events

static const String cDefaultCommandString = "\'NEW COMMAND\'";
static const String cDefaultBindingString = "\'NEW BINDING\'";

static const String cScriptColumn = "Script";
static const String cCommandColumn = "Command";
static const String cBindingColumn = "Shortcut";
static const String cTagsColumn = "Tags";
static const String cDescriptionColumn = "Description";

// Orange
static const Vec4 cZeroCommandColor(1, 0.647f, 0, 1);

static const bool cNotUsingHotKeyResource = true;
static const bool cHotKeysEditable = false;

/// HotKeyBinding ref when a binding conflict has occurred
class BindingConflictEvent : public Event
{
public:
  BindingConflictEvent(CommandEntry* newCommand, CommandEntry* existingComnnad, HotKeyBinding* newBinding) :
      mNewCommand(newCommand),
      mExistingCommad(existingComnnad),
      mNewBinding(newBinding)
  {
  }

  CommandEntry* mNewCommand;
  CommandEntry* mExistingCommad;
  HotKeyBinding* mNewBinding;
};

class CogCommandScriptEditor : public ValueEditor
{
public:
  typedef CogCommandScriptEditor ZilchSelf;
  Element* mIcon;
  HandleOf<CogCommand> mCommand;

  CogCommandScriptEditor(Composite* parent) : ValueEditor(parent)
  {
    mIcon = CreateAttached<Element>("EditScript");
    mIcon->SetTranslation(Pixels(0, 2, 0));

    ConnectThisTo(mIcon, Events::LeftClick, OnLeftClick);
    ConnectThisTo(mIcon, Events::MouseEnter, OnMouseEnter);
  }

  void SetVariant(AnyParam variant) override
  {
    mCommand = variant.Get<CogCommand*>();
    if (mCommand == nullptr)
      mIcon->SetVisible(false);
    else
      mIcon->SetVisible(true);
  }

  void GetVariant(Any& variant) override
  {
    variant = mCommand;
  }

  void OnLeftClick(MouseEvent* event)
  {
    if (mIcon->GetVisible() == false)
      return;

    // Open script.
    MetaResource* meta = mCommand->mScriptComponentType->HasInherited<MetaResource>();
    ResourceId resourceId = meta->mResourceId;
    Resource* resource = Z::gResources->GetResource(resourceId);
    ReturnIf(resource == nullptr, , "Could not find %s script", mCommand->GetName().c_str());
    Z::gEditor->EditResource(resource);
  }

  void OnMouseEnter(MouseEvent* event)
  {
    if (mIcon->GetVisible() == false)
      return;

    ToolTip* toolTip;
    if (!mCommand->ToolTip.Empty())
      toolTip = new ToolTip(this, mCommand->ToolTip);
    else
      toolTip = new ToolTip(this, BuildString("Edit '", mCommand->GetName(), "' script"));

    ToolTipPlacement placement;
    placement.SetScreenRect(mIcon->GetScreenRect());
    placement.SetPriority(IndicatorSide::Left, IndicatorSide::Right, IndicatorSide::Bottom, IndicatorSide::Top);

    toolTip->SetArrowTipTranslation(placement);
  }
};

class BindingEditor : public InPlaceTextEditor
{
public:
  typedef BindingEditor ZilchSelf;

  // <Keys::Enum>
  unsigned mModifier1;
  unsigned mModifier2;
  unsigned mMainKey;

  String mBindingStr;

  BindingEditor(Composite* parent) : InPlaceTextEditor(parent, 0) // InPlaceTextEditorFlags::EditOnDoubleClick)
  {
    mModifier1 = Keys::Unknown;
    mModifier2 = Keys::Unknown;
    mMainKey = Keys::Unknown;

    // ConnectThisTo(this, Events::KeyDown, OnKeyDown);
    // ConnectThisTo(this, Events::KeyUp, OnKeyUp);
  }

  void GetVariant(Any& variant)
  {
    // HotKeyBinding binding(mModifier1, mModifier2, mMainKey);
    // binding.mString = mBindingStr;
    variant = new HotKeyBinding(mModifier1, mModifier2, mMainKey, mBindingStr);
  }

  void SetVariant(AnyParam variant) override
  {
    mText->SetText(variant.ToString());
  }

  void OnKeyDown(KeyboardEvent* event)
  {
    // if(mainKey == Keys::Unknown && event->Key >= Keys::A && event->Key <=
    // Keys::Backslash)
    //  mainKey = event->Key;
    // if(modifier1 == Keys::Unknown && event->Key >= Keys::Up && event->Key <=
    // Keys::Decimal)
    //  modifier1 = event->Key;
    // if(modifier2 == Keys::Unknown && event->Key >= Keys::Up && event->Key <=
    // Keys::Decimal)
    //  modifier2 = event->Key;

    if (!mEdit || mEdit->mEditTextField->mTakeFocusMode != FocusMode::Hard)
      return;

    if (event->Key != Keys::Control && event->Key != Keys::Alt && event->Key != Keys::Shift)
    {
      if (mMainKey == Keys::Unknown && event->Key >= Keys::A && event->Key <= Keys::Decimal)
      {
        mMainKey = event->Key;

        Edit();
        mText->SetText(mBindingStr = mEdit->GetText());

        // HotKeyBindingEvent e(modifier1, modifier2, mainKey);
        // DispatchBubble(Events::CommitHotKeyBinding, &e);

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
    else if (event->Key == Keys::Control || event->Key == Keys::Alt || event->Key == Keys::Shift)
    {
      if (mModifier1 == Keys::Unknown)
        mModifier1 = event->Key;
      else if (mModifier2 == Keys::Unknown)
        mModifier2 = event->Key;
    }

    if (mModifier2 > mModifier1)
      Math::Swap(mModifier1, mModifier2);

    Edit();
  }

  void OnKeyUp(KeyboardEvent* event)
  {
    if (!event->mKeyboard->KeyIsDown(Keys::Control) && !event->mKeyboard->KeyIsDown(Keys::Shift) &&
        !event->mKeyboard->KeyIsDown(Keys::Alt))
    {
      mModifier1 = Keys::Unknown;
      mModifier2 = Keys::Unknown;

      if (mEdit)
        mEdit->Destroy();

      return;
    }
    else if (mModifier1 == event->Key)
    {
      mModifier1 = Keys::Unknown;
      if (event->mKeyboard->KeyIsDown(Keys::Control) && mModifier2 != Keys::Control)
        mModifier1 = Keys::Control;
      if (event->mKeyboard->KeyIsDown(Keys::Alt) && mModifier2 != Keys::Alt)
        mModifier1 = Keys::Alt;
      if (event->mKeyboard->KeyIsDown(Keys::Shift) && mModifier2 != Keys::Shift)
        mModifier1 = Keys::Shift;
    }
    else if (mModifier2 == event->Key)
    {
      mModifier2 = Keys::Unknown;
      if (event->mKeyboard->KeyIsDown(Keys::Control) && mModifier1 != Keys::Control)
        mModifier2 = Keys::Control;
      if (event->mKeyboard->KeyIsDown(Keys::Alt) && mModifier1 != Keys::Alt)
        mModifier2 = Keys::Alt;
      if (event->mKeyboard->KeyIsDown(Keys::Shift) && mModifier1 != Keys::Shift)
        mModifier2 = Keys::Shift;
    }

    if (mModifier2 > mModifier1)
      Math::Swap(mModifier1, mModifier2);

    Edit();
  }

  void BuildBindingString(String& out)
  {
    char m0[16];
    const char* pm0 = m0;
    char m1[16];
    const char* pm1 = m1;
    char m2[16];
    const char* pm2 = m2;

    pm0 = (mMainKey == Keys::Unknown) ? "\0" : HotKeyEditor::sKeyMap[mMainKey].c_str();
    pm1 = (mModifier1 == Keys::Unknown) ? "\0" : HotKeyEditor::sKeyMap[mModifier1].c_str();
    pm2 = (mModifier2 == Keys::Unknown) ? "\0" : HotKeyEditor::sKeyMap[mModifier2].c_str();

    out = BuildString(pm1, (pm1 && (pm2 || pm0)) ? " + " : "\0", pm2, (pm2 && pm0) ? " + " : "\0", pm0);
  }

  void Edit()
  {
    // String text;
    // BuildBindingString(text);

    // TextBox* edit = new TextBox(this);
    // edit->SetTranslation(Vec3(0, 0, 0));
    // edit->SetText(text);
    // edit->SetSize(this->GetSize());
    // edit->SetEditable(true);
    // edit->TakeFocus();

    // ConnectThisTo(edit->mEditTextField, Events::TextBoxChanged,
    // OnTextBoxChanged); ConnectThisTo(edit->mEditTextField, Events::FocusLost,
    // OnTextLostFocus);

    // mEdit = edit;
  }

  void OnTextBoxChanged(ObjectEvent* event)
  {
    // InPlaceTextEditor::OnTextBoxChanged(event);
  }

  void OnTextLostFocus(FocusEvent* event)
  {
    // mModifier1 = Keys::Unknown;
    // mModifier2 = Keys::Unknown;

    // InPlaceTextEditor::OnTextLostFocus(event);
  }
};

ValueEditor* CreateCogCommandScriptEditor(Composite* parent, AnyParam data, u32 flags)
{
  return new CogCommandScriptEditor(parent);
}

ValueEditor* CreateBindingEditor(Composite* parent, AnyParam data, u32 flags)
{
  return new BindingEditor(parent);
}

ValueEditor* CreateCommandEditor(Composite* parent, AnyParam data, u32 flags)
{
  return new InPlaceTextEditor(parent, 0); //, InPlaceTextEditorFlags::EditOnDoubleClick);
}

ValueEditor* CreateDescriptionEditor(Composite* parent, AnyParam data, u32 flags)
{
  return new InPlaceTextEditor(parent, 0); // InPlaceTextEditorFlags::EditOnDoubleClick);
}

ValueEditor* CreateTagEditor(Composite* parent, AnyParam data, u32 flags)
{
  return new InPlaceTextEditor(parent, 0); // InPlaceTextEditorFlags::EditOnDoubleClick);
}

DeclareEnum3(FilterSearchMode, CommandName, BindingString, CommandTag);

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

    if (filterString.Empty())
      return;

    DataEntry* root = mSource->GetRoot();

    SubFilterString = filterString.All();

    FilterSearchMode::Enum searchMode = mSearchMode;

    Rune rune = filterString.Front();
    if (rune == '$')
    {
      SubFilterString.PopFront();
      searchMode = FilterSearchMode::BindingString;
    }
    else if (rune == '.')
    {
      SubFilterString.PopFront();
      searchMode = FilterSearchMode::CommandTag;
    }

    if (SubFilterString.Empty())
      return;

    if (searchMode == FilterSearchMode::CommandName)
    {
      BindMethodPtr<HotKeyFilter, &HotKeyFilter::FilterCommand> filter(this);
      FilterNodes(filter, root);
      return;
    }
    else if (searchMode == FilterSearchMode::BindingString)
    {
      BindMethodPtr<HotKeyFilter, &HotKeyFilter::FilterBinding> filter(this);
      FilterNodes(filter, root);
      return;
    }
    else if (searchMode == FilterSearchMode::CommandTag)
    {
      BindMethodPtr<HotKeyFilter, &HotKeyFilter::FilterTag> filter(this);
      FilterNodes(filter, root);
      return;
    }
  }

  bool FilterCommand(DataEntry* dataEntry)
  {
    CommandEntry* row = ((CommandEntry*)dataEntry);

    int priority = PartialMatch(SubFilterString, row->mName.All(), CaseInsensitiveCompare);
    return priority != cNoMatch;
  }

  bool FilterBinding(DataEntry* dataEntry)
  {
    CommandEntry* row = ((CommandEntry*)dataEntry);

    int priority = PartialMatch(SubFilterString, row->mBindingStr.All(), CaseInsensitiveCompare);
    return priority != cNoMatch;
  }

  bool FilterTag(DataEntry* dataEntry)
  {
    CommandEntry* row = ((CommandEntry*)dataEntry);

    int priority = PartialMatch(SubFilterString, row->mTags.All(), CaseInsensitiveCompare);
    return priority != cNoMatch;
  }
};

class TreeViewSearchHotKeys : public TreeViewSearch
{
public:
  typedef TreeViewSearchHotKeys ZilchSelf;
  HotKeyFilter* mHotKeyFilter;

  TreeViewSearchHotKeys(Composite* parent) : TreeViewSearch(parent, nullptr, nullptr)
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

    ConnectMenu(menu, "By Command", OnCommand, true);
    ConnectMenu(menu, "By Binding ($)", OnBinding, true);
    ConnectMenu(menu, "By Tag (.)", OnTag, true);
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

// Simple adapter class to bind a engine selection to a DataSelection.
class CogCommandSelection : public DataSelection
{
public:
  MetaSelection* mSelection;
  HotKeyCommands* mSource;

  CogCommandSelection(MetaSelection* selection, HotKeyCommands* source) : mSelection(selection)
  {
    mSource = source;
  }

  void GetSelected(Array<DataIndex>& selected) override
  {
    if (Cog* primary = mSelection->GetPrimaryAs<Cog>())
      selected.PushBack(primary->GetId().ToUint64());
  }

  uint Size() override
  {
    return 1;
  }

  bool IsSelected(DataIndex index) override
  {
    CommandEntry* row = ((CommandEntry*)mSource->ToEntry(index));

    if (!row->mIsACogCommand)
      return false;

    Cog* object = ((CogCommand*)row->mZeroCommand)->mCog;
    return mSelection->Contains(object);
  }

  void Select(DataIndex index, bool sendsEvents) override
  {
    CommandEntry* row = ((CommandEntry*)mSource->ToEntry(index));

    if (!row->mIsACogCommand)
      return;

    Cog* object = ((CogCommand*)row->mZeroCommand)->mCog;
    if (object)
      mSelection->SelectOnly(object->GetComponentByName(row->mZeroCommand->Name));
  }

  void Deselect(DataIndex index) override
  {
    CommandEntry* row = ((CommandEntry*)mSource->ToEntry(index));

    if (!row->mIsACogCommand)
      return;

    Cog* object = ((CogCommand*)row->mZeroCommand)->mCog;
    mSelection->Remove(object);
  }

  void SelectNone(bool sendsEvents) override
  {
    mSelection->Clear((SendsEvents::Enum)sendsEvents);
  }

  void SelectionModified() override
  {
    mSelection->SelectionChanged();
    DataSelection::SelectionModified();
  }

  void SelectFinal() override
  {
    mSelection->FinalSelectionChanged();
    DataSelection::SelectFinal();
  }
};

#define CommandSortFunctor(name, operation)                                                                            \
  struct name : public binary_function<CommandEntry, CommandEntry, bool>                                               \
  {                                                                                                                    \
    bool operator()(const CommandEntry& left, const CommandEntry& right) const                                         \
    {                                                                                                                  \
      return left.operation(right);                                                                                    \
    }                                                                                                                  \
  };

bool CommandEntry::operator<(const CommandEntry& rhs) const
{
  return mName < rhs.mName;
}

bool CommandEntry::operator==(const CommandEntry& rhs) const
{
  return mName == rhs.mName;
}

bool CommandEntry::operator==(const Command& rhs) const
{
  return mName == rhs.Name;
}

CommandSortFunctor(CompareCogCommand, IsCogCommand) bool CommandEntry::IsCogCommand(const CommandEntry& rhs) const
{
  if (mIsACogCommand && !rhs.mIsACogCommand)
    return true;
  else if (!mIsACogCommand && rhs.mIsACogCommand)
    return false;
  else if (mIsACogCommand && rhs.mIsACogCommand)
    return LessName(rhs);
  else
    return false;
}

CommandSortFunctor(CompareNotCogCommand,
                   IsNotCogCommand) bool CommandEntry::IsNotCogCommand(const CommandEntry& rhs) const
{
  if (!mIsACogCommand && rhs.mIsACogCommand)
    return true;
  else if (mIsACogCommand && !rhs.mIsACogCommand)
    return false;
  else if (!mIsACogCommand && !rhs.mIsACogCommand)
    return LessName(rhs);
  else
    return false;
}

CommandSortFunctor(CompareCommandName, LessName) bool CommandEntry::LessName(const CommandEntry& rhs) const
{
  if (mName.Empty())
    return false;
  else if (rhs.mName.Empty())
    return true;
  else
    return mName < rhs.mName;
}

CommandSortFunctor(CompareCommandBinding, LessBinding) bool CommandEntry::LessBinding(const CommandEntry& rhs) const
{
  if (mBindingStr.Empty())
    return false;
  else if (rhs.mBindingStr.Empty())
    return true;
  else
    return mBindingStr < rhs.mBindingStr;
}

CommandSortFunctor(CompareCommandTags, LessTags) bool CommandEntry::LessTags(const CommandEntry& rhs) const
{
  if (mTags.Empty())
    return false;
  else if (rhs.mTags.Empty())
    return true;
  else if (mTags == rhs.mTags)
    return LessName(rhs);
  else
    return mTags < rhs.mTags;
}

HotKeyCommands::HotKeyCommands()
{
}

void HotKeyCommands::CopyCommandData(Array<Command*>& commands)
{
  // Already loaded, no need to do it again.
  if (!mCommand.Empty())
    return;

  forRange (Command* command, commands.All())
  {
    AddCommand(command);
  }
}

static void CopyCommand(CommandEntry& lhs, Command* rhs)
{
  lhs.mIsACogCommand = (rhs->ZilchGetDerivedType() == ZilchTypeId(CogCommand));
  lhs.mDevOnly = rhs->DevOnly;

  lhs.mZeroCommand = rhs;
  lhs.mName = rhs->Name;
  lhs.mDescription = rhs->Description;

  lhs.mIconName = rhs->IconName;
  lhs.mFunction = rhs->Function;

  lhs.mTags = rhs->Tags;

  lhs.mBindingStr = rhs->Shortcut.Replace("+", " + ");
}

void HotKeyCommands::AddCommand(Command* command, bool checkForDuplicate)
{
  if (checkForDuplicate)
  {
    if (mCommand.FindIndex(*command) != CommandSet::InvalidIndex)
      return;
  }

  CommandEntry& entry = mCommand.PushBack();
  entry.mIndex = mCommand.Size() - 1;

  CopyCommand(entry, command);

  entry.mModifier1 = (unsigned)Keys::Unknown;
  entry.mModifier2 = (unsigned)Keys::Unknown;
  entry.mMainKey = (unsigned)Keys::Unknown;
}

void HotKeyCommands::RemoveCommand(Command* command)
{
  mCommand.Erase(mCommand.FindPointer(*command));
}

DataEntry* HotKeyCommands::GetRoot()
{
  return &mCommand;
}

DataEntry* HotKeyCommands::ToEntry(DataIndex index)
{
  DataEntry* root = &mCommand;
  if (((DataEntry*)index.Id) == root)
    return ((DataEntry*)index.Id);
  else if (index.Id >= mCommand.Size())
    return nullptr;

  return &mCommand[(unsigned)index.Id];
}

DataIndex HotKeyCommands::ToIndex(DataEntry* dataEntry)
{
  DataEntry* root = &mCommand;
  if (dataEntry == root)
    return (DataIndex)((u64)dataEntry);

  return DataIndex(((CommandEntry*)dataEntry)->mIndex);
}

DataEntry* HotKeyCommands::Parent(DataEntry* dataEntry)
{
  // everyone but the root has the parent of the root
  DataEntry* root = &mCommand;
  if (dataEntry == root)
    return nullptr;

  return root;
}

uint HotKeyCommands::ChildCount(DataEntry* dataEntry)
{
  // only the root has children, no one else does
  DataEntry* root = &mCommand;
  if (dataEntry == root)
    return mCommand.Size();

  return 0;
}

DataEntry* HotKeyCommands::GetChild(DataEntry* dataEntry, uint index, DataEntry* prev)
{
  if (index >= 0 && index < mCommand.Size())
  {
    // mCommand[index].mIndex = index;
    return &mCommand[index];
  }

  DataEntry* root = &mCommand;
  if (dataEntry == root)
  {
    // mCommand.Front().mIndex = 0;
    return &mCommand.Front();
  }

  return nullptr;
}

bool HotKeyCommands::IsExpandable()
{
  return false;
}

bool HotKeyCommands::IsExpandable(DataEntry* dataEntry)
{
  // only the root is expandable
  DataEntry* root = &mCommand;
  if (dataEntry == root)
    return true;

  return false;
}

void HotKeyCommands::GetData(DataEntry* dataEntry, Any& variant, StringParam column)
{
  CommandEntry* row = ((CommandEntry*)dataEntry);

  if (column == cScriptColumn)
  {
    variant = nullptr;

    if (row->mIsACogCommand)
      variant = (CogCommand*)row->mZeroCommand;
  }
  else if (column == cCommandColumn)
  {
    variant = row->mName;
  }
  else if (column == cBindingColumn)
  {
    variant = row->mBindingStr;
  }
  else if (column == cTagsColumn)
  {
    variant = row->mTags;
  }
  else if (column == cDescriptionColumn)
  {
    variant = row->mDescription;
  }
}

bool HotKeyCommands::SetData(DataEntry* dataEntry, AnyParam variant, StringParam column)
{
  CommandEntry* row = ((CommandEntry*)dataEntry);

  if (column == cCommandColumn)
  {
    row->mName = variant.ToString();

    // Sort from root down.
    Sort(&mCommand, cCommandColumn, false);

    ObjectEvent objectEvent(this);
    DispatchEvent(Events::CommandRenamed, &objectEvent);
  }
  else if (column == cBindingColumn)
  {
    HotKeyBinding* binding = variant.Get<HotKeyBinding*>();

    int size = (int)mCommand.Size();
    for (int i = 0; i < size; ++i)
    {
      if (mCommand[i].mBindingStr == binding->mString)
      {
        if (mCommand[i].mName == row->mName)
          return false;

        DispatchEvent(Events::BindingOverwrite, new BindingConflictEvent(row, &mCommand[i], binding));
        return false;
      }
    }

    row->mBindingStr = binding->mString;
    row->mModifier1 = binding->mModifier1;
    row->mModifier2 = binding->mModifier2;
    row->mMainKey = binding->mMainKey;

    delete binding;
  }
  else if (column == cTagsColumn)
  {
    row->mTags = variant.ToString();
  }
  else if (column == cDescriptionColumn)
  {
    row->mDescription = variant.ToString();
  }

  MetaOperations::NotifyObjectModified(mCommand);
  return true;
}

bool HotKeyCommands::Remove(DataEntry* dataEntry)
{
  // Probably won't happen, as the root cannot be selected, but protect against
  // removal of the entire data source.
  DataEntry* root = &mCommand;
  if (dataEntry == root)
    return false;

  CommandEntry* row = ((CommandEntry*)dataEntry);

  // Index must be recorded before removing the entry from the DataSource.
  DataEvent e;
  e.Index = row->mIndex;

  mCommand.Erase(row);

  this->DispatchEvent(Events::DataRemoved, &e);
  return true;
}

ZilchDefineType(HotKeyEditor, builder, type)
{
}

HashMap<unsigned, String> HotKeyEditor::sKeyMap;

HotKeyEditor::HotKeyEditor(Composite* parent) :
    Composite(parent),
    mCogCommandSortToggle(true),
    mCurrentSort(CommandCompare::None)
{
  mHotKeys = HotKeyCommands::GetInstance();

  if (!cNotUsingHotKeyResource)
  {
    String userHotKeyFile = FilePath::Combine(GetUserDocumentsApplicationDirectory(), "Default.HotKeyDataSet.data");

    if (!FileExists(userHotKeyFile))
    {
      String coreHotKeyFile; // =
                             // HotKeyManager::GetDefault()->mContentItem->GetFullPath();

      CopyFileInternal(userHotKeyFile, coreHotKeyFile);
    }
  }

  ConnectThisTo(ZilchManager::GetInstance(), Events::ScriptsCompiledPostPatch, OnScriptsCompiled);

  CommandManager* commands = CommandManager::GetInstance();
  ConnectThisTo(commands, Events::CommandAdded, OnGlobalCommandAdded);
  ConnectThisTo(commands, Events::CommandRemoved, OnGlobalCommandRemoved);
  ConnectThisTo(commands, Events::CommandUpdated, OnGlobalCommandUpdated);

  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 2), Thickness::cZero));

  if (cHotKeysEditable)
    ConnectThisTo(mHotKeys, Events::CommandRenamed, OnRenamedCommand);

  MetaSelection* selection = Z::gEditor->GetSelection();

  // Search must be declared before the TreeView so that it shows up in the
  // layout before the TreeView.
  mSearch = new TreeViewSearchHotKeys(this);
  mSearch->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(20));

  mTreeView = new TreeView(this);
  mTreeView->SetSelection(new CogCommandSelection(selection, mHotKeys));

  mSearch->mTreeView = mTreeView;

  ConnectThisTo(mTreeView, Events::TreeViewHeaderAdded, OnTreeViewHeaderAdded);

  if (cHotKeysEditable)
  {
    ConnectThisTo(mTreeView, Events::TreeRightClick, OnCommandRightClick);
    ConnectThisTo(mTreeView, Events::KeyDown, OnKeyDown);
  }

  TreeFormatting formatting;
  BuildFormat(formatting);
  mTreeView->SetFormat(formatting);

  float sizeX = 0.0f;
  int columnCount = (int)formatting.Columns.Size();
  for (int x = 0; x < columnCount; ++x)
    sizeX += formatting.Columns[x].MinWidth * 4.0f;

  mTreeView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);
  mTreeView->SetSizing(SizeAxis::X, SizePolicy::Flex, sizeX);

  if (cHotKeysEditable)
  {
    mAddCommand = new TextButton(this);
    mAddCommand->SetText("Add Command");
    mAddCommand->SetSizing(SizePolicy::Fixed, mAddCommand->GetMinSize() + mAddCommand->GetPadding().Size());
    mAddCommand->mHorizontalAlignment = HorizontalAlignment::Right;

    ConnectThisTo(mAddCommand, Events::LeftMouseUp, OnAddCommand);

    mHotKeySetDropdown = new ComboBox(this);
    mHotKeySetDropdown->SetTranslation(Vec3(-5, -20, 0));
    mHotKeySetDropdown->mHorizontalAlignment = HorizontalAlignment::Right;
    mHotKeySetDropdown->SetSizing(SizePolicy::Fixed, Pixels(150, 20));

    ConnectThisTo(mHotKeySetDropdown, Events::ItemSelected, OnCommandSetSelected);

    ConnectThisTo(mHotKeys, Events::BindingOverwrite, OnConfirmBindingOverwrite);
  }

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
  sKeyMap[Keys::Comma] = ",";
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
  sKeyMap[Keys::Add] = "NumPadPlus";
  sKeyMap[Keys::Multiply] = "NumPadMultiply";
  sKeyMap[Keys::Subtract] = "NumPadMinus";
  sKeyMap[Keys::Divide] = "NumPadDivide";
  sKeyMap[Keys::Decimal] = "NumPadDecimal";
}

void HotKeyEditor::BuildFormat(TreeFormatting& formatting)
{
  formatting.Flags.SetFlag(FormatFlags::ShowHeaders);

  // script column (for CogCommands)
  ColumnFormat* format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->FixedSize = Pixels(24, 20);
  format->MinWidth = Pixels(24);
  format->Name = cScriptColumn;
  format->HeaderIcon = "EditScript";
  format->Editable = true;
  format->ColumnType = ColumnType::Fixed;
  format->CustomEditor = "CogCommandScriptEditor";

  // command column
  format = &formatting.Columns.PushBack();
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
  format->HeaderName = "Shortcut";
  format->ColumnType = ColumnType::Flex;
  format->FlexSize = 2;
  format->MinWidth = Pixels(100.0f);
  format->Editable = true;
  format->CustomEditor = "ShortcutEditor";

  // tag column
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = cTagsColumn;
  format->HeaderName = "Tags";
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
  format->MinWidth = Pixels(600.0f);
  format->Editable = true;
  format->CustomEditor = "DescriptionEditor";
}

void HotKeyEditor::UpdateTransform()
{
  Composite::UpdateTransform();
}

void HotKeyEditor::Refresh()
{
  mSearch->CancelFilter();
  mTreeView->SetDataSource(mHotKeys);
}

void HotKeyEditor::DisplayResource()
{
  mTreeView->SetDataSource(mHotKeys);

  if (cHotKeysEditable)
    mHotKeySetDropdown->SetListSource(&mSetNames);
}

bool HotKeyEditor::TakeFocusOverride()
{
  this->HardTakeFocus();
  return true;
}

void HotKeyEditor::AutoClose()
{
  if (HasFocus())
    LoseFocus();

  CloseTabContaining(this);
}

void HotKeyEditor::OnCancel(SearchViewEvent* event)
{
  AutoClose();
}

void HotKeyEditor::OnScriptsCompiled(Event*)
{
  mHotKeys->mCommand.Clear();

  CommandManager* commands = CommandManager::GetInstance();
  HotKeyCommands::GetInstance()->CopyCommandData(commands->mCommands);

  if (mCurrentSort != CommandCompare::None)
    Sort(true, mCurrentSort);

  Refresh();
}

void HotKeyEditor::OnCommandRename(ObjectEvent* event)
{
  TreeRow* row = mTreeView->FindRowByIndex(mRightClickedRowIndex);
  row->Edit(cCommandColumn);

  Sort(true);

  Refresh();

  MetaOperations::NotifyObjectModified(mHotKeys->mCommand);
}

void HotKeyEditor::OnCommandRebind(ObjectEvent* event)
{
  TreeRow* row = mTreeView->FindRowByIndex(mRightClickedRowIndex);

  row->Edit(cBindingColumn);
}

void HotKeyEditor::OnCommandDelete(ObjectEvent* event)
{
  TreeRow* row = mTreeView->FindRowByIndex(mRightClickedRowIndex);
  row->Remove(); // dispatch event

  mTreeView->mRows.EraseValue(row);

  Sort(false);

  int size = (int)mHotKeys->mCommand.Size();
  for (int i = 0; i < size; ++i)
  {
    mHotKeys->mCommand[i].mIndex = i;

    // +1 as the first row is the header
    mTreeView->mRows[i + 1]->mIndex = i;
    mTreeView->mRows[i + 1]->mVisibleRowIndex = i;
  }

  Array<unsigned> toErase;

  HashDataSelection* data = (HashDataSelection*)mTreeView->GetSelection();

  if (data->mSelection.Empty())
    return;

  forRange (u64 selection, data->mSelection.All())
  {
    mHotKeys->mCommand.EraseAt((unsigned)selection);
    toErase.PushBack((unsigned)selection);
  }

  UpdateIndexes();

  // deselect UI elements
  data->mSelection.Clear();

  Refresh();

  MetaOperations::NotifyObjectModified(mHotKeys->mCommand);
}

void HotKeyEditor::OnCommandRightClick(TreeEvent* event)
{
  ContextMenu* menu = new ContextMenu(event->Row);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0, 0));
  mRightClickedRowIndex = event->Row->mIndex;
  ConnectMenu(menu, "Rename", OnCommandRename, false);
  ConnectMenu(menu, "Rebind", OnCommandRebind, false);
  ConnectMenu(menu, "Delete", OnCommandDelete, false);
}

void HotKeyEditor::UpdateIndexes(int start)
{
  int size = (int)mHotKeys->mCommand.Size();
  for (int i = start; i < size; ++i)
    mHotKeys->mCommand[i].mIndex = i;
}

void HotKeyEditor::Sort(bool updateIndexes, CommandCompare::Enum sortBy)
{
  CommandSet& set = mHotKeys->mCommand;

  int i = 0;
  CommandSet::range sortRange = set.All();
  while (sortRange[0].mName == cDefaultCommandString || sortRange[0].mBindingStr == cDefaultBindingString)
  {
    sortRange.PopFront();
    ++i;
  }

  mCurrentSort = sortBy;

  switch (sortBy)
  {
  case CommandCompare::IsCogCommand:
    QuickSort(sortRange.Begin(), sortRange.End(), &sortRange.Front(), CompareCogCommand());
    break;

  case CommandCompare::IsNotCogCommand:
    QuickSort(sortRange.Begin(), sortRange.End(), &sortRange.Front(), CompareNotCogCommand());
    break;

  case CommandCompare::None:
  case CommandCompare::CommandName:
    QuickSort(sortRange.Begin(), sortRange.End(), &sortRange.Front(), CompareCommandName());
    break;

  case CommandCompare::CommandBinding:
    QuickSort(sortRange.Begin(), sortRange.End(), &sortRange.Front(), CompareCommandBinding());
    break;

  case CommandCompare::CommandTags:
    QuickSort(sortRange.Begin(), sortRange.End(), &sortRange.Front(), CompareCommandTags());
    break;
  }

  mTreeView->mArea->SetScrolledPercentage(Vec2(0, 0));

  if (updateIndexes)
    UpdateIndexes(i);
}

void HotKeyEditor::OnCogCommandSort(MouseEvent* event)
{
  if (mCogCommandSortToggle)
  {
    mCogCommandSortToggle = false;
    Sort(true, CommandCompare::IsCogCommand);

    if (ToolTip* toolTip = mSortToolTip)
    {
      toolTip->ClearText();
      toolTip->AddText("Click to sort by:", Vec4(1));
      toolTip->AddText("  Zero Commands", cZeroCommandColor);
    }
  }
  else
  {
    mCogCommandSortToggle = true;
    Sort(true, CommandCompare::IsNotCogCommand);

    if (ToolTip* toolTip = mSortToolTip)
    {
      toolTip->ClearText();
      toolTip->AddText("Click to sort by:", Vec4(1));
      toolTip->AddText("  User Commands", Vec4(1));
    }
  }

  Refresh();
}

void HotKeyEditor::OnCommandNameSort(MouseEvent* event)
{
  // Reset command type sorting to default of: click to sort by user commands.
  mCogCommandSortToggle = true;

  Sort(true);
  Refresh();
}

void HotKeyEditor::OnCommandBindingSort(MouseEvent* event)
{
  // Reset command type sorting to default of: click to sort by user commands.
  mCogCommandSortToggle = true;

  Sort(true, CommandCompare::CommandBinding);
  Refresh();
}

void HotKeyEditor::OnCommandTagsSort(MouseEvent* event)
{
  // Reset command type sorting to default of: click to sort by user commands.
  mCogCommandSortToggle = true;

  Sort(true, CommandCompare::CommandTags);
  Refresh();
}

void HotKeyEditor::CreateCommandHeaderToolTip(Widget* source, StringParam sortName, Vec4Param color)
{
  mSortToolTip.SafeDestroy();
  mSortToolTip = new ToolTip(source);

  ToolTip* toolTip = mSortToolTip;
  toolTip->SetDestroyOnMouseExit(false);

  toolTip->AddText("Click to sort by:", Vec4(1));
  toolTip->AddText(BuildString("  ", sortName), color);

  ToolTipPlacement placement;
  placement.SetScreenRect(source->GetScreenRect());
  placement.SetPriority(IndicatorSide::Left, IndicatorSide::Right, IndicatorSide::Bottom, IndicatorSide::Top);

  toolTip->SetArrowTipTranslation(placement);
}

void HotKeyEditor::OnMouseEnterIconHeader(MouseEvent* event)
{
  Widget* source = mTreeView->mHeaders[0]->mBackground;

  if (mCogCommandSortToggle)
    CreateCommandHeaderToolTip(source, "User Commands", Vec4(1));
  else
    CreateCommandHeaderToolTip(source, "Native Commands", cZeroCommandColor);
}

void HotKeyEditor::OnMouseEnterCommandHeader(MouseEvent* event)
{
  Widget* source = mTreeView->mHeaders[1]->mBackground;

  String sortString = BuildString(cCommandColumn, " Name");
  CreateCommandHeaderToolTip(source, sortString, Vec4(1));
}

void HotKeyEditor::OnMouseEnterBindingHeader(MouseEvent* event)
{
  Widget* source = mTreeView->mHeaders[2]->mBackground;
  CreateCommandHeaderToolTip(source, cBindingColumn, Vec4(1));
}

void HotKeyEditor::OnMouseEnterTagsHeader(MouseEvent* event)
{
  Widget* source = mTreeView->mHeaders[3]->mBackground;
  CreateCommandHeaderToolTip(source, cTagsColumn, Vec4(1));
}

void HotKeyEditor::OnMouseExitHeader(MouseEvent* event)
{
  mSortToolTip.SafeDestroy();
}

void HotKeyEditor::OnKeyDown(KeyboardEvent* event)
{
  // only delete if not editing a command, as 'delete' is a valid hotkey
  if (event->Handled)
    return;

  // only key with functionality currently (if not editing a command/binding)
  if (event->Key == Keys::Delete)
  {
    // HashDataSelection* data = (HashDataSelection*)mTreeView->GetSelection();
    // forRange(u64 selection, data->mSelection.All())
    //{
    //  mHotKeys->mSet->mCommand.EraseAt((unsigned)selection);
    //}

    // mRightClickedRowIndex = event->Row->mIndex;
    OnCommandDelete(nullptr);
  }
}

void HotKeyEditor::OnTreeViewHeaderAdded(TreeViewHeaderAddedEvent* event)
{
  uint index = event->mHeaderIndex;

  // Left to right - headers beyond the tags header do not have sorting
  // capability.
  if (index > Tags)
    return;

  ColumnHeader* header = event->mNewHeader;

  MouseEventHandler handler[8] = {&HotKeyEditor::OnCogCommandSort,
                                  &HotKeyEditor::OnCommandNameSort,
                                  &HotKeyEditor::OnCommandBindingSort,
                                  &HotKeyEditor::OnCommandTagsSort,
                                  &HotKeyEditor::OnMouseEnterIconHeader,
                                  &HotKeyEditor::OnMouseEnterCommandHeader,
                                  &HotKeyEditor::OnMouseEnterBindingHeader,
                                  &HotKeyEditor::OnMouseEnterTagsHeader};

  Zero::Connect(header, Events::LeftMouseUp, this, handler[index]);
  Zero::Connect(header, Events::MouseEnterHierarchy, this, handler[index + 4]);
  ConnectThisTo(header, Events::MouseExitHierarchy, OnMouseExitHeader);
}

void HotKeyEditor::OnGlobalCommandAdded(CommandUpdateEvent* event)
{
  mHotKeys->AddCommand(event->mCommand, true);

  Sort(true);

  Refresh();
}

void HotKeyEditor::OnGlobalCommandRemoved(CommandUpdateEvent* event)
{
  DataIndex i = mHotKeys->mCommand.FindIndex(*event->mCommand);
  TreeRow* row = mTreeView->FindRowByIndex(i);

  // Dispatch data source remove event.
  row->Remove();
  UpdateIndexes();

  // Don't need to delete the row from the TreeView, as 'Refresh' will cause
  // the TreeView's UI elements [ie, TreeRows] to update.
  Refresh();
}

void HotKeyEditor::OnGlobalCommandUpdated(CommandUpdateEvent* event)
{
  size_t index = mHotKeys->mCommand.FindIndex(*event->mCommand);

  if (index == CommandSet::InvalidIndex)
    return;

  CopyCommand(mHotKeys->mCommand[index], event->mCommand);

  // Shortcut or Tags could have changed.  If the commands are sorted by either
  // of these, then a resort needs to occur.
  if (mCurrentSort != CommandCompare::None)
    Sort(true, mCurrentSort);

  Refresh();
}

void HotKeyEditor::OnRenamedCommand(ObjectEvent* event)
{
  Refresh();
}

void HotKeyEditor::OnAddCommand(MouseEvent* event)
{
  mHotKeys->mCommand.InsertAt(0, CommandEntry());

  mHotKeys->mCommand[0].mName = cDefaultCommandString;
  mHotKeys->mCommand[0].mBindingStr = cDefaultBindingString;

  // Sort(mHotKeys->mCommand.All());
  UpdateIndexes();
  Refresh();

  MetaOperations::NotifyObjectModified(mHotKeys->mCommand);
}

void HotKeyEditor::OnCommandSetSelected(ObjectEvent* event)
{
  int index = mHotKeySetDropdown->GetSelectedItem();
  if (index < 0 || index >= int(mSetNames.Strings.Size()))
    return;

  mTreeView->ClearAllRows();
  mTreeView->mRows[0]->mChildren.Clear();
  mTreeView->mRows.Clear();
  mTreeView->mRowMap.Clear();
  mTreeView->Refresh();

  // mHotKeys->mSet = (HotKeyDataSet
  // *)mHotKeyManager->GetResource(mSetNames.Strings[index],
  // ResourceNotFound::ReturnNull);
  mTreeView->SetDataSource(mHotKeys);
}

void HotKeyEditor::OnConfirmBindingOverwrite(BindingConflictEvent* event)
{
  String prompt(BuildString("Overwrite Command: \'", event->mExistingCommad->mName, "\'?"));
  ModalConfirmAction* modal = new ModalConfirmAction(this, prompt);
  modal->mUserData = event;

  ConnectThisTo(modal, Events::ModalConfirmResult, OnModalOption);
  ConnectThisTo(modal, Events::ModalClosed, OnModalClosed);
}

void HotKeyEditor::OnModalOption(ModalConfirmEvent* event)
{
  BindingConflictEvent* effectedData = (BindingConflictEvent*)event->mUserData;

  if (event->mConfirmed)
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
  effectedData = nullptr;

  Refresh();

  MetaOperations::NotifyObjectModified(mHotKeys->mCommand);
}

void HotKeyEditor::OnModalClosed(ModalConfirmEvent* event) // unused, not firing
{
  BindingConflictEvent* effectedData = (BindingConflictEvent*)event->mUserData;

  if (effectedData == nullptr)
    return;

  effectedData->mNewCommand->mBindingStr.Clear();
  effectedData->mNewCommand->mModifier1 = Keys::Unknown;
  effectedData->mNewCommand->mModifier2 = Keys::Unknown;
  effectedData->mNewCommand->mMainKey = Keys::Unknown;

  delete effectedData->mNewBinding;
  delete effectedData;

  MetaOperations::NotifyObjectModified(mHotKeys->mCommand);
}

ZilchDefineType(HotKeyBinding, builder, type)
{
  ZilchBindFieldProperty(mModifier1);
  ZilchBindFieldProperty(mModifier2);
  ZilchBindFieldProperty(mMainKey);
}

void RegisterHotKeyEditors()
{
  ValueEditorFactory* factory = ValueEditorFactory::GetInstance();
  factory->RegisterEditor("CogCommandScriptEditor", CreateCogCommandScriptEditor);
  factory->RegisterEditor("CommandEditor", CreateCommandEditor);
  factory->RegisterEditor("ShortcutEditor", CreateBindingEditor);
  factory->RegisterEditor("TagEditor", CreateTagEditor);
  factory->RegisterEditor("DescriptionEditor", CreateDescriptionEditor);
}

} // namespace Zero
