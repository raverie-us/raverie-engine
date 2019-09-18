// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace MainWindowUi
{
const cstr cLocation = "EditorUi/MainWindow";
Tweakable(Vec4, TitleBarColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, BorderColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec2, BorderPadding, Vec2(1, 1), cLocation);

Tweakable(Vec4, ButtonColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, ButtonHover, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, ButtonClick, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, CloseColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, CloseHover, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, CloseClick, Vec4(1, 1, 1, 1), cLocation);
} // namespace MainWindowUi

namespace Events
{
DefineEvent(MainWindowTransformUpdated);
}

ZilchDefineType(MainWindowTransformEvent, builder, type)
{
  ZilchBindFieldGetter(mTargetWindow);
  ZilchBindFieldGetter(mOldScreenPosition);
  ZilchBindFieldGetter(mNewScreenPosition);
  ZilchBindFieldGetter(mOldScreenSize);
  ZilchBindFieldGetter(mNewScreenSize);
}

MainWindowTransformEvent::MainWindowTransformEvent(OsWindow* window,
                                                   Vec2Param oldScreenPosition,
                                                   Vec2Param newScreenPosition,
                                                   Vec2Param oldScreenSize,
                                                   Vec2Param newScreenSize) :
    mTargetWindow(window),
    mOldScreenPosition(oldScreenPosition),
    mNewScreenPosition(newScreenPosition),
    mOldScreenSize(oldScreenSize),
    mNewScreenSize(newScreenSize)
{
}

ZilchDefineType(MainWindow, builder, type)
{
  ZeroBindEvent(Events::MainWindowTransformUpdated, MainWindowTransformEvent);
}

