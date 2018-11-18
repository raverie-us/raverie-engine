///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

WidgetTest::WidgetTest(Composite* parent, StringParam name)
  :ColorBlock(parent)
{
  Name = name;
  ConnectThisTo(this, Events::LeftMouseUp, OnMouseEvent);
  ConnectThisTo(this, Events::LeftMouseDown, OnMouseEvent);
  ConnectThisTo(this, Events::RightMouseUp,  OnMouseEvent);
  ConnectThisTo(this, Events::RightMouseDown, OnMouseEvent);
  ConnectThisTo(this, Events::MiddleMouseDown,  OnMouseEvent);
  ConnectThisTo(this, Events::MiddleMouseUp, OnMouseEvent);

  ConnectThisTo(this, Events::LeftMouseDrag,  OnMouseEvent);
  ConnectThisTo(this, Events::RightMouseDrag,  OnMouseEvent);
  
  ConnectThisTo(this, Events::LeftClick,  OnMouseEvent);
  ConnectThisTo(this, Events::RightClick,  OnMouseEvent);
  ConnectThisTo(this, Events::MiddleClick,  OnMouseEvent);

  ConnectThisTo(this, Events::DoubleClick, OnMouseEvent);
  
  ConnectThisTo(this, Events::MouseExit, OnMouseEvent);
  ConnectThisTo(this, Events::MouseEnter, OnMouseEvent);

  ConnectThisTo(this, Events::MouseMove, OnMouseEvent);
  ConnectThisTo(this, Events::MouseScroll, OnMouseEvent);

  ConnectThisTo(this, Events::MouseHold, OnMouseEvent);
  ConnectThisTo(this, Events::MouseHover, OnMouseEvent);

  ConnectThisTo(this, Events::MouseUpdate, OnMouseEvent);

  ConnectThisTo(this, Events::KeyDown, OnKeyboardEvent);
  ConnectThisTo(this, Events::KeyUp, OnKeyboardEvent);

  ConnectThisTo(this, Events::FocusLost, OnFocusEvent);
  ConnectThisTo(this, Events::FocusGained, OnFocusEvent);
  ConnectThisTo(this, Events::FocusReset, OnFocusEvent);
}

void WidgetTest::OnMouseEvent(MouseEvent* event)
{
  ZPrint("%s Mouse %s\n",  Name.c_str(), event->EventId.c_str());
  //ZPrint("%s Mouse Position %g, %g\n",  Name.c_str(), event->Position.x, event->Position.y);
}

void WidgetTest::OnKeyboardEvent(KeyboardEvent* event)
{
  ZPrint("%s KeyboardEvent %s\n",  Name.c_str(), event->EventId.c_str());
  //ZPrint("%s Key %s\n", KeyToName(event->Key));
}

void WidgetTest::OnFocusEvent(FocusEvent* event)
{
  ZPrint("%s Focus Event %s\n", Name.c_str(), event->EventId.c_str());
}

ParentWidgetTest::ParentWidgetTest(Composite* parent, StringParam name)
  :Composite(parent)
{
  Name = name;
  ConnectThisTo(this, Events::FocusLost, OnFocusEvent);
  ConnectThisTo(this, Events::FocusGained, OnFocusEvent);

  ConnectThisTo(this, Events::FocusLostHierarchy, OnFocusEvent);
  ConnectThisTo(this, Events::FocusGainedHierarchy, OnFocusEvent);

  ConnectThisTo(this, Events::FocusReset, OnFocusEvent);

  ConnectThisTo(this, Events::MouseEnterHierarchy, OnMouseEvent);
  ConnectThisTo(this, Events::MouseExitHierarchy, OnMouseEvent);

}

void ParentWidgetTest::OnFocusEvent(FocusEvent* event)
{
  ZPrint("%s Focus Event %s\n",  Name.c_str(),  event->EventId.c_str());
}

void ParentWidgetTest::OnMouseEvent(MouseEvent* event)
{
  ZPrint("%s Mouse %s\n",  Name.c_str(), event->EventId.c_str());
  ZPrint("%s Mouse Position %g, %g\n",  Name.c_str(), event->Position.x, event->Position.y);
}

void FlexMinSizeLayoutTest(Composite* testWindow)
{
  testWindow->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness::cZero));

  // This widget has a large min size so it
  // will take most of the size in spite of 
  // of all the boxes having equal flex
  // Check to make sure the last button is visible
  TextButton* b0 = new TextButton(testWindow);
  b0->SetMinSize(Vec2(100, 100));
  b0->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  b0->SetText("Min Size 100 + Flex");

  TextButton* b1 = new TextButton(testWindow);
  b1->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  b1->SetText("Flex 20");

  TextButton* b2 = new TextButton(testWindow);
  b2->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  b2->SetText("Flex 20");

  TextButton* minButton = new TextButton(testWindow);
  minButton->SetText("Min size button");
}

