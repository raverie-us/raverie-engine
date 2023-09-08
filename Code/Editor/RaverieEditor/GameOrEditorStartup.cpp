// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Foundation/Platform/Stub/PlatformCommunication.hpp"

namespace Zero
{

void CreateEditor(OsWindow* mainWindow, StringParam projectFile, StringParam newProjectName);
void CreateGame(OsWindow* mainWindow, StringParam projectFile, Cog* projectCog);
void LoadGamePackages(StringParam projectFile, Cog* projectCog);

StartupPhase::Enum GameOrEditorStartup::RunIteration()
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
    // This will keep running until all startup jobs are completed.
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
  case StartupPhase::ImportsInitialize:
    ImportsInitialize();
    NextPhase();
    break;
  case StartupPhase::EngineUpdate:
    // Handles changing to the next phase internally.
    EngineUpdate();
    break;
  case StartupPhase::Shutdown:
    Shutdown();
    NextPhase();
    break;
  case StartupPhase::Terminate:
    // This phase does nothing, its only to signal that the application should terminate
    break;
  }

  return mPhase;
}

void GameOrEditorStartup::Initialize()
{
  CommandLineToStringArray();

  // Set the log and error handlers so debug printing and asserts will print to
  // the any debugger output (such as the debugger Output Window).
  mDebuggerListener = new DebuggerListener();
  Zero::Console::Add(mDebuggerListener);

  // Start the profiling system used to performance counters and timers.
  Profile::ProfileSystem::Initialize();
  mFileSystemInitializer = new FileSystemInitializer(&PopulateVirtualFileSystemWithZip);

  // Mirror console output to a log file.
  mFileListener = new FileListener();
  Zero::Console::Add(mFileListener);

  CrashHandler::Enable();

  Environment* environment = Environment::GetInstance();
  environment->ParseCommandArgs(gCommandLineArguments);

  ProfileScopeFunction();

  if (Environment::GetValue<bool>("BeginTracing", false))
    Profile::ProfileSystem::Instance->BeginTracing();

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

  LoadConfig(&InitializeConfigExternal, this);

  Tweakables::Load();

  Shortcuts::GetInstance()->Load(
      FilePath::Combine(Z::gEngine->GetConfigCog()->has(MainConfig)->DataDirectory, "Shortcuts.data"));

  // Load documentation for all native libraries
  DocumentationLibrary::GetInstance()->LoadDocumentation(
      FilePath::Combine(Z::gEngine->GetConfigCog()->has(MainConfig)->DataDirectory, "Documentation.data"));

  ZPrint("Os: %s\n", Os::GetVersionString().c_str());
}

void GameOrEditorStartup::InitializeConfigExternal(Cog* configCog, void* userData)
{
  ((GameOrEditorStartup*)userData)->UserInitializeConfig(configCog);
}

OsShell* CreateOsShellSystem();
System* CreateSoundSystem();
System* CreateGraphicsSystem();
System* CreatePhysicsSystem();
System* CreateTimeSystem();

