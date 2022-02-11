// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
Shell* Shell::sInstance;

// These values must be defined to handle the WM_MOUSEHWHEEL on
// Windows 2000 and Windows XP, the first two values will be defined
// in the Longhorn SDK and the last value is a default value
// that will not be defined in the Longhorn SDK but will be needed for
// handling WM_MOUSEHWHEEL messages emulated by IntelliType Pro
// or IntelliPoint (if implemented in future versions).
#define WM_MOUSEHWHEEL 0x020E

cwstr cWindowClassName = L"ZeroWindow";

struct ShellPrivateData
{
  ITaskbarList3* mTaskbar;
  uint mTaskbarButtonCreated;
  HCURSOR mCursor;
};

// Documentation says the character limit is 32k limit for ANSI
// http://msdn.microsoft.com/en-us/library/ms646927(v=vs.85).aspx
// only using a fourth of the limit
const int cFileBufferSize = 8192;

void FileDialog(FileDialogInfo& config, bool opening)
{
  // Create Windows OPENFILENAME structure and zero it
  OPENFILENAME fileDialog = {0};
  fileDialog.lStructSize = sizeof(OPENFILENAME);
  fileDialog.hwndOwner = 0;

  // Set up buffers.
  wchar_t fileName[cFileBufferSize] = {0};
  fileDialog.lpstrFile = fileName;
  fileDialog.nMaxFile = cFileBufferSize;

  wchar_t fileFilter[cFileBufferSize] = {0};
  fileDialog.lpstrFilter = fileFilter;

  wchar_t* fileFilterPosition = fileFilter;
  forRange (FileDialogFilter& fileFilter, config.mSearchFilters.All())
  {
    WString description = Widen(fileFilter.mDescription);
    WString filter = Widen(fileFilter.mFilter);
    ZeroStrCpyW(fileFilterPosition, description.Size(), description.c_str());
    fileFilterPosition += description.Size();
    ZeroStrCpyW(fileFilterPosition, filter.Size(), filter.c_str());
    fileFilterPosition += filter.Size();
  }

  // File filters are double null terminated, so add one more at the end
  WString nullChar = Widen("\0");
  ZeroStrCpyW(fileFilterPosition, 1, nullChar.c_str());

  // Set the default file name
  WString defaultFileName = Widen(config.DefaultFileName);
  ZeroStrCpyW(fileName, cFileBufferSize, defaultFileName.c_str());

  // Object containing the data is needed to properly keep it alive while the
  // dialog is open
  WString title = Widen(config.Title);
  WString startingDirectory = Widen(config.StartingDirectory);

  fileDialog.lpstrFileTitle = NULL;
  fileDialog.lpstrInitialDir = startingDirectory.c_str();
  fileDialog.lpstrTitle = title.c_str();

  // Extensions is ".ext" so skip one character
  if (!config.mDefaultSaveExtension.Empty())
  {
    WString defaultExtension = Widen(config.mDefaultSaveExtension);
    fileDialog.lpstrDefExt = defaultExtension.c_str();
  }

  // Use explorer interface
  fileDialog.Flags |= OFN_EXPLORER;

  if (config.Flags & FileDialogFlags::Folder)
  {
    ZeroStrCpyW(fileName, cFileBufferSize, L"Folder");
  }

  if (config.Flags & FileDialogFlags::MultiSelect)
    fileDialog.Flags |= OFN_ALLOWMULTISELECT;

  BOOL success = 0;
  if (opening)
    success = GetOpenFileName(&fileDialog);
  else
    success = GetSaveFileName(&fileDialog);

  if (success)
  {
    // If nFileExtension is zero, there is multiple files
    // from a multiselect
    if (fileDialog.nFileExtension == 0 && config.Flags & FileDialogFlags::MultiSelect)
    {
      // lpstrFile will be a multi null terminated string
      // beginning with the directory name then a list of
      // files and finally a null terminator. (so two at the end)
      wchar_t* current = fileDialog.lpstrFile;

      String directoryName;

      // Check for null termination
      // or double null termination which signals the
      // end of the file list
      while (*current != '\0')
      {
        cwstr currentFile = current;

        // On the first loop this sets the directory name
        if (directoryName.Empty())
          directoryName = Narrow(currentFile);
        else
        {
          // Append a full file path Append the file name to the path
          config.mFiles.PushBack(FilePath::Combine(directoryName, Narrow(currentFile)));
        }

        // Find null terminator
        while (*current != '\0')
          ++current;
        // Skip the terminator
        ++current;
      }
    }
    else
    {
      config.mFiles.PushBack(Narrow(fileDialog.lpstrFile).c_str());
    }
  }

  // We assume that the first item is the path if we were multi selecting, but
  // if the user types in a random string then we can get something that looks
  // like a path (no extension) and then get no files. So if we didn't get any
  // files for some reason then mark that we actually failed.
  if (config.mCallback)
    config.mCallback(config.mFiles, config.mUserData);
}

// Convert bitmap to image. Hdc is hdc bitmap was created in
void ConvertImage(HDC hdc, HBITMAP bitmapHandle, Image* image)
{
  BITMAP bmpScreen;
  GetObject(bitmapHandle, sizeof(BITMAP), &bmpScreen);

  BITMAPINFOHEADER bi;
  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = bmpScreen.bmWidth;
  bi.biHeight = bmpScreen.bmHeight;
  bi.biPlanes = 1;
  bi.biBitCount = 32;
  bi.biCompression = BI_RGB;
  bi.biSizeImage = 0;
  bi.biXPelsPerMeter = 0;
  bi.biYPelsPerMeter = 0;
  bi.biClrUsed = 0;
  bi.biClrImportant = 0;

  // Row size is padded to 32 bits
  DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;
  byte* rawBitmapData = (byte*)zAllocate(dwBmpSize);

  // Gets the "bits" from the bitmap and copies them into a buffer
  // which is pointed to by lpbitmap.
  GetDIBits(hdc, bitmapHandle, 0, (UINT)bmpScreen.bmHeight, rawBitmapData, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

  // Allocate the image and copy over pixels
  image->Allocate(bmpScreen.bmWidth, bmpScreen.bmHeight);

  for (int y = 0; y < bmpScreen.bmHeight; ++y)
  {
    for (int x = 0; x < bmpScreen.bmWidth; ++x)
    {
      byte* data = rawBitmapData + bmpScreen.bmWidth * 4 * (bmpScreen.bmHeight - y - 1) + x * 4;
      image->GetPixel(x, y) = ByteColorRGBA(data[2], data[1], data[0], 255);
    }
  }
}

bool GetWindowImage(HWND windowHandle, Image* image)
{
  RECT windowRect;
  GetWindowRect(windowHandle, &windowRect);

  int width = windowRect.right - windowRect.left;
  int height = windowRect.bottom - windowRect.top;

  HDC hdcScreen = GetDC(NULL);

  HDC hdcDest = CreateCompatibleDC(hdcScreen);
  HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);

  // Select the compatible bitmap into the compatible memory DC.
  HGDIOBJ old = SelectObject(hdcDest, hBitmap);

  // Bit block transfer into our compatible memory DC.
  if (!BitBlt(hdcDest, 0, 0, width, height, hdcScreen, windowRect.left, windowRect.top, SRCCOPY))
  {
    return false;
  }

  SelectObject(hdcDest, old);
  ConvertImage(hdcScreen, hBitmap, image);
  DeleteDC(hdcDest);
  ReleaseDC(windowHandle, hdcScreen);

  DeleteObject(hBitmap);

  return true;
}

RECT GetClientRectInMonitor(HWND windowHandle)
{
  RECT clientRect;
  // GetClientRect will set the left/top to (0,0), since the
  // client area in client space is always at the origin...
  ::GetClientRect(windowHandle, &clientRect);
  ::ClientToScreen(windowHandle, (LPPOINT)&clientRect.left);
  ::ClientToScreen(windowHandle, (LPPOINT)&clientRect.right);
  return clientRect;
}

enum WindowArea
{
  SC_SIZE_HTLEFT = 1,
  SC_SIZE_HTRIGHT = 2,
  SC_SIZE_HTTOP = 3,
  SC_SIZE_HTTOPLEFT = 4,
  SC_SIZE_HTTOPRIGHT = 5,
  SC_SIZE_HTBOTTOM = 6,
  SC_SIZE_HTBOTTOMLEFT = 7,
  SC_SIZE_HTBOTTOMRIGHT = 8
};

