///////////////////////////////////////////////////////////////////////////////
///
/// \file ConsoleUi.cpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(ConsolePrint);
}//namespace Events

ZilchDefineType(ConsoleTextEvent, builder, type)
{
}

ZilchDefineType(ConsoleUi, builder, type)
{
}

void ConsoleUi::UiConsoleListener::Print(FilterType filterType, cstr message)
{
  Owner->ConsoleLog(filterType, message);
}

ConsoleUi::ConsoleUi(Composite* parent)
  : TextEditor(parent), mListener(this)
{
  this->SetLexer(Lexer::Console);
  this->SetReadOnly(true);

  ConnectThisTo(this, Events::LeftClick, OnLeftClick);
  ConnectThisTo(this, Events::ConsolePrint, OnConsolePrint);
  ConnectThisTo(this, Events::WidgetShown, OnWidgetShown);

  mHotspots.PushBack(new HyperLinkHotspot());
  mHotspots.PushBack(new ResourceHotspot());
  mHotspots.PushBack(new ObjectHotspot());
  mHotspots.PushBack(new FileHotspot());
  mHotspots.PushBack(new CommandHotspot());
}

ConsoleUi::~ConsoleUi()
{

}

bool ConsoleUi::TakeFocusOverride()
{
  //do nothing, so the console does not take
  //focus on open
  return false;
}

void ConsoleUi::AddLine(StringParam line)
{
  /*
   if(line == mLastLine)
   {
     SetReadOnly(false);
     mRepeatCount += 1;
     int lastLine = GetCurrentLine() - 1;
     String text = this->GetLineText(lastLine);
     int length = this->GetLineLength(lastLine);
     this->RemoveRange(GetPositionFromLine(lastLine), length);
     String dupString = String::Format("(%d) %s", mRepeatCount, mLastLine.c_str());
     this->Append(dupString);
     SetReadOnly(true);
     return;
   }
   */
   
   mLastLine = line;
   mRepeatCount = 1;

  const int LineOverflow = 1000;
  const int MaxLineSize = 10000;
  this->SetReadOnly(false);

  //Prevent the console from growing to large
  if( GetLineCount() >= MaxLineSize)
  {
    int start = 0;
    int end = this->GetPositionFromLine(LineOverflow);
    this->Select(start, end);
    ReplaceSelection(String());
    // Remove text
  }

  this->Append(line);

  TextEditorHotspot::MarkHotspots(this, this->GetCurrentLine());
  this->Append("\n");

  if(GetLineCount() > 10)
    this->GoToLine(this->GetLineCount() - 1);

  this->ClearUndo();
  this->SetReadOnly(true);
}

void ConsoleUi::AddBlock(StringParam text)
{
  //split on lines and then add each manually (needed for hotspots)
  StringTokenRange range(text.All(), '\n');
  for(; !range.Empty(); range.PopFront())
    AddLine(range.Front());
}

void ConsoleUi::OnWidgetShown(Event* event)
{
  // Scroll to the end of the console every time we open it up
  GoToPosition(GetLength());
}

void ConsoleUi::OnConsolePrint(ConsoleTextEvent* event)
{
  StringRange eventText(event->Text);
  while(!eventText.Empty())
  {
    if(eventText.Front() == '\n')
    {
      AddLine(mCurrentLine.ToString());
      mCurrentLine.Deallocate();
    }
    else
    {
      mCurrentLine.Append(eventText.Front());
    }
    eventText.PopFront();
  }
}

void ConsoleUi::OnKeyDown(KeyboardEvent* event)
{
  if(event->CtrlPressed && event->Key == Keys::Delete)
  {
    this->ClearAllReadOnly();
  }
  
  // Make sure to call the base's key down so we'll be able to copy text
  TextEditor::OnKeyDown(event);
}

void ConsoleUi::OnLeftClick(MouseEvent* event)
{
  //TextEditorHotspot::ClickHotspots(this, HotSpots);
}

void ConsoleUi::ConsoleLog(FilterType filterType, cstr message)
{
  //Send this class an event to prevent make sure the printing happens 
  //on the main ui thread.
  ConsoleTextEvent* event = new ConsoleTextEvent();
  event->Text = message;
  Z::gDispatch->Dispatch(ThreadContext::Main, this, Events::ConsolePrint, event);
}

}
