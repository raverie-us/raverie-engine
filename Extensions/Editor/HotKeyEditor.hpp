///////////////////////////////////////////////////////////////////////////////
///
/// \file HotKeyEditor.hpp
/// Declaration of the HotKey Editor class.
///
/// Authors: Ryan Edgemon
/// Copyright 2015-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
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

//------------------------------------------------------------- CommandEntry ---
struct CommandEntry
{
  bool operator<(const CommandEntry& rhs) const;

  bool mDevOnly;

  unsigned mIndex;

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

//----------------------------------------------------------- HotKeyCommands ---
class HotKeyCommands : public ExplicitSingleton<HotKeyCommands, DataSource>
{
public:
  ZilchDeclareType(HotKeyCommands, TypeCopyMode::ReferenceType);

  HotKeyCommands( );

  void CopyCommandData(Array<Command*>& commands);

  DataEntry* GetRoot( ) override;

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

//------------------------------------------------------------- HotKeyEditor ---
class HotKeyEditor : public Composite
{
public:
  ZilchDeclareType(HotKeyEditor, TypeCopyMode::ReferenceType);

  HotKeyEditor(Composite* parent);
  void BuildFormat(TreeFormatting& formatting);

  void UpdateTransform( ) override;

  void DisplayResource( );
 
  bool TakeFocusOverride( ) override;

  void AutoClose( );
  void OnCancel(SearchViewEvent* event);
  
  void OnCommandRename(ObjectEvent* event);
  void OnCommandRebind(ObjectEvent* event);
  void OnCommandDelete(ObjectEvent* event);
  void OnCommandRightClick(TreeEvent* event);

  void OnKeyDown(KeyboardEvent* event);
  
  void OnSelectionChanged(Event* event);

  void OnRenamedCommand(ObjectEvent* event);
  void OnAddCommand(MouseEvent* event);  /// Add button call back
  void OnCommandSetSelected(ObjectEvent* event);  /// ComboBox (set dropdown) call back

  void OnConfirmBindingOverwrite(BindingConflictEvent *event);

  void OnModalOption(ModalConfirmEvent* event);
  void OnModalClosed(ModalConfirmEvent* event);


public:
  static HashMap<unsigned, String> sKeyMap;  // <Keys::Enum, "CommandName">

  DataIndex mRowIndex;

  TreeView* mTreeView;
  TextButton* mAddCommand;
  ComboBox* mHotKeySetDropdown;

  StringSource mSetNames;

  HotKeyCommands* mHotKeys;
};

//------------------------------------------------------------ HotKeyBinding ---
class HotKeyBinding : public Object
{
public:
  ZilchDeclareType(HotKeyBinding, TypeCopyMode::ReferenceType);

  unsigned mModifier1;
  unsigned mModifier2;
  unsigned mMainKey;

  String mString;

  HotKeyBinding(unsigned m1, unsigned m2, unsigned mk, StringParam bindStr)
    : mModifier1(m1), mModifier2(m2), mMainKey(mk), mString(bindStr) {}

};

//------------------------------------------------------------------------------

void HotKeySortHelper(bool updateIndexes, CommandSet& set);
void RegisterHotKeyEditors( );


}//namespace Zero