uint RectWidth(RECT& rect)
{
  return rect.right - rect.left;
}

uint RectHeight(RECT& rect)
{
  return rect.bottom - rect.top;
}

IntRect FromRECT(RECT& rect)
{
  IntRect p;
  p.X = rect.left;
  p.Y = rect.top;
  p.SizeX = RectWidth(rect);
  p.SizeY = RectHeight(rect);
  return p;
}

RECT ToRECT(const IntRect& rect)
{
  RECT p;
  p.left = rect.Left();
  p.top = rect.Top();
  p.right = rect.Right();
  p.bottom = rect.Bottom();
  return p;
}

IntVec2 ToIntVec2(POINT& point)
{
  return IntVec2(point.x, point.y);
}

void ManipulateWindow(ShellWindow* window, WindowBorderArea::Enum area)
{
  WPARAM param = 0;
  switch (area)
  {
  case WindowBorderArea::Title:
    param = SC_MOVE | HTCAPTION;
    break;
  case WindowBorderArea::TopLeft:
    param = SC_SIZE | SC_SIZE_HTTOPLEFT;
    break;
  case WindowBorderArea::Top:
    param = SC_SIZE | SC_SIZE_HTTOP;
    break;
  case WindowBorderArea::TopRight:
    param = SC_SIZE | SC_SIZE_HTTOPRIGHT;
    break;
  case WindowBorderArea::Left:
    param = SC_SIZE | SC_SIZE_HTLEFT;
    break;
  case WindowBorderArea::Right:
    param = SC_SIZE | SC_SIZE_HTRIGHT;
    break;
  case WindowBorderArea::BottomLeft:
    param = SC_SIZE | SC_SIZE_HTBOTTOMLEFT;
    break;
  case WindowBorderArea::Bottom:
    param = SC_SIZE | SC_SIZE_HTBOTTOM;
    break;
  case WindowBorderArea::BottomRight:
    param = SC_SIZE | SC_SIZE_HTBOTTOMRIGHT;
    break;
  case WindowBorderArea::None:
    return;
  }

  SendMessage((HWND)window->mHandle, WM_SYSCOMMAND, param, 0);

  // The window procedure never gets a mouse up after dragging
  // so send one now for double click to work
  POINT monitorCursorPos;
  GetCursorPos(&monitorCursorPos);
  IntVec2 clientPosition = window->MonitorToClient(ToIntVec2(monitorCursorPos));
  if (window->mOnMouseUp)
    window->mOnMouseUp(clientPosition, MouseButtons::Left, window);
}

DWORD Win32StyleFromWindowStyle(WindowStyleFlags::Enum styleFlags)
{
  DWORD win32Style = WS_POPUP | WS_VISIBLE;
  if (styleFlags & WindowStyleFlags::NotVisible)
    win32Style &= ~WS_VISIBLE;
  if (styleFlags & WindowStyleFlags::TitleBar)
    win32Style |= WS_CAPTION;
  if (styleFlags & WindowStyleFlags::Resizable)
    win32Style |= WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
  // WS_SYSMENU shows buttons on titlebar, don't use when in client only.
  // The buttons interfere with input on some drivers, even when border is
  // disabled.
  if (styleFlags & WindowStyleFlags::Close && !(styleFlags & WindowStyleFlags::ClientOnly))
    win32Style |= WS_SYSMENU | WS_MINIMIZEBOX;
  // Removing the caption flag causes a border flicker sometimes when the
  // application gains focus. However, not removing it causes an incorrect
  // window size sometimes when toggling between windowed/maximized.
  if (styleFlags & WindowStyleFlags::ClientOnly)
    win32Style &= ~WS_CAPTION;
  return win32Style;
}

DWORD GetWin32ExStyle(WindowStyleFlags::Enum styleFlags)
{
  if (styleFlags & WindowStyleFlags::OnTaskBar)
    return WS_EX_APPWINDOW;
  else
    return WS_EX_TOOLWINDOW;
}

