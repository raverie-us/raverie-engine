// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Enums
RaverieDefineEnum(GizmoGrab);
RaverieDefineEnum(GizmoBasis);
RaverieDefineEnum(GizmoPivot);
RaverieDefineEnum(UpdateMode);
RaverieDefineEnum(IncludeMode);
RaverieDefineEnum(GizmoDragMode);
RaverieDefineEnum(GizmoGrabMode);
RaverieDefineEnum(GizmoSnapMode);
RaverieDefineEnum(DockArea);
RaverieDefineEnum(EditorMode);
RaverieDefineEnum(MultiConvexMeshDrawMode);
RaverieDefineEnum(MultiConvexMeshSnappingMode);
RaverieDefineEnum(MultiConvexMeshAutoComputeMode);
RaverieDefineEnum(MultiConvexMeshAutoComputeMethod);
RaverieDefineEnum(ControlMode);
RaverieDefineEnum(CameraDragMode);
RaverieDefineEnum(Placement);
RaverieDefineEnum(ArrowHeadType);
RaverieDefineEnum(JointToolTypes);
RaverieDefineEnum(TimeDisplay);
RaverieDefineEnum(HeightTool);
RaverieDefineEnum(HeightTextureSelect);
RaverieDefineEnum(CellIndexType);
RaverieDefineEnum(SpriteOrigin);
RaverieDefineEnum(TileEditor2DSubToolType);
RaverieDefineEnum(SpringSubTools);
RaverieDefineEnum(ImportMode);
RaverieDefineEnum(PlayGameOptions);
RaverieDefineEnum(ImportFrames);

RaverieDefineType(MetaCompositionWrapper, builder, type)
{
}

