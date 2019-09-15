// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
int ZeroStartup::sReturnCode = 0;

int ZeroStartup::Run()
{
  RunMainLoop(&ZeroStartup::MainLoopFunction, this);
  return sReturnCode;
}

void ZeroStartup::UserInitializeLibraries()
{
}

void ZeroStartup::UserInitializeConfig(Cog* configCog)
{
}

void ZeroStartup::UserInitialize()
{
}

void ZeroStartup::UserStartup()
{
}

void ZeroStartup::UserCreation()
{
}

void ZeroStartup::UserShutdownLibraries()
{
}

void ZeroStartup::UserShutdown()
{
}

void ZeroStartup::Exit(int returnCode)
{
  ZPrint("Exit %d\n", returnCode);
  mExit = true;
  sReturnCode = returnCode;
}

void ZeroStartup::MainLoop()
{
  switch (mPhase)
  {
  case StartupPhase::Initialize:
    Initialize();
    NextPhase();
    break;
  case StartupPhase::UserInitialize:
    UserInitialize();
    NextPhase();
    break;
  case StartupPhase::Startup:
    Startup();
    NextPhase();
    break;
  case StartupPhase::UserStartup:
    UserStartup();
    NextPhase();
    break;
  case StartupPhase::ProcessJobs:
    // Handles changing to the next phase internally.
    ProcessJobs();
    break;
  case StartupPhase::JobsComplete:
    JobsComplete();
    NextPhase();
    break;
  case StartupPhase::UserCreation:
    UserCreation();
    NextPhase();
    break;
  case StartupPhase::EngineUpdate:
    // Handles changing to the next phase internally.
    EngineUpdate();
    break;
  case StartupPhase::UserShutdown:
    UserShutdown();
    NextPhase();
    break;
  case StartupPhase::Shutdown:
    Shutdown();
    break;
  }

  if (mExit)
  {
    ZPrint("Stopping main loop");
    StopMainLoop();
    delete this;
  }
}

void ZeroStartup::MainLoopFunction(void* userData)
{
  ZeroStartup* self = (ZeroStartup*)userData;
  self->MainLoop();
}

void ZeroStartup::Initialize()
{
  TimerBlock startUp("Initialize");

  // Set the log and error handlers so debug printing and asserts will print to
  // the any debugger output (such as the Visual Studio Output Window).
  mDebuggerListener = new DebuggerListener();
  Zero::Console::Add(mDebuggerListener);

  mFileSystemInitializer = new FileSystemInitializer(&PopulateVirtualFileSystemWithZip);

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

  CrashHandler::RestartOnCrash(Environment::GetValue<bool>("autorestart", false));

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
  ZeroRegisterThreadSafeReferenceCountedHandleManager(ThreadSafeReferenceCounted);
  ZeroRegisterThreadSafeReferenceCountedHandleManager(GraphicsBlendSettings);
  ZeroRegisterThreadSafeReferenceCountedHandleManager(GraphicsDepthSettings);

  // Setup the core Zilch library
  mZilchSetup = new ZilchSetup(SetupFlags::DoNotShutdownMemory);

  // We need the calling state to be set so we can create Handles for Meta
  // Components
  Zilch::Module module;
  mState = module.Link();

#if !defined(ZeroDebug) && !defined(WelderTargetOsEmscripten)
  mState->SetTimeout(5);
#endif

  ExecutableState::CallingState = mState;

  MetaDatabase::Initialize();

  // Add the core library to the meta database
  MetaDatabase::GetInstance()->AddNativeLibrary(Core::GetInstance().GetLibrary());

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

  UserInitializeLibraries();

  LoadConfig(&InitializeConfigExternal, this);

  Tweakables::Load();

  Shortcuts::GetInstance()->Load(
      FilePath::Combine(Z::gEngine->GetConfigCog()->has(MainConfig)->DataDirectory, "Shortcuts.data"));

  // Load documentation for all native libraries
  DocumentationLibrary::GetInstance()->LoadDocumentation(
      FilePath::Combine(Z::gEngine->GetConfigCog()->has(MainConfig)->DataDirectory, "Documentation.data"));

  ZPrint("Os: %s\n", Os::GetVersionString().c_str());
}

