///////////////////////////////////////////////////////////////////////////////
///
/// \file ContextMenu.cpp
/// Implementation of the PopUp.
///
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace MenuUi
{
  const cstr cLocation = "EditorUi/Controls/Menu";
  Tweakable(Vec4, BackgroundColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BorderColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec2, BorderPadding, Vec2(1,1), cLocation);

  Tweakable(Vec4, ItemTextColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, ItemBackgroundColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, ItemBorderColor, Vec4(1,1,1,1), cLocation);

  Tweakable(Vec4, ItemSelectedTextColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, ItemSelectedBorderColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, ItemSelectedBackgroundColor, Vec4(1,1,1,1), cLocation);

  Tweakable(Vec4, ItemDisabledTextColor, Vec4(1,1,1,1), cLocation);

  Tweakable(Vec4, MenuBarItemPadding, Vec4(4,4,4,4), cLocation);
  Tweakable(Vec4, GutterColor, Vec4(1,1,1,1), cLocation);
}

namespace Events
{
  DefineEvent(MenuItemSelected);
}

Composite* CreateLineDivider(Composite* parent, Vec4 color)
{
  return new ContextMenuDivider(parent, color);
}

ZilchDefineType(MenuBarItem, builder, type)
{
}

ZilchDefineType(MenuBar, builder, type)
{
}

//------------------------------------------------------------ ContextMenuItem

ContextMenuItem::ContextMenuItem(ContextMenu* parent, StringParam name)
  :Composite(parent)
{
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBorder = CreateAttached<Element>(cWhiteSquare);

  Thickness thickness(MenuUi::BorderPadding);
  thickness.Right = Pixels(2);
  SetLayout( CreateStackLayout(LayoutDirection::LeftToRight, Vec2(0,0), thickness));

  mCheck = CreateAttached<Element>("CheckIcon");

  mText = new Text(this, cText);
  mText->SetSizing(SizeAxis::X, SizePolicy::Flex, 20);
  mText->SetText(name);

  Spacer* spacer = new Spacer(this);
  spacer->SetSize(Vec2(20, 10));

  mShortcut = new Text(this, cText);
  mIcon = NULL;

  parent->mItems.PushBack(this);

  mCommand = NULL;
  mEnabled = true;
  mActive = false;

  ConnectThisTo(this, Events::LeftMouseUp, OnLeftMouseUp);
  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);

  parent->SizeToContents();
}

void ContextMenuItem::OnMouseExit(MouseEvent* event)
{
  MarkAsNeedsUpdate();
}

void ContextMenuItem::OnMouseEnter(MouseEvent* event)
{
  MarkAsNeedsUpdate();
}

Vec2 ContextMenuItem::GetMinSize()
{
  return Composite::GetMinSize();
}

void ContextMenuItem::UpdateTransform()
{
  mBackground->SetSize(mSize);
  mBorder->SetSize(mSize);
  mText->SetSize(mSize);
  mShortcut->SetSize(mSize);

  mCheck->SetVisible(mActive);
  Vec3 checkPos(Pixels(1), mSize.y * 0.5f - mCheck->GetSize().y * 0.5f, 0);
  mCheck->SetTranslation(checkPos);

  if(!mEnabled)
  {
    mText->SetColor(MenuUi::ItemDisabledTextColor);
    mShortcut->SetColor(MenuUi::ItemDisabledTextColor);
    Composite::UpdateTransform();
    return;
  }

  if(IsMouseOver())
  {
    mBackground->SetColor(MenuUi::ItemSelectedBackgroundColor);
    mBorder->SetColor(MenuUi::ItemSelectedBackgroundColor);
    mText->SetColor(MenuUi::ItemSelectedTextColor);
    mShortcut->SetColor(MenuUi::ItemSelectedTextColor);
  }
  else
  {
    mBackground->SetColor(MenuUi::ItemBackgroundColor);
    mBorder->SetColor(MenuUi::ItemBackgroundColor);
    mText->SetColor(MenuUi::ItemTextColor);
    mShortcut->SetColor(MenuUi::ItemTextColor);
  }

  Composite::UpdateTransform();
}

void ContextMenuItem::SetName(StringParam name, StringParam icon)
{
  mText->SetText(name);
  MarkAsNeedsUpdate();
}

