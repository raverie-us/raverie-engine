// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

#define InfoSpan (8.0f + mInfoIcon->mSize.x + 6.0f)

namespace Events
{
DefineEvent(GetToolInfo);
DefineEvent(ShortcutInfoEnter);
} // namespace Events

ZilchDefineType(ToolUiEvent, builder, type)
{
  ZilchBindFieldProperty(mNeedsPropertyGrid);
  ZilchBindGetterProperty(Parent);
  ZilchBindGetterProperty(SelectTool);
  ZeroBindDocumented();
}

ToolUiEvent::ToolUiEvent(Composite* parent) :
    mParent(parent),
    mNeedsPropertyGrid(false),
    mCustomUi(nullptr),
    mSelectTool(nullptr)
{
}

Composite* ToolUiEvent::GetParent()
{
  return mParent;
}

void ToolUiEvent::SetCustomUi(Composite* customUi)
{
  mCustomUi = customUi;
}

Composite* ToolUiEvent::GetCustomUi()
{
  return mCustomUi;
}

Cog* ToolUiEvent::GetSelectTool()
{
  return mSelectTool;
}

// We're using a custom property interface to only show components marked with
// the 'Tool' tag. This allows the author of the tool to hide irrelevant
// components. We're also going to hide the ScriptSource Property.
class ToolPropertyInterface : public PropertyInterface
{
public:
  ToolPropertyInterface()
  {
  }

  ObjectPropertyNode* BuildObjectTree(ObjectPropertyNode* parent,
                                      HandleParam object,
                                      Property* objectProperty = nullptr) override
  {
    BoundType* objectType = object.StoredType;
    if (objectType->IsA(ZilchTypeId(Cog)))
    {
      // All components with the "Tool" tag
      static Array<uint> sValidComponentIndices;
      sValidComponentIndices.Clear();

      MetaComposition* cogComposition = objectType->HasInherited<MetaComposition>();
      // The amount of components we have contained
      uint objectCount = cogComposition->GetComponentCount(object);

      // Find all components with the "Tool" tag
      for (uint i = 0; i < objectCount; ++i)
      {
        Handle subInstance = cogComposition->GetComponentAt(object, i);

        if (subInstance.StoredType->HasAttributeInherited(ObjectAttributes::cTool))
        {
          sValidComponentIndices.PushBack(i);
          break;
        }
      }

      if (sValidComponentIndices.Size() == 1)
      {
        uint componentIndex = sValidComponentIndices.Front();
        Handle subInstance = cogComposition->GetComponentAt(object, componentIndex);
        return BuildObjectTree(parent, subInstance);
      }
      else
      {
        ObjectPropertyNode* node = new ObjectPropertyNode(parent, object, objectProperty);

        // Loop through and add each one
        for (uint i = 0; i < sValidComponentIndices.Size(); ++i)
        {
          uint componentIndex = sValidComponentIndices[i];
          Handle subInstance = cogComposition->GetComponentAt(object, componentIndex);
          BoundType* componentType = subInstance.StoredType;
          ErrorIf(componentType == nullptr, "Contained object does not have meta initialized.");

          // Don't show hidden sub objects
          if (componentType->HasAttribute(ObjectAttributes::cHidden))
            continue;

          // Create a new node for this sub object
          ObjectPropertyNode* subNode = BuildObjectTree(node, subInstance);
          node->mContainedObjects.PushBack(subNode);
        }

        return node;
      }
    }

    ObjectPropertyNode* node = PropertyInterface::BuildObjectTree(parent, object, objectProperty);
    RemoveScriptSourceProperties(node);
    return node;
  }

  void RemoveScriptSourceProperties(ObjectPropertyNode* node)
  {
    forRange (ObjectPropertyNode* propertyNode, node->mProperties.All())
    {
      if (!propertyNode->mProperty)
        continue;

      Property* metaProperty = propertyNode->mProperty;
      if (metaProperty->Name == "ScriptSource")
      {
        node->mProperties.EraseValueError(propertyNode);
        delete propertyNode;
        break;
      }
    }
  }