void FlexTests(Composite* testWindow)
{
  testWindow->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness::cZero));

  Composite* flexRow = new Composite(testWindow);
  flexRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(4, 4), Thickness(0, 0, 0, 0)));

  TextButton* frb1 = new TextButton(flexRow);
  frb1->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
  frb1->SetText("Flex Row Button 0");

  TextButton* frb2 = new TextButton(flexRow);
  frb2->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
  frb2->SetText("Flex Row Button 1");

  Composite* someRow = new Composite(testWindow);
  someRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(4, 4), Thickness(0, 0, 0, 0)));

  Spacer* s0 = new Spacer(someRow);
  s0->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);

  TextButton* rb1 = new TextButton(someRow);
  rb1->SetText("Row Button 0");

  Spacer* s1 = new Spacer(someRow);
  s1->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);

  TextButton* rb2 = new TextButton(someRow);
  rb2->SetText("Row Button 1");

  Spacer* s2 = new Spacer(someRow);
  s2->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);

  Composite* bottom = new Composite(testWindow);
  bottom->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(4, 4), Thickness(4, 4, 4, 4)));

  TextButton* t8 = new TextButton(bottom);
  t8->SetText("Vertical Button 1");

  TextButton* t9 = new TextButton(bottom);
  t9->SetText("Vertical Button 2");

  TextButton* flexButton0 = new TextButton(testWindow);
  flexButton0->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  flexButton0->SetText("Flex 20%");

  TextButton* flexButton1 = new TextButton(testWindow);
  flexButton1->SetSizing(SizeAxis::Y, SizePolicy::Flex, 80);
  flexButton1->SetText("Flex 80%");

  TextButton* minButton = new TextButton(testWindow);
  minButton->SetText("Min size button");
}

void StandardControlsLayoutTest(Composite* testWindow)
{
  testWindow->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness::cZero));

  StringSource* source = new StringSource();
  source->Strings.PushBack("One");
  source->Strings.PushBack("Two");
  source->Strings.PushBack("Three");
  source->Strings.PushBack("Four");

  TextButton* t0 = new TextButton(testWindow);
  t0->SetText("Some text");

  ComboBox* t2 = new ComboBox(testWindow);
  t2->SetListSource(source);

  TextCheckBox* t3 = new TextCheckBox(testWindow);
  t3->SetText("Blah");

  Slider* t4 = new Slider(testWindow, SliderType::Number);
  t4->SetPercentage(0.5f, true);

  TextBox* t5 = new TextBox(testWindow);
  t5->SetText("Some text");
  t5->SetEditable(true);

  ListBox* t6 = new ListBox(testWindow);
  t6->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  t6->SetDataSource(source);

  TextButton* minButton = new TextButton(testWindow);
  minButton->SetText("Min size button");
}

template<typename attach>
void TestLayout(Composite* owner, StringParam name, attach doAttach)
{
  Window* window = new Window(owner);
  window->SetTitle(name);
  Composite* inner = new Composite(window);
  inner->SetName(name);
  doAttach(inner);
  window->SizeToContents();
  CenterToWindow(owner, window, true);
}


void OpenTestWidgets(Composite* owner)
{
  TestLayout(owner, "FlexMinSizeLayoutTest", FlexMinSizeLayoutTest);
  TestLayout(owner, "StandardControlsLayoutTest", StandardControlsLayoutTest);
  TestLayout(owner, "FlexTests", FlexTests);

  OpenSeperateWindow(NULL);
}


void OpenSeperateWindow(OsWindow* mainWindow)
{
  OsShell* shell = Z::gEngine->has(OsShell);
  IntVec2 windowPos = IntVec2(0,0);
  IntVec2 windowSize = IntVec2(800, 800);
  WindowStyleFlags::Enum flags = (WindowStyleFlags::Enum)(WindowStyleFlags::Close | WindowStyleFlags::Resizable | WindowStyleFlags::TitleBar);
  OsWindow* newWindow = shell->CreateOsWindow("Testing", windowSize, windowPos, mainWindow, flags);
  RootWidget* rootWidget = new RootWidget(newWindow);
  Window* testWindow = new Window(rootWidget);
  testWindow->SetSize(Pixels(400, 400));
  StandardControlsLayoutTest(testWindow);
}

}
