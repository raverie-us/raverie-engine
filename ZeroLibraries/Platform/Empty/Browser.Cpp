///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

Browser::Browser(const BrowserSetup& setup) :
  mUserData(nullptr),
  mLastSetUrl(setup.mUrl),
  mScrollSpeed(setup.mScrollSpeed),
  mBackgroundColor(setup.mBackgroundColor),
  mTransparent(setup.mTransparent),
  mSize(setup.mSize)
{
}

Browser::~Browser()
{
}

Math::IntVec2 Browser::GetSize()
{
  return mSize;
}

void Browser::SetSize(Math::IntVec2Param size)
{
  mSize = size;
}

bool Browser::GetCanGoForward()
{
  return false;
}

bool Browser::GetCanGoBackward()
{
  return false;
}

void Browser::GoForward()
{
}

void Browser::GoBackward()
{
}

bool Browser::GetIsLoading()
{
  return false;
}

void Browser::Reload(bool useCache)
{
}

void Browser::SetFocus(bool focus)
{
}

bool Browser::GetFocus()
{
  return false;
}

void Browser::SetVisible(bool visible)
{
}

bool Browser::GetVisible()
{
  return false;
}

Math::Vec4 Browser::GetBackgroundColor()
{
  return mBackgroundColor;
}

void Browser::SetBackgroundColor(Math::Vec4Param color)
{
  mBackgroundColor = color;
}

bool Browser::GetTransparent()
{
  return mTransparent;
}

void Browser::SetTransparent(bool transparent)
{
  mTransparent = transparent;
}

String Browser::GetStatus()
{
  return mStatus;
}

String Browser::GetTitle()
{
  return mTitle;
}

Math::Vec2 Browser::GetScrollSpeed()
{
  return mScrollSpeed;
}

void Browser::SetScrollSpeed(Math::Vec2Param pixelsPerScroll)
{
  mScrollSpeed = pixelsPerScroll;
}

void Browser::SetUrl(StringParam url)
{
  mLastSetUrl = url;
}

String Browser::GetUrl()
{
  return mLastSetUrl;
}

void Browser::ExecuteScriptFromLocation(StringParam script, StringParam url, int line)
{
}

void Browser::SimulateKey(int key, bool down, BrowserModifiers::Enum modifiers)
{
}

void Browser::SimulateTextTyped(int character, BrowserModifiers::Enum modifiers)
{
}

void Browser::SimulateMouseMove(Math::IntVec2Param localPosition, BrowserModifiers::Enum modifiers)
{
}

void Browser::SimulateMouseClick(Math::IntVec2Param localPosition, MouseButtons::Enum button, bool down, BrowserModifiers::Enum modifiers)
{
}

void Browser::SimulateMouseDoubleClick(Math::IntVec2Param localPosition, MouseButtons::Enum button, BrowserModifiers::Enum modifiers)
{
}

void Browser::SimulateMouseScroll(Math::IntVec2Param localPosition, Math::Vec2Param delta, BrowserModifiers::Enum modifiers)
{
}

// This must be called before any browsers are created
void Browser::PlatformCreate() 
{
}

void Browser::PlatformDestroy()
{
}

void Browser::PlatformUpdate()
{
}

//------------------------------------------------------------------------------ BrowserSubProcess
int BrowserSubProcess::Execute()
{
  return 0;
}

}// namespace Zero