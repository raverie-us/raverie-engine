///////////////////////////////////////////////////////////////////////////////
///
/// \file ToolControl.hpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
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
}

//---------------------------------------------------------------- Tool Ui Event
class ToolUiEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

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
  Cog* mSelectTool;
};

//-------------------------------------------------------------------- Tool Data
class ToolData
{
public:
  /// Constructors.
  ToolData(){}
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

//---------------------------------------------------------- Tool Object Manager
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
  Space* GetSpace(ToolData* object) override;
  void CreateOrUpdateCog(ToolData* object) override;

  /// All registered tools.
  Array<ToolData*> mToolArray;

  ToolControl* mToolControl;
};

//----------------------------------------------------------------- Tool Control
class ToolControl : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

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

  /// Tool Selection.
  void SelectToolIndex(uint index, ShowToolProperties::Enum showTool = ShowToolProperties::Auto);
  void SelectToolName(StringParam toolName, ShowToolProperties::Enum showTool  = ShowToolProperties::Auto);

  /// Whether or not the default selection tool is currently selected.
  bool IsSelectToolActive();

  SelectTool* mSelectTool;
  CreationTool* mCreationTool;

  /// The currently selected tool.
  ToolData* mActiveTool;
  ToolObjectManager mTools;

private:
  friend class ToolObjectManager;

  /// Change the tool when selected in the dropdown.
  void OnToolPulldownSelect(ObjectEvent*);

  /// We want to forward keyboard input to the last viewport to execute shortcuts.
  void OnKeyDown(KeyboardEvent* e);

  /// When scripts are compiled, re-select the active tool to refresh any
  /// changes to script tools.
  void OnScriptsCompiled(Event*);

  void SelectToolInternal(ToolData* tool, ShowToolProperties::Enum showTool);

  PropertyInterface* mPropertyInterface;
  Editor* mEditor;
  ComboBox* mToolBox;
  PropertyView* mPropertyGrid;
  UniquePointer<ListSource> mToolSource;
  ScrollArea* mScrollArea;
  Composite* mCustomUi;
};

}//namespace Zero