MainWindow::MainWindow(OsWindow* window) : RootWidget(window)
{
  static const String className = "Window";
  mDefSet = mDefSet->GetDefinitionSet(className);

  mSize = Math::ToVec2(window->GetClientSize());
  mPreviousPosition = Math::ToVec2(window->GetMonitorClientPosition());

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
  menuArea->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness(0, 0, 0, 0)));

  Composite* mainMenu = new Composite(menuArea);
  mainMenu->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness(2, 2, 2, 2)));
  Label* zeroLabel = new Label(mainMenu);
  zeroLabel->SetText(window->GetTitle());
  zeroLabel->mText->ChangeDefinition(mDefSet->GetDefinition("TitleText"));
  Spacer* spacer = new Spacer(mainMenu);
  spacer->SetSize(Vec2(4, 0));
  Element* arrow = mainMenu->CreateAttached<Element>("TitleDown");
  arrow->mNotInLayout = false;
  Spacer* spacer2 = new Spacer(mainMenu);
  spacer2->SetSize(Vec2(4, 0));
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
  mTitleText->mText->ChangeDefinition(mDefSet->GetDefinition("TitleText"));

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

  if (window->mWindow.HasOwnMinMaxExitButtons())
  {
    mMin->SetActive(false);
    mMax->SetActive(false);
    mClose->SetActive(false);
  }

  mLayoutSize = Vec2(0, 0);
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
  if (attachType == AttachType::Direct)
    Composite::AttachChildWidget(child);
  else
    mClientWidget->AttachChildWidget(child);
  mLayoutSize = Vec2(0, 0);
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
  OsWindow* osWindow = GetOsWindow();
  Vec2 position = Math::ToVec2(osWindow->GetMonitorClientPosition());
  Vec2 size = ToVec2(osWindow->GetClientSize());

  mSizeGrips->MoveToFront();
  mPopUp->MoveToFront();

  WindowState::Type windowState = osWindow->GetState();

  // Has to be set outside of resize because maximize and fullscreen could be
  // same size.
  mSizeGrips->SetActive(windowState == WindowState::Windowed);
  mTitleGrip->SetActive(windowState != WindowState::Fullscreen);

  bool sizeUpdated = false;
  MainWindowTransformEvent eventToSend(osWindow, position, position, size, size);

  // Do not resize all child widgets unless
  // a child has been added or the size of the OS window
  // has changed this prevents animation issues
  if (size != mLayoutSize)
  {
    // Skip resizing if Minimized
    if (windowState == WindowState::Minimized)
      return;

    // Can't be set outside of resize or the event connection is lost.
    // There aren't any cases where it's diplayed wrong anyway.
    if (windowState != WindowState::Windowed)
      mMax->SetIcon("TitleRestore");
    else
      mMax->SetIcon("TitleMaximize");

    Vec2 previousSize = mSize;

    mLayoutSize = mSize;
    mSize = size;
    mSizeGrips->SetSize(size);
    mPopUp->SetSize(mSize);

    mTitleGrip->SetTranslation(Vec3(0, 0, 0));

    Thickness border = Thickness(MainWindowUi::BorderPadding);

    Vec2 titleBarSize = Vec2(size.x - border.Size().x, 20.0f);
    mTitleGrip->SetSize(titleBarSize);
    mBorder->SetSize(size);

    mTitleBar->SetSize(titleBarSize);
    mTitleBar->SetTranslation(Vec3(border.TopLeft()));
    mBorder->SetColor(MainWindowUi::BorderColor);

    mTitleBack->SetColor(MainWindowUi::TitleBarColor);
    mTitleBack->SetSize(titleBarSize);

    WidgetRect currentRect = RemoveThicknessRect(border, size);

    WidgetRect titleRect = currentRect;
    titleRect.SizeY = titleBarSize.y;

    // Center the tile text on the title bar
    mTitleText->SizeToContents();
    PlaceCenterToRect(titleRect, mTitleText, Vec2(0, -3));

    // Hide if it overlaps the menu
    WidgetRect menuRect = mMenu->GetScreenRect();
    WidgetRect titleTextRect = mTitleText->GetScreenRect();
    mTitleText->SetVisible(!menuRect.Overlap(titleTextRect));

    // Remove title bar
    currentRect.RemoveThickness(Thickness(0, titleBarSize.y, 0, 0));

    PlaceWithRect(currentRect, mClientWidget);

    WidgetListRange children = mClientWidget->GetChildren();
    if (!children.Empty())
      children.Front().SetSize(currentRect.GetSize());

    float diffX = Math::Abs(previousSize.x - mSize.x);
    float diffY = Math::Abs(previousSize.y - mSize.y);

    // Size must change by at least one pixel.
    if (diffX > 1.0f || diffY > 1.0f)
    {
      sizeUpdated = true;

      eventToSend.mOldScreenSize = previousSize;
      eventToSend.mNewScreenSize = mSize;
    }
  }

  RootWidget::UpdateTransform();

  // First check if the window position changed, must change by at least one
  // pixel.
  float distanceSq = (position - mPreviousPosition).LengthSq();
  if (distanceSq > 1.0f)
  {
    eventToSend.mOldScreenPosition = mPreviousPosition;
    eventToSend.mNewScreenPosition = position;

    // Size changes will be included, if any.
    Z::gEngine->DispatchEvent(Events::MainWindowTransformUpdated, &eventToSend);
    osWindow->DispatchEvent(Events::MainWindowTransformUpdated, &eventToSend);

    mPreviousPosition = position;
  }
  // If the position didn't change, did the size?
  else if (sizeUpdated)
  {
    Z::gEngine->DispatchEvent(Events::MainWindowTransformUpdated, &eventToSend);
    osWindow->DispatchEvent(Events::MainWindowTransformUpdated, &eventToSend);
  }
}

WindowBorderArea::Enum OsDocker::GetWindowBorderArea(Widget* widget, DockMode::Enum direction)
{
  RootWidget* rootWidget = widget->GetRootWidget();
  OsWindow* window = rootWidget->GetOsWindow();
  if (direction == DockMode::DockFill)
    return WindowBorderArea::Title;
  else if (direction == DockMode::DockTop)
    return WindowBorderArea::Top;
  else if (direction == DockMode::DockLeft)
    return WindowBorderArea::Left;
  else if (direction == DockMode::DockRight)
    return WindowBorderArea::Right;
  else if (direction == DockMode::DockBottom)
    return WindowBorderArea::Bottom;
  else if (direction == (DockMode::DockBottom | DockMode::DockRight))
    return WindowBorderArea::BottomRight;
  else if (direction == (DockMode::DockBottom | DockMode::DockLeft))
    return WindowBorderArea::BottomLeft;
  else if (direction == (DockMode::DockTop | DockMode::DockRight))
    return WindowBorderArea::TopRight;
  else if (direction == (DockMode::DockTop | DockMode::DockLeft))
    return WindowBorderArea::TopLeft;
  return WindowBorderArea::None;
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

} // namespace Zero
