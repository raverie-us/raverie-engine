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
  ZilchBindGetter(TargetWindow);
  ZilchBindFieldGetter(mOldScreenSize);
  ZilchBindFieldGetter(mNewScreenSize);
}

MainWindowTransformEvent::MainWindowTransformEvent(Vec2Param oldScreenSize,
                                                   Vec2Param newScreenSize) :
    mOldScreenSize(oldScreenSize),
    mNewScreenSize(newScreenSize)
{
}

OsWindow* MainWindowTransformEvent::GetTargetWindow() {
  return OsWindow::sInstance;
}

ZilchDefineType(MainWindow, builder, type)
{
  ZeroBindEvent(Events::MainWindowTransformUpdated, MainWindowTransformEvent);
}

MainWindow::MainWindow() : RootWidget()
{
  static const String className = "Window";
  mDefSet = mDefSet->GetDefinitionSet(className);

  mSize = Math::ToVec2(Shell::sInstance->GetClientSize());

  mWindowWidget = new Composite(this, AttachType::Direct);
  mPopUp = new Composite(this, AttachType::Direct);

  mDocker = new MainDocker();

  mBorder = mWindowWidget->CreateAttached<Element>(cWhiteSquareBorder);

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
  Label* label = new Label(mainMenu);
  label->SetText("Raverie Editor");
  label->mText->ChangeDefinition(mDefSet->GetDefinition("TitleText"));
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

  mTitleText = new Label(mTitleBar, DefaultTextStyle, GetOrganization() + " " + GetApplicationName());
  mTitleText->SetInteractive(false);
  mTitleText->SetNotInLayout(true);
  mTitleText->mText->ChangeDefinition(mDefSet->GetDefinition("TitleText"));
  mLayoutSize = Vec2(0, 0);
}

void MainWindow::LoadMenu(StringParam menuName)
{
  mMenu->LoadMenu(menuName);
  mMenu->MarkAsNeedsUpdate();
}

void MainWindow::SetTitle(StringParam title)
{
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

Composite* MainWindow::GetPopUp()
{
  return mPopUp;
}

void MainWindow::UpdateTransform()
{
  Vec2 size = ToVec2(Shell::sInstance->GetClientSize());

  mPopUp->MoveToFront();

  bool sizeUpdated = false;
  MainWindowTransformEvent eventToSend(size, size);

  // Do not resize all child widgets unless
  // a child has been added or the size of the OS window
  // has changed this prevents animation issues
  if (size != mLayoutSize)
  {
    Vec2 previousSize = mSize;

    mLayoutSize = mSize;
    mSize = size;
    mPopUp->SetSize(mSize);

    Thickness border = Thickness(MainWindowUi::BorderPadding);

    Vec2 titleBarSize = Vec2(size.x - border.Size().x, 20.0f);
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

  // If the position didn't change, did the size?
  if (sizeUpdated)
  {
    Z::gEngine->DispatchEvent(Events::MainWindowTransformUpdated, &eventToSend);
    OsWindow::sInstance->DispatchEvent(Events::MainWindowTransformUpdated, &eventToSend);
  }
}

void MainDocker::WidgetDestroyed(Widget* widget)
{
  RootWidget* root = widget->GetRootWidget();
  root->Destroy();
}

} // namespace Zero
