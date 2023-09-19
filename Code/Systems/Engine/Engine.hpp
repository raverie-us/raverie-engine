// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
namespace Events
{
DeclareEvent(DebuggerPause);
DeclareEvent(DebuggerResume);
DeclareEvent(DebuggerPauseUpdate);
DeclareEvent(LoadingProgress);
DeclareEvent(LoadingFinish);
DeclareEvent(BlockingTaskStart);
DeclareEvent(BlockingTaskFinish);
} // namespace Events

/// The engine Contains all the systems in the game. The engine is used to
/// update all system and query for other services.
class Engine : public EngineObject
{
public:
  ZilchDeclareType(Engine, TypeCopyMode::ReferenceType);

  Engine();
  ~Engine();

  /// Initializes all systems in the engine.
  void Initialize(SystemInitializer& initializer);
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
  /// Adds a system as a child 'component'. Internal and should rarely be
  /// externally called.
  void AddSystemInterface(BoundType* typeId, System* component);
  /// Get a component on the engine by system's TypeId.
  /// This will return null if the system is not found.
  System* QueryComponentId(BoundType* typeId);
  /// Type safe way of accessing system/services.
  template <typename type>
  type* Has();
  /// Destroy all systems in reverse order that they were added.
  void DestroySystems();

  /// Get the configuration object that stores settings/initialization data.
  Cog* GetConfigCog();
  /// Returns the current game project's settings.
  ProjectSettings* GetProjectSettings();

  Actions* GetActions() override;
  /// Space for engine objects.
  Space* GetEngineSpace();
  typedef InList<Space> SpaceListType;
  /// All active spaces across all game instances
  SpaceListType::range GetSpaces();
  /// Destroys all running spaces in all game sessions.
  void DestroyAllSpaces();

  void LoadingStart();
  /// Update engine while doing blocking operations.
  void LoadingUpdate(StringParam operation,
                     StringParam currentTask,
                     StringParam progress,
                     ProgressType::Enum progressType,
                     float percentage = 0.0f);
  void LoadingFinish();

  /// Should normally not be manually called.
  void Shutdown();

  /// The engine may be in read only mode (such as when debugging a breakpoint).
  bool IsReadOnly();

  /// What frame is the engine currently on. Used for debugging
  u64 mFrameCounter;

  // If we're currently debugging scripts which means we have a special very
  // limited update.
  bool mIsDebugging;

  Cog* mConfigCog;

  /// Is the engine currently running. Used to shutdown the engine on the next
  /// frame.
  bool mEngineActive;

private:
  friend class Space;
  friend class EngineMetaComposition;
  friend class GameSession;
  friend class EngineLibrary;
  friend class ZeroStartup;

  void LoadPendingLevels();

  TimeSystem* mTimeSystem;
  float mTimePassed;
  Space* mEngineSpace;
  SpaceListType mSpaceList;
  size_t mLoadingCount = 0;

  /// Systems to be updated every game loop.
  Array<System*> mSystems;

  /// Map of all system interfaces.
  typedef ArrayMultiMap<BoundType*, System*> SystemMapType;
  SystemMapType mSystemMap;

  GameSessionArray mGameSessions;
};

class EngineMetaComposition : public MetaComposition
{
public:
  ZilchDeclareType(EngineMetaComposition, TypeCopyMode::ReferenceType);

  EngineMetaComposition();

  uint GetComponentCount(HandleParam owner) override;
  Handle GetComponent(HandleParam owner, BoundType* componentType) override;
  Handle GetComponentAt(HandleParam owner, uint index) override;
};

/// Typed based interface for accessing systems.
template <typename type>
type* Engine::Has()
{
  return static_cast<type*>(QueryComponentId(ZilchTypeId(type)));
}

/// A core element of the engine has encountered a unrecoverable error.
/// the engine will display a message box and terminate.
ZeroNoReturn void FatalEngineError(cstr format, ...);

// Global Access
namespace Z
{
extern Engine* gEngine;
}

} // namespace Zero
