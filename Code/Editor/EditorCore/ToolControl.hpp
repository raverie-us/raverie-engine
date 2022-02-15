// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class Editor;
class ComboBox;
class Tool;
class SelectTool;
class PropertyView;
class ToolControl;
class CreationTool;
class ScrollArea;
class ResourceEvent;
class PropertyInterface;

DeclareEnum2(ShowToolProperties, Show, Auto);

namespace Events
{
DeclareEvent(GetToolInfo);
DeclareEvent(SelectTool);
DeclareEvent(ShortcutInfoEnter);
} // namespace Events

/// Allows Ui customization for Tools. This will be sent on the Tool every time
/// it is activated.
class ToolUiEvent : public Event
{
public:
  ZilchDeclareType(ToolUiEvent, TypeCopyMode::ReferenceType);

  /// Constructor.
  ToolUiEvent(Composite* parent);

  /// Getters / setters.
  Composite* GetParent();
  void SetCustomUi(Composite* customUi);
  Composite* GetCustomUi();
  Cog* GetSelectTool();

  /// Whether or not to force show the tools window when switched to this tool.
  bool mNeedsPropertyGrid;
  Composite* mParent;
  Composite* mCustomUi;
  /// Easy access to the Select Tool. It's commonly used in other Tools (such as
  /// ray casting).
  Cog* mSelectTool;
};

class ToolData
{
public:
  /// Constructors.
  ToolData()
  {
  }
  ToolData(Archetype* archetype);
  ToolData(BoundType* componentMeta);

  ~ToolData();

  String ToString(bool shortFormat = false) const;
  String GetName() const;

  Space* GetSpace();

  /// If it was created from an Archetype with the [Tool] tag.
  HandleOf<Archetype> mArchetype;

  /// If it was created from a script component with the 'autoRegister'
  /// property set to true on the [Tool] attribute.
  BoundTypeHandle mScriptComponentType;

  HandleOf<Space> mSpace;
  HandleOf<Cog> mCog;
};

class ToolObjectManager : public EditorScriptObjects<ToolData>
{
public:
  /// Constructor.
  ToolObjectManager(ToolControl* toolControl);
  ~ToolObjectManager();

  /// EditorScriptObject Interface.
  void AddObject(ToolData* object) override;
  void RemoveObject(ToolData* object) override;
  ToolData* GetObject(StringParam objectName) override;
  uint GetObjectCount() override;
  ToolData* GetObject(uint index) override;
  ToolData* UpdateData(StringParam objectName) override;
  Space* GetSpace(ToolData* object) override;
  void CreateOrUpdateCog(ToolData* object) override;

  /// All registered tools.
  Array<ToolData*> mToolArray;

  ToolControl* mToolControl;
};

class ToolControl : public Composite
{
public:
  ZilchDeclareType(ToolControl, TypeCopyMode::ReferenceType);

  ToolControl(Composite* parent);
  ~ToolControl();

  /// Composite Interface.
  void UpdateTransform() override;

  /// Creates a tool with the given archetype. If a tool already exists,
  /// it will re-create the tool to reflect any changes in the Archetype.
  Cog* AddOrUpdateTool(Archetype* toolArchetype);

  /// Removes the tool of the given archetype.
  void RemoveTool(Archetype* toolArchetype);

  /// Returns the active tool cog.
  Cog* GetActiveCog();
  Cog* GetToolByName(StringParam typeName);

  /// Returns the PropertyGrid widget.
  PropertyView* GetPropertyGrid() const;

  /// Tool Selection.
  void SelectToolIndex(uint index, ShowToolProperties::Enum showTool = ShowToolProperties::Auto);
  void SelectToolName(StringParam toolName, ShowToolProperties::Enum showTool = ShowToolProperties::Auto);

  /// Whether or not the default selection tool is currently selected.
  bool IsSelectToolActive();

  /// Update the shortcut tool when hovering of info icon and tool is changed.
  void UpdateShortcutsTip();

  SelectTool* mSelectTool;
  CreationTool* mCreationTool;

  /// The currently selected tool.
  ToolData* mActiveTool;
  ToolObjectManager mTools;

private:
  friend class ToolObjectManager;

  /// Display the mouse/keyboard shortcuts unique to the selected tool.
  void OnInfoMouseEnter(MouseEvent*);
  void OnInfoMouseExit(MouseEvent*);

  /// Change the tool when selected in the dropdown.
  void OnToolPulldownSelect(ObjectEvent*);

  /// We want to forward keyboard input to the last viewport to execute
  /// shortcuts.
  void OnKeyDown(KeyboardEvent* e);

  /// When scripts are compiled, re-select the active tool to refresh any
  /// changes to script tools.
  void OnScriptsCompiled(Event*);

  void SelectToolInternal(ToolData* tool, ShowToolProperties::Enum showTool);

  void BuildShortcutsToolTip(const ShortcutSet* entries);
  void BuildShorcutsFormat(TreeFormatting* formatting);

  PropertyInterface* mPropertyInterface;
  Editor* mEditor;
  Element* mInfoIcon;
  ComboBox* mToolBox;

  PropertyView* mPropertyGrid;
  ScrollArea* mScrollArea;
  Composite* mCustomUi;

  HandleOf<ToolTip> mShortcutsTip;
  ListView* mShortcutsView;

  UniquePointer<ListSource> mToolSource;
  ShortcutSource mShortcutSource;
};

} // namespace Zero
