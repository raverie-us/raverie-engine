///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Enums
ZilchDefineEnum(GizmoGrab);
ZilchDefineEnum(GizmoBasis);
ZilchDefineEnum(GizmoPivot);
ZilchDefineEnum(UpdateMode);
ZilchDefineEnum(GizmoDragMode);
ZilchDefineEnum(GizmoGrabMode);
ZilchDefineEnum(GizmoSnapMode);
ZilchDefineEnum(DockArea);
ZilchDefineEnum(EditorMode);
ZilchDefineEnum(MultiConvexMeshDrawMode);
ZilchDefineEnum(MultiConvexMeshSnappingMode);
ZilchDefineEnum(MultiConvexMeshAutoComputeMode);
ZilchDefineEnum(MultiConvexMeshAutoComputeMethod);
ZilchDefineEnum(ControlMode);
ZilchDefineEnum(CameraDragMode);
ZilchDefineEnum(Placement);
ZilchDefineEnum(ArrowHeadType);
ZilchDefineEnum(JointToolTypes);
ZilchDefineEnum(TimeDisplay);
ZilchDefineEnum(HeightTool);
ZilchDefineEnum(HeightTextureSelect);
ZilchDefineEnum(CellIndexType);
ZilchDefineEnum(SpriteOrigin);
ZilchDefineEnum(TileEditor2DSubToolType);
ZilchDefineEnum(SpringSubTools);
ZilchDefineEnum(ImportMode);

ZilchDefineType(MetaCompositionWrapper, builder, type)
{
}

