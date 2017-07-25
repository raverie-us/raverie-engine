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

cwstr Eula = L"As a condition of your accessing this area, you agree to be bound by the following terms and conditions: "
  L"The games software was created by students of DigiPen Institute of Technology (DigiPen), and all copyright and other "
  L"rights in such is owned by DigiPen. While DigiPen allows you to access, download and use the software for non-commercial,"
  L" home use you hereby expressly agree that you will not otherwise copy, distribute, modify, or (to the extent not otherwise"
  L" permitted by law) decompile, disassemble or reverse engineer the games software. "
  L"  THE GAMES SOFTWARE IS MADE AVAILABLE BY DIGIPEN AS-IS AND WITHOUT WARRANTY OF ANY KIND BY DIGIPEN. DIGIPEN HEREBY "
  L"EXPRESSLY DISCLAIMS ANY SUCH WARRANTY, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS"
  L" FOR A PARTICULAR PURPOSE. "
  L"  WITHOUT LIMITING THE GENERALITY OF THE FOREGOING, DIGIPEN SHALL NOT BE LIABLE IN DAMAGES OR OTHERWISE FOR ANY DAMAGE"
  L" OR INJURY FROM DOWNLOADING, DECOMPRESSING, RUNNING OR ATTEMPTING TO RUN, USING OR OTHERWISE DEALING WITH, IN ANY WAY,"
  L" THE GAMES SOFTWARE CONTAINED IN THIS AREA, NOR SHALL DIGIPEN BE LIABLE FOR ANY INCIDENTAL, CONSEQUENTIAL, EXEMPLARY OR OTHER"
  L" TYPES OF DAMAGES ARISING FROM ACCESS TO OR USE OF THE GAMES SOFTWARE. "
  L"  YOU HEREBY AGREE TO INDEMNIFY, DEFEND AND HOLD HARMLESS DIGIPEN AND ITS DIRECTORS, OFFICERS, EMPLOYEES, "
  L"AGENTS, CONSULTANTS AND CONTRACTORS AGAINST ALL LIABILITY OF ANY KIND ARISING OUT OF YOUR DOWNLOADING, DECOMPRESSING, "
  L"RUNNING OR ATTEMPTING TO RUN, USING OR OTHERWISE DEALING WITH, IN ANY WAY, THE GAMES SOFTWARE. "
  L"  DIGIPEN MAKES NO WARRANTIES OR REPRESENTATIONS THAT THE GAMES SOFTWARE IS FREE OF MALICIOUS PROGRAMMING, "
  L"INCLUDING, WITHOUT LIMITATION, VIRUSES, TROJAN HORSE PROGRAMS, WORMS, MACROS AND THE LIKE. AS THE PARTY ACCESSING "
  L"THE GAMES SOFTWARE IT IS YOUR RESPONSIBILITY TO GUARD AGAINST AND DEAL WITH THE EFFECTS OF ANY SUCH MALICIOUS PROGRAMMING. ";

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
      SetWindowText(editText, Eula);
      ShowWindow(hwnd, TRUE);
      return FALSE;
    }
  case WM_COMMAND: 
    switch (LOWORD(wParam)) 
    { 
    case IDOK:
      EndDialog(hwnd, LOWORD(wParam));
      return TRUE; 
    case IDCANCEL:
      HWND parentHwnd = GetParent(hwnd);
      WinLaunchWindow* window = (WinLaunchWindow*)PointerFromWindow(parentHwnd);
      RemoveConfig(window->mConfig);
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

  HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
  mWindowHandle = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_CONFIGDIALOG), NULL, DialogProc);
  SetWindowPointer(mWindowHandle, this);

  // Show eula
  if (mainConfig->mConfigDidNotExist)
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

bool RunLauncher(CogId configId, CogId projectCogId)
{
  WinLaunchWindow* win = new WinLaunchWindow(configId, projectCogId);
  win->RunMessageLoop();
  delete win;
  return true;
}

}
