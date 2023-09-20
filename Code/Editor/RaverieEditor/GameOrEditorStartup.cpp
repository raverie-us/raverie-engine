// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Foundation/Platform/PlatformCommunication.hpp"

namespace Raverie
{

void CreateEditor(StringParam projectFile);
void CreateGame(StringParam projectFile, Cog* projectCog);
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

  mStdoutListener = new StdOutListener();
  Raverie::Console::Add(mStdoutListener);

  // Start the profiling system used to performance counters and timers.
  Profile::ProfileSystem::Initialize();
  mFileSystemInitializer = new FileSystemInitializer(&PopulateVirtualFileSystemWithZip);

  // Mirror console output to a log file.
  mFileListener = new FileListener();
  Raverie::Console::Add(mFileListener);

  Environment* environment = Environment::GetInstance();
  environment->ParseCommandArgs(gCommandLineArguments);

  ProfileScopeFunction();

  if (Environment::GetValue<bool>("BeginTracing", false))
    Profile::ProfileSystem::Instance->BeginTracing();

  CommonLibrary::Initialize();

  // Temporary location for registering handle managers
  // RaverieRegisterSharedHandleManager(ReferenceCountedHandleManager);
  RaverieRegisterSharedHandleManager(CogHandleManager);
  RaverieRegisterSharedHandleManager(ComponentHandleManager);
  RaverieRegisterSharedHandleManager(ResourceHandleManager);
  RaverieRegisterSharedHandleManager(WidgetHandleManager);
  RaverieRegisterSharedHandleManager(ContentItemHandleManager);

  RegisterCommonHandleManagers();

  RaverieRegisterHandleManager(ContentComposition);

  // Graphics specific
  RaverieRegisterThreadSafeReferenceCountedHandleManager(ThreadSafeReferenceCounted);
  RaverieRegisterThreadSafeReferenceCountedHandleManager(GraphicsBlendSettings);
  RaverieRegisterThreadSafeReferenceCountedHandleManager(GraphicsDepthSettings);

  // Setup the core Raverie library
  mRaverieSetup = new RaverieSetup(SetupFlags::DoNotShutdownMemory);

  // We need the calling state to be set so we can create Handles for Meta
  // Components
  Raverie::Module module;
  mState = module.Link();

  ExecutableState::CallingState = mState;

  MetaDatabase::Initialize();

  // Add the core library to the meta database
  MetaDatabase::GetInstance()->AddNativeLibrary(Core::GetInstance().GetLibrary());

  // Initialize Raverie Libraries
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

  RaverieScriptLibrary::Initialize();

  NativeBindingList::ValidateTypes();

  LoadConfig(&InitializeConfigExternal, this);

  Tweakables::Load();

  Shortcuts::GetInstance()->Load(FilePath::Combine(Z::gEngine->GetConfigCog()->has(MainConfig)->DataDirectory, "Shortcuts.data"));

  // Load documentation for all native libraries
  DocumentationLibrary::GetInstance()->LoadDocumentation(FilePath::Combine(Z::gEngine->GetConfigCog()->has(MainConfig)->DataDirectory, "Documentation.data"));
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

  String name = BuildString(GetOrganization(), " ", GetApplicationName());

  // We only ever create a single OsWindow
  OsWindow* mainWindow = new OsWindow();

  // Pass window handle to initialize the graphics api
  auto graphics = engine->has(GraphicsEngine);
  graphics->CreateRenderer(mainWindow);
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
    RaverieScriptLibrary::Shutdown();

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

    // ClearLibrary
    RaverieScriptLibrary::GetInstance().ClearLibrary();

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
    RaverieScriptLibrary::Destroy();

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

    RaverieManager::Destroy();
    MetaDatabase::Destroy();

    delete mState;
    delete mRaverieSetup;

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

void ExtractArchiveTo(ByteBufferBlock& buffer, StringParam basePath)
{
  Archive archive(ArchiveMode::Decompressing);
  archive.ReadBuffer(ArchiveReadFlags::All, buffer);
  forRange (ArchiveEntry& archiveEntry, archive.GetEntries())
  {
    String absolutePath = FilePath::Combine(basePath, archiveEntry.Name);
    AddVirtualFileSystemEntry(absolutePath, &archiveEntry.Full, archiveEntry.ModifiedTime);
  }
}

void GameOrEditorStartup::UserInitialize()
{
  // If there was no specified project file (or it doesn't exist) and we're not
  // creating a new project, then use a fall-back project that we open from our
  // data directory. This project should be read-only, but is useful for testing
  // platforms before the full pipeline is implemented. Note that if the
  // 'projectFile' does not exist, but is specified, we will not use the fall-back.
  Cog* configCog = Z::gEngine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);

  const String basePath = FilePath::Combine(GetUserDocumentsDirectory(), "Project");
  String projectFile;
  if (mProjectArchive.GetBegin())
  {
    ExtractArchiveTo(mProjectArchive, basePath);
    projectFile = FilePath::Combine(basePath, "Project.raverieproj");
  }
  else if (mBuiltContentArchive.GetBegin())
  {
    ExtractArchiveTo(mBuiltContentArchive, basePath);
    projectFile = FilePath::Combine(basePath, "Project.raverieproj");
    mPlayGame = true;
  }
  else
  {
    projectFile = FilePath::Combine(mainConfig->DataDirectory, "Fallback", "Project.raverieproj");
  }

  // The options defaults are already tailored to the Editor.
  // If we're playing the game, we need to load the project Cog.
  // We'll also potentially derive some window settings from the project.
  Cog* projectCog = nullptr;
  if (mPlayGame)
  {
    projectCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), projectFile, 0, nullptr);
    if (projectCog == nullptr)
    {
      FatalEngineError("Failed load project '%s'", projectFile.c_str());
      return;
    }
  }

  mLoadContent = !mPlayGame;

  mProjectCog = projectCog;
  mProjectFile = projectFile;
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
    coreLibs.PushBack("EngineCore");
    coreLibs.PushBack("UiWidget");
    coreLibs.PushBack("EditorUi");
    coreLibs.PushBack("Editor");
    LoadCoreContent(coreLibs);
  }
}

void GameOrEditorStartup::UserCreation()
{
  if (mPlayGame)
  {
    CreateGame(mProjectFile, mProjectCog);
  }
  else
  {
    CreateEditor(mProjectFile);
  }

  // Update once to force things like shader compositing so we can still show a loading screen
  Z::gEngine->Update();

  // The loading system assumes we are always loading from initialization to this point
  // rather than each time a content library begins and ends building (this batches everything)
  Shell::sInstance->mInitialLoadingComplete = true;
  Shell::sInstance->SetProgress(nullptr, 1.0f);
}

} // namespace Raverie
