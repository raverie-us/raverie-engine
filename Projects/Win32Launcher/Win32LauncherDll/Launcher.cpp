///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Global Access
namespace Z
{
  Launcher* gLauncher = nullptr;
}

void OnLauncherTweakablesModified()
{
  Tweakables::Save();
}

IntVec2 Launcher::mEulaWindowSize = IntVec2(457, 520);
IntVec2 Launcher::mLauncherWindowSize = IntVec2(1024, 595);

//--------------------------------------------------------------------- Launcher
//******************************************************************************
Launcher::Launcher(Cog* configCog, StringMap& arguments) : 
  mConfigCog(configCog),
  mArguments(arguments)
{
  mOsWindow = nullptr;
  mMainWindow = nullptr;
  mEulaWindow = nullptr;
  mLauncherWindow = nullptr;
}

//******************************************************************************
void Launcher::Startup()
{
  mMainWindow = new MainWindow(mOsWindow);
  mMainWindow->SetTitle("");

  // Don't pre-open the launcher unless the user has accepted the eula.
  // Technically the launcher runs logic on initialize even if it is inactive
  // that shouldn't happen unless the user has first accepted the eula.
  
  // If the newest Eula has been accepted, show the Eula window.
  // Otherwise, set the launcher window to visible as it has already been created
  bool showEula = ShouldOpenEula();
  if(showEula)
    OpenEulaWindow();
  else
    OpenLauncherWindow();

  if(Os::IsDebuggerAttached())
    OpenTweakablesWindow();
}

//******************************************************************************
void Launcher::EulaAccepted()
{
  // There's some delay/resizing issues now, so first open the launcher and then
  // resize after we destroy the eula window. This helps minimize
  // (although not remove) popping and sizes not updating.
  OpenLauncherWindow();

  // Store that we've accepted the eula
  TimeType eulaTime = GetEulaDateTime();
  UserConfig* userConfig = HasOrAdd<UserConfig>(mConfigCog);
  userConfig->LastAcceptedEula = (u64)eulaTime;
  SaveLauncherConfig(mConfigCog);
  mEulaWindow->SetActive(false);
  mEulaWindow->Destroy();
  mEulaWindow = nullptr;

  // Display the launcher
  mOsWindow->SetSize(mLauncherWindowSize);
  mOsWindow->SetMinSize(mLauncherWindowSize);

  // Re-center the window
  OsShell* osShell = Z::gEngine->has(OsShell);
  PixelRect desktopRect = osShell->GetDesktopRect();
  IntVec2 position = desktopRect.Center(mLauncherWindowSize);
  mOsWindow->SetPosition(position);
}

//******************************************************************************
void Launcher::OpenEulaWindow()
{
  mOsWindow->SetMinSize(mEulaWindowSize);
  mOsWindow->SetSize(mEulaWindowSize);
  
  EulaWindow* window = new EulaWindow(mConfigCog, mMainWindow);
  window->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);
  mEulaWindow = window;
}

void Launcher::Initialize()
{
  // Hack for BraveCobra
  Cog* configCog = mConfigCog;
  IntVec2 minWindowSize = mLauncherWindowSize;
  // Ideally we should change the window size here, but the loading progress bar looks wrong with
  // the window of this size. Fix later when we can override the loading.
  //if(ShouldOpenEula())
  //  minWindowSize = mEulaWindowSize;

  String windowName = "Zero Launcher";
  bool mainWindow = true;
  bool visible = false;

  OsShell* osShell = Z::gEngine->has(OsShell);
  PixelRect desktopRect = osShell->GetDesktopRect();

  IntVec2 size = minWindowSize;
  IntVec2 position = desktopRect.Center(size);

  BitField<WindowStyleFlags::Enum> mainStyle;
  mainStyle.U32Field = WindowStyleFlags::OnTaskBar | WindowStyleFlags::ClientOnly;

  if(mainWindow)
    mainStyle.SetFlag(WindowStyleFlags::MainWindow);

  if(!visible)
    mainStyle.SetFlag(WindowStyleFlags::NotVisible);

  OsWindow* window = osShell->CreateOsWindow(windowName, size, position, nullptr, mainStyle.Field);
  window->SetMinSize(minWindowSize);
  window->SetState(WindowState::Windowed);
  mOsWindow = window;

  Z::gEngine->has(GraphicsEngine)->CreateRenderer(window->GetWindowHandle());
}

