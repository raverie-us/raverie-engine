// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

namespace Events
{
DeclareEvent(CommandRenamed);
}

class TreeView;
class TreeEvent;
struct TreeFormatting;

struct SearchData;
class SearchViewEvent;

class ResourceLibraryDataSource;

class BindingConflictEvent;
class ModalConfirmEvent;

DeclareEnum6(CommandCompare, IsCogCommand, IsNotCogCommand, CommandName, CommandBinding, CommandTags, None);

struct CommandEntry
{
  bool operator<(const CommandEntry& rhs) const;
  bool operator==(const CommandEntry& rhs) const;
  bool operator==(const Command& rhs) const;

  bool IsCogCommand(const CommandEntry& rhs) const;
  bool IsNotCogCommand(const CommandEntry& rhs) const;
  bool LessName(const CommandEntry& rhs) const;
  bool LessBinding(const CommandEntry& rhs) const;
  bool LessTags(const CommandEntry& rhs) const;

  bool mIsACogCommand;
  bool mDevOnly;

  unsigned mIndex;

  Command* mZeroCommand;

  String mName;
  String mDescription;

  String mIconName;
  String mFunction;

  String mTags;

  String mBindingStr;
  unsigned mModifier1;
  unsigned mModifier2;
  unsigned mMainKey;
};

typedef Array<CommandEntry> CommandSet;

class HotKeyCommands : public ExplicitSingleton<HotKeyCommands, DataSource>
{
public:
  ZilchDeclareType(HotKeyCommands, TypeCopyMode::ReferenceType);

  HotKeyCommands();

  void CopyCommandData(Array<Command*>& commands);

  void AddCommand(Command* command, bool checkForDuplicate = false);
  void RemoveCommand(Command* command);

  DataEntry* GetRoot() override;

  DataEntry* ToEntry(DataIndex index) override;
  DataIndex ToIndex(DataEntry* dataEntry) override;

  DataEntry* Parent(DataEntry* dataEntry) override;
  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override;

  uint ChildCount(DataEntry* dataEntry) override;

  bool IsExpandable() override;
  bool IsExpandable(DataEntry* dataEntry) override;

  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override;
  bool SetData(DataEntry* dataEntry, AnyParam variant, StringParam column) override;

  bool Remove(DataEntry* dataEntry) override;

public:
  CommandSet mCommand;
};

class HotKeyEditor : public Composite
{
public:
  ZilchDeclareType(HotKeyEditor, TypeCopyMode::ReferenceType);

  typedef void (HotKeyEditor::*MouseEventHandler)(MouseEvent*);

  enum Headers
  {
    CommandType = 0,
    Name,
    Binding,
    Tags,
    Description
  };

  HotKeyEditor(Composite* parent);
  void BuildFormat(TreeFormatting& formatting);

  void UpdateTransform() override;
  void Refresh();

  void DisplayResource();

  bool TakeFocusOverride() override;

  void AutoClose();
  void OnCancel(SearchViewEvent* event);

  void OnScriptsCompiled(Event* event);
  void OnCommandRename(ObjectEvent* event);
  void OnCommandRebind(ObjectEvent* event);
  void OnCommandDelete(ObjectEvent* event);
  void OnCommandRightClick(TreeEvent* event);

  void UpdateIndexes(int start = 0);
  void Sort(bool updateIndexes, CommandCompare::Enum sortBy = CommandCompare::CommandName);

  void OnCogCommandSort(MouseEvent* event);
  void OnCommandNameSort(MouseEvent* event);
  void OnCommandBindingSort(MouseEvent* event);
  void OnCommandTagsSort(MouseEvent* event);

  void CreateCommandHeaderToolTip(Widget* source, StringParam sortName, Vec4Param color);

  void OnMouseEnterIconHeader(MouseEvent* event);
  void OnMouseEnterCommandHeader(MouseEvent* event);
  void OnMouseEnterBindingHeader(MouseEvent* event);
  void OnMouseEnterTagsHeader(MouseEvent* event);
  void OnMouseExitHeader(MouseEvent* event);

  void OnKeyDown(KeyboardEvent* event);

  void OnTreeViewHeaderAdded(TreeViewHeaderAddedEvent* event);

  void OnGlobalCommandAdded(CommandUpdateEvent* event);
  void OnGlobalCommandRemoved(CommandUpdateEvent* event);
  void OnGlobalCommandUpdated(CommandUpdateEvent* event);

  void OnRenamedCommand(ObjectEvent* event);
  void OnAddCommand(MouseEvent* event);          /// Add button call back
  void OnCommandSetSelected(ObjectEvent* event); /// ComboBox (set dropdown) call back

  void OnConfirmBindingOverwrite(BindingConflictEvent* event);

  void OnModalOption(ModalConfirmEvent* event);
  void OnModalClosed(ModalConfirmEvent* event);

public:
  static HashMap<unsigned, String> sKeyMap; // <Keys::Enum, "CommandName">

  bool mCogCommandSortToggle;

  DataIndex mRightClickedRowIndex;

  TreeViewSearch* mSearch;
  TreeView* mTreeView;
  TextButton* mAddCommand;
  ComboBox* mHotKeySetDropdown;

  HandleOf<ToolTip> mSortToolTip;
  CommandCompare::Enum mCurrentSort;

  StringSource mSetNames;

  HotKeyCommands* mHotKeys;
};

class HotKeyBinding : public Object
{
public:
  ZilchDeclareType(HotKeyBinding, TypeCopyMode::ReferenceType);

  unsigned mModifier1;
  unsigned mModifier2;
  unsigned mMainKey;

  String mString;

  HotKeyBinding(unsigned m1, unsigned m2, unsigned mk, StringParam bindStr) :
      mModifier1(m1),
      mModifier2(m2),
      mMainKey(mk),
      mString(bindStr)
  {
  }
};

void RegisterHotKeyEditors();

} // namespace Zero