// This function is intentionally used idenitically in the below two functions
// since it is used for documentation purposes to denote which LPARAMs have
// client vs monitor positions.
IntVec2 GenericPositionFromLParam(LPARAM lParam)
{
  // Systems with multiple monitors can have negative x and y coordinates
  return IntVec2(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
}

IntVec2 ClientPositionFromLParam(LPARAM lParam)
{
  return GenericPositionFromLParam(lParam);
}

IntVec2 MonitorPositionFromLParam(LPARAM lParam)
{
  return GenericPositionFromLParam(lParam);
}

// From 'The Old New Thing' blog:
// https://blogs.msdn.microsoft.com/oldnewthing/20131017-00/?p=2903
BOOL UnadjustWindowRect(LPRECT prc, DWORD dwStyle, BOOL fMenu)
{
  RECT rc;
  SetRectEmpty(&rc);
  BOOL fRc = AdjustWindowRect(&rc, dwStyle, fMenu);
  if (fRc)
  {
    prc->left -= rc.left;
    prc->top -= rc.top;
    prc->right -= rc.right;
    prc->bottom -= rc.bottom;
  }
  return fRc;
}

bool IsTaskbarHidden(UINT edge, RECT monitorRect)
{
  APPBARDATA appbardata = {sizeof(APPBARDATA), 0, 0, edge, monitorRect};
  // Multi-monitor taskbar settings on Windows 8 and later.
  if (IsWindows8OrGreater())
    return SHAppBarMessage(ABM_GETAUTOHIDEBAREX, &appbardata);
  else
    return SHAppBarMessage(ABM_GETAUTOHIDEBAR, &appbardata);
}

MONITORINFO GetActualMonitorInfo(HMONITOR monitor)
{
  MONITORINFO monitorInfo = {sizeof(monitorInfo)};
  GetMonitorInfo(monitor, &monitorInfo);

  RECT monitorRect = monitorInfo.rcMonitor;
  RECT workRect = monitorInfo.rcWork;

  // If rect is the same as the full monitor size then check for a
  // hidden task bar and remove a pixel from the size,
  // otherwise the taskbar is not accessable because windows does not
  // account for its own hidden taskbar in the working desktop size.
  if (workRect.left == monitorRect.left && workRect.top == monitorRect.top && workRect.right == monitorRect.right &&
      workRect.bottom == monitorRect.bottom)
  {
    if (IsTaskbarHidden(ABE_BOTTOM, monitorRect))
      --workRect.bottom;
    else if (IsTaskbarHidden(ABE_LEFT, monitorRect))
      ++workRect.left;
    else if (IsTaskbarHidden(ABE_TOP, monitorRect))
      ++workRect.top;
    else if (IsTaskbarHidden(ABE_RIGHT, monitorRect))
      --workRect.right;
  }

  return monitorInfo;
}

Keys::Enum VKToKey(int vk)
{
  if ((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9'))
  {
    return (Keys::Enum)vk;
  }
  else
  {
    switch (vk)
    {

#define ProcessInput(VKValue, ZeroValue)                                                                               \
  case VKValue:                                                                                                        \
    return ZeroValue;
#include "Keys.inl"
#undef ProcessInput
    }
  }
  return Keys::Unknown;
}

int KeyToVK(Keys::Enum key)
{
  if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9'))
  {
    return (int)key;
  }
  else
  {
    switch (key)
    {
#define ProcessInput(VKValue, ZeroValue)                                                                               \
  case ZeroValue:                                                                                                      \
    return VKValue;
#include "Keys.inl"
#undef ProcessInput
    }
  }
  return VK_CANCEL;
}

MouseButtons::Enum VKToMouseButton(int vk)
{
  switch (vk)
  {
#define ProcessInput(VKValue, ZeroValue)                                                                               \
  case VKValue:                                                                                                        \
    return ZeroValue;
#include "MouseButtons.inl"
#undef ProcessInput
  }
  return MouseButtons::None;
}

int MouseButtonToVK(MouseButtons::Enum button)
{
  switch (button)
  {
#define ProcessInput(VKValue, ZeroValue)                                                                               \
  case ZeroValue:                                                                                                      \
    return VKValue;
#include "MouseButtons.inl"
#undef ProcessInput
  }
  return VK_CANCEL;
}

MouseButtons::Enum MouseButtonFromWParam(WPARAM wParam)
{
  if (HIWORD(wParam) == XBUTTON1)
    return MouseButtons::XOneBack;
  else
    return MouseButtons::XTwoForward;
}

// Get the nice name of the joystick from the windows registry
// using venderId productId
String GetJoystickName(uint venderId, uint productId)
{
  // Name is in registry in key
  // HKEY_CURRENT_USER\System\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VidPidName
  // VidPidName is in the format VID_045E&PID_028E
  String vidPidName = String::Format("VID_%04X&PID_%04X", venderId, productId);

  /// Open OEM Key from ...MediaProperties
  HKEY hRoot = HKEY_CURRENT_USER;
  String keyPath = String::Format("%s\\%s", REGSTR_PATH_JOYOEM, vidPidName.c_str());

  HKEY hKey;
  LONG result = RegOpenKeyEx(hRoot, Widen(keyPath).c_str(), 0, KEY_QUERY_VALUE, &hKey);

  if (result != ERROR_SUCCESS)
    return vidPidName;

  // Read OEM Name from the key
  char nameBuffer[256] = {0};
  DWORD length = sizeof(nameBuffer);

  result = RegQueryValueEx(hKey, REGSTR_VAL_JOYOEMNAME, 0, 0, (LPBYTE)nameBuffer, &length);
  RegCloseKey(hKey);

  if (result != ERROR_SUCCESS)
    return vidPidName;

  return nameBuffer;
}

#define MAX_BUTTONS 128

void ScanDevice(Array<PlatformInputDevice>& devices, HANDLE deviceHandle, RID_DEVICE_INFO& ridDeviceInfo)
{
  UINT deviceInfoSize = ridDeviceInfo.cbSize;
  ReturnIf(GetRawInputDeviceInfo(deviceHandle, RIDI_DEVICEINFO, &ridDeviceInfo, &deviceInfoSize) < 0,
           ,
           "Unable to read device information");

  // Only map Raw Input devices that are RIM_TYPEHID (not mice or keyboards)
  if (ridDeviceInfo.dwType == RIM_TYPEHID && ridDeviceInfo.hid.usUsagePage == UsbUsagePage::GenericDesktop)
  {
    // Get a nice name
    String deviceName = GetJoystickName(ridDeviceInfo.hid.dwVendorId, ridDeviceInfo.hid.dwProductId);

    UINT bufferSize = 0;
    ReturnIf(GetRawInputDeviceInfo(deviceHandle, RIDI_PREPARSEDDATA, NULL, &bufferSize) != 0,
             ,
             "Unable to get device info length");

    PHIDP_PREPARSED_DATA preparsedData = (PHIDP_PREPARSED_DATA)alloca(bufferSize);
    ZeroMemory(preparsedData, bufferSize);

    ReturnIf((int)GetRawInputDeviceInfo(deviceHandle, RIDI_PREPARSEDDATA, preparsedData, &bufferSize) < 0,
             ,
             "Unable to get device info");

    HIDP_CAPS caps;
    ReturnIf(HidP_GetCaps(preparsedData, &caps) != HIDP_STATUS_SUCCESS, , "Unable to get device capabilities");

    if (caps.NumberInputButtonCaps == 0)
      return;

    HIDP_BUTTON_CAPS* buttonCaps = (PHIDP_BUTTON_CAPS)alloca(sizeof(HIDP_BUTTON_CAPS) * caps.NumberInputButtonCaps);

    USHORT capsLength = caps.NumberInputButtonCaps;
    ReturnIf(HidP_GetButtonCaps(HidP_Input, buttonCaps, &capsLength, preparsedData) != HIDP_STATUS_SUCCESS,
             ,
             "Unable to get button capabilities");

    PlatformInputDevice& device = devices.PushBack();
    device.mName = deviceName;
    device.mDeviceHandle = deviceHandle;

    // Basically though we don't get a caps per button, we actually do have
    // 'button sets' and thus we can have more than one set of caps per button
    // set
    uint buttonIndex = 0;
    for (uint i = 0; i < caps.NumberInputButtonCaps; i++)
    {
      auto& buttonCap = buttonCaps[i];
      uint buttonCountInRange = buttonCap.Range.UsageMax - buttonCap.Range.UsageMin + 1;

      for (uint j = 0; j < buttonCountInRange; ++j)
      {
        PlatformButton& button = device.mButtons.PushBack();
        button.mName = String::Format("Button%d", buttonIndex);
        button.mOffset = buttonCap.Range.UsageMin;
        button.mBit = j;
        ++buttonIndex;
      }
    }

    // Value caps
    if (caps.NumberInputValueCaps != 0)
    {
      HIDP_VALUE_CAPS* valueCaps = (PHIDP_VALUE_CAPS)alloca(sizeof(HIDP_VALUE_CAPS) * caps.NumberInputValueCaps);
      capsLength = caps.NumberInputValueCaps;
      ReturnIf(HidP_GetValueCaps(HidP_Input, valueCaps, &capsLength, preparsedData) != HIDP_STATUS_SUCCESS,
               ,
               "Unable to get value capabilities");

      HashMap<uint, String>& usageNames = GetUsageNames();

      for (uint i = 0; i < caps.NumberInputValueCaps; i++)
      {
        auto& valueCap = valueCaps[i];

        PlatformAxis& axis = device.mAxes.PushBack();
        axis.mOffset = valueCap.Range.UsageMin;
        axis.mSize = valueCap.Range.UsageMax - valueCap.Range.UsageMin;
        axis.mMax = valueCap.LogicalMax + 1;

        if (valueCap.LogicalMax == -1)
        {
          if (valueCap.BitSize == 8)
          {
            axis.mMax = 0xFF;
          }
          else if (valueCap.BitSize == 16)
          {
            axis.mMax = 0xFFFF;
          }
          else if (valueCap.BitSize == 32)
          {
            axis.mMax = 0xFFFFFFFF;
          }
        }

        axis.mMin = valueCap.LogicalMin;
        axis.mName = usageNames.FindValue(valueCap.Range.UsageMin, "Unknown");
        axis.mCanBeDisabled = (valueCap.HasNull != 0);
      }
    }

    // Build a guid using the name hash, and vendor / product / version info
    Guid guid = deviceName.Hash();
    guid = guid ^ (u64)ridDeviceInfo.hid.dwVendorId * 4187;
    guid = guid ^ (u64)ridDeviceInfo.hid.dwProductId;
    guid = guid ^ (u64)ridDeviceInfo.hid.dwVersionNumber;
    device.mGuid = guid;

    const bool detailedDeviceInfo = false;

    // Print details
    if (detailedDeviceInfo)
    {
      ZPrint("Device Name: %s ", deviceName.c_str());
      ZPrint("Vendor Id: %d ", ridDeviceInfo.hid.dwVendorId);
      ZPrint("Product Id: %d ", ridDeviceInfo.hid.dwProductId);
      ZPrint("Version Number: %d ", ridDeviceInfo.hid.dwVersionNumber);
      ZPrint("Usage: %X ", ridDeviceInfo.hid.usUsage);
      ZPrint("Usage Page: %X ", ridDeviceInfo.hid.usUsagePage);
      ZPrint("\n");
    }
  }
}

void RawInputMessage(ShellWindow* window, WPARAM wParam, LPARAM lParam)
{
  // Get the raw input buffer size by passing NULL
  // for the buffer
  UINT bufferSize = 0;
  GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &bufferSize, sizeof(RAWINPUTHEADER));

  // Allocate a local buffer for the data
  RAWINPUT* rawInput = (RAWINPUT*)alloca(bufferSize);

  // Get the raw input data
  GetRawInputData((HRAWINPUT)lParam, RID_INPUT, rawInput, &bufferSize, sizeof(RAWINPUTHEADER));

  if (rawInput->header.dwType == RIM_TYPEMOUSE)
  {
    if (window->mOnRawMouseChanged)
    {
      int xMovement = rawInput->data.mouse.lLastX;
      int yMovement = rawInput->data.mouse.lLastY;
      window->mOnRawMouseChanged(IntVec2(xMovement, yMovement), window);
    }
  }
  else
  {
    if (window->mOnInputDeviceChanged)
    {
      HANDLE deviceHandle = rawInput->header.hDevice;

      forRange (PlatformInputDevice& inputDevice, window->mShell->mInputDevices)
      {
        if (inputDevice.mDeviceHandle != deviceHandle)
          continue;

        Array<uint> axes;
        uint buttons = 0;

        UINT bufferSize;
        if (GetRawInputDeviceInfo(deviceHandle, RIDI_PREPARSEDDATA, NULL, &bufferSize) == 0)
        {
          PHIDP_PREPARSED_DATA pPreparsedData = (PHIDP_PREPARSED_DATA)alloca(bufferSize);

          ReturnIf((int)GetRawInputDeviceInfo(deviceHandle, RIDI_PREPARSEDDATA, pPreparsedData, &bufferSize) < 0,
                   ,
                   "Unable to get device info");

          HIDP_CAPS Caps;
          ReturnIf(HidP_GetCaps(pPreparsedData, &Caps) != HIDP_STATUS_SUCCESS, , "Unable to get device capabilities");

          HIDP_BUTTON_CAPS* pButtonCaps =
              (PHIDP_BUTTON_CAPS)alloca(sizeof(HIDP_BUTTON_CAPS) * Caps.NumberInputButtonCaps);

          USHORT capsLength = Caps.NumberInputButtonCaps;
          ReturnIf(HidP_GetButtonCaps(HidP_Input, pButtonCaps, &capsLength, pPreparsedData) != HIDP_STATUS_SUCCESS,
                   ,
                   "Unable to get button capabilities");

          // Basically though we don't get a caps per button, we actually do
          // have 'button sets' and thus we can have more than one set of caps
          // per button set
          INT numberOfButtons = 0;
          for (uint i = 0; i < Caps.NumberInputButtonCaps; i++)
          {
            auto& buttonCaps = pButtonCaps[i];
            numberOfButtons += buttonCaps.Range.UsageMax - buttonCaps.Range.UsageMin + 1;
          }

          // Value caps
          HIDP_VALUE_CAPS* pValueCaps = (PHIDP_VALUE_CAPS)alloca(sizeof(HIDP_VALUE_CAPS) * Caps.NumberInputValueCaps);
          capsLength = Caps.NumberInputValueCaps;
          ReturnIf(HidP_GetValueCaps(HidP_Input, pValueCaps, &capsLength, pPreparsedData) != HIDP_STATUS_SUCCESS,
                   ,
                   "Unable to get value capabilities");

          ULONG usageLength = numberOfButtons;

          USAGE usage[MAX_BUTTONS];
          ReturnIf(HidP_GetUsages(HidP_Input,
                                  pButtonCaps->UsagePage,
                                  0,
                                  usage,
                                  &usageLength,
                                  pPreparsedData,
                                  (PCHAR)rawInput->data.hid.bRawData,
                                  rawInput->data.hid.dwSizeHid) != HIDP_STATUS_SUCCESS,
                   ,
                   "Unable to get input usages");

          BOOL bButtonStates[MAX_BUTTONS];
          ZeroMemory(bButtonStates, sizeof(bButtonStates));
          for (uint i = 0; i < usageLength; i++)
            bButtonStates[usage[i] - pButtonCaps->Range.UsageMin] = TRUE;

          for (int i = 0; i < numberOfButtons; ++i)
          {
            if (bButtonStates[i] == TRUE)
              buttons |= (1 << i);
          }

          for (uint i = 0; i < Caps.NumberInputValueCaps; i++)
          {
            if (i >= inputDevice.mAxes.Size())
            {
              Error("Got an axis index that was outside our mapped platform "
                    "indices (need to call QueryInputDevices again?)");
              break;
            }

            PlatformAxis& axis = inputDevice.mAxes[i];

            ULONG value = 0;

            ReturnIf(HidP_GetUsageValue(HidP_Input,
                                        pValueCaps[i].UsagePage,
                                        0,
                                        axis.mOffset,
                                        &value,
                                        pPreparsedData,
                                        (PCHAR)rawInput->data.hid.bRawData,
                                        rawInput->data.hid.dwSizeHid) != HIDP_STATUS_SUCCESS,
                     ,
                     "Unable to get input value");

            axes.PushBack(value);
            // ZPrint("Axis with usage %x is %d\n",
            // pValueCaps[i].Range.UsageMin, value);
          }
        }

        // Send the raw buffer to the joystick to be interpreted by the custom
        // mapping
        byte* bytes = (byte*)rawInput->data.hid.bRawData;

        window->mOnInputDeviceChanged(inputDevice, buttons, axes, DataBlock(bytes, bufferSize), window);
      }
    }
  }
}

const uint MessageHandled = 0;

bool WindowsOpenClipboard(UINT format)
{
  if (!OpenClipboard((HWND)0))
    return false;
  if (!IsClipboardFormatAvailable(format))
  {
    CloseClipboard();
    return false;
  }
  return true;
}

bool WindowsGetClipboardText(String* out)
{
  if (!WindowsOpenClipboard(CF_UNICODETEXT))
    return false;

  bool result = false;
  HGLOBAL globalHandle = GetClipboardData(CF_UNICODETEXT);
  if (globalHandle != nullptr)
  {
    LPWSTR clipboardCstr = (LPWSTR)GlobalLock(globalHandle);
    if (clipboardCstr != nullptr)
    {
      *out = Narrow(clipboardCstr);
      GlobalUnlock(globalHandle);
      result = true;
    }
  }
  CloseClipboard();
  return result;
}

void WindowsSetClipboardText(StringParam text)
{
  WString wideText = Widen(text);
  if (!OpenClipboard((HWND)0))
    return;
  EmptyClipboard();

  uint numCharacters = wideText.SizeInBytes();
  // Allocate a global memory object for the text.

  HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (numCharacters + 1) * sizeof(wchar_t));
  if (hglbCopy == NULL)
  {
    CloseClipboard();
    return;
  }

  // Lock the handle and copy the text to the buffer.
  wchar_t* data = (wchar_t*)GlobalLock(hglbCopy);
  memcpy(data, wideText.Data(), wideText.SizeInBytes());

  // Null terminate the buffer
  data[numCharacters] = (wchar_t)0;
  GlobalUnlock(hglbCopy);

  // Place the handle on the clipboard.
  SetClipboardData(CF_UNICODETEXT, hglbCopy);

  CloseClipboard();
}

bool WindowsGetClipboardImage(Image* image)
{
  if (!WindowsOpenClipboard(CF_BITMAP))
    return false;

  HBITMAP hbmScreen = (HBITMAP)GetClipboardData(CF_BITMAP);
  HDC hdcScreen = GetDC(NULL);
  ConvertImage(hdcScreen, hbmScreen, image);

  CloseClipboard();
  return true;
}

LRESULT CALLBACK ShellWindowWndProc(ShellWindow* window, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  Shell* shell = window->mShell;
  switch (msg)
  {
  case WM_CLOSE:
  {
    if (window->mOnClose)
      window->mOnClose(window);
    return MessageHandled;
  }

  // Sent when a window is being destroyed.
  case WM_DESTROY:
  {
    return MessageHandled;
  }

  case WM_SYSCHAR:
  {
    // This will stop the beep when pressing alt
    return MessageHandled;
  }

  // Window has been activated
  case WM_ACTIVATE:
  {
    DWORD activated = LOWORD(wParam);
    if (window->mOnFocusChanged)
      window->mOnFocusChanged(activated != 0, window);
    return MessageHandled;
  }

  // Device has been added so a controller may have been plugged in.
  case WM_DEVICECHANGE:
  {
    if (window->mOnDevicesChanged)
      window->mOnDevicesChanged(window);
    return MessageHandled;
  }

  // Raw input data has been sent
  case WM_INPUT:
  {
    RawInputMessage(window, wParam, lParam);
    return MessageHandled;
  }

  // Files have been dropped on this window
  case WM_DROPFILES:
  {
    HDROP fileDrop = (HDROP)wParam;

    if (window->mOnMouseDropFiles)
    {
      // Get the drop mouse position
      IntVec2 clientPosition;
      DragQueryPoint(fileDrop, (POINT*)&clientPosition);

      // Passing 0xFFFFFFFF will request the number of files.
      uint itemCount = DragQueryFile(fileDrop, 0xFFFFFFFF, 0, 0);

      Array<String> files;

      // Get all the file names
      wchar_t buffer[MAX_PATH + 1] = {0};
      for (uint i = 0; i < itemCount; ++i)
      {
        // Get the file at this index and add to the list
        DragQueryFile((HDROP)wParam, i, buffer, MAX_PATH);
        files.PushBack(Narrow(buffer).c_str());
      }

      window->mOnMouseDropFiles(clientPosition, files, window);
    }

    // Clean up the drag
    DragFinish(fileDrop);

    return MessageHandled;
  }

  case WM_TIMER:
  {
    if (window->mOnFrozenUpdate)
      window->mOnFrozenUpdate(window);

    return MessageHandled;
  }

  // Start resizing
  case WM_ENTERSIZEMOVE:
  {
    // We need to keep getting updated while we're moving. Note that this could
    // re-enter the WndProc.
    SetTimer(hwnd, 1, 0, nullptr);
    return MessageHandled;
  }

  // Stop resizing
  case WM_EXITSIZEMOVE:
  {
    KillTimer(hwnd, 1);

    if (window->mOnClientSizeChanged)
    {
      IntVec2 clientSize = window->GetClientSize();

      if (clientSize != window->mClientSize)
      {
        window->mOnClientSizeChanged(clientSize, window);
        window->mClientSize = clientSize;
      }
    }

    return MessageHandled;
  }

  case WM_SYSCOMMAND:
  {
    if (wParam == SC_MINIMIZE)
    {
      if (window->mOnMinimized)
        window->mOnMinimized(window);
    }
    else if (wParam == SC_RESTORE)
    {
      if (window->mOnRestored)
        window->mOnRestored(window);
    }
    break;
  }

  case WM_NCCALCSIZE:
    if (window->mStyle.IsSet(WindowStyleFlags::ClientOnly))
    {
      if (window->GetState() == WindowState::Maximized)
      {
        // Get window location.
        DefWindowProc(hwnd, WM_NCCALCSIZE, wParam, lParam);
        RECT* rect = (RECT*)lParam;

        // Get the nearest monitor.
        // Cannot call MonitorFromWindow because it returns the wrong monitor
        // from minimized state.
        HMONITOR appMonitor = MonitorFromRect(rect, MONITOR_DEFAULTTONEAREST);

        // Get the monitor information.
        MONITORINFO monitorInfo = GetActualMonitorInfo(appMonitor);
        RECT workRect = monitorInfo.rcWork;

        // Size window to exact monitor working area.
        *rect = monitorInfo.rcWork;
      }

      // Returning 0 from WM_NCCALCSIZE disables the window border rendering.
      return 0;
    }
    else
    {
      return DefWindowProc(hwnd, msg, wParam, lParam);
    }

  case WM_GETMINMAXINFO:
  {
    MINMAXINFO* minMaxInfo = (MINMAXINFO*)lParam;

    // Set min size
    minMaxInfo->ptMinTrackSize.x = window->mMinClientSize.x;
    minMaxInfo->ptMinTrackSize.y = window->mMinClientSize.y;

    if (window->mStyle.IsSet(WindowStyleFlags::ClientOnly))
    {
      // Get the monitor this window is closest to
      HMONITOR appMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

      // Get the monitor information
      MONITORINFO monitorInfo = GetActualMonitorInfo(appMonitor);
      RECT monitorRect = monitorInfo.rcMonitor;
      RECT rect = monitorInfo.rcWork;

      minMaxInfo->ptMaxPosition.x = rect.left - monitorRect.left;
      minMaxInfo->ptMaxPosition.y = rect.top - monitorRect.top;
      minMaxInfo->ptMaxSize.x = rect.right - rect.left;
      minMaxInfo->ptMaxSize.y = rect.bottom - rect.top;
    }

    return MessageHandled;
  }

  // User has typed text
  case WM_CHAR:
  {
    if (window->mOnTextTyped)
      window->mOnTextTyped(Rune(Utf16ToUtf8(wParam)), window);
    return MessageHandled;
  }

  case WM_KEYUP:
  case WM_SYSKEYUP:
  {
    if (window->mOnKeyUp)
    {
      Keys::Enum key = VKToKey(wParam);
      window->mOnKeyUp(key, wParam, window);
    }

    return MessageHandled;
  }

  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  {
    bool repeated = (lParam & (1 << 30)) != 0;

    Keys::Enum key = VKToKey(wParam);
    if (window->mOnKeyDown)
      window->mOnKeyDown(key, wParam, repeated, window);

    // Handle paste explicitly to be more like a browser platform
    if (shell->IsKeyDown(Keys::Control) && key == Keys::V && shell->mOnPaste)
    {
      ClipboardData data;
      data.mHasText = WindowsGetClipboardText(&data.mText);
      data.mHasImage = WindowsGetClipboardImage(&data.mImage);
      shell->mOnPaste(data, shell);
    }

    // Handle cut/copy explicitly to be more like a browser platform
    if (shell->IsKeyDown(Keys::Control) && (key == Keys::C || key == Keys::X) && shell->mOnCopy)
    {
      ClipboardData data;
      shell->mOnCopy(data, key == Keys::X, shell);
      ErrorIf(!data.mHasText && !data.mText.Empty(), "Clipboard Text was not empty, but HasText was not set");
      if (data.mHasText)
        WindowsSetClipboardText(data.mText.c_str());
      ErrorIf(data.mHasImage, "Copying image data not yet supported");
    }

    return MessageHandled;
  }

  // Mouse Button Up
  case WM_LBUTTONUP:
  {
    if (window->mOnMouseUp)
      window->mOnMouseUp(ClientPositionFromLParam(lParam), MouseButtons::Left, window);
    return MessageHandled;
  }
  case WM_RBUTTONUP:
  {
    if (window->mOnMouseUp)
      window->mOnMouseUp(ClientPositionFromLParam(lParam), MouseButtons::Right, window);
    return MessageHandled;
  }
  case WM_MBUTTONUP:
  {
    if (window->mOnMouseUp)
      window->mOnMouseUp(ClientPositionFromLParam(lParam), MouseButtons::Middle, window);
    return MessageHandled;
  }
  case WM_XBUTTONUP:
  {
    if (window->mOnMouseUp)
      window->mOnMouseUp(ClientPositionFromLParam(lParam), MouseButtonFromWParam(wParam), window);
    return MessageHandled;
  }

  // Mouse Button Down
  case WM_LBUTTONDOWN:
  {
    IntVec2 clientPosition = ClientPositionFromLParam(lParam);
    if (window->mOnMouseDown)
      window->mOnMouseDown(clientPosition, MouseButtons::Left, window);

    if (window->mOnHitTest)
    {
      WindowBorderArea::Enum result = window->mOnHitTest(clientPosition, window);
      if (result != WindowBorderArea::None)
        ManipulateWindow(window, result);
    }
    return MessageHandled;
  }
  case WM_RBUTTONDOWN:
  {
    if (window->mOnMouseDown)
      window->mOnMouseDown(ClientPositionFromLParam(lParam), MouseButtons::Right, window);
    return MessageHandled;
  }
  case WM_MBUTTONDOWN:
  {
    if (window->mOnMouseDown)
      window->mOnMouseDown(ClientPositionFromLParam(lParam), MouseButtons::Middle, window);
    return MessageHandled;
  }
  case WM_XBUTTONDOWN:
  {
    if (window->mOnMouseDown)
      window->mOnMouseDown(ClientPositionFromLParam(lParam), MouseButtonFromWParam(wParam), window);
    return MessageHandled;
  }

  // Mouse has moved on the window
  case WM_MOUSEMOVE:
  {
    IntVec2 clientPosition = ClientPositionFromLParam(lParam);

    // WM_MOUSEMOVE can be sent as a side effect of many other windows messages
    // and OS operations even if the mouse has not moved. Check against the
    // previous position and only process the event if the mouse has moved since
    // the last time this message was recieved
    if (window->mClientMousePosition == clientPosition)
      return MessageHandled;

    if (window->mOnMouseMove)
      window->mOnMouseMove(clientPosition, window);

    // Track the last position the mouse was at when this message was processed
    window->mClientMousePosition = clientPosition;

    return MessageHandled;
  }

  // Mouse Wheel (veritcal) changed
  case WM_MOUSEWHEEL:
  {
    // WM_MOUSEWHEEL message is not in client coordinates
    IntVec2 monitorPosition = MonitorPositionFromLParam(lParam);
    IntVec2 clientPosition = window->MonitorToClient(monitorPosition);

    float scrollAmount = GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;

    if (window->mOnMouseScrollY)
      window->mOnMouseScrollY(clientPosition, scrollAmount, window);

    return MessageHandled;
  }

  // Mouse Wheel (horizontal) changed
  case WM_MOUSEHWHEEL:
  {
    // WM_MOUSEHWHEEL message is not in client coordinates
    IntVec2 monitorPosition = MonitorPositionFromLParam(lParam);
    IntVec2 clientPosition = window->MonitorToClient(monitorPosition);

    float scrollAmount = GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;

    if (window->mOnMouseScrollX)
      window->mOnMouseScrollX(clientPosition, scrollAmount, window);

    return MessageHandled;
  }
  }

  // Let the default window procedure handle the message
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ShellWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (msg == WM_CREATE)
  {
    // Window is being created get the WindowsOsWindow from the lParam
    // this is passed in from CreateWindow
    ShellWindow* window = (ShellWindow*)((CREATESTRUCT*)(lParam))->lpCreateParams;

    // Set it on the user data section of the window
    SetWindowPointer(hwnd, window);

    window->mHandle = hwnd;
  }

  // Get the window associated with this hwnd from the user data section
  ShellWindow* window = (ShellWindow*)PointerFromWindow(hwnd);
  if (window != nullptr)
  {
    Shell* shell = window->mShell;
    ShellPrivateData* shellPrivateData = (ShellPrivateData*)shell->mPrivateData;

    if (msg == shellPrivateData->mTaskbarButtonCreated)
    {
      // Create the task bar button for loading progress
      DWORD dwMajor = LOBYTE(LOWORD(GetVersion()));
      DWORD dwMinor = HIBYTE(LOWORD(GetVersion()));
      if (dwMajor > 6 || (dwMajor == 6 && dwMinor > 0))
      {
        CoCreateInstance(
            CLSID_TaskbarList, nullptr, CLSCTX_ALL, __uuidof(ITaskbarList3), (LPVOID*)&shellPrivateData->mTaskbar);
      }
    }

    return ShellWindowWndProc(window, hwnd, msg, wParam, lParam);
  }
  else
  {
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
}

Shell::Shell() : mCursor(Cursor::Arrow), mMainWindow(nullptr), mUserData(nullptr), mOnCopy(nullptr), mOnPaste(nullptr)
{
  sInstance = this;
  DisableProcessWindowsGhosting();

  ZeroConstructPrivateData(ShellPrivateData);
  memset(self, 0, sizeof(*self));
  self->mTaskbarButtonCreated = RegisterWindowMessageW(L"TaskbarButtonCreated");

  HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(nullptr);

  // Register the main windows class used by main window
  WNDCLASSEX winClass;
  ZeroMemory(&winClass, sizeof(WNDCLASSEX));

  winClass.lpszClassName = cWindowClassName;
  winClass.cbSize = sizeof(WNDCLASSEX);
  winClass.style = CS_OWNDC;
  winClass.lpfnWndProc = ShellWndProc;
  winClass.hInstance = hInstance;
  winClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ZeroDefaultIcon));
  winClass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(ZeroDefaultIcon));
  winClass.hCursor = nullptr;
  winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  winClass.lpszMenuName = nullptr;
  winClass.cbClsExtra = sizeof(void*);
  winClass.cbWndExtra = sizeof(void*);
  ATOM registeredClass = RegisterClassEx(&winClass);
  ErrorIf(!registeredClass, "Unable to register window class");
}

