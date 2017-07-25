///////////////////////////////////////////////////////////////////////////////
///
/// \file Launcher.hpp
/// Launcher window.
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// The launch window is used to configure the engine before graphics resources
/// are initialized. This prevents every prototype game from having to build Ui
/// for adjusting resolution.
class WinLaunchWindow
{
public:
  WinLaunchWindow(CogId config, CogId projectCog);
  bool mBlockActive;
  Array<Resolution> Resolutions;
  HWND mWindowHandle;
  CogId mConfig;
  CogId mProjectCog;
  HWND mResolutionBox;
  HWND mFullscreenCheck;
  HWND mAspectBox;
  HWND mQualityBox;
  LRESULT WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
  void RunMessageLoop();
  void UpdateResolutions();
  void ApplySettings();
  void ReadSettings();
};

///Run the launcher window. Launcher will block until config is complete.
bool RunLauncher(CogId configId, CogId projectCogId);

}
