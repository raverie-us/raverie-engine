///////////////////////////////////////////////////////////////////////////////
///
/// \file Game.cpp
/// Implementation of the Game class.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(GameSetup);
  DefineEvent(GameLoad);
  DefineEvent(GameQuit);
  DefineEvent(GameStarted);
  DefineEvent(GameFocusIn);
  DefineEvent(GameFocusOut);
  DefineEvent(LevelStarted);
  DefineEvent(GameRequestQuit);
  DefineEvent(EditSpaces);
}//namespace Events

//------------------------------------------------------------GameEvent

GameEvent::GameEvent()
{
  mHandled = false;
  mGame = nullptr;
  mSpace = nullptr;
}

ZilchDefineType(GameEvent, builder, type)
{
  ZeroBindDocumented();
  ZilchBindFieldProperty(LevelName);
  ZilchBindFieldProperty(mGame);
  ZilchBindFieldProperty(mSpace);
  ZilchBindFieldProperty(mHandled);
}

//----------------------------------------------------------------- Game Session
//******************************************************************************
ZilchDefineType(GameSession, builder, type)
{
  ZeroBindDocumented();

  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindMethod(Start);
  ZilchBindMethod(Quit);
  ZilchBindMethod(RequestQuit);
  ZilchBindMethod(CreateSpace);
  ZilchBindMethod(CreateNamedSpace);

  ZilchBindMethod(FindSpaceByName);
  ZilchBindMethod(FindAllSpacesByName);
  ZilchBindMethod(IsEditorMode);
  ZilchBindGetter(AllSpaces);

  ZilchBindGetter(Focused);
  ZilchBindGetter(Resolution);
  ZilchBindGetter(FullScreen);

  ZeroBindEvent(Events::GameSetup, GameEvent);
  ZeroBindEvent(Events::GameLoad, GameEvent);

  ZeroBindEvent(Events::GameFocusIn, GameEvent);
  ZeroBindEvent(Events::GameFocusOut, GameEvent);

  ZeroBindEvent(Events::GameQuit, GameEvent);
  ZeroBindEvent(Events::GameRequestQuit, GameEvent);
  ZeroBindEvent(Events::GameStarted, GameEvent);

  ZeroBindEvent(Events::LevelStarted, GameEvent);
  ZeroBindEvent(Events::EditSpaces, Event);
}

//******************************************************************************
GameSession::GameSession()
{
  // METAREFACTOR this was previous set to a passed in MetaCreateContext context->Flags
  mCreationFlags = CreationFlags::Default;
  mInEditor = false;
  mQuiting = false;
  mPaused = false;
  mStarted = false;
  mMainWindow = nullptr;
  Z::gEngine->mGameSessions.PushBack(this);
}

//******************************************************************************
GameSession::~GameSession()
{
  Mouse::GetInstance()->SetTrapped(false);
  Z::gEngine->mGameSessions.EraseValue(this);
}

//******************************************************************************
void GameSession::Initialize(CogInitializer& initializer)
{
  if(!IsEditorMode())
    ZPrintFilter(Filter::DefaultFilter, "---------------- Begin Game ----------------\n");

  Cog::Initialize(initializer);
}

//******************************************************************************
void GameSession::Destroy()
{
  // Make sure we haven't already been destroyed
  if(mFlags.IsSet(CogFlags::Destroyed))
    return;

  while(!mTrackedSpaces.Empty())
  {
    Space* space = mTrackedSpaces.Front().second;
    space->Destroy();
  }

  ErrorIf(!mTrackedSpaces.Empty(), "All spaces should have been cleaned up.");
  mTrackedSpaces.Clear();

  Cog::Destroy();

  if(!IsEditorMode())
    ZPrint("----------------  End Game  ----------------\n");
}

//******************************************************************************
GameSession* GameSession::GetGameSession()
{
  return this;
}

