///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Engine/OsShell.hpp"
#include "Engine/Configuration.hpp"
#include "Editor/Export.hpp"
#include "Support/StringMap.hpp"
#include "Widget/Command.hpp"
#include "Engine/SystemObjectManager.hpp"
#include "Platform/CrashHandler.hpp"
#include "Engine/Tweakables.hpp"
#include "Editor/BackgroundTask.hpp"
#include "../Win32Shared/Importer.hpp"
#include "Platform/FileSystem.hpp"
#include "Platform/FilePath.hpp"

namespace Zero
{

bool RunLauncher(CogId config, CogId projectCogId);

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

  String project = GetStringValue<String>(arguments, "file", String());
  bool playGame = GetStringValue<bool>(arguments, "play", false);
  bool defaultConfig = GetStringValue<bool>(arguments, "safe", false);
  bool noRunLauncher = GetStringValue<bool>(arguments, "nolauncher", false);
  bool autoRestart = GetStringValue<bool>(arguments, "autorestart", false);
  String newProject = GetStringValue<String>(arguments, "newProject", String());

  if(autoRestart)
    CrashHandler::RestartOnCrash(true);

  if(FileExists(projectFile))
    playGame = true;
  
  // Load config object
  Cog* configCog = Z::gEngine->GetConfigCog();

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

    //Initialize all systems.
    engine->Initialize(initializer);
  }

  // Initialize Extensions
  ZPrint("Loading Extensions\n");

  if(playGame)
  {
    // Creating project cog outside of CreateGame() so that launcher can make changes if needed.
    Cog* projectCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(),  projectFile, 0, nullptr);
    if(projectCog == nullptr)
    {
      FatalEngineError("Failed load project '%s'", projectFile.c_str());
      return false;
    }

    RunLauncher(configCog, projectCog);
    CreateGame(configCog, projectCog, projectFile);
  }
  else
  {
    CreateEditor(configCog, project, newProject);
  }

  // Check specifically for the WriteBuildInfo command needed by the build tools.
  // If it exists as a command-line argument then invoke it early. This has to be
  // done here because if there's no valid project when the editor runs then command
  // line arguments aren't processed.
  Environment* environment = Environment::GetInstance();
  if(environment->mParsedCommandLineArguments.ContainsKey("WriteBuildInfo"))
  {
    CommandManager* commandManager = CommandManager::GetInstance();
    Command* command = commandManager->GetCommand("WriteBuildInfo");
    if(command != nullptr)
      command->ExecuteCommand();
  }

  return true;
}

}//namespace Zero
