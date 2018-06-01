///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

Browser::Browser(const BrowserSetup& setup)
{
  Error("Not implemented");
}

Browser::~Browser()
{
  Error("Not implemented");
}

Math::IntVec2 Browser::GetSize()
{
  Error("Not implemented");
  return Math::IntVec2::cZero;
}

void Browser::SetSize(Math::IntVec2Param size)
{
  Error("Not implemented");
}

bool Browser::GetCanGoForward()
{
  Error("Not implemented");
  return false;
}

bool Browser::GetCanGoBackward()
{
  Error("Not implemented");
  return false;
}

void Browser::GoForward()
{
  Error("Not implemented");
}

void Browser::GoBackward()
{
  Error("Not implemented");
}

bool Browser::GetIsLoading()
{
  Error("Not implemented");
  return false;
}

void Browser::Reload(bool useCache)
{
  Error("Not implemented");
}

void Browser::SetFocus(bool focus)
{
  Error("Not implemented");
}

bool Browser::GetFocus()
{
  Error("Not implemented");
  return false;
}

void Browser::SetVisible(bool visible)
{
  Error("Not implemented");
}

bool Browser::GetVisible()
{
  Error("Not implemented");
  return false;
}

Math::Vec4 Browser::GetBackgroundColor()
{
  Error("Not implemented");
  return Math::Vec4::cZero;
}

void Browser::SetBackgroundColor(Math::Vec4Param color)
{
  Error("Not implemented");
}

bool Browser::GetTransparent()
{
  Error("Not implemented");
  return false;
}

void Browser::SetTransparent(bool transparent)
{
  Error("Not implemented");
}

String Browser::GetStatus()
{
  Error("Not implemented");
  return String();
}

String Browser::GetTitle()
{
  Error("Not implemented");
  return String();
}

Math::Vec2 Browser::GetScrollSpeed()
{
  Error("Not implemented");
  return Math::Vec2::cZero;
}

void Browser::SetScrollSpeed(Math::Vec2Param pixelsPerScroll)
{
  Error("Not implemented");
}

void Browser::SetUrl(StringParam url)
{
  Error("Not implemented");
}

String Browser::GetUrl()
{
  Error("Not implemented");
  return String();
}

void Browser::ExecuteScriptFromLocation(StringParam script, StringParam url, int line)
{
  Error("Not implemented");
}

void Browser::SimulateKey(int key, bool down, BrowserModifiers::Enum modifiers)
{
  Error("Not implemented");
}

void Browser::SimulateTextTyped(int character, BrowserModifiers::Enum modifiers)
{
  Error("Not implemented");
}

void Browser::SimulateMouseMove(Math::IntVec2Param localPosition, BrowserModifiers::Enum modifiers)
{
  Error("Not implemented");
}

void Browser::SimulateMouseClick(Math::IntVec2Param localPosition, MouseButtons::Enum button, bool down, BrowserModifiers::Enum modifiers)
{
  Error("Not implemented");
}

void Browser::SimulateMouseDoubleClick(Math::IntVec2Param localPosition, MouseButtons::Enum button, BrowserModifiers::Enum modifiers)
{
  Error("Not implemented");
}

void Browser::SimulateMouseScroll(Math::IntVec2Param localPosition, Math::Vec2Param delta, BrowserModifiers::Enum modifiers)
{
  Error("Not implemented");
}

// This must be called before any browsers are created
void Browser::PlatformCreate() 
{
  Error("Not implemented");
}

void Browser::PlatformDestroy()
{
  Error("Not implemented");
}

void Browser::PlatformUpdate()
{
  Error("Not implemented");
}

//------------------------------------------------------------------------------ BrowserSubProcess
int BrowserSubProcess::Execute()
{
  Error("Not implemented");
  return 0;
}

}// namespace Zero