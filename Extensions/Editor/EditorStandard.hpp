///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Widget/WidgetStandard.hpp"
#include "Networking/NetworkingStandard.hpp"
#include "Gameplay/GameplayStandard.hpp"
#include "Geometry/GeometryStandard.hpp"
#include "Sound/SoundStandard.hpp"
#include "ZilchScript/ZilchScriptStandard.hpp"

namespace Zero
{


// Editor library
class ZeroNoImportExport EditorLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(EditorLibrary, "ZeroEngine");

  static void Initialize();
  static void Shutdown();
};

}//namespace Zero

// Misc.
#include "LauncherCommunicationEvent.hpp"
#include "NetOperations.hpp"
#include "EditorScriptObject.hpp"
#include "BackgroundTask.hpp"
#include "Downloads.hpp"
#include "EditorSearchProviders.hpp"
#include "BackgroundTaskUi.hpp"
#include "SimpleBackgroundTasks.hpp"
#include "StressTest.hpp"
#include "CurveEditor.hpp"

// Commands
#include "CommandSelector.hpp"
#include "AllCommands.hpp"
#include "CogCommand.hpp"
#include "EditorCommands.hpp"
#include "GraphicsCommands.hpp"

// Data Editors
#include "MetaCompositionWrapper.hpp"
#include "PropertyInterface.hpp"
#include "UndoPropertyInterface.hpp"
#include "MultiPropertyInterface.hpp"
#include "Property/PropertyWidget.hpp"
#include "Property/PropertyWidgets.hpp"
#include "Property/PropertyWidgetObject.hpp"
#include "Property/BasicPropertyEditors.hpp"
#include "Property/PropertyView.hpp"
#include "Property/VariantEditor.hpp"
#include "Property/DataBaseTemplate.hpp"
#include "Property/GraphView.hpp"
#include "Editor/TileLayout.hpp"
#include "PreviewWidget.hpp"
#include "Property/TileView.hpp"
#include "TreeViewFilter.hpp"
#include "Property/TreeView.hpp"
#include "TreeViewSearch.hpp"
#include "ItemList.hpp"

// Content Importing
#include "ContentPackage.hpp"
#include "ContentPackageImporter.hpp"
#include "ContentPackageItem.hpp"
#include "ContentStore.hpp"
#include "ContentUploader.hpp"

// Editor Core
#include "Editor.hpp"
#include "EditorProject.hpp"
#include "EditorUtility.hpp"
#include "Export.hpp"
#include "MainPropertyView.hpp"

// Scintilla
#include "ColorScheme.hpp"
#include "TextEditorHotspot.hpp"
#include "TextEditor.hpp"

// Editor3D
#include "OrientationGizmoViewport.hpp"
#include "GridDraw.hpp"
#include "EditorCameraController.hpp"
#include "EditorCameraMouseDrag.hpp"
#include "EditorViewport.hpp"
#include "EditorViewportMenu.hpp"

// Gizmos
#include "Gizmo.hpp"
#include "GizmoDrag.hpp"
#include "BasicGizmos.hpp"
#include "TransformGizmos.hpp"
#include "RotationBasisGizmos.hpp"

// Resource Editors
#include "ResourceOperations.hpp"
#include "AddWindow.hpp"
#include "ContentLogic.hpp"
#include "EditorDrop.hpp"
#include "EditorImport.hpp"
#include "GroupImport.hpp"
#include "SpriteEditor.hpp"
#include "SpriteImporter.hpp"
#include "HeightMapImporter.hpp"
#include "CollisionTableEditor.hpp"
#include "ColorGradientEditor.hpp"
#include "MultiConvexMeshEditor.hpp"
#include "ResourceTableEditor.hpp"
#include "SampleCurveEditor.hpp"
#include "ResourceEditors.hpp"
#include "HeightMapDebugging.hpp"

// Tools
#include "Tool.hpp"
#include "Tools.hpp"
#include "ToolControl.hpp"
#include "TransformTools.hpp"
#include "ObjectTransformTools.hpp"
#include "JointTools.hpp"
#include "ClothTools.hpp"

// HeightMap
#include "HeightMapUndoRedo.hpp"
#include "HeightMapStateManager.hpp"
#include "HeightMapTool.hpp"

// TileMap
#include "TileEditor2D.hpp"
#include "TilePaletteProperty.hpp"
#include "TilePaletteSource.hpp"
#include "TilePaletteView.hpp"

// Editor Ui
#include "AddRemoveListBox.hpp"
#include "ColorPicker.hpp"
#include "Document.hpp"
#include "ErrorList.hpp"
#include "ExtraWidgets.hpp"
#include "GraphWidget.hpp"
#include "MessageBox.hpp"
#include "ScrollingGraph.hpp"
#include "WatchView.hpp"
#include "Scratchboard.hpp"
#include "SelectionHistory.hpp"
#include "ObjectView.hpp"
#include "HotKeyEditor.hpp"
#include "EditorHotspots.hpp"
#include "BugReport.hpp"
#include "BroadPhaseEditor.hpp"
#include "ScriptEditor.hpp"
#include "NotificationUi.hpp"
#include "NetPropertyIcon.hpp"
#include "Loading.hpp"
#include "FindDialog.hpp"
#include "MetaDrop.hpp"
#include "LibraryView.hpp"
#include "ConsoleUi.hpp"

// Animator
#include "AnimationEditorData.hpp"
#include "AnimationSettings.hpp"
#include "AnimationControls.hpp"
#include "AnimationGraph.hpp"
#include "AnimationScrubber.hpp"
#include "AnimationTrackView.hpp"
#include "OnionSkinning.hpp"
#include "AnimationEditor.hpp"

#include "EditorMain.hpp"
