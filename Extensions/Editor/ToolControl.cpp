///////////////////////////////////////////////////////////////////////////////
///
/// \file ToolControl.cpp
/// 
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(GetToolInfo);
}

//---------------------------------------------------------------- Tool Ui Event
ZilchDefineType(ToolUiEvent, builder, type)
{
  ZilchBindFieldProperty(mNeedsPropertyGrid);
  ZilchBindGetterProperty(Parent);
  ZilchBindGetterProperty(SelectTool);
}

//******************************************************************************
ToolUiEvent::ToolUiEvent(Composite* parent) :
  mParent(parent),
  mCustomUi(nullptr),
  mNeedsPropertyGrid(false),
  mSelectTool(nullptr)
{

}

//******************************************************************************
Composite* ToolUiEvent::GetParent()
{
  return mParent;
}

//******************************************************************************
void ToolUiEvent::SetCustomUi(Composite* customUi)
{
  mCustomUi = customUi;
}

//******************************************************************************
Composite* ToolUiEvent::GetCustomUi()
{
  return mCustomUi;
}

//******************************************************************************
Cog* ToolUiEvent::GetSelectTool()
{
  return mSelectTool;
}

//------------------------------------------------------ Tool Property Interface
// We're using a custom property interface to only show components marked with
// the 'Tool' tag. This allows the author of the tool to hide irrelevant
// components. We're also going to hide the ScriptSource Property.
class ToolPropertyInterface : public PropertyInterface
{
public:
  ToolPropertyInterface()
  {

  }

