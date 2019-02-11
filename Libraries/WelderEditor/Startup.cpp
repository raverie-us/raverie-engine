// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

OsShell* CreateOsShellSystem();
System* CreateSoundSystem();
System* CreateGraphicsSystem();
System* CreatePhysicsSystem();
System* CreateTimeSystem();

void CreateEditor(Cog* config, StringParam project, StringParam newProjectName);
void CreateGame(Cog* config, Cog* projectCog, StringParam projectFile);

bool Startup(Engine* engine, StringMap& arguments, String projectFile)
{
  TimerBlock startUp("Engine Startup");

  bool playGame = GetStringValue<bool>(arguments, "play", false);
  bool defaultConfig = GetStringValue<bool>(arguments, "safe", false);
  bool noRunLauncher = GetStringValue<bool>(arguments, "nolauncher", false);
  bool autoRestart = GetStringValue<bool>(arguments, "autorestart", false);
  String newProject = GetStringValue<String>(arguments, "newProject", String());

  if (autoRestart)
    CrashHandler::RestartOnCrash(true);

  // Check to see if there was a project file in the same directory.
  if (FileExists(projectFile))
  {
    playGame = true;
  }
  else
  {
    // Otherwise, we could be trying to load a project (from the launcher)
    projectFile = GetStringValue<String>(arguments, "file", String());
  }

  // Load config object
  Cog* configCog = engine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);
  EditorConfig* editorConfig = configCog->has(EditorConfig);

  // If there was no specified project file (or it doesn't exist) and we're not
  // creating a new project, then use a fall-back project that we open from our
  // data directory. This project should be read-only, but is useful for testing
  // platforms before the full launcher pipeline is implemented. Note that if
  // the 'projectFile' does not exist, but is specified, we will not use the
  // fall-back.
  if (mainConfig && projectFile.Empty() && newProject.Empty() &&
      (editorConfig == nullptr || editorConfig->EditingProject.Empty()))
    projectFile = FilePath::Combine(
        mainConfig->DataDirectory, "Fallback", "Fallback.zeroproj");

  {
    TimerBlock block("Initializing core systems.");

    // Create all core systems
    engine->AddSystem(CreateUnitTestSystem());
    engine->AddSystem(CreateOsShellSystem());
    engine->AddSystem(CreateTimeSystem());
    engine->AddSystem(CreatePhysicsSystem());
    engine->AddSystem(CreateSoundSystem());
    engine->AddSystem(CreateGraphicsSystem());

    SystemInitializer initializer;
    initializer.mEngine = engine;
    initializer.Config = configCog;

    // Initialize all systems.
    engine->Initialize(initializer);
  }

  // Initialize Extensions
  ZPrint("Loading Extensions\n");

  if (playGame)
  {
    // Creating project cog outside of CreateGame() so that eula window can make
    // changes if needed. Note: We don't currently show a eula window so this
    // may need to be refactored.
    Cog* projectCog =
        Z::gFactory->Create(engine->GetEngineSpace(), projectFile, 0, nullptr);
    if (projectCog == nullptr)
    {
      FatalEngineError("Failed load project '%s'", projectFile.c_str());
      return false;
    }

    CreateGame(configCog, projectCog, projectFile);
  }
  else
  {
    CreateEditor(configCog, projectFile, newProject);
  }

  // Check specifically for the WriteBuildInfo command needed by the build
  // tools. If it exists as a command-line argument then invoke it early. This
  // has to be done here because if there's no valid project when the editor
  // runs then command line arguments aren't processed.
  Environment* environment = Environment::GetInstance();
  if (environment->mParsedCommandLineArguments.ContainsKey("WriteBuildInfo"))
  {
    CommandManager* commandManager = CommandManager::GetInstance();
    Command* command = commandManager->GetCommand("WriteBuildInfo");
    if (command != nullptr)
      command->ExecuteCommand();
  }

  return true;
}

} // namespace Zero
