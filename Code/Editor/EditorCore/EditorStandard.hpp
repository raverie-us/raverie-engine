// MIT Licensed (see LICENSE.md).
#pragma once

#include "Extensions/Widget/WidgetStandard.hpp"
#include "Systems/Networking/NetworkingStandard.hpp"
#include "Extensions/Gameplay/GameplayStandard.hpp"
#include "Foundation/Geometry/GeometryStandard.hpp"
#include "Systems/Sound/SoundStandard.hpp"
#include "Systems/RaverieScript/RaverieScriptStandard.hpp"
#include "Extensions/UiWidget/UiWidgetStandard.hpp"

namespace Raverie
{

// Editor library
class EditorLibrary : public Raverie::StaticLibrary
{
public:
  RaverieDeclareStaticLibraryInternals(EditorLibrary);

  static void Initialize();
  static void Shutdown();
};

} // namespace Raverie

// Misc.
#include "NetOperations.hpp"
#include "Downloads.hpp"
#include "EditorSearchProviders.hpp"
#include "BackgroundTaskUi.hpp"
#include "CurveEditor.hpp"

// Commands
#include "CommandSelector.hpp"
#include "AllCommands.hpp"
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
#include "Editor/EditorCore/TileLayout.hpp"
#include "PreviewWidget.hpp"
#include "Property/TileView.hpp"
#include "TreeViewFilter.hpp"
#include "Property/TreeView.hpp"
#include "TreeViewSearch.hpp"
#include "ItemList.hpp"
#include "ListView.hpp"

// Content Importing
#include "ContentPackage.hpp"
#include "ContentPackageImporter.hpp"
#include "ContentPackageItem.hpp"
#include "ContentUploader.hpp"

// Editor Core
#include "Editor.hpp"
#include "EditorProject.hpp"
#include "EditorUtility.hpp"
#include "MainPropertyView.hpp"

// Depends on Editor.hpp
#include "EditorScriptObject.hpp"
#include "CogCommand.hpp"

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
#include "BroadPhaseEditor.hpp"
#include "ScriptEditor.hpp"
#include "NotificationUi.hpp"
#include "NetPropertyIcon.hpp"
#include "Loading.hpp"
#include "FindDialog.hpp"
#include "MetaDrop.hpp"
#include "LibraryView.hpp"
#include "ConsoleUi.hpp"
#include "UiLegacyToolTip.hpp"
#include "PreviewWidgets.hpp"
#include "RenderGroupHierarchies.hpp"

// Animator
#include "AnimationEditorData.hpp"
#include "AnimationSettings.hpp"
#include "AnimationControls.hpp"
#include "AnimationGraph.hpp"
#include "AnimationScrubber.hpp"
#include "AnimationTrackView.hpp"
#include "OnionSkinning.hpp"
#include "AnimationEditor.hpp"

// CodeTranslator
#include "CodeTranslator/CodeSplitWindow.hpp"
#include "CodeTranslator/CodeTranslatorCommands.hpp"
#include "CodeTranslator/ShaderTranslationDebugHelper.hpp"

#include "EditorMain.hpp"