void ZeroStartup::InitializeConfigExternal(Cog* configCog, void* userData)
{
  ((ZeroStartup*)userData)->UserInitializeConfig(configCog);
}

OsShell* CreateOsShellSystem();
System* CreateSoundSystem();
System* CreateGraphicsSystem();
System* CreatePhysicsSystem();
System* CreateTimeSystem();

void ZeroStartup::Startup()
{
  TimerBlock startUp("Startup");
  Engine* engine = Z::gEngine;
  Cog* configCog = engine->GetConfigCog();

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

  if (mLoadContent)
    LoadContentConfig();

  ZPrint("Creating main window.\n");

  OsShell* osShell = engine->has(OsShell);

  IntVec2 size = mWindowSize;
  if (mWindowSize == IntVec2::cZero)
  {
    size = osShell->GetPrimaryMonitorSize();
  }
  WindowState::Enum state = mWindowState;

  String name = BuildString(GetOrganization(), " ", GetApplicationName());

  if (mWindowSettingsFromProjectCog)
  {
    WindowLaunchSettings* windowLaunch = mWindowSettingsFromProjectCog->has(WindowLaunchSettings);
    if (windowLaunch != nullptr)
    {
      size = windowLaunch->mWindowedResolution;
      if (windowLaunch->mLaunchFullscreen)
        state = WindowState::Fullscreen;
    }

    ProjectSettings* projectSettings = mWindowSettingsFromProjectCog->has(ProjectSettings);
    if (projectSettings != nullptr)
    {
      name = projectSettings->ProjectName;
    }
  }

  // On Emscripten, the window full screen can only be done by a user
  // action. Setting it on startup causes an abrupt change the first time
  // the user click or hits a button.
#if !defined(WelderTargetOsEmscripten)
  if (state == WindowState::Fullscreen)
    state = WindowState::Maximized;
#endif

  IntVec2 minSize = Math::Min(mMinimumWindowSize, size);
  IntVec2 monitorClientPos = IntVec2(0, 0);

  if (mWindowCentered)
  {
    IntRect monitorRect = osShell->GetPrimaryMonitorRectangle();
    monitorClientPos = monitorRect.Center(size);
  }

  OsWindow* mainWindow = osShell->CreateOsWindow(name, size, monitorClientPos, nullptr, mWindowStyle, state);
  mainWindow->SetMinClientSize(minSize);

  // Pass window handle to initialize the graphics api
  auto graphics = engine->has(GraphicsEngine);
  graphics->CreateRenderer(mainWindow);

  if (mUseSplashScreen)
    graphics->SetSplashscreenLoading();

  // Fix any issues related to Intel drivers (we call SetState twice on purpose to fix the driver issues).
  mainWindow->PlatformSpecificFixup();

  // Used for trapping the mouse.
  Z::gMouse->mActiveWindow = mainWindow;

  // Note that content and resources are loaded after CreateRenderer so that they may use the Renderer API to upload
  // textures, meshes, etc.
  mMainWindow = mainWindow;
}

void ZeroStartup::ProcessJobs()
{
  Z::gJobs->RunJobsTimeSliced();

  if (Z::gJobs->AreAllJobsCompleted())
  {
    NextPhase();
  }
  else if (ThreadingEnabled)
  {
    // This should be a proper wait, not a spin wait with a sleep...
    Os::Sleep(10);
  }

  Z::gDispatch->DispatchEvents();
}

void ZeroStartup::JobsComplete()
{
  Z::gResources->SetupDefaults();
}

void ZeroStartup::EngineUpdate()
{
  Z::gEngine->Update();
  if (Z::gEngine->mEngineActive)
    return;

  NextPhase();
}

void ZeroStartup::Shutdown()
{
  Zero::TimerBlock block("Shutting down Libraries.");
  Z::gEngine->Shutdown();

  UserShutdownLibraries();

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

  ZPrint("Terminated\n");

  mExit = true;
}

void ZeroStartup::NextPhase()
{
  ZPrint("Completed phase: %s\n", StartupPhase::Names[mPhase]);
  mPhase = (StartupPhase::Enum)(mPhase + 1);
  ZPrint("Next phase: %s\n", StartupPhase::Names[mPhase]);
}

} // namespace Zero
