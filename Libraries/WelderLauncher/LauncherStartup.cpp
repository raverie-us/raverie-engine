// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

OsShell* CreateOsShellSystem();
System* CreateSoundSystem();
System* CreateGraphicsSystem();
System* CreatePhysicsSystem();
System* CreateTimeSystem();

bool LoadContent(Cog* configCog)
{
  Z::gContentSystem->DefaultBuildStream = new TextStreamDebugPrint();
  Z::gContentSystem->EnumerateLibraries();

  ZPrint("Loading Editor Content...\n");
  EditorPackageLoader* loader = EditorPackageLoader::GetInstance();

  Timer timer;
  timer.Update();

  String docDirectory = GetUserDocumentsDirectory();

  LoadContentLibrary("FragmentCore", true);
  bool coreContent = LoadContentLibrary("Loading", true);

  Array<String> coreLibs;

  coreLibs.PushBack("ZeroCore");
  coreLibs.PushBack("ZeroLauncherResources");

  forRange(String libraryName, coreLibs.All())
  {
    coreContent = coreContent && LoadContentLibrary(libraryName, true);
  }

  if (!coreContent)
  {
    FatalEngineError("Failed to load core content library for editor. Resources"
                     " need to be in the working directory.");
    return false;
  }

  float time = (float)timer.UpdateAndGetTime();
  ZPrint("Finished Loading Editor Content in %.2f\n", time);
  return true;
}

bool LoadResources(Cog* configCog)
{
  LoadContentConfig(configCog);

  Z::gLauncher->Initialize();

  if (!LoadContent(configCog))
    return false;

  return true;
}

bool ZeroLauncherStartup(Engine* engine,
                         StringMap& arguments,
                         StringParam dllPath)
{
  TimerBlock startUp("Engine Startup");
  String applicationName = "ZeroVersionSelector";

  String project = GetStringValue<String>(arguments, "file", String());
  bool defaultConfig = GetStringValue<bool>(arguments, "safe", false);

  SaveConfig();
  Cog* configCog = Z::gEngine->GetConfigCog();

  // Profile initializing systems
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

  Tweakables::Load();

  Z::gLauncher = new Launcher();
  LoadResources(configCog);

  Event event;
  engine->DispatchEvent(Events::NoProjectLoaded, &event);

  // Start up the launcher
  Z::gLauncher->Startup();

  CommandManager* commands = CommandManager::GetInstance();
  BindAppCommands(configCog, commands);
  commands->RunParsedCommands();

  // Extra debug stuff to test crashing
  if (arguments.ContainsKey("CrashEngine") && configCog->has(DeveloperConfig))
  {
    CrashHandler::FatalError(1);
  }

  return true;
}

void LauncherStartup::InitializeExternal()
{
  LauncherDllLibrary::Initialize();
}

void LauncherStartup::InitializeConfig(Cog* configCog)
{
  // Force certain config components to exist. There's a few upgrade cases
  // where one of these could be missing otherwise.
  HasOrAdd<TextEditorConfig>(configCog);
  HasOrAdd<RecentProjects>(configCog);

  LauncherConfig* versionConfig = HasOrAdd<LauncherConfig>(configCog);
  versionConfig->mLauncherLocation = GetApplication();

  // Apply any command line arguments (mostly auto-run settings).
  versionConfig->ApplyCommandLineArguments();
}

void LauncherStartup::ShutdownExternal()
{
  LauncherDllLibrary::Shutdown();
  LauncherDllLibrary::GetInstance().ClearLibrary();
}

} // namespace Zero
