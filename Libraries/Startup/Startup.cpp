// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
OsShell* CreateOsShellSystem();
System* CreateSoundSystem();
System* CreateGraphicsSystem();
System* CreatePhysicsSystem();
System* CreateTimeSystem();

Engine* ZeroStartup::Initialize()
{
  TimerBlock startUp("Initialize");

  // Set the log and error handlers so debug printing and asserts will print to
  // the any debugger output (such as the Visual Studio Output Window).
  mDebuggerListener = new DebuggerListener();
  Zero::Console::Add(mDebuggerListener);

  mFileSystemInitializer =
      new FileSystemInitializer(&PopulateVirtualFileSystemWithZip);

  // Mirror console output to a log file.
  mFileListener = new FileListener();
  Zero::Console::Add(mFileListener);

  mTotalEngineTimer = new TimerBlock("Total run time:");

  CrashHandler::Enable();

  // CrashHandler::SetPreMemoryDumpCallback(Zero::CrashPreMemoryDumpCallback,
  //                                       NULL);
  // CrashHandler::SetCustomMemoryCallback(Zero::CrashCustomMemoryCallback,
  // NULL); CrashHandler::SetLoggingCallback(Zero::CrashLoggingCallback,
  // mFileListener);
  // CrashHandler::SetSendCrashReportCallback(Zero::SendCrashReport, NULL);
  // CrashHandler::SetCrashStartCallback(Zero::CrashStartCallback, NULL);

  Environment* environment = Environment::GetInstance();
  environment->ParseCommandArgs(gCommandLineArguments);

  // Add stdout listener (requires engine initialization to get the Environment
  // object)
  if (!environment->GetParsedArgument("logStdOut").Empty())
  {
    mStdoutListener = new StdOutListener();
    Zero::Console::Add(mStdoutListener);
  }

  CrashHandler::RestartOnCrash(
      Environment::GetValue<bool>("autorestart", false));

  CommonLibrary::Initialize();

  // Temporary location for registering handle managers
  // ZilchRegisterSharedHandleManager(ReferenceCountedHandleManager);
  ZilchRegisterSharedHandleManager(CogHandleManager);
  ZilchRegisterSharedHandleManager(ComponentHandleManager);
  ZilchRegisterSharedHandleManager(ResourceHandleManager);
  ZilchRegisterSharedHandleManager(WidgetHandleManager);
  ZilchRegisterSharedHandleManager(ContentItemHandleManager);

  RegisterCommonHandleManagers();

  ZeroRegisterHandleManager(ContentComposition);

  // Graphics specific
  ZeroRegisterThreadSafeReferenceCountedHandleManager(
      ThreadSafeReferenceCounted);
  ZeroRegisterThreadSafeReferenceCountedHandleManager(GraphicsBlendSettings);
  ZeroRegisterThreadSafeReferenceCountedHandleManager(GraphicsDepthSettings);

  // Setup the core Zilch library
  mZilchSetup = new ZilchSetup(SetupFlags::DoNotShutdownMemory);

  // We need the calling state to be set so we can create Handles for Meta
  // Components
  Zilch::Module module;
  mState = module.Link();

#if !defined(ZeroDebug) && !defined(PLATFORM_EMSCRIPTEN)
  mState->SetTimeout(5);
#endif

  ExecutableState::CallingState = mState;

  MetaDatabase::Initialize();

  // Add the core library to the meta database
  MetaDatabase::GetInstance()->AddNativeLibrary(
      Core::GetInstance().GetLibrary());

  // Initialize Zero Libraries
  PlatformLibrary::Initialize();
  GeometryLibrary::Initialize();
  // Geometry doesn't know about the Meta Library, so it cannot add itself to
  // the MetaDatabase
  MetaDatabase::GetInstance()->AddNativeLibrary(GeometryLibrary::GetLibrary());
  MetaLibrary::Initialize();
  SerializationLibrary::Initialize();
  ContentMetaLibrary::Initialize();
  SpatialPartitionLibrary::Initialize();

  EngineLibrary::Initialize();
  GraphicsLibrary::Initialize();
  PhysicsLibrary::Initialize();
  NetworkingLibrary::Initialize();
  SoundLibrary::Initialize();

  WidgetLibrary::Initialize();
  GameplayLibrary::Initialize();
  EditorLibrary::Initialize();
  UiWidgetLibrary::Initialize();

  ZilchScriptLibrary::Initialize();

  NativeBindingList::ValidateTypes();

  InitializeExternal();

  if (LoadConfig(&InitializeConfigExternal, this) == nullptr)
    return false;

  Tweakables::Load();

  Shortcuts::GetInstance()->Load(FilePath::Combine(
      Z::gEngine->GetConfigCog()->has(MainConfig)->DataDirectory,
      "Shortcuts.data"));

  // Load documentation for all native libraries
  DocumentationLibrary::GetInstance()->LoadDocumentation(FilePath::Combine(
      Z::gEngine->GetConfigCog()->has(MainConfig)->DataDirectory,
      "Documentation.data"));

  ZPrint("Os: %s\n", Os::GetVersionString().c_str());

  return Z::gEngine;
}

