///////////////////////////////////////////////////////////////////////////////
///
/// \file ToolBar.cpp
/// Implementation of the Toolbar classes.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace ToolbarUi
{
const cstr cLocation = "EditorUi/Controls/Toolbar";
Tweakable(Vec4, BackgroundColor, Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, DividerColor, Vec4(1,1,1,1), cLocation);
}

//------------------------------------------------------------- Command Sub Item
CommandSubItem::CommandSubItem(ToolBarGroupPopUp* parent, Command* command)
  : Composite(parent)
{
  mPopUp = parent;
  mCommand = command;
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(ToFloatColor(ByteColorRGBA(66, 66, 66, 255)));
  mText = new Text(this, cText);
  mText->SetText(command->Name);

  mIcon = CreateAttached<Element>(command->IconName);
  mIcon->SetInteractive(false);
  mText->SetInteractive(false);

  ConnectThisTo(mBackground, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(mBackground, Events::MouseExit, OnMouseExit);
  ConnectThisTo(mBackground, Events::LeftClick, OnLeftClick);
}

const float cMargins = Pixels(2);
const float cSpacing = Pixels(7);

float CommandSubItem::MeasureWidth()
{
  // 5 extra pixels on the right just because it looks nice
  float margins = cMargins * 2.0f + Pixels(8);
  float iconSize = mIcon->GetSize().x;
  float spacing = cSpacing;
  float textSize = mText->GetMinSize().x;
  float width = margins + iconSize + spacing + textSize;
  return Math::Max(width, Pixels(150));
}

void CommandSubItem::UpdateTransform()
{
  mIcon->SetTranslation(Vec3(cMargins, cMargins,0));
  mIcon->SetSize(Pixels(28, 28));

  Vec3 textTranslation(cMargins + mIcon->mSize.x + cSpacing, cMargins + Pixels(7), 0);
  mText->SetTranslation(textTranslation);
  mText->SetSize(mText->GetMinSize());

  mBackground->SetSize(mSize);
  Composite::UpdateTransform();
}

Vec2 CommandSubItem::GetMinSize()
{
  return Pixels(28, 28) + Vec2(cMargins, cMargins);
}

void CommandSubItem::OnMouseEnter(MouseEvent* e)
{
  mBackground->SetColor(ToFloatColor(ByteColorRGBA(94,94,94, 255)));
}

void CommandSubItem::OnMouseExit(MouseEvent* e)
{
  mBackground->SetColor(ToFloatColor(ByteColorRGBA(66, 66, 66, 255)));
}

void CommandSubItem::OnLeftClick(MouseEvent* e)
{
  CommandCaptureContextEvent commandCaptureEvent;
  commandCaptureEvent.ActiveSet = CommandManager::GetInstance();
  this->DispatchBubble(Events::CommandCaptureContext, &commandCaptureEvent);
  mCommand->Execute();
  mPopUp->Destroy();
}

//--------------------------------------------------------- Tool Bar Group Popup
ToolBarGroupPopUp::ToolBarGroupPopUp(Composite* parent, ToolBarGroup* group)
  : PopUp(parent, PopUpCloseMode::MouseDistance)
{
  Array<Command*>& commands = group->mCommands;
  mCommandWidgets.Reserve(commands.Size());
  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(2,2), Thickness::All(1)));

  float maxWidth = 0.0f;
  for(uint i = 0; i < commands.Size(); ++i)
  {
    CommandSubItem* item = new CommandSubItem(this, commands[i]);

    float currWidth = item->MeasureWidth();
    maxWidth = Math::Max(currWidth, maxWidth);

    mCommandWidgets.PushBack(item);
  }

  // Set the size of all of them to the max size
  for(uint i = 0; i < mCommandWidgets.Size(); ++i)
    mCommandWidgets[i]->SetSize(Vec2(maxWidth, Pixels(32)));

  SetClipping(true);

  LayoutArea data;
  data.Size = Vec2(maxWidth, 0);
  data.Offset = Vec3::cZero;
  data.HorizLimit = LimitMode::Limited;
  data.VerticalLimit = LimitMode::Unlimited;
  Vec2 size = GetLayout()->DoLayout(this, data);

  SetSize(Vec2(size.x, 0));
  ActionSequence* seq = new ActionSequence(this);
  seq->Add(SizeWidgetAction(this, size, 0.1f));
}