Shell::~Shell()
{
  HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(nullptr);
  UnregisterClass(cWindowClassName, hInstance);

  while (!mWindows.Empty())
    delete mWindows.Front();

  ZeroDestructPrivateData(ShellPrivateData);
}

String Shell::GetOsName()
{
  static const String name("Windows");
  return name;
}

uint Shell::GetScrollLineCount()
{
  uint scrollLines = 0;
  SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scrollLines, 0);

  if (scrollLines == 0)
    scrollLines = 1;

  return scrollLines;
}

IntRect Shell::GetPrimaryMonitorRectangle()
{
  POINT ptZero = {0};
  HMONITOR hmonPrimary = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
  MONITORINFO monitorinfo = {0};
  monitorinfo.cbSize = sizeof(monitorinfo);
  GetMonitorInfo(hmonPrimary, &monitorinfo);
  return FromRECT(monitorinfo.rcWork);
}

IntVec2 Shell::GetPrimaryMonitorSize()
{
  return IntVec2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
}

ByteColor Shell::GetColorAtMouse()
{
  // Get the cursorPos
  POINT cursorPos;
  GetCursorPos(&cursorPos);

  HDC dc = GetDC(nullptr);
  COLORREF color = GetPixel(dc, cursorPos.x, cursorPos.y);
  ReleaseDC(nullptr, dc);

  // COLORREF is 0x00BBGGRR
  return ByteColorRGBA(GetRValue(color), GetGValue(color), GetBValue(color), 0xFF);
}