  /* METAREFACTOR - Not sure how to resolve these yet
  //
  bool SupportsComponentRemoval() override { return false; }

  //
  bool CanAddComponents(HandleParam instance) override { return false; }
  */
};

ToolData::ToolData(Archetype* archetype) : mArchetype(archetype), mScriptComponentType(nullptr)
{
}

ToolData::ToolData(BoundType* componentMeta) : mScriptComponentType(componentMeta)
{
}

ToolData::~ToolData()
{
  mSpace.SafeDestroy();
}

String ToolData::ToString(bool shortFormat) const
{
  return GetName();
}

String ToolData::GetName() const
{
  if (Archetype* archetype = mArchetype)
    return archetype->Name;

  return mScriptComponentType->Name;
}

Space* ToolData::GetSpace()
{
  if (mSpace.IsNull())
  {
    GameSession* gameSession = Z::gEditor->GetEditGameSession();

    if (gameSession)
    {
      Archetype* spaceArchetype = ArchetypeManager::Find(CoreArchetypes::DefaultSpace);
      mSpace = gameSession->CreateSpace(spaceArchetype);
    }
    else
    {
      mSpace = Z::gFactory->CreateSpace(CoreArchetypes::DefaultSpace, CreationFlags::Default, nullptr);
    }
  }

  return mSpace;
}

ToolObjectManager::ToolObjectManager(ToolControl* toolControl) : EditorScriptObjects<ToolData>(ObjectAttributes::cTool)
{
  mToolControl = toolControl;
}

ToolObjectManager::~ToolObjectManager()
{
  DeleteObjectsInContainer(mToolArray);
}

void ToolObjectManager::AddObject(ToolData* object)
{
  mToolArray.PushBack(object);
}

void ToolObjectManager::RemoveObject(ToolData* object)
{
  // If this tool is selected, select the default tool
  if (mToolControl->mActiveTool == object)
    mToolControl->SelectToolIndex(0);

  mToolArray.EraseValueError(object);

  delete object;
}

ToolData* ToolObjectManager::GetObject(StringParam objectName)
{
  forRange (ToolData* tool, mToolArray.All())
  {
    if (tool->GetName() == objectName)
      return tool;
  }

  return nullptr;
}

uint ToolObjectManager::GetObjectCount()
{
  return mToolArray.Size();
}

ToolData* ToolObjectManager::GetObject(uint index)
{
  return mToolArray[index];
}

ToolData* ToolObjectManager::UpdateData(StringParam objectName)
{
  return GetObject(objectName);
}

Space* ToolObjectManager::GetSpace(ToolData* object)
{
  return object->GetSpace();
}

void ToolObjectManager::CreateOrUpdateCog(ToolData* object)
{
  EditorScriptObjects<ToolData>::CreateOrUpdateCog(object);

  // Re-select the active tool
  mToolControl->SelectToolInternal(mToolControl->mActiveTool, ShowToolProperties::Auto);
}

ZilchDefineType(ToolControl, builder, type)
{
  ZeroBindEvent(Events::GetToolInfo, ToolUiEvent);
}

