///////////////////////////////////////////////////////////////////////////////
///
/// \file Game.hpp
/// Declaration of the Game class.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class GameSession;
class Composite;
class OsWindowEvent;
class Composite;
class GameWidget;
class GameEvent;
class OsWindow;
class GameWidget;

namespace Events
{
  DeclareEvent(GameSetup);
  DeclareEvent(GameQuit);
  DeclareEvent(GameStarted);
  DeclareEvent(GameFocusIn);
  DeclareEvent(GameFocusOut);
  DeclareEvent(LevelStarted);
  DeclareEvent(GameRequestQuit);
  DeclareEvent(EditSpaces);
}//namespace Events

/// Typedefs.
typedef ArrayMultiMap<String, Space*> SpaceMap;

//------------------------------------------------------------------- Game Event

/// Event type used by the GameSession
class GameEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  GameEvent();

  /// Has this event been handled?
  /// Used to for the engine to poll for 
  /// Request Quit and other operations.
  bool mHandled;
  /// GameSession Object
  GameSession* mGame;
  /// Space related to his event
  Space* mSpace;
  /// Name of the current level.
  String LevelName;
};

//----------------------------------------------------------------- Game Session
/// The GameSession manages all spaces and data for a a game.
class GameSession : public Cog
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  GameSession();
  ~GameSession();

  /// Cog Interface.
  void Initialize(CogInitializer& initializer) override;
  void Destroy() override;
  GameSession* GetGameSession() override;
  bool IsEditorMode() override;

  /// Game Session cannot be cloned, so throw an exception.
  Cog* Clone() override;

  /// Start the game
  void Start();

  /// Quit the game and exit the engine if not in editor.
  void Quit();

  /// Step the game one frame
  void Step();

  /// Pause all space on game
  void Pause();

  /// Create a space in the game. Use the archetype's name.
  Space* CreateSpace(Archetype* archetype);

  /// Create a space for use in the editor. Scripts without the [RunInEditor]
  /// attribute will be proxied (will not run).
  Space* CreateEditorSpace(Archetype* archetype);

  Space* CreateSpaceFlags(Archetype* archetype, CreationFlags::Type flags);

  /// Create a space from an archetype with the given name.
  Space* CreateNamedSpace(StringParam name, Archetype* archetype);

  /// Find a named space
  Space* FindSpaceByName(StringParam name);
  SpaceMap::valueRange FindAllSpacesByName(StringParam name);
  SpaceMap::valueRange GetAllSpaces();

  /// Request to quit sending out the GameRequestQuit event.
  void RequestQuit();
  
  /// Set the in editor flag.
  void SetInEditor(bool inEditor);

  // Send event to all spaces
  void DispatchOnSpaces(StringParam eventName, Event* event);

  Vec2 GetResolution();

  bool GetFullScreen();

  // Does the game have focus?
  bool GetFocused();

  // In the editor bring up space editing
  void EditSpaces();

//Internals
  void OnClose(OsWindowEvent* event);
  void OnFocusLost(OsWindowEvent* event);
  void OnFocusGained(OsWindowEvent* event);
  void OnSpaceLoaded(ObjectEvent* event);

  /// Is the game running in the editor?
  /// This property specifies if the editor was present (as opposed to an exported game) in this space.
  /// Currently this only gets set on game sessions created by the editor but not the editor's space itself.
  /// Note: this is not if this is an editor (or editing) space.
  bool mInEditor;

  /// Is the game running in edit mode?
  BitField<CreationFlags::Enum> mCreationFlags;

  /// Used to control pausing on all spaces
  bool mPaused;

  /// Has the game started prevents double starts
  bool mStarted;

  /// Is the game quiting?
  bool mQuiting;

  // Spaces managed by this game session
  SpaceMap mTrackedSpaces;

  void InternalAdd(Space* space);
  void InternalRenamed(Space* space);
  void InternalRemove(Space* space);

  // Main widget holding the game
  HandleOf<GameWidget> mGameWidget;

  /// Os Window holding the game. This is used to pass on Os events to the user,
  /// set resolution, set fullscreen, etc...
  OsWindow* mMainWindow;
};

}//namespace Zero
