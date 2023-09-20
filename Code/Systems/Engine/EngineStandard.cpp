// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Ranges
RaverieDefineRange(HierarchyNameRange);
RaverieDefineRange(HierarchyListNameRange);
RaverieDefineRange(HierarchyRange);
RaverieDefineRange(CogNameRange);
RaverieDefineRange(CogRootNameRange);
RaverieDefineRange(HierarchyList::range);
RaverieDefineRange(HierarchyList::reverse_range);
RaverieDefineRange(Space::range);
RaverieDefineRange(SpaceMap::valueRange);
RaverieDefineRange(ObjectLinkRange);
RaverieDefineRange(CogHashSetRange);
RaverieDefineRange(ResourceTableEntryList::range);
RaverieDefineRange(OperationListRange);
RaverieDefineRange(Engine::GameSessionArray::range);

// Enums
RaverieDefineEnum(ActionExecuteMode);
RaverieDefineEnum(ActionState);
RaverieDefineEnum(AnimationBlendMode);
RaverieDefineEnum(AnimationBlendType);
RaverieDefineEnum(AnimationPlayMode);
RaverieDefineEnum(Buttons);
RaverieDefineEnum(CogPathPreference);
RaverieDefineEnum(Cursor);
RaverieDefineEnum(EaseType);
RaverieDefineEnum(FlickedStick);
RaverieDefineEnum(KeyState);
RaverieDefineEnum(Math::CurveType);
RaverieDefineEnum(MouseButtons);
RaverieDefineEnum(SplineType);
RaverieDefineEnum(StoreResult);
RaverieDefineEnum(TabWidth);
RaverieDefineEnum(TimeMode);

void LocationBind(LibraryBuilder& builder, BoundType* type)
{
  // We need to alias RaverieSelf for the method binding macros
  namespace RaverieSelf = Location;

  RaverieBindMethod(IsCardinal);
  RaverieBindMethod(GetCardinalAxis);
  RaverieBindOverloadedMethod(GetDirection, RaverieStaticOverload(Vec2, Location::Enum));
  RaverieBindOverloadedMethod(GetDirection, RaverieStaticOverload(Vec2, Location::Enum, Location::Enum));
  RaverieBindMethod(GetOpposite);
}

RaverieDefineExternalBaseType(Location::Enum, TypeCopyMode::ValueType, builder, type)
{
  RaverieFullBindEnum(builder, type, SpecialType::Enumeration);
  RaverieBindEnumValues(Location);

  LocationBind(builder, type);
}

// Arrays
RaverieDefineArrayType(Array<ContentLibraryReference>);

// The keys enum has to be declared special since it skips values
RaverieDefineExternalBaseType(Keys::Enum, TypeCopyMode::ValueType, builder, type)
{
  SetUpKeyNames();
  RaverieFullBindEnum(builder, type, SpecialType::Enumeration);

  // For now, just iterate over all keys in the name map and if there was no
  // saved name then assume that the key doesn't exist (linear but whatever)
  for (size_t i = 0; i < Keys::Size; ++i)
  {
    if (KeyNames[i] == nullptr)
      continue;

    RaverieFullBindEnumValue(builder, type, i, KeyNames[i]);
  }
}