ToolControl::ToolControl(Composite* parent) : Composite(parent), mTools(this), mCustomUi(nullptr)
{
  mEditor = Z::gEditor;
  mActiveTool = nullptr;
  SetHideOnClose(true);
  SetLayout(CreateStackLayout());
  SetName("Tools");

  // METAREFACTOR (also, find anywhere else people could be doing this!)
  MetaDatabase::GetInstance()->mEventMap[Events::ToolActivate] = ZilchTypeId(Event);
  MetaDatabase::GetInstance()->mEventMap[Events::ToolDeactivate] = ZilchTypeId(Event);
  MetaDatabase::GetInstance()->mEventMap[Events::ToolDraw] = ZilchTypeId(Event);

  Composite* toolRow = new Composite(this);
  toolRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2(6, 0), Thickness(6, 0, 0, 0)));
  {
    Composite* iconLayout = new Composite(toolRow);
    iconLayout->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero));
    {
      Spacer* spacer = new Spacer(iconLayout);
      spacer->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
      spacer->SetSizing(SizeAxis::Y, SizePolicy::Fixed, 3);

      mInfoIcon = iconLayout->CreateAttached<Element>("Info");
      mInfoIcon->SetSizing(SizePolicy::Fixed, mInfoIcon->mSize);
      mInfoIcon->SetNotInLayout(false);
    }

    iconLayout->SizeToContents();

    mToolBox = new ComboBox(toolRow);
    mToolBox->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
    mToolSource = new ContainerSource<Array<ToolData*>>(&mTools.mToolArray);
    mToolBox->SetListSource(mToolSource);
  }

  mScrollArea = new ScrollArea(this);
  mScrollArea->GetClientWidget()->SetLayout(CreateStackLayout());
  mScrollArea->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);
  mScrollArea->DisableScrollBar(SizeAxis::X);

  mPropertyGrid = new PropertyView(mScrollArea);
  mPropertyGrid->ActivateAutoUpdate();
  mPropertyGrid->mFixedHeight = true;
  mPropertyInterface = new ToolPropertyInterface();
  mPropertyGrid->SetPropertyInterface(mPropertyInterface);

  ConnectThisTo(mInfoIcon, Events::MouseEnter, OnInfoMouseEnter);
  ConnectThisTo(mInfoIcon, Events::MouseExit, OnInfoMouseExit);

  ConnectThisTo(mToolBox, Events::ItemSelected, OnToolPulldownSelect);
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);

  ZilchScriptManager* zilchManager = ZilchScriptManager::GetInstance();
  ConnectThisTo(zilchManager, Events::ScriptsCompiledPostPatch, OnScriptsCompiled);
}

ToolControl::~ToolControl()
{
  SafeDelete(mPropertyInterface);
}

void ToolControl::UpdateTransform()
{
  float toolBoxHeight = mToolBox->GetMinSize().y;

  real remainingHeight = mSize.y - toolBoxHeight;

  Vec2 size = mScrollArea->GetClientWidget()->GetMinSize();
  size.x = mSize.x;

  // Either make space for scrollbar or fill remaining area
  if (size.y > remainingHeight)
    size.x -= mScrollArea->GetScrollBarSize();
  else
    size.y = remainingHeight;

  mScrollArea->SetClientSize(size);
  mScrollArea->GetClientWidget()->SetSize(size);

  Composite::UpdateTransform();
}

Cog* ToolControl::AddOrUpdateTool(Archetype* toolArchetype)
{
  ToolData* tool = mTools.AddOrUpdate(toolArchetype);
  if (tool)
    return tool->mCog;

  return nullptr;
}

void ToolControl::RemoveTool(Archetype* toolArchetype)
{
  ToolData* tool = mTools.GetObject(toolArchetype->Name);
  // If this tool is selected, select the default tool
  if (mActiveTool == tool)
    SelectToolIndex(0);

  mTools.RemoveObject(tool);
}

Cog* ToolControl::GetActiveCog()
{
  if (mActiveTool)
    return mActiveTool->mCog;
  return nullptr;
}

Cog* ToolControl::GetToolByName(StringParam typeName)
{
  ToolData* tool = mTools.GetObject(typeName);
  if (tool)
    return tool->mCog;

  return nullptr;
}

PropertyView* ToolControl::GetPropertyGrid() const
{
  return mPropertyGrid;
}

void ToolControl::SelectToolIndex(uint index, ShowToolProperties::Enum showTool)
{
  if (index < mTools.mToolArray.Size())
    SelectToolInternal(mTools.mToolArray[index], showTool);
}

void ToolControl::SelectToolName(StringParam toolName, ShowToolProperties::Enum showTool)
{
  ToolData* tool = mTools.GetObject(toolName);
  if (tool)
    SelectToolInternal(tool, showTool);
}

bool ToolControl::IsSelectToolActive()
{
  return GetActiveCog() == mSelectTool->GetOwner();
}

