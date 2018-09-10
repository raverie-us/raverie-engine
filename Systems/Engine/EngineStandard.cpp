///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Ranges
ZilchDefineRange(HierarchyNameRange);
ZilchDefineRange(HierarchyListNameRange);
ZilchDefineRange(HierarchyRange);
ZilchDefineRange(CogNameRange);
ZilchDefineRange(CogRootNameRange);
ZilchDefineRange(HierarchyList::range);
ZilchDefineRange(HierarchyList::reverse_range);
ZilchDefineRange(Space::range);
ZilchDefineRange(SpaceMap::valueRange);
ZilchDefineRange(ObjectLinkRange);
ZilchDefineRange(JoystickRange);
ZilchDefineRange(CogHashSetRange);
ZilchDefineRange(ResourceTableEntryList::range);
ZilchDefineRange(OperationListRange);
ZilchDefineRange(Engine::GameSessionArray::range);

// Enums
ZilchDefineEnum(ActionExecuteMode);
ZilchDefineEnum(ActionState);
ZilchDefineEnum(AnimationBlendMode);
ZilchDefineEnum(AnimationBlendType);
ZilchDefineEnum(AnimationPlayMode);
ZilchDefineEnum(Buttons);
ZilchDefineEnum(CogPathPreference);
ZilchDefineEnum(Cursor);
ZilchDefineEnum(EaseType);
ZilchDefineEnum(FlickedStick);
ZilchDefineEnum(InputDevice);
ZilchDefineEnum(KeyState);
ZilchDefineEnum(LauncherAutoRunMode);
ZilchDefineEnum(Math::CurveType);
ZilchDefineEnum(MouseButtons);
ZilchDefineEnum(SplineType);
ZilchDefineEnum(StoreResult);
ZilchDefineEnum(StreamType);
ZilchDefineEnum(TabWidth);
ZilchDefineEnum(TimeMode);
ZilchDefineEnum(Verbosity);
ZilchDefineEnum(WindowState);
ZilchDefineEnum(WindowStyleFlags);

ZilchDefineExternalBaseType(Location::Enum, TypeCopyMode::ValueType, builder, type)
{
  ZilchFullBindEnum(builder, type, SpecialType::Enumeration);
  ZilchBindEnumValues(Location);
  
  // We need to alias ZilchSelf for the method binding macros
  namespace ZilchSelf = Location;

  ZilchBindMethod(IsCardinal);
  ZilchBindMethod(GetCardinalAxis);
  ZilchBindOverloadedMethod(GetDirection, ZilchStaticOverload(Vec2, Location::Enum));
  ZilchBindOverloadedMethod(GetDirection, ZilchStaticOverload(Vec2, Location::Enum, Location::Enum));
  ZilchBindMethod(GetOpposite);
}

// Arrays
ZeroDefineArrayType(Array<ContentLibraryReference>);

// The keys enum has to be declared special since it skips values
ZilchDefineExternalBaseType(Keys::Enum, TypeCopyMode::ValueType, builder, type)
{
  SetUpKeyNames();
  ZilchFullBindEnum(builder, type, SpecialType::Enumeration);

  // For now, just iterate over all keys in the name map and if there was no saved name then
  // assume that the key doesn't exist (linear but whatever)
  for(size_t i = 0; i < Keys::Size; ++i)
  {
    if(KeyNames[i] == nullptr)
      continue;

    ZilchFullBindEnumValue(builder, type, i, KeyNames[i]);
  }
}

//**************************************************************************************************
Cog* ZeroStartupSettings::LoadConfig()
{
  // Arguments
  Environment* environment = Environment::GetInstance();
  StringMap& arguments = environment->mParsedCommandLineArguments;

  String applicationName = "ZeroEditor";
  bool playGame = GetStringValue<bool>(arguments, "play", false);

  //Exported games use a different settings file
  if(mEmbeddedPackage || playGame)
    applicationName = "Zero";

  // If we're in safe mode, just use the default config
  bool defaultConfig = GetStringValue<bool>(arguments, "safe", false);

  return Zero::LoadConfig(applicationName, defaultConfig, *this);
}

