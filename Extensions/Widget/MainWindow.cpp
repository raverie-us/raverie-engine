///////////////////////////////////////////////////////////////////////////////
///
/// \file MainWindow.cpp
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace MainWindowUi
{
  const cstr cLocation = "EditorUi/MainWindow";
  Tweakable(Vec4, TitleBarColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BorderColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec2, BorderPadding, Vec2(1,1), cLocation);

  Tweakable(Vec4, ButtonColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, ButtonHover, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, ButtonClick, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, CloseColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, CloseHover, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, CloseClick, Vec4(1,1,1,1), cLocation);
}

ZilchDefineType(MainWindow, builder, type)
{
}

MainWindow::MainWindow(OsWindow* window)
  :RootWidget(window)
{
  static const String className = "Window";
  mDefSet = mDefSet->GetDefinitionSet(className);

  mSize = Math::ToVec2(window->GetSize());

  mWindowWidget = new Composite(this, AttachType::Direct);
  mPopUp = new Composite(this, AttachType::Direct);

  mSizeGrips = new GripZones(this, this);
  mDocker = new OsDocker();

  mBorder = mWindowWidget->CreateAttached<Element>(cWhiteSquareBorder);
  mTitleGrip = new Gripper(mWindowWidget, this, DockMode::DockFill);
  ConnectThisTo(mTitleGrip, Events::DoubleClick, OnDoubleClickTitle);

  mClientWidget = new Composite(this, AttachType::Direct);
  mClientWidget->SetName("Client");
  mClientWidget->SetClipping(true);

  mTitleBar = new Composite(mWindowWidget);
  mTitleBack = mTitleBar->CreateAttached<Element>(cWhiteSquare);
  mTitleBack->SetInteractive(false);

  Composite* menuArea = new Composite(mTitleBar);
  mMenuArea = menuArea;
  menuArea->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness(0,0,0,0)));

  Composite* mainMenu = new Composite(menuArea);
  mainMenu->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness(2,2,2,2)));
  Label* zeroLabel = new Label(mainMenu);
  zeroLabel->SetText("Zero");
  zeroLabel->mText->ChangeDefinition( mDefSet->GetDefinition("TitleText") );
  Spacer* spacer = new Spacer(mainMenu);
  spacer->SetSize(Vec2(4,0));
  Element* arrow = mainMenu->CreateAttached<Element>("TitleDown");
  arrow->mNotInLayout = false; 
  Spacer* spacer2 = new Spacer(mainMenu);
  spacer2->SetSize(Vec2(4,0));
  mMainMenu = mainMenu;
  mMainMenu->SetActive(false);

  mMenu = new MenuBar(menuArea);

  mTitleBar->SetName("TitleBar");
  mTitleBar->SetLayout(CreateRowLayout());

  Spacer* leftSpacer = new Spacer(mTitleBar);
  leftSpacer->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
  leftSpacer->SetInteractive(false);

  mTitleText = new Label(mTitleBar, DefaultTextStyle, "Title");
  mTitleText->SetInteractive(false);
  mTitleText->SetNotInLayout(true);
  mTitleText->mText->ChangeDefinition( mDefSet->GetDefinition("TitleText") );

  mMin = new IconButton(mTitleBar);
  mMin->SetToolTip("Minimize Window");
  mMin->SetIcon("TitleMinimize");
  mMin->mPadding = Thickness::All(4);
  mMin->mBackgroundColor = ToByteColor(MainWindowUi::ButtonColor);
  mMin->mBackgroundHoverColor = ToByteColor(MainWindowUi::ButtonHover);
  mMin->mBackgroundClickedColor = ToByteColor(MainWindowUi::ButtonClick);
  mMin->mBorder->SetVisible(false);
  ConnectThisTo(mMin, Events::ButtonPressed, OnClickMin);

  mMax = new IconButton(mTitleBar);
  mMax->SetToolTip("Maximize Window");
  mMax->mPadding = Thickness::All(4);
  mMax->SetIcon("TitleMaximize");
  mMax->mBackgroundColor = ToByteColor(MainWindowUi::ButtonColor);
  mMax->mBackgroundHoverColor = ToByteColor(MainWindowUi::ButtonHover);
  mMax->mBackgroundClickedColor = ToByteColor(MainWindowUi::ButtonClick);
  mMax->mBorder->SetVisible(false);
  ConnectThisTo(mMax, Events::ButtonPressed, OnClickMax);

  mClose = new IconButton(mTitleBar);
  mClose->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(38));
  mClose->SetToolTip("Close Window");
  mClose->SetIcon("TitleClose");
  mClose->mPadding = Thickness::All(4);
  mClose->mBackgroundColor = ToByteColor(MainWindowUi::CloseColor);
  mClose->mBackgroundHoverColor = ToByteColor(MainWindowUi::CloseHover);
  mClose->mBackgroundClickedColor = ToByteColor(MainWindowUi::CloseClick);
  mClose->mBorder->SetVisible(false);
  mClose->MarkAsNeedsUpdate();
  ConnectThisTo(mClose, Events::ButtonPressed, OnClickClose);

  mLayoutSize = Vec2(0,0);
}

void MainWindow::LoadMenu(StringParam menuName)
{
  mMenu->LoadMenu(menuName);
  mMenu->MarkAsNeedsUpdate();
}

void MainWindow::SetTitle(StringParam title)
{
  GetOsWindow()->SetTitle(title);
  mTitleText->SetText(title);
  mTitleText->MarkAsNeedsUpdate();
  mLayoutSize = Vec2(0, 0);
}