//******************************************************************************
Cog* GameSession::Clone()
{
  DoNotifyException("Failed to Clone", "Game Session's cannot be cloned");
  return nullptr;
}

//******************************************************************************
void GameSession::Start()
{
  //Can't start the game twice
  if(mStarted)
    return;

  mStarted = true;
  Space* loadedSpace = nullptr;

  //Run Setup
  {
    GameEvent gameEvent;
    gameEvent.mGame = this;
    this->DispatchEvent(Events::GameSetup, &gameEvent);
    loadedSpace = gameEvent.mSpace;
  }

  //Run load
  {
    GameEvent gameEvent;
    gameEvent.mGame = this;
    this->DispatchEvent(Events::GameLoad, &gameEvent);
  }

  GameEvent gameEvent;
  gameEvent.mGame = this;
  gameEvent.mSpace = loadedSpace;

  this->DispatchEvent(Events::GameStarted, &gameEvent);
  this->DispatchOnSpaces(Events::GameStarted, &gameEvent);
}

//******************************************************************************
void GameSession::Quit()
{
  if(mQuiting)
    return;

  mQuiting = true;
  this->Destroy();

  GameEvent event;
  event.mGame = this;
  this->DispatchEvent(Events::GameQuit, &event);

  if(!mInEditor)
    Z::gEngine->Terminate();
}

//******************************************************************************
void GameSession::Step()
{
  mPaused = true;

  forRange(Space* space, mTrackedSpaces.AllValues())
  {
    TimeSpace* timeSpace = space->has(TimeSpace);
    timeSpace->Step();
  }
}

//******************************************************************************
void GameSession::Pause()
{
  mPaused = true;
}

//******************************************************************************
Space* GameSession::CreateSpace(Archetype* archetype)
{
  return CreateSpaceFlags(archetype, CreationFlags::Default);
}

//******************************************************************************
Space* GameSession::CreateEditorSpace(Archetype* archetype)
{
  return CreateSpaceFlags(archetype, CreationFlags::Editing);
}

//******************************************************************************
Space* GameSession::CreateSpaceFlags(Archetype* archetype, CreationFlags::Type flags)
{
  if(!archetype)
    return nullptr;

  Space* space = Z::gFactory->CreateSpace(archetype->ResourceIdName, flags, this);
  if(space && !space->GetMarkedForDestruction())
    InternalAdd(space);
  return space;
}

//******************************************************************************
Space* GameSession::CreateNamedSpace(StringParam name, Archetype* archetype)
{
  if(!archetype)
    return nullptr;

  Space* space = Z::gFactory->CreateSpace(archetype->ResourceIdName, CreationFlags::Default, this);
  if(space && !space->GetMarkedForDestruction())
  {
    InternalAdd(space);
    space->SetName(name);
  }
  return space;
}

//******************************************************************************
Space* GameSession::FindSpaceByName(StringParam name)
{
  return mTrackedSpaces.FindValue(name, nullptr);
}

//******************************************************************************
SpaceMap::valueRange GameSession::FindAllSpacesByName(StringParam name)
{
  return mTrackedSpaces.FindAll(name);
}

//******************************************************************************
SpaceMap::valueRange GameSession::GetAllSpaces()
{
  return mTrackedSpaces.AllValues();
}

//******************************************************************************
void GameSession::RequestQuit()
{
  GameEvent event;
  event.mGame = this;
  this->DispatchEvent(Events::GameRequestQuit, &event);
  if(!event.mHandled)
    Quit();
}

//******************************************************************************
bool GameSession::IsEditorMode()
{
  return mCreationFlags.IsSet(CreationFlags::Editing);
}

//******************************************************************************
void GameSession::SetInEditor(bool inEditor)
{
  mInEditor = inEditor;
  if(!inEditor)
  {
    // Listen to events from the main window
    ConnectThisTo(mMainWindow, Events::OsClose, OnClose);
    ConnectThisTo(mMainWindow, Events::OsFocusGained, OnFocusGained);
    ConnectThisTo(mMainWindow, Events::OsFocusLost, OnFocusLost);
  }
}

