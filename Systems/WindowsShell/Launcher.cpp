///////////////////////////////////////////////////////////////////////////////
///
/// \file Launcher.cpp
/// Launcher window.
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

LRESULT WinLaunchWindow::WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) 
  { 
  case WM_INITDIALOG:
    {
      ShowWindow(mWindowHandle, TRUE);
    return FALSE;
    }
  case WM_COMMAND: 
    switch (LOWORD(wParam)) 
    { 
    case IDOK: 
      this->ApplySettings();
      mBlockActive = false;
      DestroyWindow(mWindowHandle);
      return TRUE; 
    case IDC_ASPECT:
      if ( HIWORD(wParam) == CBN_SELCHANGE )
      {
        this->UpdateResolutions();
      }
      return TRUE;
    case IDCANCEL:
      exit(0);
      DestroyWindow(mWindowHandle);
      return TRUE; 
    } 
  } 
  return FALSE;
}

INT_PTR CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  WinLaunchWindow* window = (WinLaunchWindow*)PointerFromWindow(hwnd);
  if(window != NULL)
  {
    return window->WindowProcedure(hwnd, msg, wParam, lParam);
  }
  else
  {
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
}

INT_PTR CALLBACK EulaProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) 
  {
  case WM_INITDIALOG:
    {
      HWND editText = GetDlgItem(hwnd, IDC_EDITEULA);
      HWND parentHwnd = GetParent(hwnd);
      // Get the eula file's contents to display
      WinLaunchWindow* window = (WinLaunchWindow*)PointerFromWindow(parentHwnd);
      WString wideEula = Widen(window->mEulaText);
      SetWindowText(editText, wideEula.c_str());
      ShowWindow(hwnd, TRUE);
      return FALSE;
    }
  case WM_COMMAND: 
    switch (LOWORD(wParam)) 
    { 
    case IDOK:
    {
      HWND parentHwnd = GetParent(hwnd);
      // Write out that they accepted the eula
      WinLaunchWindow* window = (WinLaunchWindow*)PointerFromWindow(parentHwnd);
      String eulaFilePath = window->GetEulaAcceptedFilePath();
      WriteStringRangeToFile(eulaFilePath, "Accepted");
      EndDialog(hwnd, LOWORD(wParam));
      return TRUE;
    }
    case IDCANCEL:
      HWND parentHwnd = GetParent(hwnd);
      WinLaunchWindow* window = (WinLaunchWindow*)PointerFromWindow(parentHwnd);
      exit(0);
      return TRUE; 
    } 
  default:
    return FALSE;
  }
}

WinLaunchWindow::WinLaunchWindow(CogId config, CogId projectCog)
{
  mBlockActive = true;
  mConfig = config;
  mProjectCog = projectCog;

  MainConfig* mainConfig = mConfig.has(MainConfig);

  String eulaPath = FilePath::CombineWithExtension(mainConfig->DataDirectory, "ZeroLauncherEula", ".txt");
  mEulaText = ReadFileIntoString(eulaPath);

  HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
  mWindowHandle = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_CONFIGDIALOG), NULL, DialogProc);
  SetWindowPointer(mWindowHandle, this);

  // Check if the user has accepted this eula
  if (!FileExists(GetEulaAcceptedFilePath()))
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_EULA), mWindowHandle, EulaProc);

  // Skip popup if not using it
  //WindowLaunchSettings* windowLaunch = mProjectCog.has(WindowLaunchSettings);
  //if (windowLaunch != nullptr && windowLaunch->mUseLaunchOptionsPopup == false)
  if (true)
  {
    mBlockActive = false;
    return;
  }

  HBITMAP bannerBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BANNER));
  HICON hIcon = LoadIcon( GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MAIN) );
  SendMessage(mWindowHandle, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

  Array<String> mQuality;
  mQuality.PushBack("Low");
  mQuality.PushBack("Medium");
  mQuality.PushBack("High");

  Array<String> mAspect;
  mAspect.PushBack("Any");
  mAspect.PushBack("4x3");
  mAspect.PushBack("5x4");
  mAspect.PushBack("16x9");
  mAspect.PushBack("16x10");
  
  HWND banner = GetDlgItem(mWindowHandle, IDC_BANNER);

  uint returnValue = SendMessage(banner, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bannerBitmap);

  mFullscreenCheck = GetDlgItem(mWindowHandle, IDC_FULLSCREEN);
  SendMessage(mFullscreenCheck, BM_SETCHECK, BST_CHECKED, 0);

  mAspectBox = GetDlgItem(mWindowHandle, IDC_ASPECT);
  SetStrings(mAspectBox, mAspect.All());

  mQualityBox = GetDlgItem(mWindowHandle, IDC_QUALITY);
  SetStrings(mQualityBox, mQuality.All());

  mResolutionBox = GetDlgItem(mWindowHandle, IDC_RES);

  ReadSettings();

  ShowWindow(mWindowHandle, TRUE);
}

void WinLaunchWindow::ReadSettings()
{
  Resolution desktopRes = GetDesktopResolution();
  int aspectIndex = GetAspectIndex(desktopRes);

  // Set aspect ratio it will be used
  // as a filter for valid resolutions
  SendMessage(mAspectBox, CB_SETCURSEL, aspectIndex, 0);
  UpdateResolutions();
}

void WinLaunchWindow::ApplySettings()
{
  WindowLaunchSettings* launchSettings = mProjectCog.has(WindowLaunchSettings);
  if(launchSettings)
  {
    int index = SendMessage(mResolutionBox, CB_GETCURSEL, 0, 0);
    bool fullScreen = SendMessage(mFullscreenCheck, BM_GETSTATE, 0, 0) == BST_CHECKED;

    Resolution& res = Resolutions[index];

    // Apply settings to config
    launchSettings->mLaunchFullscreen = fullScreen;
    launchSettings->mWindowedResolution = IntVec2(res.Width, res.Height);

    // Prevent sleeping
    SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
  }
}

void WinLaunchWindow::UpdateResolutions()
{
  Resolution aspect = AspectAny;
  int aspectIndex = SendMessage(mAspectBox, CB_GETCURSEL, 0, 0);
  aspect = Aspects[aspectIndex];

  Enumerate(Resolutions, 0, aspect);
  SetStrings(mResolutionBox, Resolutions.All());

  int goodResolution = FindMinResolution(Resolutions, 1000, 1000);
  SendMessage(mResolutionBox, CB_SETCURSEL, goodResolution, 0);
}

void WinLaunchWindow::RunMessageLoop()
{
  MSG msg;
  while (mBlockActive && GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

String WinLaunchWindow::GetEulaAcceptedFilePath()
{
  MainConfig* mainConfig = mConfig.has(MainConfig);

  // Make a file for each unique eula (use the hash to identify)
  String eulaGuid = ToString(mEulaText.Hash());
  String eulaFileName = BuildString("EulaAccepted", eulaGuid);
  String path = FilePath::Combine(GetUserDocumentsDirectory(), "Zero");
  return FilePath::CombineWithExtension(path, eulaFileName, ".txt");
}

bool RunLauncher(CogId configId, CogId projectCogId)
{
  WinLaunchWindow* win = new WinLaunchWindow(configId, projectCogId);
  win->RunMessageLoop();
  delete win;
  return true;
}

}