void ZeroStartup::Startup()
{
  TimerBlock startUp("Startup");
  Engine* engine = Z::gEngine;
  Cog* configCog = engine->GetConfigCog();

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
}

void ZeroStartup::InitializeExternal()
{
}

void ZeroStartup::InitializeConfig(Cog* configCog)
{
}

void ZeroStartup::ShutdownExternal()
{
}

void ZeroStartup::InitializeConfigExternal(Cog* configCog, void* userData)
{
  ((ZeroStartup*)userData)->InitializeConfig(configCog);
}

void ZeroStartup::Shutdown()
{
  Zero::TimerBlock block("Shutting down Libraries.");
  ShutdownExternal();

  Core::GetInstance().GetLibrary()->ClearComponents();

  // Shutdown in reverse order
  ZilchScriptLibrary::Shutdown();

  UiWidgetLibrary::Shutdown();
  EditorLibrary::Shutdown();
  GameplayLibrary::Shutdown();
  WidgetLibrary::Shutdown();

  SoundLibrary::Shutdown();
  NetworkingLibrary::Shutdown();
  PhysicsLibrary::Shutdown();
  GraphicsLibrary::Shutdown();
  EngineLibrary::Shutdown();

  SpatialPartitionLibrary::Shutdown();
  ContentMetaLibrary::Shutdown();
  SerializationLibrary::Shutdown();
  MetaLibrary::Shutdown();
  GeometryLibrary::Shutdown();
  PlatformLibrary::Shutdown();

  // ClearLibrary
  ZilchScriptLibrary::GetInstance().ClearLibrary();

  UiWidgetLibrary::GetInstance().ClearLibrary();
  EditorLibrary::GetInstance().ClearLibrary();
  GameplayLibrary::GetInstance().ClearLibrary();
  WidgetLibrary::GetInstance().ClearLibrary();

  SoundLibrary::GetInstance().ClearLibrary();
  NetworkingLibrary::GetInstance().ClearLibrary();
  PhysicsLibrary::GetInstance().ClearLibrary();
  GraphicsLibrary::GetInstance().ClearLibrary();
  EngineLibrary::GetInstance().ClearLibrary();

  SpatialPartitionLibrary::GetInstance().ClearLibrary();
  ContentMetaLibrary::GetInstance().ClearLibrary();
  SerializationLibrary::GetInstance().ClearLibrary();
  MetaLibrary::GetInstance().ClearLibrary();
  GeometryLibrary::GetInstance().ClearLibrary();

  // Destroy
  ZilchScriptLibrary::Destroy();

  UiWidgetLibrary::Destroy();
  EditorLibrary::Destroy();
  GameplayLibrary::Destroy();
  WidgetLibrary::Destroy();

  SoundLibrary::Destroy();
  NetworkingLibrary::Destroy();
  PhysicsLibrary::Destroy();
  GraphicsLibrary::Destroy();
  EngineLibrary::Destroy();

  SpatialPartitionLibrary::Destroy();
  ContentMetaLibrary::Destroy();
  SerializationLibrary::Destroy();
  MetaLibrary::Destroy();
  GeometryLibrary::Destroy();

  ZilchManager::Destroy();
  MetaDatabase::Destroy();

  delete mState;
  delete mZilchSetup;

  CommonLibrary::Shutdown();
}

} // namespace Zero