  //****************************************************************************
  ObjectPropertyNode* BuildObjectTree(ObjectPropertyNode* parent,
                                      HandleParam object,
                                      Property* objectProperty = nullptr) override
  {
    BoundType* objectType = object.StoredType;
    if(objectType->IsA(ZilchTypeId(Cog)))
    {
      // All components with the "Tool" tag
      static Array<uint> sValidComponentIndices;
      sValidComponentIndices.Clear();

      MetaComposition* cogComposition = objectType->HasInherited<MetaComposition>();
      // The amount of components we have contained
      uint objectCount = cogComposition->GetComponentCount(object);

      // Find all components with the "Tool" tag
      for(uint i = 0; i < objectCount; ++i)
      {
        Handle subInstance = cogComposition->GetComponentAt(object, i);

        // Any base class could have the tool tag, so search them all
        forRange(CogComponentMeta* zeroMeta, subInstance.StoredType->HasAll<CogComponentMeta>())
        {
          if (zeroMeta->mTags.Contains(Tags::Tool))
          {
            sValidComponentIndices.PushBack(i);
            break;
          }
        }
      }

      if(sValidComponentIndices.Size() == 1)
      {
        uint componentIndex = sValidComponentIndices.Front();
        Handle subInstance = cogComposition->GetComponentAt(object, componentIndex);
        return BuildObjectTree(parent, subInstance);
      }
      else
      {
        ObjectPropertyNode* node = new ObjectPropertyNode(parent, object, objectProperty);

        // Loop through and add each one
        for(uint i = 0; i < sValidComponentIndices.Size(); ++i)
        {
          uint componentIndex = sValidComponentIndices[i];
          Handle subInstance = cogComposition->GetComponentAt(object, componentIndex);
          BoundType* componentType = subInstance.StoredType;
          ErrorIf(componentType == nullptr,"Contained object does not have meta initialized.");

          // Don't show hidden sub objects
          if(componentType->HasAttribute(ObjectAttributes::cHidden))
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

  //****************************************************************************
  void RemoveScriptSourceProperties(ObjectPropertyNode* node)
  {
    forRange(ObjectPropertyNode* propertyNode, node->mProperties.All())
    {
      Property* metaProperty = propertyNode->mProperty;
      if(metaProperty->Name == "ScriptSource")
      {
        node->mProperties.EraseValueError(propertyNode);
        delete propertyNode;
        break;
      }
    }
  }

  /* METAREFACTOR - Not sure how to resolve these yet
  //****************************************************************************
  bool SupportsComponentRemoval() override { return false; }

  //****************************************************************************
  bool CanAddComponents(HandleParam instance) override { return false; }
  */
};

//-------------------------------------------------------------------- Tool Data
//******************************************************************************
ToolData::ToolData(Archetype* archetype) :
  mArchetype(archetype),
  mScriptComponentType(nullptr)
{

}

//******************************************************************************
ToolData::ToolData(BoundType* componentMeta) :
  mScriptComponentType(componentMeta)
{

}

//******************************************************************************
ToolData::~ToolData()
{
  mSpace.SafeDestroy();
}

//******************************************************************************
String ToolData::ToString(bool shortFormat) const
{
  return GetName();
}

//******************************************************************************
String ToolData::GetName() const
{
  if(Archetype* archetype = mArchetype)
    return archetype->Name;

  return mScriptComponentType->Name;
}

//******************************************************************************
Space* ToolData::GetSpace()
{
  if(mSpace.IsNull())
  {
    GameSession* gameSession = Z::gEditor->GetEditGameSession();

    if(gameSession)
    {
      Archetype* spaceArchetype = ArchetypeManager::Find(CoreArchetypes::DefaultSpace);
      mSpace = gameSession->CreateSpace(spaceArchetype);
    }
    else
    {
      mSpace = Z::gFactory->CreateSpace(CoreArchetypes::DefaultSpace,
                                        CreationFlags::Default, nullptr);
    }
  }

  return mSpace;
}

//---------------------------------------------------------- Tool Object Manager
//******************************************************************************
ToolObjectManager::ToolObjectManager(ToolControl* toolControl) :
  EditorScriptObjects<ToolData>("Tool")
{
  mToolControl = toolControl;
}

//******************************************************************************
ToolObjectManager::~ToolObjectManager()
{
  DeleteObjectsInContainer(mToolArray);
}

//******************************************************************************
void ToolObjectManager::AddObject(ToolData* object)
{
  mToolArray.PushBack(object);
}

//******************************************************************************
void ToolObjectManager::RemoveObject(ToolData* object)
{
  // If this tool is selected, select the default tool
  if(mToolControl->mActiveTool == object)
    mToolControl->SelectToolIndex(0);

  mToolArray.EraseValueError(object);

  delete object;
}

//******************************************************************************
ToolData* ToolObjectManager::GetObject(StringParam objectName)
{
  forRange(ToolData* tool, mToolArray.All())
  {
    if(tool->GetName() == objectName)
      return tool;
  }

  return nullptr;
}

//******************************************************************************
uint ToolObjectManager::GetObjectCount()
{
  return mToolArray.Size();
}

//******************************************************************************
ToolData* ToolObjectManager::GetObject(uint index)
{
  return mToolArray[index];
}

//******************************************************************************
Space* ToolObjectManager::GetSpace(ToolData* object)
{
  return object->GetSpace();
}

//******************************************************************************
void ToolObjectManager::CreateOrUpdateCog(ToolData* object)
{
  EditorScriptObjects<ToolData>::CreateOrUpdateCog(object);

  // Re-select the active tool
  mToolControl->SelectToolInternal(mToolControl->mActiveTool, ShowToolProperties::Auto);
}

//----------------------------------------------------------------- Tool Control
ZilchDefineType(ToolControl, builder, type)
{
  ZeroBindEvent(Events::GetToolInfo, ToolUiEvent);
}

//******************************************************************************
ToolControl::ToolControl(Composite* parent) :
  Composite(parent),
  mCustomUi(nullptr),
  mTools(this)
{
  mEditor = Z::gEditor;
  mActiveTool = nullptr;
  SetHideOnClose(true);
  SetLayout(CreateStackLayout());
  SetName("Tools");

  //METAREFACTOR (also, find anywhere else people could be doing this!)
  MetaDatabase::GetInstance()->mEventMap[Events::ToolActivate] = ZilchTypeId(Event);
  MetaDatabase::GetInstance()->mEventMap[Events::ToolDeactivate] = ZilchTypeId(Event);
  MetaDatabase::GetInstance()->mEventMap[Events::ToolDraw] = ZilchTypeId(Event);

  mToolBox = new ComboBox(this);
  mToolSource = new ContainerSource< Array<ToolData*> >(&mTools.mToolArray);
  mToolBox->SetListSource(mToolSource);

  mScrollArea = new ScrollArea(this);
  mScrollArea->GetClientWidget()->SetLayout(CreateStackLayout());
  mScrollArea->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);
  mScrollArea->DisableScrollBar(SizeAxis::X);

  mPropertyGrid = new PropertyView(mScrollArea);
  mPropertyGrid->ActivateAutoUpdate();
  mPropertyGrid->mFixedHeight = true;
  mPropertyInterface = new ToolPropertyInterface();
  mPropertyGrid->SetPropertyInterface(mPropertyInterface);

  ConnectThisTo(mToolBox, Events::ItemSelected, OnToolPulldownSelect);
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);

  ZilchScriptManager* zilchManager = ZilchScriptManager::GetInstance();
  ConnectThisTo(zilchManager, Events::ScriptsCompiledPostPatch, OnScriptsCompiled);
}

//******************************************************************************
ToolControl::~ToolControl()
{
  SafeDelete(mPropertyInterface);
}

//******************************************************************************
void ToolControl::UpdateTransform()
{
  Vec2 size = mScrollArea->GetClientWidget()->GetMinSize();
  size.x = mSize.x;

  real remainingHeight = mSize.y - mToolBox->GetMinSize().y;

  // Either make space for scrollbar or fill remaining area
  if (size.y > remainingHeight)
    size.x -= mScrollArea->GetScrollBarSize();
  else
    size.y = remainingHeight;

  mScrollArea->SetClientSize(size);
  mScrollArea->GetClientWidget()->SetSize(size);

  Composite::UpdateTransform();
}

//******************************************************************************
Cog* ToolControl::AddOrUpdateTool(Archetype* toolArchetype)
{
  ToolData* tool = mTools.AddOrUpdate(toolArchetype);
  if(tool)
    return tool->mCog;

  return nullptr;
}

//******************************************************************************
void ToolControl::RemoveTool(Archetype* toolArchetype)
{
  ToolData* tool = mTools.GetObject(toolArchetype->Name);
  // If this tool is selected, select the default tool
  if(mActiveTool == tool)
    SelectToolIndex(0);

  mTools.RemoveObject(tool);
}

//******************************************************************************
Cog* ToolControl::GetActiveCog()
{
  if(mActiveTool)
    return mActiveTool->mCog;
  return nullptr;
}

//******************************************************************************
Cog* ToolControl::GetToolByName(StringParam typeName)
{
  ToolData* tool = mTools.GetObject(typeName);
  if(tool)
    return tool->mCog;

  return nullptr;
}

//******************************************************************************
void ToolControl::SelectToolIndex(uint index, ShowToolProperties::Enum showTool)
{
  if(index < mTools.mToolArray.Size())
    SelectToolInternal(mTools.mToolArray[index], showTool);
}

//******************************************************************************
void ToolControl::SelectToolName(StringParam toolName, ShowToolProperties::Enum showTool)
{
  ToolData* tool = mTools.GetObject(toolName);
  if(tool)
    SelectToolInternal(tool, showTool);
}

//******************************************************************************
bool ToolControl::IsSelectToolActive()
{
  return GetActiveCog() == mSelectTool->GetOwner();
}

//******************************************************************************
void ToolControl::OnToolPulldownSelect(ObjectEvent*)
{
  SelectToolIndex(mToolBox->GetSelectedItem(), ShowToolProperties::Show);
}

//******************************************************************************
void ToolControl::OnKeyDown(KeyboardEvent* e)
{
  //EditorViewport* lastViewport = Z::gEditor->mActiveViewport;

  //if(lastViewport)
  //  ExecuteShortCuts(lastViewport->GetTargetSpace(), lastViewport, e);
}

//******************************************************************************
void ToolControl::OnScriptsCompiled(Event*)
{
  SelectToolInternal(mActiveTool, ShowToolProperties::Auto);
}

//******************************************************************************
void ToolControl::SelectToolInternal(ToolData* tool, ShowToolProperties::Enum showTool)
{
  if(tool == nullptr)
    return;

  CommandManager* commands = CommandManager::GetInstance();

  // Deactivate the old tool before switching to the new one
  if(mActiveTool)
  {
    if(Cog* activeToolCog = mActiveTool->mCog)
    {
      Event deactivateEvent;
      activeToolCog->DispatchEvent(Events::ToolDeactivate, &deactivateEvent);

      // De-activate the command so it's no longer selected in the tools Ui
      Command* command = commands->GetCommand(mActiveTool->GetName());

      // Script tools may not have commands associated with them
      if(command)
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
  if(uiEvent.mNeedsPropertyGrid || showTool == ShowToolProperties::Show)
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
  if(command)
    command->SetActive(true);

  // Select the correct toolbox index
  uint toolIndex = mTools.mToolArray.FindIndex(tool);
  mToolBox->SetSelectedItem(toolIndex, false);
}

}//namespace Zero