void ContextMenuItem::SetCommand(Command* command)
{
  SetName(command->DisplayName);
  mShortcut->SetText(command->Shortcut);
  mCommand = command;
  mEnabled = command->IsEnabled();
  mActive = command->IsActive();
}

void ContextMenuItem::OnLeftMouseUp(MouseEvent* event)
{
  ObjectEvent eventToSend(this);
  this->DispatchEvent(Events::MenuItemSelected, &eventToSend);
  this->GetParent()->DispatchEvent(Events::MenuItemSelected, &eventToSend);
  this->GetParent()->Destroy();

  if(mCommand)
    mCommand->Execute();
}

//------------------------------------------------------------ ContextMenu

ContextMenu::ContextMenu(Widget* target)
  :PopUp(target, PopUpCloseMode::MouseDistance)
{
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mGutter = CreateAttached<Element>(cWhiteSquare);

  Thickness thickness(Pixels(2,2));
  SetLayout( CreateStackLayout(LayoutDirection::TopToBottom, Vec2(0,0), thickness));
  SizeToContents();
}

ContextMenu::~ContextMenu()
{
  // when the context menu loses focus and deletes itself we need
  // to clear the currently open menu references so returning the menu bar
  // requires you to click an item to open it again
  if(Widget* target = mTarget.Get<Widget*>())
  {
    FocusEvent event(nullptr, target);
    target->DispatchEvent(Events::FocusLost, &event);
  }
}

Vec2 ContextMenu::GetMinSize()
{
  return Composite::GetMinSize() + Vec2(MenuUi::BorderPadding);
}

void ContextMenu::SizeToContents()
{
  SetSize(GetMinSize());
  // Check
  Composite::UpdateTransform();
}

uint ContextMenu::ItemCount()
{
  return mItems.Size();
}

void ContextMenu::UpdateTransform()
{
  mBackground->SetColor(MenuUi::BackgroundColor);
  mBorder->SetColor(MenuUi::BorderColor);
  mBackground->SetSize(mSize);
  mBorder->SetSize(mSize);

  mGutter->SetColor(MenuUi::GutterColor);
  mGutter->SetSize(Vec2(Pixels(1), mSize.y - Pixels(3)));
  mGutter->SetTranslation(Vec3(Vec2(MenuUi::BorderPadding).x - Pixels(4), Vec2(MenuUi::BorderPadding).y, 0));
  PopUp::UpdateTransform();
}

void ContextMenu::LoadMenu(StringParam menuName)
{
  DeveloperConfig* devConfig = Z::gEngine->GetConfigCog()->has(DeveloperConfig);
  CommandManager* commandManager = CommandManager::GetInstance();
  MenuDefinition* menuDef = commandManager->mMenus.FindValue(menuName, NULL);
  ReturnIf(menuDef == NULL,, "Could not find menu definition '%s'", menuName.c_str());

  forRange(String& name, menuDef->Entries.All())
  {
    // Divider
    if(name == Divider)
    {
      AddDivider();
      continue;
    }

    // Command 
    Command* command = commandManager->GetCommand(name);
    if(command)
    {
      // If it's a dev only command and there's no dev config, don't add it
      if(command->DevOnly && devConfig == NULL)
        continue;

      AddCommand(command);
      continue;
    }

    // Sub Menu
    MenuDefinition* menuDef = commandManager->mMenus.FindValue(name, NULL);
    if(menuDef != NULL)
    {
      ErrorIf(true, "Todo sub menus");
      continue;
    }

    ErrorIf(true, "Invalid menu entry '%s'", name.c_str());
  }
}

void ContextMenu::CloseContextMenu()
{
  this->Destroy();
}

ContextMenuItem* ContextMenu::AddCommand(Command* command)
{
  ContextMenuItem* item = new ContextMenuItem(this);
  item->SetCommand(command);
  return item;
}

ContextMenuItem* ContextMenu::AddCommandByName(StringParam commandName)
{
  CommandManager* commandManager = CommandManager::GetInstance();
  Command* command = commandManager->GetCommand(commandName);
  if(command)
    return AddCommand(command);
  return NULL;
}