void GameOrEditorStartup::Startup()
{
  ProfileScopeFunction();
  Engine* engine = Z::gEngine;
  Cog* configCog = engine->GetConfigCog();

  // Create all core systems
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

  IntVec2 minSize = Math::Min(mMinimumWindowSize, size);
  IntVec2 monitorClientPos = IntVec2(0, 0);

  if (mWindowCentered)
  {
    IntRect monitorRect = osShell->GetPrimaryMonitorRectangle();
    monitorClientPos = monitorRect.Center(size);
  }

  // We only ever create a single OsWindow
  OsWindow* mainWindow = new OsWindow(osShell, name, size, monitorClientPos, nullptr, mWindowStyle, state);

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

void GameOrEditorStartup::ProcessJobs()
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

void GameOrEditorStartup::JobsComplete()
{
  Z::gResources->SetupDefaults();
}

void GameOrEditorStartup::EngineUpdate()
{
  Z::gEngine->Update();
  if (Z::gEngine->mEngineActive)
    return;

  NextPhase();
}

void GameOrEditorStartup::Shutdown()
{
  {
    ProfileScopeFunction();
    Z::gEngine->Shutdown();

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
  }

  Profile::ProfileSystem::Shutdown();
}

void GameOrEditorStartup::NextPhase()
{
  ZPrint("Completed phase: %s\n", StartupPhase::Names[mPhase]);
  mPhase = (StartupPhase::Enum)(mPhase + 1);
  ZPrint("Next phase: %s\n", StartupPhase::Names[mPhase]);
}

void GameOrEditorStartup::UserInitializeConfig(Cog* configCog)
{
  HasOrAdd<EditorSettings>(configCog);
  HasOrAdd<ContentConfig>(configCog);
  HasOrAdd<TextEditorConfig>(configCog);
}

void GameOrEditorStartup::UserInitialize()
{
  String projectFile = Environment::GetValue<String>("file");
  bool playGame = Environment::GetValue<bool>("play", false);
  String newProject = Environment::GetValue<String>("newProject");

  // Check to see if there was a project file in the same directory.
  static const String cDefaultProjectFile("Project.zeroproj");
  if (FileExists(cDefaultProjectFile))
  {
    projectFile = cDefaultProjectFile;
    playGame = true;
  }

  // If there was no specified project file (or it doesn't exist) and we're not
  // creating a new project, then use a fall-back project that we open from our
  // data directory. This project should be read-only, but is useful for testing
  // platforms before the full pipeline is implemented. Note that if the
  // 'projectFile' does not exist, but is specified, we will not use the fall-back.
  Cog* configCog = Z::gEngine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);
  EditorConfig* editorConfig = configCog->has(EditorConfig);
  if (mainConfig && projectFile.Empty() && newProject.Empty() &&
      (editorConfig == nullptr || editorConfig->EditingProject.Empty()))
  {
    projectFile = FilePath::Combine(mainConfig->DataDirectory, "Fallback", "Fallback.zeroproj");
  }

  // The options defaults are already tailored to the Editor.
  // If we're playing the game, we need to load the project Cog.
  // We'll also potentially derive some window settings from the project.
  Cog* projectCog = nullptr;
  if (playGame)
  {
    projectCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), projectFile, 0, nullptr);
    if (projectCog == nullptr)
    {
      FatalEngineError("Failed load project '%s'", projectFile.c_str());
      return;
    }

    // Since we don't create a resiziable wigdet/close button, etc.
    // for the game, then we want the typical OS border to appear.
    mWindowStyle = (WindowStyleFlags::Enum)(mWindowStyle & ~WindowStyleFlags::ClientOnly);
  }

  mLoadContent = !playGame;
  mUseSplashScreen = playGame;

  mPlayGame = playGame;
  mProjectCog = projectCog;
  mProjectFile = projectFile;
  mNewProject = newProject;
}

void GameOrEditorStartup::UserStartup()
{
  if (mPlayGame)
  {
    LoadGamePackages(mProjectFile, mProjectCog);
  }
  else
  {
    Array<String> coreLibs;
    coreLibs.PushBack("ZeroCore");
    coreLibs.PushBack("UiWidget");
    coreLibs.PushBack("EditorUi");
    coreLibs.PushBack("Editor");
    LoadCoreContent(coreLibs);
  }

  String cloneUrl = Environment::GetValue<String>("cloneUrl");
  if (!cloneUrl.Empty() && !FileExists(mProjectFile))
  {
    GitCloneJob* job = new GitCloneJob();
    job->mUrl = cloneUrl;
    job->mDirectory = FilePath::GetDirectoryPath(mProjectFile);
    ZPrint("Cloning url '%s' to directory '%s'\n", job->mUrl.c_str(), job->mDirectory.c_str());
    Z::gJobs->AddJob(job);
  }
}

void GameOrEditorStartup::UserCreation()
{
  if (mPlayGame)
  {
    CreateGame(mMainWindow, mProjectFile, mProjectCog);
  }
  else
  {
    CreateEditor(mMainWindow, mProjectFile, mNewProject);
  }
}

void GameOrEditorStartup::ImportsInitialize() {
  gDeferImports = false;
  auto graphics = Z::gEngine->has(GraphicsEngine);
  graphics->InitializeRenderer();
}

} // namespace Zero
