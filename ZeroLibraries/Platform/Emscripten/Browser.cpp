///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
EM_JS(void, IFrameCreate, (int32_t id, int32_t w, int32_t h, int32_t x, int32_t y, const char* url),
{
  if (!document) return;
  var iframe = document.createElement('iframe');
  iframe.id = id.toString();
  iframe.style.border = '0px';
  iframe.style.position = 'absolute';
  iframe.style.width = w + 'px';
  iframe.style.height = h + 'px';
  iframe.style.left = x + 'px';
  iframe.style.top = y + 'px';
  iframe.src = UTF8ToString(url);
  document.body.appendChild(iframe);
});

EM_JS(void, IFrameDestroy, (int32_t id),
{
  if (!document) return;
  var iframe = document.getElementById(id.toString());
  if (!iframe)
    return console.error('No iframe found with id ' + id);
  iframe.remove();
});

EM_JS(void, IFrameSetSize, (int32_t id, int32_t w, int32_t h),
{
  if (!document) return;
  var iframe = document.getElementById(id.toString());
  if (!iframe)
    return console.error('No iframe found with id ' + id);
  iframe.style.width  = w + 'px';
  iframe.style.height = h + 'px';
});

EM_JS(void, IFrameSetClientPosition, (int32_t id, int32_t x, int32_t y),
{
  if (!document) return;
  var iframe = document.getElementById(id.toString());
  if (!iframe)
    return console.error('No iframe found with id ' + id);
  iframe.style.left  = x + 'px';
  iframe.style.top = y + 'px';
});

EM_JS(void, IFrameSetZIndex, (int32_t id, int32_t zindex),
{
  if (!document) return;
  var iframe = document.getElementById(id.toString());
  if (!iframe)
    return console.error('No iframe found with id ' + id);
  iframe.style.zIndex = zindex;
});

EM_JS(void, IFrameSetUrl, (int32_t id, const char* url),
{
  if (!document) return;
  var iframe = document.getElementById(id.toString());
  if (!iframe)
    return console.error('No iframe found with id ' + id);
  var urlStr = UTF8ToString(url);
  iframe.src = urlStr;
});

EM_JS(void, IFrameForward, (int32_t id),
{
  if (!document) return;
  var iframe = document.getElementById(id.toString());
  if (!iframe)
    return console.error('No iframe found with id ' + id);
  iframe.contentWindow.history.forward();
});

EM_JS(void, IFrameBack, (int32_t id),
{
  if (!document) return;
  var iframe = document.getElementById(id.toString());
  if (!iframe)
    return console.error('No iframe found with id ' + id);
  iframe.contentWindow.history.back();
});

EM_JS(void, IFrameReload, (int32_t id),
{
  if (!document) return;
  var iframe = document.getElementById(id.toString());
  if (!iframe)
    return console.error('No iframe found with id ' + id);
  // This seems really silly, but there's a lot of cross domain
  // issues unless we just directly modify the src.
  iframe.src += '';
});

EM_JS(void, IFrameSetVisible, (int32_t id, int32_t visible),
{
  if (!document) return;
  var iframe = document.getElementById(id.toString());
  if (!iframe)
    return console.error('No iframe found with id ' + id);
  if (visible)
    iframe.style.visibility = 'visible';
  else
    iframe.style.visibility = 'hidden';
});

EM_JS(void, IFrameSetFocus, (int32_t id, int32_t focus),
{
  if (!document) return;
  var iframe = document.getElementById(id.toString());
  if (!iframe)
    return console.error('No iframe found with id ' + id);
  if (focus)
    iframe.focus();
  else
    iframe.blur();
});

EM_JS(int32_t, IFrameGetFocus, (int32_t id),
{
  if (!document) return;
  var iframe = document.getElementById(id.toString());
  if (!iframe)
    return console.error('No iframe found with id ' + id);
  return (iframe == document.activeElement) ? 1 : 0;
});

Browser::Browser(const BrowserSetup& setup) :
  mUserData(nullptr),
  mLastSetUrl(setup.mUrl),
  mScrollSpeed(setup.mScrollSpeed),
  mBackgroundColor(setup.mBackgroundColor),
  mTransparent(setup.mTransparent),
  mSize(setup.mSize),
  mClientPosition(setup.mClientPosition),
  mZIndex(0),
  mVisible(true)
{
  static int32_t mCounter = 0;
  mHandle = (OsHandle)(size_t)mCounter;
  ++mCounter;

  IFrameCreate((int32_t)mHandle, mSize.x, mSize.y, mClientPosition.x, mClientPosition.y, setup.mUrl.c_str());
}

Browser::~Browser()
{
  IFrameDestroy((int32_t)mHandle);
}

bool Browser::IsFloatingOnTop()
{
  return true;
}

bool Browser::IsSecurityRestricted()
{
  return true;
}

Math::IntVec2 Browser::GetSize()
{
  return mSize;
}

void Browser::SetSize(Math::IntVec2Param size)
{
  if (size == mSize)
    return;
  mSize = size;
  IFrameSetSize((int32_t)mHandle, size.x, size.y);
}

Math::IntVec2 Browser::GetClientPosition()
{
  return mClientPosition;
}

void Browser::SetClientPosition(Math::IntVec2Param clientPosition)
{
  if (clientPosition == mClientPosition)
    return;
  mClientPosition = clientPosition;
  IFrameSetClientPosition((int32_t)mHandle, clientPosition.x, clientPosition.y);
}

int Browser::GetZIndex()
{
  return mZIndex;
}

void Browser::SetZIndex(int zindex)
{
  if (zindex == mZIndex)
    return;
  mZIndex = zindex;
  IFrameSetZIndex((int32_t)mHandle, zindex);
}

bool Browser::GetCanGoForward()
{
  return true;
}

bool Browser::GetCanGoBackward()
{
  return true;
}

void Browser::GoForward()
{
  IFrameForward((int32_t)mHandle);
}

void Browser::GoBackward()
{
  IFrameBack((int32_t)mHandle);
}

bool Browser::GetIsLoading()
{
  return false;
}

void Browser::Reload(bool useCache)
{
  IFrameReload((int32_t)mHandle);
}

void Browser::SetFocus(bool focus)
{
  IFrameSetFocus((int32_t)mHandle, (int32_t)focus);
}

bool Browser::GetFocus()
{
  return IFrameGetFocus((int32_t)mHandle) != 0;
}

void Browser::SetVisible(bool visible)
{
  if (visible == mVisible)
    return;

  mVisible = visible;
  IFrameSetVisible((int32_t)mHandle, (int32_t)visible);
}

bool Browser::GetVisible()
{
  return mVisible;
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
  IFrameSetUrl((int32_t)mHandle, url.c_str());
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