//******************************************************************************
void Launcher::OpenLauncherWindow()
{
  mOsWindow->SetTitle("Zero Launcher");

  LauncherConfig* versionConfig = mConfigCog->has(LauncherConfig);
  
  LauncherWindow* launcher = new LauncherWindow(mMainWindow, mConfigCog);
  launcher->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);
  
  // If there were any command-line arguments then create an event to send to the launcher's ui
  if(mArguments.Empty() == false)
  {
    LauncherCommunicationEvent toSend;
    toSend.LoadFromCommandArguments(mArguments);
    if(toSend.EventId.Empty() == false)
      launcher->DispatchEvent(toSend.EventId, &toSend);
  }
  
  // If the user has no recent projects then display the new project's page
  RecentProjects* recentProjects = mConfigCog->has(RecentProjects);
  if(recentProjects == nullptr || recentProjects->GetRecentProjectsCount() == 0)
  {
    LauncherCommunicationEvent toSend;
    launcher->DispatchEvent(Events::LauncherNewProject, &toSend);
  }
  
  mLauncherWindow = launcher;
}

//******************************************************************************
void Launcher::OpenTweakablesWindow()
{
  return;
  OsShell* osShell = Z::gEngine->has(OsShell);
  PixelRect desktopRect = osShell->GetDesktopRect();
  IntVec2 size = IntVec2(230, desktopRect.SizeY);

  // Create a window to hold the tweakables (so they can be moved, closed, etc...)
  Window* tweakablesWindow = new Window(mMainWindow);
  tweakablesWindow->SetSize(Pixels(300, 500));
  PropertyView* propertyGrid = new PropertyView(tweakablesWindow);
  propertyGrid->SetObject(Z::gTweakables);
  propertyGrid->SetSize(Vec2(float(size.x), float(size.y)));

  // Set the tweakables modified callback so that we can update the Ui
  Tweakables::sModifiedCallback = &OnLauncherTweakablesModified;
}

//******************************************************************************
TimeType Launcher::GetEulaDateTime()
{
  String eulaFile = GetEulaFilePath(mConfigCog);
  return GetFileModifiedTime(eulaFile);
}

//******************************************************************************
bool Launcher::ShouldOpenEula()
{
  UserConfig* userConfig = HasOrAdd<UserConfig>(mConfigCog);
  TimeType eulaModifiedDate = GetEulaDateTime();

  return (userConfig->LastAcceptedEula < (u64)eulaModifiedDate);
}

//******************************************************************************
MainWindow* Launcher::CreateOsWindow(Cog* configCog, const IntVec2& minWindowSize,
                                     StringParam windowName, bool mainWindow,
                                     bool visible)
{
  OsShell* osShell = Z::gEngine->has(OsShell);
  PixelRect desktopRect = osShell->GetDesktopRect();

  IntVec2 size = minWindowSize;
  IntVec2 position = desktopRect.Center(size);

  BitField<WindowStyleFlags::Enum> mainStyle;
  mainStyle.SetFlag(WindowStyleFlags::OnTaskBar);
  mainStyle.SetFlag(WindowStyleFlags::Close);
  mainStyle.SetFlag(WindowStyleFlags::ClientOnly);
  mainStyle.SetFlag(WindowStyleFlags::Resizable);

  if(mainWindow)
    mainStyle.SetFlag(WindowStyleFlags::MainWindow);

  if(!visible)
    mainStyle.SetFlag(WindowStyleFlags::NotVisible);

  OsWindow* window = osShell->CreateOsWindow(windowName, size, position, nullptr, mainStyle.Field);
  window->SetMinSize(minWindowSize);

  MainWindow* rootWidget = new MainWindow(window);
  rootWidget->SetTitle("");
  window->SetTitle(windowName);

  return rootWidget;
}

}//namespace Zero