ContextMenuItem* ContextMenu::CreateContextItem(StringParam name)
{
  ContextMenuItem* item = new ContextMenuItem(this);
  item->SetName(name);
  return item;
}

void ContextMenu::AddDivider()
{
  new ContextMenuDivider(this, MenuUi::GutterColor);
}

//------------------------------------------------------------

MenuBarItem::MenuBarItem(Composite* parent)
  :Composite(parent), mContextMenu(nullptr)
{
  mMenuBar = (MenuBar*)parent;
  mBackground =  CreateAttached<Element>(cHighlight);
  mBackground->SetVisible(false);
  mText = new Text(this, cText);
  ConnectThisTo(this, Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::FocusLost, ClearOpenMenu);
}

void MenuBarItem::UpdateTransform()
{
  mBackground->SetSize(mSize);
  mText->SetSize(mSize);
  Rect rect = RemoveThicknessRect( Thickness(MenuUi::MenuBarItemPadding), mSize);
  PlaceWithRect(rect, mText);
  Composite::UpdateTransform();
}

Vec2 MenuBarItem::GetMinSize()
{
  return ExpandSizeByThickness( Thickness(MenuUi::MenuBarItemPadding), mText->GetMinSize());
}

void MenuBarItem::OnLeftMouseDown(MouseEvent* mouseEvent)
{
  if (GetMenuBar()->GetOpenMenuBarItem())
    CloseContextMenu();
  else
    OpenContextMenu();
}

void MenuBarItem::OnMouseEnter(MouseEvent* mouseEvent)
{
  MenuBarItem* openMenuBarItem = GetMenuBar()->GetOpenMenuBarItem();
  if (openMenuBarItem && openMenuBarItem != this)
  {
    openMenuBarItem->CloseContextMenu();
    OpenContextMenu();
  }
}

void MenuBarItem::OpenContextMenu()
{
  ContextMenu* contextMenu = new ContextMenu(this);
  contextMenu->SetTranslation(this->GetScreenPosition() + Pixels(0, mSize.y, 0));
  contextMenu->LoadMenu(mName);
  contextMenu->SizeToContents();
  mContextMenu = contextMenu;
  GetMenuBar()->mOpenMenuBarItem = this;
}

MenuBar* MenuBarItem::GetMenuBar()
{
  return mMenuBar;
}

void MenuBarItem::CloseContextMenu()
{
  // check if we have an open context menu attached our menu bar item
  mContextMenu->CloseContextMenu();
}

void MenuBarItem::ClearOpenMenu(FocusEvent* event)
{
  // if this is the open menu bar we need to clear the menu bars reference to us
  // this check handles both closing an open menu to update to a new open menu
  // and when focus is lost on the context menu and it deletes itself
  MenuBar* menuBar = GetMenuBar();
  if (!menuBar)
    return;

  MenuBarItem* openMenuBar = menuBar->mOpenMenuBarItem;
  if((MenuBarItem*)GetMenuBar()->mOpenMenuBarItem == this)
  {
    // null out handles for closed items
    mContextMenu = nullptr;
    GetMenuBar()->mOpenMenuBarItem = nullptr;
  }
}

//------------------------------------------------------------  MenuBar

MenuBar::MenuBar(Composite* parent)
  : Composite(parent), mOpenMenuBarItem(nullptr)
{
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2(9.0f, 0), Thickness(0,0)));
}

void MenuBar::LoadMenu(StringParam menuName)
{
  CommandManager* commandManager = CommandManager::GetInstance();
  MenuDefinition* menuDef = commandManager->mMenus.FindValue(menuName, NULL);
  ReturnIf(menuDef == NULL,, "Could not find menu definition '%s'", menuName.c_str());

  forRange(String& menuName, menuDef->Entries.All())
  {
    MenuDefinition* menuDef = commandManager->mMenus.FindValue(menuName, NULL);
    ErrorIf(menuDef == NULL, "Could not find menu definition '%s'", menuName.c_str());
    if(menuDef)
    {
      MenuBarItem* entry = new MenuBarItem(this);
      entry->mText->SetText(menuName);
      entry->mName = menuName;
    }
  }
}

MenuBarItem* MenuBar::GetOpenMenuBarItem()
{
  return mOpenMenuBarItem;
}

}