RaverieDefineStaticLibrary(EngineLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  RaverieInitializeRange(HierarchyNameRange);
  RaverieInitializeRange(HierarchyListNameRange);
  RaverieInitializeRange(HierarchyRange);
  RaverieInitializeRange(CogNameRange);
  RaverieInitializeRange(CogRootNameRange);
  RaverieInitializeRangeAs(HierarchyList::range, "HierarchyListRange");
  RaverieInitializeRangeAs(HierarchyList::reverse_range, "HierarchyListReverseRange");
  RaverieInitializeRangeAs(Space::range, "SpaceRange");
  RaverieInitializeRangeAs(SpaceMap::valueRange, "SpaceMapValueRange");
  RaverieInitializeRange(ObjectLinkRange);
  RaverieInitializeRange(CogHashSetRange);
  RaverieInitializeRangeAs(ResourceTableEntryList::range, "ResourceTableEntryRange");
  RaverieInitializeRange(OperationListRange);
  RaverieInitializeRangeAs(Engine::GameSessionArray::range, "GameSessionRange");

  // Enums
  RaverieInitializeEnum(ActionExecuteMode);
  RaverieInitializeEnum(ActionState);
  RaverieInitializeEnum(AnimationBlendMode);
  RaverieInitializeEnum(AnimationBlendType);
  RaverieInitializeEnum(AnimationPlayMode);
  RaverieInitializeEnum(Buttons);
  RaverieInitializeEnum(CogPathPreference);
  RaverieInitializeEnum(Cursor);
  RaverieInitializeEnumAs(EaseType, "Ease");
  RaverieInitializeEnum(FlickedStick);
  RaverieInitializeEnum(Keys);
  RaverieInitializeEnum(KeyState);
  RaverieInitializeEnum(Location);
  RaverieInitializeEnumAs(Math::CurveType, "CurveType");
  RaverieInitializeEnum(MouseButtons);
  RaverieInitializeEnum(SplineType);
  RaverieInitializeEnum(StoreResult);
  RaverieInitializeEnum(TabWidth);
  RaverieInitializeEnum(TimeMode);

  // Arrays
  RaverieInitializeArrayTypeAs(Array<ContentLibraryReference>, "ContentLibraryReferenceArray");
  RaverieInitializeType(DataSource);

  RaverieInitializeType(System);

  // Meta Components
  RaverieInitializeType(TransformMetaTransform);
  RaverieInitializeType(CogMetaComposition);
  RaverieInitializeType(CogMetaDataInheritance);
  RaverieInitializeType(CogMetaDisplay);
  RaverieInitializeType(CogMetaSerialization);
  RaverieInitializeType(CogMetaOperations);
  RaverieInitializeType(CogMetaTransform);
  RaverieInitializeType(CogArchetypeExtension);
  RaverieInitializeType(CogSerializationFilter);
  RaverieInitializeType(CogPathMetaSerialization);
  RaverieInitializeType(ComponentMetaDataInheritance);
  RaverieInitializeType(DataResourceInheritance);
  RaverieInitializeType(ResourceMetaSerialization);
  RaverieInitializeType(EngineMetaComposition);
  RaverieInitializeType(HierarchyComposition);
  RaverieInitializeType(MetaResource);
  RaverieInitializeType(ComponentMetaOperations);
  RaverieInitializeType(ResourceMetaOperations);
  RaverieInitializeType(CogArchetypePropertyFilter);
  RaverieInitializeType(CogPathMetaComposition);
  RaverieInitializeType(MetaEditorScriptObject);
  RaverieInitializeType(MetaDependency);
  RaverieInitializeType(MetaInterface);
  RaverieInitializeType(RaycasterMetaComposition);

  // Events
  RaverieInitializeType(CogPathEvent);
  RaverieInitializeType(UpdateEvent);
  RaverieInitializeType(ResourceEvent);
  RaverieInitializeType(GameEvent);
  RaverieInitializeType(AnimationGraphEvent);
  RaverieInitializeType(KeyboardEvent);
  RaverieInitializeType(FileEditEvent);
  RaverieInitializeType(KeyboardTextEvent);
  RaverieInitializeType(OsMouseEvent);
  RaverieInitializeType(HierarchyEvent);
  RaverieInitializeType(CogInitializerEvent);
  RaverieInitializeType(ObjectLinkEvent);
  RaverieInitializeType(ObjectLinkPointChangedEvent);
  RaverieInitializeType(HeightMapEvent);
  RaverieInitializeType(AreaEvent);
  RaverieInitializeType(GamepadEvent);
  RaverieInitializeType(OperationQueueEvent);
  RaverieInitializeType(OsWindowEvent);
  RaverieInitializeType(OsWindowBorderHitTest);
  RaverieInitializeType(OsMouseDropEvent);
  RaverieInitializeType(SavingEvent);
  RaverieInitializeType(ScriptEvent);
  RaverieInitializeType(DataEvent);
  RaverieInitializeType(DataReplaceEvent);
  RaverieInitializeType(CogReplaceEvent);
  RaverieInitializeType(TextEvent);
  RaverieInitializeType(TextErrorEvent);
  RaverieInitializeType(OsFileSelection);
  RaverieInitializeType(ClipboardEvent);
  RaverieInitializeType(RaverieCompiledEvent);
  RaverieInitializeType(RaverieCompileFragmentEvent);
  RaverieInitializeType(RaverieCompileEvent);
  RaverieInitializeType(SplineEvent);
  RaverieInitializeType(BlockingTaskEvent);

  // Components
  RaverieInitializeType(Component);
  RaverieInitializeType(Transform);
  RaverieInitializeType(Hierarchy);
  RaverieInitializeType(TimeSpace);
  RaverieInitializeType(ObjectLink);
  RaverieInitializeType(ObjectLinkAnchor);
  RaverieInitializeType(Hierarchy);
  RaverieInitializeType(AnimationGraph);
  RaverieInitializeType(SimpleAnimation);
  RaverieInitializeType(HeightMap);
  RaverieInitializeType(Area);
  RaverieInitializeType(AnimationGraph);
  RaverieInitializeType(ProjectSettings);
  RaverieInitializeType(ProjectDescription);

  RaverieInitializeType(Cog);
  RaverieInitializeType(Space);
  RaverieInitializeType(ResourceDisplayFunctions);
  RaverieInitializeType(Resource);
  RaverieInitializeType(DataResource);
  RaverieInitializeType(ResourceSystem);
  RaverieInitializeType(ResourcePackageDisplay);
  RaverieInitializeType(ResourcePackage);
  RaverieInitializeType(ResourceLibrary);

  RaverieInitializeType(CogPath);

  RaverieInitializeType(Engine);
  RaverieInitializeType(GameSession);

  RaverieInitializeType(AnimationNode);
  RaverieInitializeType(PoseNode);
  RaverieInitializeType(BasicAnimation);
  RaverieInitializeType(DualBlend<DirectBlend>);
  RaverieInitializeType(DirectBlend);
  RaverieInitializeType(DualBlend<CrossBlend>);
  RaverieInitializeType(CrossBlend);
  RaverieInitializeType(DualBlend<SelectiveNode>);
  RaverieInitializeType(SelectiveNode);
  RaverieInitializeType(DualBlend<ChainNode>);
  RaverieInitializeType(ChainNode);
  RaverieInitializeType(ObjectTrack);
  RaverieInitializeType(Animation);
  RaverieInitializeType(Environment);

  RaverieInitializeType(Keyboard);

  RaverieInitializeType(System);
  RaverieInitializeType(TimeSystem);
  RaverieInitializeType(OsShell);
  RaverieInitializeType(OsWindow);
  RaverieInitializeType(Mouse);
  RaverieInitializeType(Factory);
  RaverieInitializeType(Operation);
  RaverieInitializeType(OperationQueue);
  RaverieInitializeType(OperationBatch);
  RaverieInitializeType(PropertyOperation);
  RaverieInitializeType(Tracker);
  RaverieInitializeType(Spline);
  RaverieInitializeType(SplineSampleData);
  RaverieInitializeType(SplineBakedPoints);
  RaverieInitializeType(SplineBakedPoint);
  RaverieInitializeType(SplineControlPoints);
  RaverieInitializeType(SplineControlPoint);

  RaverieInitializeType(Action);
  RaverieInitializeType(ActionSet);
  RaverieInitializeType(Actions);
  RaverieInitializeType(ActionGroup);
  RaverieInitializeType(ActionSequence);
  RaverieInitializeType(ActionSpace);
  RaverieInitializeType(ActionDelay);

  RaverieInitializeType(CogInitializer);

  RaverieInitializeType(Thickness);
  RaverieInitializeType(Rectangle);

  RaverieInitializeType(LinkId);
  RaverieInitializeType(Named);
  RaverieInitializeType(EditorFlags);
  RaverieInitializeType(SpaceObjects);
  RaverieInitializeType(Archetype);
  RaverieInitializeType(Archetyped);
  RaverieInitializeType(ArchetypeInstance);

  RaverieInitializeType(MetaSelection);

  RaverieInitializeType(ObjectLinkEdge);
  RaverieInitializeType(ObjectLinkAnchor);
  RaverieInitializeType(ObjectLink);

  RaverieInitializeType(Level);

  RaverieInitializeType(DebugDraw);

  RaverieInitializeType(DocumentResource);
  RaverieInitializeType(TextBlock);

  RaverieInitializeType(MainConfig);
  RaverieInitializeType(EditorConfig);
  RaverieInitializeType(FrameRateSettings);
  RaverieInitializeType(DebugSettings);
  RaverieInitializeType(ExportSettings);
  RaverieInitializeType(ContentConfig);
  RaverieInitializeType(UserConfig);
  RaverieInitializeType(DeveloperConfig);
  RaverieInitializeType(TextEditorConfig);
  RaverieInitializeType(EditorSettings);

  RaverieInitializeType(HierarchySpline);

  RaverieInitializeType(ObjectStore);
  RaverieInitializeType(ResourceTable);
  RaverieInitializeType(ResourceTableEntry);

  RaverieInitializeType(SampleCurve);

  RaverieInitializeType(HeightMap);
  RaverieInitializeType(HeightPatch);
  RaverieInitializeType(HeightMapSource);

  RaverieInitializeType(SceneGraphSource);

  RaverieInitializeType(ColorGradient);

  RaverieInitializeType(Area);

  RaverieInitializeType(ProjectSettings);
  RaverieInitializeType(ContentLibraryReference);
  RaverieInitializeType(SharedContent);
  RaverieInitializeType(ProjectDescription);

  RaverieInitializeType(RaycastProvider);
  RaverieInitializeType(Raycaster);

  RaverieInitializeType(Gamepad);
  RaverieInitializeType(Gamepads);

  RaverieInitializeType(Tweakables);

  RaverieInitializeType(EventDirectoryWatcher);
  RaverieInitializeType(Job);
  RaverieInitializeType(DocumentationLibrary);
  RaverieInitializeType(Shortcuts);
  RaverieInitializeTypeAs(ProxyObject<Component>, "ComponentProxy");

  RaverieInitializeType(RaverieLibraryResource);
  RaverieInitializeType(RaverieDocumentResource);

  BindActionFunctions(builder);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

bool EngineLibrary::Initialize()
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

  // Allow the user to specify an extra log file (use 2 different log files so
  // the crash reporter works and because we need to start system objects for
  // this one in order to get the log file)
  String logFile = environment->GetParsedArgument("log");
  FileListener extraListener;
  if (!logFile.Empty())
  {
    extraListener.OverrideLogFile(logFile);
    Raverie::Console::Add(&extraListener);
  }

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

  // Create the engine.
  Engine* engine = new Engine();

  // This must be called right after the engine is created because it connects
  // to the engine.
  StartThreadSystem();

  Keyboard::Initialize();
  Mouse::Initialize();
  Gamepads::Initialize();
  LocalModifications::Initialize();
  ObjectStore::Initialize();
  // Need to initialize raverie here as it can be used in the factory below.
  RaverieManager::Initialize();

  Space* engineSpace = new Space();
  engineSpace->SetName("EngineSpace");
  CogInitializer init(nullptr);
  engineSpace->Initialize(init);
  init.AllCreated();

  engine->mEngineSpace = engineSpace;

  // Create the factory and Tracker for object creation.
  Tracker* tracker = Tracker::StaticInitialize();
  engine->AddSystemInterface(RaverieTypeId(Tracker), tracker);

  Factory* factory = Factory::StaticInitialize(engine, tracker);
  engine->AddSystemInterface(RaverieTypeId(Factory), factory);

  MetaDatabase::GetInstance()->AddAlternateName("Project", RaverieTypeId(ProjectSettings));
  return true;
}

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

  GetLibrary()->ClearComponents();
}

} // namespace Raverie