void Shell::SetMonitorCursorClip(const IntRect& monitorRectangle)
{
  RECT rect = ToRECT(monitorRectangle);
  ::ClipCursor(&rect);
}

void Shell::ClearMonitorCursorClip()
{
  ::ClipCursor(nullptr);
}

Cursor::Enum Shell::GetMouseCursor()
{
  return mCursor;
}

void Shell::SetMonitorCursorPosition(Math::IntVec2Param monitorPosition)
{
  SetCursorPos(monitorPosition.x, monitorPosition.y);
}

Math::IntVec2 Shell::GetMonitorCursorPosition()
{
  POINT point;
  ZeroMemory(&point, sizeof(point));
  GetCursorPos(&point);
  return Math::IntVec2(point.x, point.y);
}

bool Shell::IsKeyDown(Keys::Enum key)
{
  return GetKeyState(KeyToVK(key)) < 0;
}

bool Shell::IsMouseDown(MouseButtons::Enum button)
{
  return GetKeyState(MouseButtonToVK(button)) < 0;
}

void Shell::SetMouseCursor(Cursor::Enum cursor)
{
  ZeroGetPrivateData(ShellPrivateData);

  HCURSOR hCursor = nullptr;
  LPWSTR cursorName = nullptr;

  switch (cursor)
  {
  case Cursor::Arrow:
    cursorName = IDC_ARROW;
    break;
  case Cursor::Wait:
    cursorName = IDC_WAIT;
    break;
  case Cursor::Cross:
    cursorName = IDC_CROSS;
    break;
  case Cursor::SizeNWSE:
    cursorName = IDC_SIZENWSE;
    break;
  case Cursor::SizeNESW:
    cursorName = IDC_SIZENESW;
    break;
  case Cursor::SizeWE:
    cursorName = IDC_SIZEWE;
    break;
  case Cursor::SizeNS:
    cursorName = IDC_SIZENS;
    break;
  case Cursor::SizeAll:
    cursorName = IDC_SIZEALL;
    break;
  case Cursor::TextBeam:
    cursorName = IDC_IBEAM;
    break;
  case Cursor::Hand:
    cursorName = IDC_HAND;
    break;
  }
  hCursor = LoadCursor(nullptr, cursorName);

  SetCursor(hCursor);
  mCursor = cursor;
}

