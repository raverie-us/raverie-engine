// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

void LoadResourcePackageRelative(StringParam baseDirectory,
                                 StringParam libraryName)
{
  String path = FilePath::Combine(baseDirectory, libraryName);
  String fileName = BuildString(libraryName, ".pack");
  String packageFile = FilePath::Combine(path, fileName);

  if (!FileExists(packageFile))
  {
    FatalEngineError("Failed to find needed content package. %s",
                     libraryName.c_str());
  }

  ResourcePackage* package = new ResourcePackage();
  package->Load(packageFile);
  package->Location = path;

  Status status;
  Z::gResources->LoadPackage(status, package);
  if (!status)
    DoNotifyError("Failed to load resource package.", status.Message);
}

void CreateGame(OsWindow* mainWindow, Cog* projectCog, StringParam projectFile)
{
  ZPrint("Loading in Game Mode.\n");

  String projectDirectory = FilePath::GetDirectoryPath(projectFile);
  LoadResourcePackageRelative(projectDirectory, "FragmentCore");
  LoadResourcePackageRelative(projectDirectory, "Loading");
  LoadResourcePackageRelative(projectDirectory, "ZeroCore");
  LoadResourcePackageRelative(projectDirectory, "UiWidget");
  LoadResourcePackageRelative(projectDirectory, "EditorUi");
  LoadResourcePackageRelative(projectDirectory, "Editor");

  if (SharedContent* sharedContent = projectCog->has(SharedContent))
  {
    forRange(ContentLibraryReference libraryRef,
             sharedContent->ExtraContentLibraries.All())
    {
      String libraryName = libraryRef.mContentLibraryName;
      LoadResourcePackageRelative(projectDirectory, libraryName);
    }
  }

  ProjectSettings* project = projectCog->has(ProjectSettings);

  // Set the store name based on the project name.
  ObjectStore::GetInstance()->SetStoreName(project->ProjectName);

  LoadResourcePackageRelative(projectDirectory, project->ProjectName);

  // Make sure scripts in the project are compiled
  ZilchManager::GetInstance()->TriggerCompileExternally();

  // Send after compiling since graphics uses this event to know to stop
  // displaying the splash/loading screen
  ObjectEvent event(projectCog);
  Z::gEngine->DispatchEvent(Events::ProjectLoaded, &event);

  ZPrint("Creating game...\n");

  IntVec2 intViewportSize = mainWindow->GetClientSize();
  Vec2 mainViewportSize =
      Pixels(float(intViewportSize.x), float(intViewportSize.y));

  RootWidget* rootWidget = new RootWidget(mainWindow);
  rootWidget->SetSize(mainViewportSize);

  // In game mode only one widget that has the viewport
  GameWidget* gameWidget = new GameWidget(rootWidget);
  gameWidget->TakeFocus();
  gameWidget->SetSize(mainViewportSize);

  // Create the GameSession
  GameSession* game = Z::gEngine->CreateGameSession();
  game->mMainWindow = mainWindow;
  game->SetInEditor(false);

  game->mGameWidget = gameWidget;
  gameWidget->SetGameSession(game);

  game->Start();

  // Run all parsed command-line arguments once after the game is fully loaded.
  CommandManager* commandManager = CommandManager::GetInstance();
  commandManager->RunParsedCommands();
}

} // namespace Zero
