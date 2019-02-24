// MIT Licensed (see LICENSE.md).
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

Launcher::Launcher(OsWindow* window)
{
  mOsWindow = window;
  mMainWindow = nullptr;
  mLauncherWindow = nullptr;

  Event event;
  Z::gEngine->DispatchEvent(Events::NoProjectLoaded, &event);

  mMainWindow = new MainWindow(mOsWindow);

  LauncherConfig* versionConfig =
      Z::gEngine->GetConfigCog()->has(LauncherConfig);

  LauncherWindow* launcher = new LauncherWindow(mMainWindow);
  launcher->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);

  // If there were any command-line arguments then create an event to send to
  // the launcher's ui
  auto& arguments = Environment::GetInstance()->mParsedCommandLineArguments;
  if (arguments.Empty() == false)
  {
    LauncherCommunicationEvent toSend;
    toSend.LoadFromCommandArguments(arguments);
    if (toSend.EventId.Empty() == false)
      launcher->DispatchEvent(toSend.EventId, &toSend);
  }

  // If the user has no recent projects then display the new project's page
  RecentProjects* recentProjects =
      Z::gEngine->GetConfigCog()->has(RecentProjects);
  if (recentProjects == nullptr ||
      recentProjects->GetRecentProjectsCount() == 0)
  {
    LauncherCommunicationEvent toSend;
    launcher->DispatchEvent(Events::LauncherNewProject, &toSend);
  }

  mLauncherWindow = launcher;

  if (Os::IsDebuggerAttached())
    OpenTweakablesWindow();

  CommandManager* commands = CommandManager::GetInstance();
  BindAppCommands(Z::gEngine->GetConfigCog(), commands);
  commands->RunParsedCommands();
}

void Launcher::OpenTweakablesWindow()
{
  return;
  OsShell* osShell = Z::gEngine->has(OsShell);
  IntRect monitorRect = osShell->GetPrimaryMonitorRectangle();
  IntVec2 size = IntVec2(230, monitorRect.SizeY);

  // Create a window to hold the tweakables (so they can be moved, closed,
  // etc...)
  Window* tweakablesWindow = new Window(mMainWindow);
  tweakablesWindow->SetSize(Pixels(300, 500));
  PropertyView* propertyGrid = new PropertyView(tweakablesWindow);
  propertyGrid->SetObject(Z::gTweakables);
  propertyGrid->SetSize(Vec2(float(size.x), float(size.y)));

  // Set the tweakables modified callback so that we can update the Ui
  Tweakables::sModifiedCallback = &OnLauncherTweakablesModified;
}

} // namespace Zero