void ToolControl::OnInfoMouseEnter(MouseEvent*)
{
  if (mActiveTool == nullptr)
    return;

  ShortcutSet entries;
  // Get the shortcuts documentation for all components of the tool.
  forRange (Component* component, mActiveTool->mCog->GetComponents())
  {
    BoundType* type = ZilchVirtualTypeId(component);
    const ShortcutSet* shortcuts = Z::gShortcutsDoc->FindSet(type->Name);

    if (shortcuts)
    {
      entries.Reserve(entries.Size() + shortcuts->Size());
      entries.Append(shortcuts->All());
    }
  }

  mShortcutsTip.SafeDestroy();

  // Query for description came up empty, so report that in the info tooltip.
  if (entries.Empty())
  {
    ToolTip* toolTip = mShortcutsTip = new ToolTip(mToolBox);
    toolTip->SetText("Current tool does not have any mouse/keyboard shortcuts.");
    toolTip->SetDestroyOnMouseExit(false);

    ToolTipPlacement placement;
    placement.SetScreenRect(mToolBox->GetScreenRect());
    placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left, IndicatorSide::Bottom, IndicatorSide::Top);

    // Move arrow outside of the tool's property grid.
    if (toolTip->mSide == IndicatorSide::Right)
    {
      placement.mScreenRect.X += 2.0f;
      toolTip->SetArrowTipTranslation(placement);
    }
    else if (toolTip->mSide == IndicatorSide::Left)
    {
      placement.mScreenRect.X -= InfoSpan;
      toolTip->SetArrowTipTranslation(placement);
    }
    else // No adjustment.
    {
      toolTip->SetArrowTipTranslation(placement);
    }

    return;
  }

  BuildShortcutsToolTip(&entries);
}

void ToolControl::OnInfoMouseExit(MouseEvent*)
{
  mShortcutsTip.SafeDestroy();
}

void ToolControl::OnToolPulldownSelect(ObjectEvent*)
{
  SelectToolIndex(mToolBox->GetSelectedItem(), ShowToolProperties::Show);
}

void ToolControl::OnKeyDown(KeyboardEvent* e)
{
  // EditorViewport* lastViewport = Z::gEditor->mActiveViewport;

  // if(lastViewport)
  //  ExecuteShortCuts(lastViewport->GetTargetSpace(), lastViewport, e);
}

void ToolControl::OnScriptsCompiled(Event*)
{
  SelectToolInternal(mActiveTool, ShowToolProperties::Auto);
}

void ToolControl::SelectToolInternal(ToolData* tool, ShowToolProperties::Enum showTool)
{
  if (tool == nullptr)
    return;

  CommandManager* commands = CommandManager::GetInstance();

  // Deactivate the old tool before switching to the new one
  if (mActiveTool)
  {
    if (Cog* activeToolCog = mActiveTool->mCog)
    {
      Event deactivateEvent;
      activeToolCog->DispatchEvent(Events::ToolDeactivate, &deactivateEvent);

      // De-activate the command so it's no longer selected in the tools Ui
      Command* command = commands->GetCommand(mActiveTool->GetName());

      // Script tools may not have commands associated with them
      if (command)
        command->SetActive(false);
    }
  }

  // The new tool is now our active tool
  mActiveTool = tool;

  // Destroy the old ui
  SafeDestroy(mCustomUi);

  Cog* newToolCog = (Cog*)tool->mCog;

  // Attach any custom Ui
  ToolUiEvent uiEvent(mScrollArea);
  uiEvent.mSelectTool = mSelectTool->GetOwner();
  newToolCog->DispatchEvent(Events::GetToolInfo, &uiEvent);
  mCustomUi = uiEvent.mCustomUi;

  // Show tools window if necessary
  if (uiEvent.mNeedsPropertyGrid || showTool == ShowToolProperties::Show)
    Z::gEditor->ShowWindow("Tools");

  // Set the tool to the property view
  mPropertyGrid->SetObject(newToolCog, mPropertyInterface);
  mPropertyGrid->Rebuild();

  // Activate after Ui is created
  Event activateEvent;
  newToolCog->DispatchEvent(Events::ToolActivate, &activateEvent);

  // Activate the command so it shows as selected in the tools Ui
  Command* command = commands->GetCommand(tool->GetName());

  // Script tools may not have commands associated with them
  if (command)
    command->SetActive(true);

  // Select the correct toolbox index
  uint toolIndex = mTools.mToolArray.FindIndex(tool);
  mToolBox->SetSelectedItem(toolIndex, false);

  UpdateShortcutsTip();
}