//******************************************************************************
void GameSession::DispatchOnSpaces(StringParam eventName, Event* event)
{
  forRange(Space* space, mTrackedSpaces.AllValues())
      space->DispatchEvent(eventName, event);
}

//******************************************************************************
Vec2 GameSession::GetResolution()
{
  return ToVec2(mMainWindow->GetClientSize());
}

//******************************************************************************
bool GameSession::GetFullScreen()
{
  return mMainWindow->GetState() == WindowState::Fullscreen;
}

//******************************************************************************
bool GameSession::GetFocused()
{
  if(Z::gRuntimeEditor)
    return Z::gRuntimeEditor->HasFocus(this);
  else if(mMainWindow)
    return mMainWindow->HasFocus();
  return true;
}

//******************************************************************************
void GameSession::EditSpaces()
{
  if(!Z::gRuntimeEditor || IsEditorMode() || mTrackedSpaces.Empty())
    return;

  Level* editLevel = Z::gRuntimeEditor->GetEditingLevel();
  Space* focusSpace = nullptr;
  forRange(Space* space, mTrackedSpaces.AllValues())
  {
    Z::gRuntimeEditor->OpenEditorViewport(space);
    // check if the current space is the one currently being edited
    if ((Level*)space->mLevelLoaded == editLevel)
      focusSpace = space;
  }

  // set the currently being edited space as the focus, otherwise focus on the first space
  if(focusSpace)
    Z::gRuntimeEditor->SetFocus(focusSpace);
  else
    Z::gRuntimeEditor->SetFocus(mTrackedSpaces.Front().second);

  // Send this message for someone to respond to and override any of this behavior
  Event event;
  DispatchEvent(Events::EditSpaces, &event);
}

//******************************************************************************
void GameSession::OnClose(OsWindowEvent* event)
{
  // Main window requesting quit when not in the editor.
  // Call RequestQuit on the game so the response can be
  // handled in script.
  if(!mInEditor)
  {
    RequestQuit();
  }
}

//******************************************************************************
void GameSession::OnFocusLost(OsWindowEvent* event)
{
  GameEvent gameEvent;
  gameEvent.mGame = this;
  this->DispatchEvent(Events::GameFocusOut, &gameEvent);

}

//******************************************************************************
void GameSession::OnFocusGained(OsWindowEvent* event)
{
  GameEvent gameEvent;
  gameEvent.mGame = this;
  this->DispatchEvent(Events::GameFocusIn, &gameEvent);
}

//******************************************************************************
void GameSession::OnSpaceLoaded(ObjectEvent* event)
{
  Space* space = (Space*)(event->Source);

  GameEvent gameEvent;
  gameEvent.mGame = this;
  gameEvent.mSpace = space;

  // Get the currently loaded level's name
  Level* loadedLevel = space->mLevelLoaded;
  if(loadedLevel != nullptr)
    gameEvent.LevelName = loadedLevel->Name;

  this->DispatchEvent(Events::LevelStarted, &gameEvent);
  space->DispatchEvent(Events::LevelStarted, &gameEvent);
}

//******************************************************************************
void GameSession::InternalAdd(Space* space)
{
  String name = space->GetName();
  ErrorIf(mTrackedSpaces.Contains(space), "Space already added");

  mTrackedSpaces.Insert(name, space);
  ConnectThisTo(space, Events::SpaceLevelLoaded, OnSpaceLoaded);
}

//******************************************************************************
void GameSession::InternalRenamed(Space* space)
{
  mTrackedSpaces.EraseEqualValues(space);

  String newName = space->GetName();
  mTrackedSpaces.Insert(newName, space);
}

//******************************************************************************
void GameSession::InternalRemove(Space* space)
{
  mTrackedSpaces.EraseEqualValues(space);
}

}//namespace Zero