void MainWindow::AttachChildWidget(Widget* child, AttachType::Enum attachType)
{
  if(attachType == AttachType::Direct)
    Composite::AttachChildWidget(child);
  else
    mClientWidget->AttachChildWidget(child);
  mLayoutSize = Vec2(0,0);
}

void MainWindow::OnDoubleClickTitle(MouseEvent* event)
{
  if (GetOsWindow()->GetState() != WindowState::Windowed)
    GetOsWindow()->SetState(WindowState::Windowed);
  else
    GetOsWindow()->SetState(WindowState::Maximized);
}

void MainWindow::OnClickClose(Event* event)
{
  OsWindowEvent windowEvent;
  GetOsWindow()->DispatchEvent(Events::OsClose, &windowEvent);
}

void MainWindow::OnClickMax(Event* event)
{
  if (GetOsWindow()->GetState() != WindowState::Windowed)
    GetOsWindow()->SetState(WindowState::Windowed);
  else
    GetOsWindow()->SetState(WindowState::Maximized);
}

void MainWindow::OnClickMin(Event* event)
{
  GetOsWindow()->SetState(WindowState::Minimized);
}

Composite* MainWindow::GetPopUp()
{
  return mPopUp;
}

void MainWindow::UpdateTransform()
{
  Vec2 size = ToVec2(GetOsWindow()->GetSize());
  mSizeGrips->MoveToFront();
  mPopUp->MoveToFront();

  // Do not resize all child widgets unless
  // a child has been added or the size of the OS window
  // has changed this prevents animation issues
  if(size != mLayoutSize)
  {
    WindowState::Type windowState = GetOsWindow()->GetState();

    // Skip resizing if Minimized
    if(windowState == WindowState::Minimized)
      return;

    mLayoutSize = mSize;
    mSize = size;
    mSizeGrips->SetSize(size);
    mPopUp->SetSize(mSize);

    mSizeGrips->SetActive(windowState == WindowState::Windowed);
    mTitleGrip->SetActive(windowState == WindowState::Windowed);

    if (windowState == WindowState::Windowed)
      mMax->SetIcon("TitleMaximize");
    else
      mMax->SetIcon("TitleRestore");

    mTitleGrip->SetTranslation(Vec3(0,0,0));

    Thickness border = Thickness(MainWindowUi::BorderPadding);

    Vec2 titleBarSize = Vec2(size.x - border.Size().x, 20.0f);
    mTitleGrip->SetSize(titleBarSize);
    mBorder->SetSize(size);

    mTitleBar->SetSize(titleBarSize);
    mTitleBar->SetTranslation(Vec3(border.TopLeft()));
    mBorder->SetColor(MainWindowUi::BorderColor);

    mTitleBack->SetColor(MainWindowUi::TitleBarColor);
    mTitleBack->SetSize(titleBarSize);

    Rect currentRect = RemoveThicknessRect(border, size);

    Rect titleRect = currentRect;
    titleRect.SizeY = titleBarSize.y;

    // Center the tile text on the title bar
    mTitleText->SizeToContents();
    PlaceCenterToRect(titleRect, mTitleText, Vec2(0,-3));

    // Hide if it overlaps the menu
    Rect menuRect = mMenu->GetScreenRect();
    Rect titleTextRect =  mTitleText->GetScreenRect();
    mTitleText->SetVisible(!menuRect.Overlap(titleTextRect));

    // Remove title bar
    currentRect.RemoveThickness(Thickness(0,titleBarSize.y,0,0));

    PlaceWithRect(currentRect, mClientWidget);
    
    WidgetListRange children = mClientWidget->GetChildren();
    if(!children.Empty())
      children.Front().SetSize(currentRect.GetSize());
  }

  RootWidget::UpdateTransform();
}

bool OsDocker::StartManipulation(Widget* widget, DockMode::Enum direction)
{
  RootWidget* rootWidget = widget->GetRootWidget();
  OsWindow* window = rootWidget->GetOsWindow();
  if(direction ==  DockMode::DockFill)
    window->ManipulateWindow(WindowBorderArea::Title);
  else if(direction ==  DockMode::DockTop)
    window->ManipulateWindow(WindowBorderArea::Top);
  else if(direction ==  DockMode::DockLeft)
    window->ManipulateWindow(WindowBorderArea::Left);
  else if(direction ==  DockMode::DockRight)
    window->ManipulateWindow(WindowBorderArea::Right);
  else if(direction ==  DockMode::DockBottom)
    window->ManipulateWindow(WindowBorderArea::Bottom);
  else if(direction == (DockMode::DockBottom | DockMode::DockRight))
    window->ManipulateWindow(WindowBorderArea::BottomRight);
  else if(direction == (DockMode::DockBottom | DockMode::DockLeft))
    window->ManipulateWindow(WindowBorderArea::BottomLeft);
  else if(direction == (DockMode::DockTop | DockMode::DockRight))
    window->ManipulateWindow(WindowBorderArea::TopRight);
  else if(direction == (DockMode::DockTop | DockMode::DockLeft))
    window->ManipulateWindow(WindowBorderArea::TopLeft);
  return true;
}

void OsDocker::Show(Widget* widget)
{
  RootWidget* root = widget->GetRootWidget();
  root->GetOsWindow()->TakeFocus();
}

void OsDocker::WidgetDestroyed(Widget* widget)
{
  RootWidget* root = widget->GetRootWidget();
  root->Destroy();
}

}