void ToolControl::BuildShortcutsToolTip(const ShortcutSet* entries)
{
  ToolTip* toolTip = mShortcutsTip = new ToolTip(mToolBox);
  mShortcutsView = new ListView(toolTip);

  toolTip->SetDestroyOnMouseExit(false);
  toolTip->mContentPadding = Thickness(2);

  TreeFormatting formatting;
  BuildShorcutsFormat(&formatting);
  mShortcutsView->SetFormat(&formatting);

  mShortcutSource.mSet = *entries;
  mShortcutsView->SetDataSource(&mShortcutSource);

  // Make the "Name" & "Shortcut" column width fit to the max-row's text
  // size for their respective column.
  mShortcutsView->mFitToText[0] = true;
  mShortcutsView->mFitToText[1] = true;

  ToolTipPlacement placement;
  placement.SetScreenRect(mToolBox->GetScreenRect());
  placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left, IndicatorSide::Bottom, IndicatorSide::Top);

  toolTip->mBackgroundColor = FloatColorRGBA(30, 30, 30, 255);
  toolTip->mBorderColor = FloatColorRGBA(10, 10, 10, 255);

  toolTip->SetContent(mShortcutsView);

  // Move arrow outside of the tool's property grid.
  if (toolTip->mSide == IndicatorSide::Right)
  {
    placement.mScreenRect.X += 2.0f;
    toolTip->SetArrowTipTranslation(placement);
  }
  else if (toolTip->mSide == IndicatorSide::Left)
  {
    placement.mScreenRect.X -= InfoSpan;
    toolTip->SetArrowTipTranslation(placement);
  }
  else // No adjustment.
  {
    toolTip->SetArrowTipTranslation(placement);
  }
}

void ToolControl::BuildShorcutsFormat(TreeFormatting* formatting)
{
  formatting->Flags.SetFlag(FormatFlags::ShowHeaders);
  formatting->Flags.SetFlag(FormatFlags::ShowSeparators);

  ColumnFormat* format = &formatting->Columns.PushBack();
  format->Index = formatting->Columns.Size() - 1;
  format->Name = "Name";
  format->HeaderName = "Name";
  format->ColumnType = ColumnType::Fixed;
  format->MinWidth = Pixels(120.0f);
  format->FixedSize.x = Pixels(120.0f);
  format->Editable = false;

  format = &formatting->Columns.PushBack();
  format->Index = formatting->Columns.Size() - 1;
  format->Name = "Shortcut";
  format->HeaderName = "Shortcut";
  format->ColumnType = ColumnType::Fixed;
  format->MinWidth = Pixels(150.0f);
  format->FixedSize.x = Pixels(150.0f);
  format->Editable = false;

  format = &formatting->Columns.PushBack();
  format->Index = formatting->Columns.Size() - 1;
  format->Name = "Description";
  format->HeaderName = "Description";
  format->ColumnType = ColumnType::Flex;
  format->FlexSize = 1;
  format->MinWidth = Pixels(280.0f);
  format->FixedSize.x = Pixels(280.0f);
  format->Editable = false;
}

void ToolControl::UpdateShortcutsTip()
{
  // Only update the tool tip if one was already present
  //  - [ie, mouse is over the info icon while a keyboard button 0- 9 was hit]
  if (mShortcutsTip.IsNotNull())
  {
    MouseEvent e;
    OnInfoMouseEnter(&e);
  }
}

} // namespace Zero