//**************************************************************************************************
ZilchDefineStaticLibrary(EditorLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Enums
  ZilchInitializeEnum(GizmoGrab);
  ZilchInitializeEnum(GizmoBasis);
  ZilchInitializeEnum(GizmoPivot);
  ZilchInitializeEnum(UpdateMode);
  ZilchInitializeEnum(GizmoDragMode);
  ZilchInitializeEnum(GizmoGrabMode);
  ZilchInitializeEnum(GizmoSnapMode);
  ZilchInitializeEnum(DockArea);
  ZilchInitializeEnum(EditorMode);
  ZilchInitializeEnum(MultiConvexMeshDrawMode);
  ZilchInitializeEnum(MultiConvexMeshSnappingMode);
  ZilchInitializeEnum(MultiConvexMeshAutoComputeMode);
  ZilchInitializeEnum(MultiConvexMeshAutoComputeMethod);
  ZilchInitializeEnum(ControlMode);
  ZilchInitializeEnum(CameraDragMode);
  ZilchInitializeEnum(Placement);
  ZilchInitializeEnum(ArrowHeadType);
  ZilchInitializeEnum(JointToolTypes);
  ZilchInitializeEnum(TimeDisplay);
  ZilchInitializeEnum(HeightTool);
  ZilchInitializeEnum(HeightTextureSelect);
  ZilchInitializeEnum(CellIndexType);
  ZilchInitializeEnum(SpriteOrigin);
  ZilchInitializeEnum(TileEditor2DSubToolType);
  ZilchInitializeEnum(SpringSubTools);
  ZilchInitializeEnum(ImportMode);

  // Events
  ZilchInitializeType(BackgroundTaskEvent);
  ZilchInitializeType(ToolGizmoEvent);
  ZilchInitializeType(TreeEvent);
  ZilchInitializeType(ValueEvent);
  ZilchInitializeType(ContextMenuEvent);
  ZilchInitializeType(TileViewEvent);
  ZilchInitializeType(CurveEvent);
  ZilchInitializeType(ConsoleTextEvent);
  ZilchInitializeType(ColorEvent);
  ZilchInitializeType(TextEditorEvent);
  ZilchInitializeType(ObjectPollEvent);
  ZilchInitializeType(GizmoRayTestEvent);
  ZilchInitializeType(MessageBoxEvent);

  ZilchInitializeType(MetaPropertyEditor);
  ZilchInitializeType(MetaCompositionWrapper);

  ZilchInitializeType(BackgroundTasks);
  ZilchInitializeType(StressTest);
  ZilchInitializeType(GeneralSearchView);
  ZilchInitializeTypeAs(CurveEditing::Draggable, "CurveDraggable");
  ZilchInitializeTypeAs(CurveEditing::ControlPoint, "CurveControlPoint");
  ZilchInitializeTypeAs(CurveEditing::Tangent, "CurveTangent");
  ZilchInitializeType(EditorPackageLoader);
  ZilchInitializeType(Document);
  ZilchInitializeType(DocumentManager);

  // Commands
  ZilchInitializeType(Command);
  ZilchInitializeType(CogCommand);
  ZilchInitializeType(CogCommandManager::ZilchBase);
  ZilchInitializeType(CogCommandManager);

  // Data Editors
  ZilchInitializeType(PropertyView);
  ZilchInitializeType(FormattedInPlaceText);
  ZilchInitializeType(InPlaceTextEditor);
  ZilchInitializeType(ValueEditorFactory);
  ZilchInitializeType(LauncherCommunicationEvent);
  ZilchInitializeType(PreviewWidget);
  ZilchInitializeType(PreviewWidgetFactory);
  ZilchInitializeType(TileViewWidget);
  ZilchInitializeType(TileView);
  ZilchInitializeType(TextUpdatedEvent);
  ZilchInitializeType(ItemList);
  ZilchInitializeType(WeightedComposite);
  ZilchInitializeType(ItemGroup);
  ZilchInitializeType(Item);
  ZilchInitializeType(ImportButton);

  // Content Importing
  ZilchInitializeType(ContentPackage);

  // Editor Core
  ZilchInitializeType(EditorEvent);
  ZilchInitializeType(Editor);
  ZilchInitializeType(EditorMain);
  ZilchInitializeType(LauncherOpenProjectComposite);
  ZilchInitializeType(LauncherSingletonCommunication);
  ZilchInitializeType(LauncherDebuggerCommunication);
  ZilchInitializeType(SimpleDebuggerListener);
  ZilchInitializeType(MainPropertyView);

  // Editor Core
  ZilchInitializeType(ColorScheme);
  ZilchInitializeType(TextEditor);

  // Editor3D
  ZilchInitializeType(GridDraw);
  ZilchInitializeType(EditorCameraController);
  ZilchInitializeType(EditorViewport);

  // Gizmos
  ZilchInitializeType(GizmoEvent);
  ZilchInitializeType(Gizmo);
  ZilchInitializeType(GizmoSpace);
  ZilchInitializeType(GizmoUpdateEvent);
  ZilchInitializeType(GizmoDrag);
  ZilchInitializeType(SimpleGizmoBase);
  ZilchInitializeType(SquareGizmo);
  ZilchInitializeType(ArrowGizmo);
  ZilchInitializeType(RingGizmoEvent);
  ZilchInitializeType(RingGizmo);
  ZilchInitializeType(TranslateGizmo);
  ZilchInitializeType(ScaleGizmo);
  ZilchInitializeType(RotateGizmo);
  ZilchInitializeType(ObjectTransformGizmo);
  ZilchInitializeType(ObjectTranslateGizmo);
  ZilchInitializeType(ObjectScaleGizmo);
  ZilchInitializeType(ObjectRotateGizmo);
  //ZilchInitializeType(CogSizerGizmo);
  //ZilchInitializeType(SizerGizmoEvent);
  //ZilchInitializeType(SizerGizmo);
  ZilchInitializeType(RotationBasisGizmoMetaTransform);
  ZilchInitializeType(RotationBasisGizmoInitializationEvent);
  ZilchInitializeType(RotationBasisGizmoAabbQueryEvent);
  ZilchInitializeType(RotationBasisGizmo);
  ZilchInitializeType(OrientationBasisGizmo);
  ZilchInitializeType(PhysicsCarWheelBasisGizmo);
  ZilchInitializeType(RevoluteBasisGizmo);

  // Resource Editors
  ZilchInitializeType(ResourceEditors);
  ZilchInitializeType(SpritePreview);
  ZilchInitializeType(SpriteSourceEditor);
  ZilchInitializeType(MultiConvexMeshPoint);
  ZilchInitializeType(MultiConvexMeshPropertyViewInfo);
  ZilchInitializeType(MultiConvexMeshEditor);
  // METAREFACTOR they should always be initialized, but hidden with an attribute
  if(Z::gEngine->GetConfigCog()->has(Zero::DeveloperConfig))
  {
    ZilchInitializeType(HeightMapDebugDrawer);
    ZilchInitializeType(HeightMapAabbChecker);
  }
  ZilchInitializeType(SpriteSheetImporter);
  ZilchInitializeType(HeightMapImporter);

  // Tools
  ZilchInitializeType(Tool);
  ZilchInitializeType(SelectToolFrustumEvent);
  ZilchInitializeType(SelectTool);
  ZilchInitializeType(CreationTool);
  ZilchInitializeType(ObjectConnectingTool);
  ZilchInitializeType(ParentingTool);
  ZilchInitializeType(ToolUiEvent);
  ZilchInitializeType(ToolControl);
  ZilchInitializeType(TransformTool);
  ZilchInitializeType(TranslateTool);
  ZilchInitializeType(ManipulatorTool);
  ZilchInitializeType(GizmoCreator);
  ZilchInitializeType(ObjectTransformTool);
  ZilchInitializeType(ObjectTranslateTool);
  ZilchInitializeType(ObjectScaleTool);
  ZilchInitializeType(ObjectRotateTool);
  ZilchInitializeType(JointTool);
  ZilchInitializeType(SpringSubTool);
  ZilchInitializeType(DragSelectSubTool);
  ZilchInitializeType(SelectorSpringSubTool);
  ZilchInitializeType(PointMassSelectorSubTool);
  ZilchInitializeType(AnchoringSubTool);
  ZilchInitializeType(PointSelectorSubTool);
  ZilchInitializeType(SpringSelectorSubTool);
  ZilchInitializeType(SpringCreatorSubTool);
  ZilchInitializeType(RopeCreatorSubTool);
  ZilchInitializeType(SpringPointProxy);
  ZilchInitializeType(SpringPointProxyProperty);

  // METAREFACTOR this should always be initialized, but hidden with an attribute
  if(Z::gEngine->GetConfigCog()->has(Zero::DeveloperConfig) != nullptr)
    ZilchInitializeType(SpringTools);

  ZilchInitializeType(HeightMapSubTool);
  ZilchInitializeType(HeightManipulationTool);
  ZilchInitializeType(RaiseLowerTool);
  ZilchInitializeType(SmoothSharpenTool);
  ZilchInitializeType(FlattenTool);
  ZilchInitializeType(CreateDestroyTool);
  ZilchInitializeType(WeightPainterTool);
  ZilchInitializeType(HeightMapTool);
  ZilchInitializeType(ViewportTextWidget);

  ZilchInitializeType(SpriteFrame);

  // TileMap
  ZilchInitializeType(TileEditor2DSubTool);
  ZilchInitializeType(TileEditor2DDrawTool);
  ZilchInitializeType(TileEditor2DSelectTool);
  ZilchInitializeType(TileEditor2D);
  ZilchInitializeType(TilePaletteSource);
  ZilchInitializeType(TilePaletteView);
  ZilchInitializeType(TilePaletteSprite);

  // Editor Ui
  ZilchInitializeType(ObjectView);
  ZilchInitializeType(HotKeyCommands);
  ZilchInitializeType(HotKeyEditor);
  ZilchInitializeType(MetaDropEvent);
  ZilchInitializeType(LibraryView);
  ZilchInitializeType(FloatingComposite);
  ZilchInitializeType(PopUp);
  ZilchInitializeType(AutoCompletePopUp);
  ZilchInitializeType(CallTipPopUp);
  ZilchInitializeType(RemovedEntry);
  ZilchInitializeType(ConsoleUi);
  ZilchInitializeType(DocumentEditor);
  ZilchInitializeType(AddResourceWindow);
  ZilchInitializeType(ResourceTypeSearch);
  ZilchInitializeType(ResourceTemplateSearch);
  ZilchInitializeType(ResourceTemplateDisplay);
  ZilchInitializeType(TreeView);
  ZilchInitializeType(TreeRow);
  ZilchInitializeType(PropertyWidget);
  ZilchInitializeType(PropertyWidgetObject);
  ZilchInitializeType(AddObjectWidget);
  
  // Animator
  ZilchInitializeType(AnimationEditorData);
  ZilchInitializeType(AnimationSettings);
  ZilchInitializeType(AnimationTrackView);
  ZilchInitializeType(AnimationEditor);

  // Stress test
  ZilchInitializeType(StressTestDialog);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

//**************************************************************************************************
void EditorLibrary::Initialize()
{
  BuildStaticLibrary();

  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
  
  BackgroundTasks::Initialize();
  DocumentManager::Initialize();
  Exporter::Initialize();
  PreviewWidgetFactory::Initialize();
  ResourceEditors::Initialize();
  EditorPackageLoader::Initialize();
  ValueEditorFactory::Initialize();
  ColorScheme::Initialize();

  RegisterGeneralEditors();
  RegisterEngineEditors();
  RegisterObjectViewEditors();
  RegisterHotKeyEditors();
  RegisterAnimationTrackViewEditors();

  InitializeResourceManager(TilePaletteSourceManager);
}

//**************************************************************************************************
void EditorLibrary::Shutdown()
{
  ColorScheme::Destroy();
  ValueEditorFactory::Destroy();
  EditorPackageLoader::Destroy();
  ResourceEditors::Destroy();
  PreviewWidgetFactory::Destroy();
  DocumentManager::Destroy();
  Exporter::Destroy();
  BackgroundTasks::Destroy();

  GetLibrary()->ClearComponents();
}

}//namespace Zero