///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Trevor Sundberg
/// Copyright 2016 DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Common/CommonStandard.hpp"
#include "Platform/PlatformStandard.hpp"
#include "Serialization/SerializationStandard.hpp"
#include "Meta/MetaStandard.hpp"
#include "Math/MathStandard.hpp"
#include "Support/SupportStandard.hpp"

namespace Zero
{
// Forward declarations
class Cog;
class Event;
class Component;
class Serializer;
class Level;
class Space;
class GameSession;
class GameWidget;
class Transform;
class ContentLibrary;
class ResourcePackage;
class ResourceLibrary;
class ProjectSettings;
class Hierarchy;
class KeyboardEvent;
class UpdateEvent;
class ActionSpace;
class SavingEvent;
class DocumentResource;

class ZeroStartupSettings
{
public:
  // The name of the tweakable file
  String mTweakableFileName;

  // When exporting a project, the engine will run and this will be set to true.
  // It's currently used for determining which config file the engine should load.
  bool mEmbeddedPackage;

  // When exporting a stand alone application this is the temp working directory for it
  String mEmbeddedWorkingDirectory;

  // Load the application's config file. Current this is used to allow the launcher
  // to load a different config file from the editor and run a bit of extra logic.
  virtual Cog* LoadConfig();
};

// Engine library
class ZeroNoImportExport EngineLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(EngineLibrary, "ZeroEngine");

  static bool Initialize(ZeroStartupSettings& settings);
  static void Shutdown();

private:
};

}//namespace Zero

#include "Resource.hpp"
#include "EngineBindingExtensions.hpp"
#include "EngineObject.hpp"
#include "EngineContainers.hpp"
#include "EngineMath.hpp"
#include "CogId.hpp"
#include "HierarchyRange.hpp"
#include "Cog.hpp"
#include "Component.hpp"
#include "ComponentMeta.hpp"
#include "CogMetaComposition.hpp"
//#include "Cog.hpp"
#include "CogMeta.hpp"
#include "Space.hpp"
#include "DocumentResource.hpp"
#include "ZilchResource.hpp"
#include "ResourceLibrary.hpp"
#include "JobSystem.hpp"
#include "EngineEvents.hpp"
#include "System.hpp"
#include "Time.hpp"
#include "Engine.hpp"
#include "ThreadDispatch.hpp"
#include "Game.hpp"
#include "Factory.hpp"
#include "ArchetypeRebuilder.hpp"
#include "ResourceSystem.hpp"
#include "ResourcePropertyOperations.hpp"
#include "ErrorContext.hpp"
#include "Tracker.hpp"
#include "Hierarchy.hpp"
#include "TransformSupport.hpp"
#include "Transform.hpp"
#include "Action/Action.hpp"
#include "Action/ActionSystem.hpp"
#include "Action/ActionEase.hpp"
#include "Action/MetaAction.hpp"
#include "Action/BasicActions.hpp"
#include "Action/ActionGenerator.hpp"
#include "EditorSupport.hpp"
#include "CogSerialization.hpp"
#include "ObjectLoader.hpp"
#include "ObjectSaver.hpp"
#include "CogInitializer.hpp"
#include "CogPath.hpp"
#include "ObjectLink.hpp"
#include "Keyboard.hpp"
#include "Environment.hpp"
#include "SystemObjectManager.hpp"
#include "OsWindow.hpp"
#include "OsShell.hpp"
#include "ResourceManager.hpp"
#include "Archetype.hpp"
#include "Mouse.hpp"
#include "Level.hpp"
#include "Operation.hpp"
#include "MetaOperations.hpp"
#include "CogRestoreState.hpp"
#include "CogOperations.hpp"
#include "AnimationNode.hpp"
#include "PropertyTrack.hpp"
#include "AnimationGraph.hpp"
#include "AnimationGraphEvents.hpp"
#include "Animation.hpp"
#include "CogSelection.hpp"
#include "Configuration.hpp"
#include "LauncherConfiguration.hpp"
#include "DebugDraw.hpp"
#include "ResourceLoaderTemplates.hpp"
#include "ScriptDebugEngine.hpp"
#include "TextResource.hpp"
#include "BuildVersion.hpp"
#include "EditorSettings.hpp"
#include "Documentation.hpp"
#include "ObjectStore.hpp"
#include "ResourceTable.hpp"
#include "SampleCurve.hpp"
#include "Noise.hpp"
#include "HeightMap.hpp"
#include "HeightMapSource.hpp"
#include "Gradient.hpp"
#include "ColorGradient.hpp"
#include "Area.hpp"
#include "Tweakables.hpp"
#include "SceneGraph.hpp"
#include "Project.hpp"
#include "RaycastProvider.hpp"
#include "GamepadSystem.hpp"
#include "JoystickSystem.hpp"
#include "HotKeyManager.hpp"
#include "EventDirectoryWatcher.hpp"
#include "CogRange.hpp"
#include "CogHelpers.hpp"
#include "ComponentHierarchy.hpp"
#include "DataSource.hpp"
#include "EngineLibraryExtensions.hpp"
#include "ZilchManager.hpp"
#include "Spline.hpp"
#include "HierarchySpline.hpp"

// Co-dependent libraries
#include "Content/ContentStandard.hpp"