bool Shell::GetPrimaryMonitorImage(Image* image)
{
  return GetWindowImage(GetDesktopWindow(), image);
}

void Shell::OpenFile(FileDialogInfo& config)
{
  return FileDialog(config, true);
}

void Shell::SaveFile(FileDialogInfo& config)
{
  return FileDialog(config, false);
}

void Shell::ShowMessageBox(StringParam title, StringParam message)
{
  MessageBoxA(0, message.c_str(), title.c_str(), MB_OK);
}

void Shell::Update()
{
  // Run the windows message pump. Sometimes (like with window dragging)
  // the default windows message pump will block, however we would
  // like to continue rendering and processing. DirectX9 Graphics must be on the
  // same thread as the message pump so a multi threaded approach is complicated
  // and does not solve the problem. Instead a timer is set up allowing the
  // game to continue running while inside the DefaultWindowProc. But in that
  // case WindowsSystem should not reenter the pump.
  // This is controlled with the mState variable.
  MSG messageInfo;
  while (PeekMessage(&messageInfo, nullptr, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&messageInfo);
    DispatchMessage(&messageInfo);
  }
}

const Array<PlatformInputDevice>& Shell::ScanInputDevices()
{
  mInputDevices.Clear();

  // Get the Raw input device  list
  UINT deviceCount = 0;
  GetRawInputDeviceList(NULL, &deviceCount, sizeof(RAWINPUTDEVICELIST));
  PRAWINPUTDEVICELIST pRawInputDeviceList = (PRAWINPUTDEVICELIST)alloca(sizeof(RAWINPUTDEVICELIST) * deviceCount);
  GetRawInputDeviceList(pRawInputDeviceList, &deviceCount, sizeof(RAWINPUTDEVICELIST));

  // Iterate through all raw input devices
  RID_DEVICE_INFO ridDeviceInfo;
  memset(&ridDeviceInfo, 0, sizeof(ridDeviceInfo));
  ridDeviceInfo.cbSize = sizeof(RID_DEVICE_INFO);
  for (uint i = 0; i < deviceCount; i++)
  {
    HANDLE deviceHandle = pRawInputDeviceList[i].hDevice;
    ScanDevice(mInputDevices, deviceHandle, ridDeviceInfo);
  }

  HWND mainWindowHandle = 0;
  if (mMainWindow)
    mainWindowHandle = (HWND)mMainWindow->mHandle;

  /// Register RawInput that we accept Raw input
  /// from Joysticks and Gamepads (etc, not mice, others)

  RAWINPUTDEVICE devices[3];

  devices[0].usUsagePage = UsbUsagePage::GenericDesktop;
  devices[0].usUsage = UsbUsage::Mouse;
  devices[0].dwFlags = RIDEV_INPUTSINK;
  devices[0].hwndTarget = mainWindowHandle;

  devices[1].usUsagePage = UsbUsagePage::GenericDesktop;
  devices[1].usUsage = UsbUsage::Gamepad;
  devices[1].dwFlags = RIDEV_INPUTSINK;
  devices[1].hwndTarget = mainWindowHandle;

  devices[2].usUsagePage = UsbUsagePage::GenericDesktop;
  devices[2].usUsage = UsbUsage::Joystick;
  devices[2].dwFlags = RIDEV_INPUTSINK;
  devices[2].hwndTarget = mainWindowHandle;

  BOOL success = RegisterRawInputDevices(devices, 3, sizeof(RAWINPUTDEVICE));
  CheckWindowsErrorCode(success, "Failed to RegisterRawInputDevices.");

  return mInputDevices;
}