//**************************************************************************************************
ZilchDefineStaticLibrary(EngineLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  ZilchInitializeRange(HierarchyNameRange);
  ZilchInitializeRange(HierarchyListNameRange);
  ZilchInitializeRange(HierarchyRange);
  ZilchInitializeRange(CogNameRange);
  ZilchInitializeRange(CogRootNameRange);
  ZilchInitializeRangeAs(HierarchyList::range, "HierarchyListRange");
  ZilchInitializeRangeAs(HierarchyList::reverse_range, "HierarchyListReverseRange");
  ZilchInitializeRangeAs(Space::range, "SpaceRange");
  ZilchInitializeRangeAs(SpaceMap::valueRange, "SpaceMapValueRange");
  ZilchInitializeRange(ObjectLinkRange);
  ZilchInitializeRange(JoystickRange);
  ZilchInitializeRange(CogHashSetRange);
  ZilchInitializeRangeAs(ResourceTableEntryList::range, "ResourceTableEntryRange");
  ZilchInitializeRange(OperationListRange);
  ZilchInitializeRangeAs(Engine::GameSessionArray::range, "GameSessionRange");

  // Enums
  ZilchInitializeEnum(ActionExecuteMode);
  ZilchInitializeEnum(ActionState);
  ZilchInitializeEnum(AnimationBlendMode);
  ZilchInitializeEnum(AnimationBlendType);
  ZilchInitializeEnum(AnimationPlayMode);
  ZilchInitializeEnum(Buttons);
  ZilchInitializeEnum(CogPathPreference);
  ZilchInitializeEnum(Cursor);
  ZilchInitializeEnumAs(EaseType, "Ease");
  ZilchInitializeEnum(FlickedStick);
  ZilchInitializeEnum(InputDevice);
  ZilchInitializeEnum(Keys);
  ZilchInitializeEnum(KeyState);
  ZilchInitializeEnum(LauncherAutoRunMode);
  ZilchInitializeEnum(Location);
  ZilchInitializeEnumAs(Math::CurveType, "CurveType");
  ZilchInitializeEnum(MouseButtons);
  ZilchInitializeEnum(SplineType);
  ZilchInitializeEnum(StoreResult);
  ZilchInitializeEnum(StreamType);
  ZilchInitializeEnum(TabWidth);
  ZilchInitializeEnum(TimeMode);
  ZilchInitializeEnum(Verbosity);
  ZilchInitializeEnum(WindowState);
  ZilchInitializeEnum(WindowStyleFlags);

  // Arrays
  ZeroInitializeArrayTypeAs(Array<ContentLibraryReference>, "ContentLibraryReferenceArray");

  ZilchInitializeType(System);

  // Meta Components
  ZilchInitializeType(TransformMetaTransform);
  ZilchInitializeType(CogMetaComposition);
  ZilchInitializeType(CogMetaDataInheritance);
  ZilchInitializeType(CogMetaDisplay);
  ZilchInitializeType(CogMetaSerialization);
  ZilchInitializeType(CogMetaOperations);
  ZilchInitializeType(CogMetaTransform);
  ZilchInitializeType(CogArchetypeExtension);
  ZilchInitializeType(CogSerializationFilter);
  ZilchInitializeType(CogPathMetaSerialization);
  ZilchInitializeType(ComponentMetaDataInheritance);
  ZilchInitializeType(DataResourceInheritance);
  ZilchInitializeType(ResourceMetaSerialization);
  ZilchInitializeType(EngineMetaComposition);
  ZilchInitializeType(HierarchyComposition);
  ZilchInitializeType(MetaResource);
  ZilchInitializeType(ComponentMetaOperations);
  ZilchInitializeType(ResourceMetaOperations);
  ZilchInitializeType(CogArchetypePropertyFilter);
  ZilchInitializeType(CogPathMetaComposition);
  ZilchInitializeType(MetaEditorScriptObject);
  ZilchInitializeType(MetaDependency);
  ZilchInitializeType(MetaInterface);
  ZilchInitializeType(RaycasterMetaComposition);

  // Events
  ZilchInitializeType(CogPathEvent);
  ZilchInitializeType(UpdateEvent);
  ZilchInitializeType(ResourceEvent);
  ZilchInitializeType(InputDeviceEvent);
  ZilchInitializeType(GameEvent);
  ZilchInitializeType(AnimationGraphEvent);
  ZilchInitializeType(KeyboardEvent);
  ZilchInitializeType(FileEditEvent);
  ZilchInitializeType(KeyboardTextEvent);
  ZilchInitializeType(OsMouseEvent);
  ZilchInitializeType(HierarchyEvent);
  ZilchInitializeType(JoystickEvent);
  ZilchInitializeType(CogInitializerEvent);
  ZilchInitializeType(ObjectLinkEvent);
  ZilchInitializeType(ObjectLinkPointChangedEvent);
  ZilchInitializeType(HeightMapEvent);
  ZilchInitializeType(AreaEvent);
  ZilchInitializeType(GamepadEvent);
  ZilchInitializeType(OperationQueueEvent);
  ZilchInitializeType(OsWindowEvent);
  ZilchInitializeType(OsMouseDropEvent);
  ZilchInitializeType(SavingEvent);
  ZilchInitializeType(DebugEngineEvent);
  ZilchInitializeType(DataEvent);
  ZilchInitializeType(DataReplaceEvent);
  ZilchInitializeType(CogReplaceEvent);
  ZilchInitializeType(TextEvent);
  ZilchInitializeType(TextErrorEvent);
  ZilchInitializeType(ProgressEvent);
  ZilchInitializeType(OsFileSelection);
  ZilchInitializeType(ZilchCompiledEvent);
  ZilchInitializeType(ZilchCompileFragmentEvent);
  ZilchInitializeType(ZilchCompileEvent);
  ZilchInitializeType(SplineEvent);
  ZilchInitializeType(BlockingTaskEvent);
  ZilchInitializeType(AsyncProcessEvent);

  // Components
  ZilchInitializeType(Component);
  ZilchInitializeType(Transform);
  ZilchInitializeType(Hierarchy);
  ZilchInitializeType(TimeSpace);
  ZilchInitializeType(ObjectLink);
  ZilchInitializeType(ObjectLinkAnchor);
  ZilchInitializeType(Hierarchy);
  ZilchInitializeType(AnimationGraph);
  ZilchInitializeType(SimpleAnimation);
  ZilchInitializeType(HeightMap);
  ZilchInitializeType(Area);
  ZilchInitializeType(ActionSpace);
  ZilchInitializeType(AnimationGraph);
  ZilchInitializeType(ProjectSettings);
  ZilchInitializeType(ProjectDescription);

  ZilchInitializeType(Cog);
  ZilchInitializeType(Space);
  ZilchInitializeType(ResourceDisplayFunctions);
  ZilchInitializeType(Resource);
  ZilchInitializeType(DataResource);
  ZilchInitializeType(ResourceSystem);
  ZilchInitializeType(ResourcePackageDisplay);
  ZilchInitializeType(ResourcePackage);
  ZilchInitializeType(ResourceLibrary);

  ZilchInitializeType(CogPath);

  ZilchInitializeType(Engine);
  ZilchInitializeType(GameSession);

  ZilchInitializeType(AnimationNode);
  ZilchInitializeType(PoseNode);
  ZilchInitializeType(BasicAnimation);
  ZilchInitializeType(DualBlend<DirectBlend>);
  ZilchInitializeType(DirectBlend);
  ZilchInitializeType(DualBlend<CrossBlend>);
  ZilchInitializeType(CrossBlend);
  ZilchInitializeType(DualBlend<SelectiveNode>);
  ZilchInitializeType(SelectiveNode);
  ZilchInitializeType(DualBlend<ChainNode>);
  ZilchInitializeType(ChainNode);
  ZilchInitializeType(ObjectTrack);
  ZilchInitializeType(Animation);
  ZilchInitializeType(Environment);

  ZilchInitializeType(Keyboard);

  ZilchInitializeType(System);
  ZilchInitializeType(TimeSystem);
  ZilchInitializeType(OsShell);
  ZilchInitializeType(OsWindow);
  ZilchInitializeType(Mouse);
  ZilchInitializeType(Factory);
  ZilchInitializeType(Operation);
  ZilchInitializeType(OperationQueue);
  ZilchInitializeType(OperationBatch);
  ZilchInitializeType(PropertyOperation);
  ZilchInitializeType(Tracker);
  ZilchInitializeType(Spline);
  ZilchInitializeType(SplineSampleData);
  ZilchInitializeType(SplineBakedPoints);
  ZilchInitializeType(SplineBakedPoint);
  ZilchInitializeType(SplineControlPoints);
  ZilchInitializeType(SplineControlPoint);
  ZilchInitializeType(AsyncProcess);

  ZilchInitializeType(Action);
  ZilchInitializeType(ActionSet);
  ZilchInitializeType(Actions);
  ZilchInitializeType(ActionGroup);
  ZilchInitializeType(ActionSequence);
  ZilchInitializeType(ActionSpace);
  ZilchInitializeType(ActionSystem);
  ZilchInitializeType(ActionDelay);

  ZilchInitializeType(CogInitializer);

  ZilchInitializeType(Thickness);
  ZilchInitializeType(Rectangle);

  ZilchInitializeType(LinkId);
  ZilchInitializeType(Named);
  ZilchInitializeType(EditorFlags);
  ZilchInitializeType(SpaceObjects);
  ZilchInitializeType(Archetype);
  ZilchInitializeType(Archetyped);
  ZilchInitializeType(ArchetypeInstance);

  ZilchInitializeType(MetaSelection);

  ZilchInitializeType(ObjectLinkEdge);
  ZilchInitializeType(ObjectLinkAnchor);
  ZilchInitializeType(ObjectLink);

  ZilchInitializeType(Level);

  ZilchInitializeType(DebugDraw);

  ZilchInitializeType(DocumentResource);
  ZilchInitializeType(TextBlock);

  ZilchInitializeType(MainConfig);
  ZilchInitializeType(EditorConfig);
  ZilchInitializeType(WindowLaunchSettings);
  ZilchInitializeType(FrameRateSettings);
  ZilchInitializeType(DebugSettings);
  ZilchInitializeType(ContentConfig);
  ZilchInitializeType(UserConfig);
  ZilchInitializeType(DeveloperConfig);
  ZilchInitializeType(ZilchPluginConfig);
  ZilchInitializeType(TextEditorConfig);
  ZilchInitializeType(RecentProjects);
  ZilchInitializeType(BuildInfo);
  ZilchInitializeType(EditorSettings);
  ZilchInitializeType(LauncherConfig);
  ZilchInitializeType(LauncherLegacySettings);

  ZilchInitializeType(HierarchySpline);

  ZilchInitializeType(ObjectStore);
  ZilchInitializeType(ResourceTable);
  ZilchInitializeType(ResourceTableEntry);

  ZilchInitializeType(SampleCurve);

  ZilchInitializeType(HeightMap);
  ZilchInitializeType(HeightPatch);
  ZilchInitializeType(HeightMapSource);

  ZilchInitializeType(SceneGraphSource);

  ZilchInitializeType(ColorGradient);

  ZilchInitializeType(Area);

  ZilchInitializeType(ProjectSettings);
  ZilchInitializeType(ContentLibraryReference);
  ZilchInitializeType(SharedContent);
  ZilchInitializeType(ProjectDescription);

  ZilchInitializeType(RaycastProvider);
  ZilchInitializeType(Raycaster);

  ZilchInitializeType(Gamepad);
  ZilchInitializeType(Gamepads);

  ZilchInitializeType(Tweakables);

  ZilchInitializeType(RawControlMapping);
  ZilchInitializeType(Joystick);
  ZilchInitializeType(Joysticks);

  ZilchInitializeType(EventDirectoryWatcher);
  ZilchInitializeType(Job);
  ZilchInitializeType(DocumentationLibrary);
  ZilchInitializeType(Shortcuts);
  ZilchInitializeTypeAs(ProxyObject<Component>, "ComponentProxy");

  if(!Engine::sInLauncher)
    ZilchInitializeTypeAs(LauncherProjectInfoProxy, "LauncherProjectInfo");

  ZilchInitializeType(ZilchLibraryResource);
  ZilchInitializeType(ZilchDocumentResource);

  BindActionFunctions(builder);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

//**************************************************************************************************
bool EngineLibrary::Initialize(ZeroStartupSettings& settings)
{
  // Build meta
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  RegisterClassAttribute(ObjectAttributes::cRunInEditor)->TypeMustBe(Component);
  RegisterClassAttributeType(ObjectAttributes::cCommand, MetaEditorScriptObject)->TypeMustBe(Component);
  RegisterClassAttributeType(ObjectAttributes::cTool, MetaEditorScriptObject)->TypeMustBe(Component);
  RegisterClassAttributeType(ObjectAttributes::cGizmo, MetaEditorGizmo)->TypeMustBe(Component);
  RegisterClassAttributeType(ObjectAttributes::cComponentInterface, MetaInterface)->TypeMustBe(Component);

  RegisterPropertyAttributeType(PropertyAttributes::cDependency, MetaDependency)->TypeMustBe(Component);
  RegisterPropertyAttributeType(PropertyAttributes::cResourceProperty, MetaEditorResource)->TypeMustBe(Resource);

  ZPrintFilter(Filter::DefaultFilter, "Engine Initialize...\n");

  EngineObject::sEngineHeap = new Memory::Heap("Engine", Memory::GetRoot());

  UndoMap::Initialize();

  StartSystemObjects();

  // Could be moved to Platform
  Environment* environment = Environment::GetInstance();
  CrashHandler::SetRestartCommandLine(environment->mCommandLine);

  // Allow the user to specify an extra log file (use 2 different log files so
  // the crash reporter works and because we need to start system objects for
  // this one in order to get the log file)
  String logFile = environment->GetParsedArgument("log");
  FileListener extraListener;
  if(!logFile.Empty())
  {
    extraListener.OverrideLogFile(logFile);
    Zero::Console::Add(&extraListener);
  }

  // Uncomment out this line to disable the fpu exceptions
  // FpuControlSystem::Active = false;

  // Start the profiling system used to performance counters and timers. 
  Profile::ProfileSystem::Initialize();

  // Load the debug drawer.
  Debug::DebugDraw::Initialize();

  // Resource System
  ResourceSystem::Initialize();
  Z::gSystemObjects->Add(Z::gResources, ObjectCleanup::None);

  // Core resource Managers
  InitializeResourceManager(ArchetypeManager);
  InitializeResourceManager(LevelManager);
  InitializeResourceManager(AnimationManager);
  InitializeResourceManager(CurveManager);
  InitializeResourceManager(ResourceTableManager);
  InitializeResourceManager(ColorGradientManager);
  InitializeResourceManager(TextBlockManager);
  InitializeResourceManager(HeightMapSourceManager);

  StartThreadSystem();

  //Create the engine.
  Engine* engine = new Engine();

  Keyboard::Initialize();
  Mouse::Initialize();
  Gamepads::Initialize();
  Joysticks::Initialize();
  LocalModifications::Initialize();
  ObjectStore::Initialize();
  // Need to initialize zilch here as it can be used in the factory below.
  ZilchManager::Initialize();

  Space* engineSpace = new Space();
  engineSpace->SetName("EngineSpace");
  CogInitializer init(nullptr);
  engineSpace->Initialize(init);
  init.AllCreated();

  engine->mEngineSpace = engineSpace;
  
  //Create the factory and Tracker for object creation.
  Tracker* tracker = Tracker::StaticInitialize();
  engine->AddSystemInterface(ZilchTypeId(Tracker), tracker);

  Factory* factory = Factory::StaticInitialize(engine, tracker);
  engine->AddSystemInterface(ZilchTypeId(Factory), factory);

  MetaDatabase::GetInstance()->AddAlternateName("Project", ZilchTypeId(ProjectSettings));

  Cog* config = settings.LoadConfig();
  if(config == nullptr)
    return false;

  engine->mConfigCog = config;

  Tweakables::Load(settings.mTweakableFileName);
  Shortcuts::GetInstance( )->Load(FilePath::Combine(Z::gEngine->GetConfigCog( )->has(MainConfig)->DataDirectory, "Shortcuts.data"));

  return true;
}

//**************************************************************************************************
void EngineLibrary::Shutdown()
{
  ZPrintFilter(Filter::DefaultFilter, "Shutdown Engine...\n");

  Engine* engine = Z::gEngine;

  // Clear map and all handles before final deletion of objects
  UndoMap::Shutdown();

  // Clear references to any default resources held by meta properties
  // Must be done before shutting down the resource system
  MetaDatabase::GetInstance()->ReleaseDefaults();

  // Shutdown background workers and threads
  // before all systems since most code assume
  // systems are never deleted.
  ShutdownThreadSystem();

  ShutdownContentSystem();

  ObjectStore::Destroy();
  LocalModifications::Destroy();
  Joysticks::Destroy();
  Gamepads::Destroy();
  Mouse::Destroy();
  Keyboard::Destroy();

  // Unload all resource and destroy all resource managers
  Z::gResources->UnloadAll();
  delete Z::gResources;

  CleanUpSystemObjects();

  CleanUpErrorContext();

  Debug::DebugDraw::Shutdown();
  engine->DestroySystems();
  delete engine;

  SafeDelete(Z::gTracker);
  SafeDelete(Z::gFactory);
  SafeDelete(Z::gTweakables);

  Profile::ProfileSystem::Shutdown();
  GetLibrary()->ClearComponents();
}

}//namespace Zero