//--------------------------------------------------------------- Tool Bar Group
ToolBarGroup::ToolBarGroup(Composite* parent, StringParam name)
  : Composite(parent)
{
  mButton = new IconButton(this);
  mButton->SetIcon(name);
  mButton->SetSize(Pixels(32,32));

  mExpandIcon = CreateAttached<Element>("ExpandIcon");
  mExpandIcon->SetInteractive(false);
  ConnectThisTo(mButton, Events::ButtonPressed, OnButtonPressed);
}

void ToolBarGroup::UpdateTransform()
{
  Vec3 pos = Vec3(Pixels(3), mSize.y - Pixels(3) - mExpandIcon->GetSize().y, 0);
  mExpandIcon->SetTranslation(pos);
  Composite::UpdateTransform();
}

Vec2 ToolBarGroup::GetMinSize()
{
  return mButton->GetMinSize();
}

void ToolBarGroup::AddCommand(Command* command)
{
  mCommands.PushBack(command);
}

void ToolBarGroup::OnButtonPressed(Event* e)
{
  if(ToolBarGroupPopUp* oldPopup = mPopUp)
  {
    oldPopup->Destroy();
    return;
  }

  ToolBarGroupPopUp* popUp = new ToolBarGroupPopUp(this, this);
  Vec3 screenPos = this->GetScreenPosition();
  Vec3 pos = Vec3(screenPos.x - Pixels(1), screenPos.y + GetSize().y, 0);
  popUp->SetTranslation(pos);
  mPopUp = popUp;
}

void ToolBarGroup::LoadMenu(StringParam menuName)
{
  CommandManager* commandManager = CommandManager::GetInstance();
  MenuDefinition* menuDef = commandManager->mMenus.FindValue(menuName, NULL);
  ReturnIf(menuDef == NULL,, "Could not find menu definition '%s'", menuName.c_str());

  // Add all entries
  forRange(String& name, menuDef->Entries.All())
  {
    Command* command = commandManager->GetCommand(name);
    ErrorIf(command == NULL, "Can not find command '%s'", name.c_str());
    if(command) this->AddCommand(command);
  }
}

//---------------------------------------------------------------- Tool Bar Area
//******************************************************************************
ToolBarArea::ToolBarArea(Composite* parent) : Composite(parent)
{

}

//******************************************************************************
void ToolBarArea::UpdateTransform()
{
  Thickness currentArea(0.0f, 0.0f, mSize.x, 0.0f);

  float sizeY = mSize.y - Pixels(2);

  Widget* center = NULL;
  forRange(Widget& child, GetChildren())
  {
    child.UpdateTransformExternal();
    Vec2 childSize = child.mSize;

    u32 dockMode = child.GetDockMode();
    u32 dockCenter = DockMode::DockLeft | DockMode::DockRight;
    if(dockMode == dockCenter)
    {
      center = &child;
    }
    else if(dockMode == DockMode::DockLeft)
    {
      child.SetTranslation(Vec3(currentArea.Left, 0, 0));
      child.SetSize(Vec2(childSize.x, sizeY));
      currentArea.Left += childSize.x;
    }
    else if(dockMode == DockMode::DockRight)
    {
      currentArea.Right -= childSize.x;
      child.SetTranslation(Vec3(currentArea.Right, 0, 0));
      child.SetSize(Vec2(childSize.x, sizeY));
    }
  }

  if(center)
  {
    Vec2 centerSize = center->GetSize();
    Vec3 pos = ToVector3(((mSize * 0.5f) - (centerSize * 0.5f)));
    pos.x = Math::Max(currentArea.Left, pos.x);
    center->SetTranslation(pos);
    center->SetSize(Vec2(centerSize.x, sizeY));
  }

  Composite::UpdateTransform();
}

//---------------------------------------------------------------- Tool Bar Drag
class ToolBarDrag : public MouseManipulation
{
public:
  Vec3 mStartTrans;
  ToolBar* mDraggin;

  //****************************************************************************
  ToolBarDrag(Mouse* mouse, ToolBar* toBeDragged)
    : MouseManipulation(mouse, toBeDragged->GetParent())
  {
    mStartTrans = toBeDragged->GetTranslation();
    mDraggin = toBeDragged;
  }

