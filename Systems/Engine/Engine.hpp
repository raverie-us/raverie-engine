///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
namespace Events
{
  DeclareEvent(CurrentInputDeviceChanged);
  DeclareEvent(DebuggerPause);
  DeclareEvent(DebuggerResume);
  DeclareEvent(DebuggerPauseUpdate);
  DeclareEvent(LoadingStart);
  DeclareEvent(LoadingProgress);
  DeclareEvent(LoadingFinish);
  DeclareEvent(BlockingTaskStart);
  DeclareEvent(BlockingTaskFinish);
}//namespace Events

DeclareEnum4(InputDevice, Keyboard, Mouse, Gamepad, Joystick);

//-------------------------------------------------------------------Engine
/// The engine Contains all the systems in the game. The engine is used to update
/// all system and query for other services.
class Engine : public EngineObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Engine();
  ~Engine();

  /// Initializes all systems in the engine.
  void Initialize(SystemInitializer& initializer);
  /// Update all the systems until the engine is no longer active.
  void Run(bool autoShutdown = true);
  /// Update all owned systems.
  void Update();
  /// Terminate the game engine loop.
  void Terminate();
 
  /// Creates an instance of a game session based on the current project
  GameSession* CreateGameSession();
  /// Creates an instance of a game session using an archetype
  GameSession* CreateGameSessionFromArchetype(Archetype* archetype);
  typedef Array<GameSession*> GameSessionArray;
  /// Get all currently active game sessions
  GameSessionArray::range GetGameSessions();

  /// Rebuilds all objects in all GameSessions of the given Archetype.
  void RebuildArchetypes(Archetype* archetype);

  /// Adds a new system to the engine.
  void AddSystem(System* system);
  /// Adds a system as a child 'component'. Internal and should rarely be externally called.
  void AddSystemInterface(BoundType* typeId, System* component);
  /// Get a component on the engine by system's TypeId. 
  /// This will return null if the system is not found.
  System* QueryComponentId(BoundType* typeId);
  /// Type safe way of accessing system/services.
  template<typename type>
  type* Has();
  /// Destroy all systems in reverse order that they were added.
  void DestroySystems();

  /// Get the configuration object that stores settings/initialization data.
  Cog* GetConfigCog();
  /// Returns the current game project's settings.
  ProjectSettings* GetProjectSettings();

  /// Space for engine objects.
  Space* GetEngineSpace();
  typedef InList<Space> SpaceListType;
  /// All active spaces across all game instances
  SpaceListType::range GetSpaces();
  /// Destroys all running spaces in all game sessions.
  void DestroyAllSpaces();

  void LoadingStart();
  /// Update engine while doing blocking operations.
  void LoadingUpdate(StringParam operation, StringParam currentTask, StringParam progress, ProgressType::Enum progressType, float percentage = 0.0f);
  void LoadingFinish();

  /// The input device that the user last used (pressed buttons, moved sticks or triggers, etc...)
  InputDevice::Enum GetCurrentInputDevice();
  void SetCurrentInputDevice(InputDevice::Enum device);

  /// Should normally not be manually called.
  void Shutdown();

  /// If in a debugger, this will trigger a breakpoint
  /// This is useful for debugging C++ things invoked in script
  void DebugBreak();

  /// Forcibly crash the engine. Mostly for debugging/testing crash handling.
  void CrashEngine();

  /// What frame is the engine currently on. Used for debugging
  u64 mFrameCounter;
  // If engine has resources for display of loading
  bool mHaveLoadingResources;

private:

  friend class Space;
  friend class EngineMetaComposition;
  friend class GameSession;
  friend class EngineLibrary;

  void LoadPendingLevels();

  Cog* mConfigCog;
  TimeSystem* mTimeSystem;
  float mTimePassed;
  Space* mEngineSpace;
  SpaceListType mSpaceList;
  InputDevice::Enum mCurrentInputDevice;

  /// Systems to be updated every game loop.
  Array<System*> mSystems;

  /// Map of all system interfaces.
  typedef ArrayMultiMap<BoundType*, System*> SystemMapType;
  SystemMapType mSystemMap;

  GameSessionArray mGameSessions;

  /// Is the engine currently running. Used to shutdown the engine on the next frame.
  bool mEngineActive;
};

//-------------------------------------------------------------------EngineMetaComposition
class EngineMetaComposition : public MetaComposition
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  EngineMetaComposition();

  uint GetComponentCount(HandleParam owner) override;
  Handle GetComponent(HandleParam owner, BoundType* componentType) override;
  Handle GetComponentAt(HandleParam owner, uint index) override;
};

/// Typed based interface for accessing systems.
template<typename type>
type* Engine::Has()
{
  return static_cast<type*>(QueryComponentId(ZilchTypeId(type)));
}

//-------------------------------------------------------------------InputDeviceEvent
/// An event to inform which input device is being used by the user.
class InputDeviceEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  InputDevice::Enum mDevice;
  InputDevice::Enum mLastDevice;
};

/// A core element of the engine has encountered a unrecoverable error.
/// the engine will display a message box and terminate.
void FatalEngineError(cstr format, ...);

// Global Access
namespace Z
{
  extern Engine* gEngine;
}

}//namespace Zero