RaverieDefineStaticLibrary(EditorLibrary)
{
  builder.CreatableInScriptDefault = false;

  // External Event Bindings
  RaverieBindExternalEvent(Events::ToolActivate, Event, Cog);
  RaverieBindExternalEvent(Events::ToolDeactivate, Event, Cog);
  RaverieBindExternalEvent(Events::ToolDraw, Event, Cog);

  // Enums
  RaverieInitializeEnum(GizmoGrab);
  RaverieInitializeEnum(GizmoBasis);
  RaverieInitializeEnum(GizmoPivot);
  RaverieInitializeEnum(UpdateMode);
  RaverieInitializeEnum(IncludeMode);
  RaverieInitializeEnum(GizmoDragMode);
  RaverieInitializeEnum(GizmoGrabMode);
  RaverieInitializeEnum(GizmoSnapMode);
  RaverieInitializeEnum(DockArea);
  RaverieInitializeEnum(EditorMode);
  RaverieInitializeEnum(MultiConvexMeshDrawMode);
  RaverieInitializeEnum(MultiConvexMeshSnappingMode);
  RaverieInitializeEnum(MultiConvexMeshAutoComputeMode);
  RaverieInitializeEnum(MultiConvexMeshAutoComputeMethod);
  RaverieInitializeEnum(ControlMode);
  RaverieInitializeEnum(CameraDragMode);
  RaverieInitializeEnum(Placement);
  RaverieInitializeEnum(ArrowHeadType);
  RaverieInitializeEnum(JointToolTypes);
  RaverieInitializeEnum(TimeDisplay);
  RaverieInitializeEnum(HeightTool);
  RaverieInitializeEnum(HeightTextureSelect);
  RaverieInitializeEnum(CellIndexType);
  RaverieInitializeEnum(SpriteOrigin);
  RaverieInitializeEnum(TileEditor2DSubToolType);
  RaverieInitializeEnum(SpringSubTools);
  RaverieInitializeEnum(ImportMode);
  RaverieInitializeEnum(PlayGameOptions);
  RaverieInitializeEnum(ImportFrames);

  // Events
  RaverieInitializeType(BackgroundTaskEvent);
  RaverieInitializeType(EditorEvent);
  RaverieInitializeType(MetaDropEvent);
  RaverieInitializeType(PostAddResourceEvent);
  RaverieInitializeType(TreeEvent);
  RaverieInitializeType(TreeViewHeaderAddedEvent);
  RaverieInitializeType(ValueEvent);
  RaverieInitializeType(ContextMenuEvent);
  RaverieInitializeType(TileViewEvent);
  RaverieInitializeType(CurveEvent);
  RaverieInitializeType(TextUpdatedEvent);
  RaverieInitializeType(ConsoleTextEvent);
  RaverieInitializeType(MessageBoxEvent);
  RaverieInitializeType(ColorEvent);
  RaverieInitializeType(TextEditorEvent);
  RaverieInitializeType(ObjectPollEvent);
  RaverieInitializeType(GizmoEvent);
  RaverieInitializeType(GizmoUpdateEvent);
  RaverieInitializeType(GizmoRayTestEvent);
  RaverieInitializeType(RingGizmoEvent);
  RaverieInitializeType(TranslateGizmoUpdateEvent);
  RaverieInitializeType(ScaleGizmoUpdateEvent);
  RaverieInitializeType(RotateGizmoUpdateEvent);
  RaverieInitializeType(ObjectTransformGizmoEvent);
  RaverieInitializeType(RotationBasisGizmoAabbQueryEvent);
  RaverieInitializeType(ToolGizmoEvent);
  RaverieInitializeType(ManipulatorToolEvent);
  RaverieInitializeType(SelectToolFrustumEvent);

  RaverieInitializeType(MetaPropertyEditor);
  RaverieInitializeType(MetaCompositionWrapper);
  RaverieInitializeType(BackgroundTasks);
  RaverieInitializeType(GeneralSearchView);
  RaverieInitializeTypeAs(CurveEditing::Draggable, "CurveDraggable");
  RaverieInitializeTypeAs(CurveEditing::ControlPoint, "CurveControlPoint");
  RaverieInitializeTypeAs(CurveEditing::Tangent, "CurveTangent");
  RaverieInitializeType(Document);
  RaverieInitializeType(DocumentManager);

  // Commands
  RaverieInitializeType(Command);
  RaverieInitializeType(CogCommand);
  RaverieInitializeTypeAs(EditorScriptObjects<CogCommand>, "EditorScriptObjectsCogCommand");
  RaverieInitializeType(CogCommandManager);

  // Data Editors
  RaverieInitializeType(PropertyView);
  RaverieInitializeType(FormattedInPlaceText);
  RaverieInitializeType(InPlaceTextEditor);
  RaverieInitializeType(ValueEditorFactory);
  RaverieInitializeType(PreviewWidget);
  RaverieInitializeType(PreviewWidgetFactory);
  RaverieInitializeType(TileViewWidget);
  RaverieInitializeType(TileView);
  RaverieInitializeType(ItemList);
  RaverieInitializeType(WeightedComposite);
  RaverieInitializeType(ItemGroup);
  RaverieInitializeType(Item);
  RaverieInitializeType(ImportButton);

  // Content Importing
  RaverieInitializeType(ContentPackage);

  // Editor Core
  RaverieInitializeType(Editor);
  RaverieInitializeType(EditorMain);
  RaverieInitializeType(MainPropertyView);

  // Editor Core
  RaverieInitializeType(ColorScheme);
  RaverieInitializeType(TextEditor);

  // Editor3D
  RaverieInitializeType(GridDraw);
  RaverieInitializeType(EditorCameraController);
  RaverieInitializeType(EditorViewport);

  // Gizmos
  RaverieInitializeType(Gizmo);
  RaverieInitializeType(GizmoSpace);
  RaverieInitializeType(GizmoDrag);
  RaverieInitializeType(SimpleGizmoBase);
  RaverieInitializeType(SquareGizmo);
  RaverieInitializeType(ArrowGizmo);
  RaverieInitializeType(RingGizmo);
  RaverieInitializeType(TranslateGizmo);
  RaverieInitializeType(ScaleGizmo);
  RaverieInitializeType(RotateGizmo);
  RaverieInitializeType(ObjectTransformGizmo);
  RaverieInitializeType(ObjectTranslateGizmo);
  RaverieInitializeType(ObjectScaleGizmo);
  RaverieInitializeType(ObjectRotateGizmo);
  // RaverieInitializeType(CogSizerGizmo);
  // RaverieInitializeType(SizerGizmoEvent);
  // RaverieInitializeType(SizerGizmo);
  RaverieInitializeType(RotationBasisGizmoMetaTransform);
  RaverieInitializeType(RotationBasisGizmoInitializationEvent);
  RaverieInitializeType(RotationBasisGizmo);
  RaverieInitializeType(OrientationBasisGizmo);
  RaverieInitializeType(PhysicsCarWheelBasisGizmo);
  RaverieInitializeType(RevoluteBasisGizmo);

  // Resource Editors
  RaverieInitializeType(ResourceEditors);
  RaverieInitializeType(SpritePreview);
  RaverieInitializeType(SpriteSourceEditor);
  RaverieInitializeType(MultiConvexMeshPoint);
  RaverieInitializeType(MultiConvexMeshPropertyViewInfo);
  RaverieInitializeType(MultiConvexMeshEditor);

  RaverieInitializeType(HeightMapDebugDrawer);
  RaverieInitializeType(HeightMapAabbChecker);

  RaverieInitializeType(SpriteSheetImporter);
  RaverieInitializeType(HeightMapImporter);

  // Tools
  RaverieInitializeType(Tool);
  RaverieInitializeType(SelectTool);
  RaverieInitializeType(CreationTool);
  RaverieInitializeType(ObjectConnectingTool);
  RaverieInitializeType(ParentingTool);
  RaverieInitializeType(ToolUiEvent);
  RaverieInitializeType(ToolControl);
  RaverieInitializeType(ManipulatorTool);
  RaverieInitializeType(GizmoCreator);
  RaverieInitializeType(ObjectTransformTool);
  RaverieInitializeType(ObjectTranslateTool);
  RaverieInitializeType(ObjectScaleTool);
  RaverieInitializeType(ObjectRotateTool);
  RaverieInitializeType(JointTool);
  RaverieInitializeType(SpringSubTool);
  RaverieInitializeType(DragSelectSubTool);
  RaverieInitializeType(SelectorSpringSubTool);
  RaverieInitializeType(PointMassSelectorSubTool);
  RaverieInitializeType(AnchoringSubTool);
  RaverieInitializeType(PointSelectorSubTool);
  RaverieInitializeType(SpringSelectorSubTool);
  RaverieInitializeType(SpringCreatorSubTool);
  RaverieInitializeType(RopeCreatorSubTool);
  RaverieInitializeType(SpringPointProxy);
  RaverieInitializeType(SpringPointProxyProperty);
  RaverieInitializeType(SpringTools);

  RaverieInitializeType(HeightMapSubTool);
  RaverieInitializeType(HeightManipulationTool);
  RaverieInitializeType(RaiseLowerTool);
  RaverieInitializeType(SmoothSharpenTool);
  RaverieInitializeType(FlattenTool);
  RaverieInitializeType(CreateDestroyTool);
  RaverieInitializeType(WeightPainterTool);
  RaverieInitializeType(HeightMapTool);
  RaverieInitializeType(ViewportTextWidget);

  RaverieInitializeType(SpriteFrame);

  // TileMap
  RaverieInitializeType(TileEditor2DSubTool);
  RaverieInitializeType(TileEditor2DDrawTool);
  RaverieInitializeType(TileEditor2DSelectTool);
  RaverieInitializeType(TileEditor2D);
  RaverieInitializeType(TilePaletteSource);
  RaverieInitializeType(TilePaletteView);
  RaverieInitializeType(TilePaletteSprite);

  // Editor Ui
  RaverieInitializeType(ObjectView);
  RaverieInitializeType(HotKeyEditor);
  RaverieInitializeType(LibraryView);
  RaverieInitializeType(FloatingComposite);
  RaverieInitializeType(PopUp);
  RaverieInitializeType(AutoCompletePopUp);
  RaverieInitializeType(CallTipPopUp);
  RaverieInitializeType(RemovedEntry);
  RaverieInitializeType(ConsoleUi);
  RaverieInitializeType(DocumentEditor);
  RaverieInitializeType(AddResourceWindow);
  RaverieInitializeType(ResourceTypeSearch);
  RaverieInitializeType(ResourceTemplateSearch);
  RaverieInitializeType(ResourceTemplateDisplay);
  RaverieInitializeType(TreeView);
  RaverieInitializeType(TreeRow);
  RaverieInitializeType(PropertyWidget);
  RaverieInitializeType(PropertyWidgetObject);
  RaverieInitializeType(AddObjectWidget);
  RaverieInitializeType(UiLegacyToolTip);
  RaverieInitializeType(RenderGroupHierarchies);

  RaverieInitializeType(DirectProperty);

  // Animator
  RaverieInitializeType(AnimationEditorData);
  RaverieInitializeType(AnimationSettings);
  RaverieInitializeType(AnimationTrackView);
  RaverieInitializeType(AnimationEditor);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

void EditorLibrary::Initialize()
{
  BuildStaticLibrary();

  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  BackgroundTasks::Initialize();
  DocumentManager::Initialize();
  PreviewWidgetFactory::Initialize();
  ResourceEditors::Initialize();
  ValueEditorFactory::Initialize();
  ColorScheme::Initialize();
  HotKeyCommands::Initialize();

  RegisterGeneralEditors();
  RegisterEngineEditors();
  RegisterObjectViewEditors();
  RegisterHotKeyEditors();
  RegisterAnimationTrackViewEditors();

  // Raycaster should start expanded when opening the property grid
  PropertyWidgetObject::mExpandedTypes.Insert("Raycaster");

  InitializeResourceManager(TilePaletteSourceManager);
}

void EditorLibrary::Shutdown()
{
  HotKeyCommands::Destroy();
  ColorScheme::Destroy();
  ValueEditorFactory::Destroy();
  ResourceEditors::Destroy();
  PreviewWidgetFactory::Destroy();
  DocumentManager::Destroy();
  BackgroundTasks::Destroy();

  GetLibrary()->ClearComponents();
}

} // namespace Raverie