  //****************************************************************************
  void OnMouseMove(MouseEvent* event) override
  {
    Vec3 newT = mStartTrans +  Vec3(event->Position - mMouseStartPosition);
    mDraggin->SetTranslation(newT);
  }
};

//--------------------------------------------------------------------- Tool Bar
//******************************************************************************
ToolBar::ToolBar(Composite* parent)
  :Composite(parent)
{
  static const String className = "TextButton";
  mDefSet = mDefSet->GetDefinitionSet(className);
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(0,0), Thickness::All(2)));

  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetVisible(false);
  mIconSize = Pixels(20, 20);
  mSize.y = Pixels(32);

  mToolBarMode = ToolBarMode::Dockable;
}

//******************************************************************************
void ToolBar::DoLayout()
{
  Thickness borderThickness = mBackground->GetBorderThickness();

  LayoutArea data;
  data.Size = mSize;
  data.HorizLimit = LimitMode::Limited;
  data.VerticalLimit = LimitMode::Limited;

  const float cTopSize = Pixels(1);
  const float cLeftSize = Pixels(2);
  const float cDragSize = Pixels(0);

  if(mToolBarMode == ToolBarMode::Dockable)
    data.Offset = Vec3(cDragSize,cTopSize,0);
  else
    data.Offset = Vec3(borderThickness.Left, cTopSize,0);
  
  Vec2 newSize = mLayout->DoLayout(this, data);
  if(mSize.x < newSize.x)
  {
    newSize.x += cLeftSize;
    mSize = newSize;
  }
}

//******************************************************************************
void ToolBar::OnMouseDown(MouseEvent* event)
{
  if(GetDocker())
    GetDocker()->StartManipulation(this, DockMode::DockNone);
  else
    new ToolBarDrag(event->GetMouse(), this);
}

//******************************************************************************
ToolBar::~ToolBar()
{

}

//******************************************************************************
void ToolBar::AddCommand(Command* command)
{
  if(command)
  {
    IconButton* button = new IconButton(this);
    button->SetCommand(command);
    button->SetName(BuildString(command->Name, "Command"));
    button->SetSizing(SizeAxis::Y, SizePolicy::Fixed, mSize.y);
  }
}

//******************************************************************************

Composite* CreateLineDivider(Composite* parent, Vec4 color);

void ToolBar::LoadMenu(StringParam menuName)
{
  CommandManager* commandManager = CommandManager::GetInstance();
  MenuDefinition* menuDef = commandManager->mMenus.FindValue(menuName, NULL);
  ReturnIf(menuDef == NULL,, "Could not find menu definition '%s'", menuName.c_str());

  // Add all entries
  forRange(String& name, menuDef->Entries.All())
  {
    // Entry is a valid command
    Command* command = commandManager->GetCommand(name);
    if(command)
    {
      // If the command is dev only and they don't have dev config, skip it
      if(command->DevOnly && !Z::gEngine->GetConfigCog()->has(DeveloperConfig))
        continue;

      this->AddCommand(command);
      continue;
    }

    // Entry also be a tool bar group that Contains other
    // commands
    MenuDefinition* menuDef = commandManager->mMenus.FindValue(name, NULL);
    if(menuDef != NULL)
    {
      ToolBarGroup* toolBarGroup = this->AddGroup(menuDef->Icon);
      toolBarGroup->LoadMenu(name);
      continue;
    }

    // Also can be a divider
    if(name == Divider)
    {
      CreateLineDivider(this, ToolbarUi::DividerColor);
      continue;
    }

    // Invalid entry
    ErrorIf(true, "Invalid menu entry '%s'", name.c_str());
  }
}

//******************************************************************************
ToolBarGroup* ToolBar::AddGroup(StringParam icon)
{
  ToolBarGroup* button = new ToolBarGroup(this, icon);
  return button;
}

//******************************************************************************
void ToolBar::SetIconSize(Vec2 size)
{
  mIconSize = size;
}

//******************************************************************************
void ToolBar::UpdateTransform()
{
  mBackground->SetSize(mSize);
  mBackground->SetColor(ToolbarUi::BackgroundColor);
  Composite::UpdateTransform();
}

}// namespace Zero