struct ShellWindowPrivateData
{
  WINDOWPLACEMENT mPlacement;
  WindowStyleFlags::Enum mRestoreStyle;
};

ShellWindow::ShellWindow(Shell* shell,
                         StringParam windowName,
                         Math::IntVec2Param clientSize,
                         Math::IntVec2Param monitorClientPos,
                         ShellWindow* parentWindow,
                         WindowStyleFlags::Enum flags,
                         WindowState::Enum state) :
    mShell(shell),
    mMinClientSize(IntVec2(10, 10)),
    mParent(parentWindow),
    mHandle(nullptr),
    mStyle(flags),
    mProgress(0),
    mClientSize(clientSize),
    mClientMousePosition(IntVec2(-1, -1)),
    mCapture(false),
    mUserData(nullptr),
    mOnClose(nullptr),
    mOnFocusChanged(nullptr),
    mOnMouseDropFiles(nullptr),
    mOnFrozenUpdate(nullptr),
    mOnClientSizeChanged(nullptr),
    mOnMinimized(nullptr),
    mOnRestored(nullptr),
    mOnTextTyped(nullptr),
    mOnKeyDown(nullptr),
    mOnKeyUp(nullptr),
    mOnMouseDown(nullptr),
    mOnMouseUp(nullptr),
    mOnMouseMove(nullptr),
    mOnMouseScrollY(nullptr),
    mOnMouseScrollX(nullptr),
    mOnDevicesChanged(nullptr),
    mOnRawMouseChanged(nullptr),
    mOnInputDeviceChanged(nullptr)
{
  ZeroConstructPrivateData(ShellWindowPrivateData);
  memset(self, 0, sizeof(*self));

  // Get the parent window
  HWND parentWindowHwnd = nullptr;

  if (parentWindow)
    parentWindowHwnd = (HWND)parentWindow->mHandle;

  // Get HINSTANCE
  HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(nullptr);

  // Translate the style flags
  DWORD style = Win32StyleFromWindowStyle(flags);
  DWORD exStyle = GetWin32ExStyle(flags);

  // Create the window
  HWND windowHandle = CreateWindowEx(exStyle,
                                     cWindowClassName,
                                     Widen(windowName).c_str(),
                                     style,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     parentWindowHwnd,
                                     nullptr,
                                     hInstance,
                                     (LPVOID)this);

  ReturnIf(windowHandle == nullptr, , "Failed to create application window");

  SetWindowPos(
      windowHandle, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

  // Set Handle
  mHandle = windowHandle;
  UpdateWindow(windowHandle);

  DragAcceptFiles(windowHandle, TRUE);

  // The client size must be set before the monitor position
  SetClientSize(clientSize);
  SetMonitorClientPosition(monitorClientPos);

  if (WindowStyleFlags::MainWindow & flags)
  {
    ErrorIf(shell->mMainWindow != nullptr, "Another main window already exists");
    shell->mMainWindow = this;
  }
  shell->mWindows.PushBack(this);
  SetState(state);
}

ShellWindow::~ShellWindow()
{
  Destroy();
  ZeroDestructPrivateData(ShellWindowPrivateData);
}

void ShellWindow::Destroy()
{
  if (!mHandle)
    return;

  if (mShell && mShell->mMainWindow == this)
    mShell->mMainWindow = nullptr;

  mShell->mWindows.EraseValue(this);

  // Remove pointer to window
  SetWindowPointer((HWND)mHandle, nullptr);

  DestroyWindow((HWND)mHandle);

  mHandle = nullptr;
}

IntRect ShellWindow::GetMonitorClientRectangle()
{
  RECT monitorClientRect = GetClientRectInMonitor((HWND)mHandle);
  return FromRECT(monitorClientRect);
}

void ShellWindow::SetMonitorClientRectangle(const IntRect& monitorRectangle)
{
  RECT rect = ToRECT(monitorRectangle);

  // Add the border if ClientOnly is not set since SetWindowPos expects the
  // border
  if (!mStyle.IsSet(WindowStyleFlags::ClientOnly))
    AdjustWindowRect(&rect, Win32StyleFromWindowStyle(mStyle.Field), FALSE);

  SetWindowPos((HWND)mHandle, 0, rect.left, rect.top, RectWidth(rect), RectHeight(rect), SWP_NOZORDER | SWP_NOCOPYBITS);

  mClientSize = monitorRectangle.Size();
}

IntRect ShellWindow::GetMonitorBorderedRectangle()
{
  RECT monitorBorderedRect;
  ::GetWindowRect((HWND)mHandle, &monitorBorderedRect);
  return FromRECT(monitorBorderedRect);
}

void ShellWindow::SetMonitorBorderedRectangle(const IntRect& monitorRectangle)
{
  RECT rect = ToRECT(monitorRectangle);
  SetWindowPos((HWND)mHandle, 0, rect.left, rect.top, RectWidth(rect), RectHeight(rect), SWP_NOZORDER | SWP_NOCOPYBITS);

  // Remove the border if ClientOnly is not set
  if (!mStyle.IsSet(WindowStyleFlags::ClientOnly))
    UnadjustWindowRect(&rect, Win32StyleFromWindowStyle(mStyle.Field), FALSE);

  mClientSize = IntVec2(rect.right - rect.left, rect.bottom - rect.top);
}

IntVec2 ShellWindow::GetMinClientSize()
{
  return mMinClientSize;
}

void ShellWindow::SetMinClientSize(Math::IntVec2Param minSize)
{
  mMinClientSize = minSize;
}

ShellWindow* ShellWindow::GetParent()
{
  return mParent;
}

IntVec2 ShellWindow::MonitorToClient(Math::IntVec2Param monitorPosition)
{
  IntVec2 clientPosition = monitorPosition;
  ::ScreenToClient((HWND)mHandle, (POINT*)&clientPosition);
  return clientPosition;
}

IntVec2 ShellWindow::MonitorToBordered(Math::IntVec2Param monitorPosition)
{
  return ClientToBordered(MonitorToClient(monitorPosition));
}

IntVec2 ShellWindow::ClientToMonitor(Math::IntVec2Param clientPosition)
{
  IntVec2 monitorPosition = clientPosition;
  ::ClientToScreen((HWND)mHandle, (POINT*)&monitorPosition);
  return monitorPosition;
}

IntVec2 ShellWindow::ClientToBordered(Math::IntVec2Param clientPosition)
{
  RECT rect = {clientPosition.x, clientPosition.y, 0, 0};

  if (!mStyle.IsSet(WindowStyleFlags::ClientOnly))
    UnadjustWindowRect(&rect, Win32StyleFromWindowStyle(mStyle.Field), FALSE);

  return IntVec2(rect.left, rect.top);
}

IntVec2 ShellWindow::BorderedToMonitor(Math::IntVec2Param borderedPosition)
{
  return ClientToMonitor(BorderedToClient(borderedPosition));
}

IntVec2 ShellWindow::BorderedToClient(Math::IntVec2Param borderedPosition)
{
  RECT rect = {borderedPosition.x, borderedPosition.y, 0, 0};

  if (!mStyle.IsSet(WindowStyleFlags::ClientOnly))
    AdjustWindowRect(&rect, Win32StyleFromWindowStyle(mStyle.Field), FALSE);

  return IntVec2(rect.left, rect.top);
}

WindowStyleFlags::Enum ShellWindow::GetStyle()
{
  return mStyle.Field;
}

void ShellWindow::SetStyle(WindowStyleFlags::Enum style)
{
  mStyle = style;
  DWORD win32 = Win32StyleFromWindowStyle(style);
  SetWindowLong((HWND)mHandle, GWL_STYLE, win32);
  SendMessage((HWND)mHandle, WM_SYSCOMMAND, SC_RESTORE, 0);

  // Force window to update
  SetWindowPos((HWND)mHandle,
               nullptr,
               0,
               0,
               0,
               0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
}

bool ShellWindow::GetVisible()
{
  return IsWindowVisible((HWND)mHandle) == TRUE;
}

void ShellWindow::SetVisible(bool visible)
{
  ShowWindow((HWND)mHandle, visible ? SW_SHOW : SW_HIDE);
}

String ShellWindow::GetTitle()
{
  static const size_t Size = 1024;
  wchar_t text[Size] = {0};
  GetWindowText((HWND)mHandle, text, Size);
  return Narrow(text);
}

void ShellWindow::SetTitle(StringParam title)
{
  SetWindowText((HWND)mHandle, Widen(title).c_str());
}

WindowState::Enum ShellWindow::GetState()
{
  WINDOWPLACEMENT placement = {sizeof(placement)};
  GetWindowPlacement((HWND)mHandle, &placement);
  if (placement.showCmd == SW_SHOWNORMAL)
  {
    HMONITOR monitor = MonitorFromWindow((HWND)mHandle, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo = {sizeof(monitorInfo)};
    GetMonitorInfo(monitor, &monitorInfo);
    RECT monitorRect = monitorInfo.rcMonitor;

    RECT rect = placement.rcNormalPosition;

    // Fullscreen is done in windowed mode, check if window size is the same as
    // the monitor.
    if (rect.left == monitorRect.left && rect.top == monitorRect.top && rect.right == monitorRect.right &&
        rect.bottom == monitorRect.bottom)
      return WindowState::Fullscreen;

    return WindowState::Windowed;
  }
  else if (placement.showCmd == SW_SHOWMINIMIZED)
    return WindowState::Minimized;
  else if (placement.showCmd == SW_SHOWMAXIMIZED)
    return WindowState::Maximized;
  else
    return WindowState::Windowed;
}

void ShellWindow::SetState(WindowState::Enum windowState)
{
  ZeroGetPrivateData(ShellWindowPrivateData);
  switch (windowState)
  {
  case WindowState::Minimized:
  {
    // Intel crashes if minimizing from our fullscreen state, so revert to
    // windowed first.
    if (gIntelGraphics && GetState() == WindowState::Fullscreen)
    {
      SetStyle(self->mRestoreStyle);
      SetWindowPlacement((HWND)mHandle, &self->mPlacement);
    }

    SendMessage((HWND)mHandle, WM_SYSCOMMAND, SC_MINIMIZE, 0);
    break;
  }

  case WindowState::Windowed:
  {
    // Switching back to windowed mode is the only way to leave fullscreen.
    // This is the only location that style and placement should need to be
    // reset (except intel bug above).
    if (GetState() == WindowState::Fullscreen)
    {
      SetStyle(self->mRestoreStyle);
      SetWindowPlacement((HWND)mHandle, &self->mPlacement);
    }

    SendMessage((HWND)mHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
    // Force window to update
    SetWindowPos((HWND)mHandle,
                 nullptr,
                 0,
                 0,
                 0,
                 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    break;
  }

  case WindowState::Maximized:
  {
    SendMessage((HWND)mHandle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    break;
  }

  case WindowState::Fullscreen:
  {
    // Make sure in windowed mode to save placement.
    SendMessage((HWND)mHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
    GetWindowPlacement((HWND)mHandle, &self->mPlacement);

    // Remove border and disable window's aero so it can't manipulate the window
    // without us knowing.
    self->mRestoreStyle = mStyle.Field;
    mStyle.SetFlag(WindowStyleFlags::ClientOnly);
    mStyle.ClearFlag(WindowStyleFlags::Resizable);
    mStyle.ClearFlag(WindowStyleFlags::TitleBar);
    SetStyle(mStyle.Field);

    HMONITOR monitor = MonitorFromWindow((HWND)mHandle, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo = {sizeof(monitorInfo)};
    GetMonitorInfo(monitor, &monitorInfo);
    RECT rect = monitorInfo.rcMonitor;

    SetMonitorClientPosition(IntVec2(rect.left, rect.top));
    SetClientSize(IntVec2(rect.right - rect.left, rect.bottom - rect.top));
    break;
  }

  case WindowState::Restore:
  {
    SendMessage((HWND)mHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
    break;
  }
  }
}

void ShellWindow::SetMouseCapture(bool capture)
{
  if (capture)
    ::SetCapture((HWND)mHandle);
  else
    ::ReleaseCapture();

  mCapture = capture;
}

bool ShellWindow::GetMouseCapture()
{
  return ::GetCapture() == mHandle;
}

void ShellWindow::TakeFocus()
{
  // JoshD: There's a lot of extra stuff required to make a window's window take
  // focus (required for the launcher). Talk to me if you need to change this.

  // Force the window to be un-minimized
  SetState(WindowState::Restore);

  // Sometimes windows won't actually take focus...just make it
  // happen...hopefully...
  for (size_t i = 0; i < 5; ++i)
  {
    SetForegroundWindow((HWND)mHandle);
    SetFocus((HWND)mHandle);
    BringWindowToTop((HWND)mHandle);
    SetActiveWindow((HWND)mHandle);
  }
}

bool ShellWindow::HasFocus()
{
  return GetActiveWindow() == mHandle;
}

bool ShellWindow::GetImage(Image* image)
{
  return GetWindowImage((HWND)mHandle, image);
}

void ShellWindow::Close()
{
  SendMessage((HWND)mHandle, WM_SYSCOMMAND, SC_CLOSE, 0);
}

float ShellWindow::GetProgress()
{
  return mProgress;
}

void ShellWindow::SetProgress(ProgressType::Enum progressType, float progress)
{
  mProgress = progress;

  ShellPrivateData* shellPrivateData = (ShellPrivateData*)mShell->mPrivateData;

  // If the task bar button has been created
  // Update the progress bar built into the task bar.
  ITaskbarList3* taskbar = shellPrivateData->mTaskbar;
  if (taskbar)
  {
    if (progressType == ProgressType::None)
    {
      // Disable progress
      taskbar->SetProgressState((HWND)mHandle, TBPF_NOPROGRESS);
    }
    else if (progressType == ProgressType::Indeterminate)
    {
      // Indeterminate progress (spinning animation)
      taskbar->SetProgressState((HWND)mHandle, TBPF_INDETERMINATE);
    }
    else
    {
      // Actual progress
      taskbar->SetProgressState((HWND)mHandle, TBPF_NORMAL);

      // Progress is a normalized float change it to a ULONGLONG for windows.
      const uint ProgressValueScale = 10000000;
      taskbar->SetProgressValue(
          (HWND)mHandle, (ULONGLONG)(progress * ProgressValueScale), (ULONGLONG)ProgressValueScale);
    }
  }
}

void ShellWindow::PlatformSpecificFixup()
{
  if (gIntelGraphics)
  {
    auto state = GetState();
    // Borderless window with Windows Aero does not work correctly on Intel.
    WindowStyleFlags::Enum style = mStyle.Field;
    style = (WindowStyleFlags::Enum)(style & ~WindowStyleFlags::ClientOnly);
    // SetStyle sets state to windowed to force the window to update, so reset state after.
    SetStyle(style);
    SetState(state);
  }
}

bool ShellWindow::HasOwnMinMaxExitButtons()
{
  return !mStyle.IsSet(WindowStyleFlags::ClientOnly);
}

} // namespace Zero
