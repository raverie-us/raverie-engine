///////////////////////////////////////////////////////////////////////////////
///
/// \file Time.hpp
/// Declaration of time system and time space component.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(SystemLogicUpdate);
  DeclareEvent(FrameUpdate);
  //DeclareEvent(GraphicsFrameUpdate);
  DeclareEvent(LogicUpdate);
  DeclareEvent(EngineUpdate);
  DeclareEvent(EngineShutdown);
  DeclareEvent(ActionFrameUpdate);
  DeclareEvent(ActionLogicUpdate);
}

class TimeSystem;
class TimeConfig;
System* CreateTimeSystem();

//----------------------------------------------------------------- Update Event

/// Update event Contains current time and delta time.
class UpdateEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  UpdateEvent(float dt, float rDt, float timePassed, float realTimePassed);

  /// The amount of time that passed between this frame and the last (scaled by TimeSpace.TimeScale).
  float Dt;
  /// The real amount of time that passed between this frame and the last (unscaled and unclamped).
  float RealDt;
  /// The amount of time that has passed since this space was created (TimeSpace.TimeScale affects this value).
  float TimePassed;
  /// The real amount of time that has passed since this space was created (unscaled and unclamped).
  float RealTimePassed;
};

DeclareEnum2(TimeMode, FixedFrametime, ActualFrametime);

/// Time space component controls time for a Space.
class TimeSpace : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  TimeSpace();
  ~TimeSpace();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  void Update(float dt);

  /// Toggles the state of paused.
  void TogglePause();
  void SetPaused(bool state);

  /// Allows the engine to be advance one frame forward.
  /// Useful for debugging one frame at a time.
  void Step();

  // Gets the current frame time, or zero if the time space is paused
  // This dt will include frame time 
  float GetDtOrZero();

  // Returns true if either the space is paused, or the game is globally paused
  bool GetGloballyPaused();

  /// The events that the time space sends out
  String mSystemLogicUpdateEvent;
  String mFrameUpdateEvent;
  String mLogicUpdateEvent;
  String mActionFrameUpdateEvent;
  String mActionLogicUpdateEvent;

  /// The maximum amount of time we send when running in 'ActualFrametime' mode
  /// If this value is set too high and the user does anything to pause their system or the game (example grabbing the window)
  /// then a large frame time will be sent out and physics objects will jump very far (causing tunneling and random bounces)
  float mMaxDt;

  /// The minimum amount of time we send when running in 'ActualFrametime' mode
  /// Ideally this is set to a very small non-zero value to prevent any division by zero errors
  float mMinDt;

  /// Scale the speed of time for interesting effects like bullet time or fast paced gameplay
  float mTimeScale;

  /// If the time space is paused then we cease sending out logic update events
  /// When paused, the Dt will remain at whatever it was (it will NOT be set to 0)
  bool mPaused;

  /// When set to fixed framerate the Dt/frame time will never change (it will send whatever the project frame-rate-limiter is set to)
  /// This means it is important to run with a frame-rate limiter of some kind otherwise the game will appear to run much faster/slower
  /// Note: For determinism, you should always run in FixedFrametime mode
  /// When set to actual framerate we will send out the real time that the engine is encountering (clamped by MinDt / MaxDt)
  TimeMode::Enum mTimeMode;

  // Note: These only exist because meta cannot bind member enums!
  /// When set to fixed framerate the Dt/frame time will never change (it will send whatever the project frame-rate-limiter is set to)
  /// This means it is important to run with a frame-rate limiter of some kind otherwise the game will appear to run much faster/slower
  /// Note: For determinism, you should always run in FixedFrametime mode
  /// When set to actual framerate we will send out the real time that the engine is encountering (clamped by MinDt / MaxDt)
  TimeMode::Enum GetTimeMode() const;
  void SetTimeMode(TimeMode::Enum value);

  /// The current frame we are on (starts at 0 and counts up for every frame that is run)
  /// This value counts up regardless of if the space is paused
  int mFrame;

  float mRealTimePassed;
  float mScaledClampedTimePassed;
  
  float mRealDt;
  float mScaledClampedDt;

  /// Causes the engine to update multiple times before rendering a frame.
  uint mStepCount;

  //Internals
  Link<TimeSpace> link;
  TimeSystem* mTimeSystem;
};

///Time system updates all time spaces.
class TimeSystem : public System
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  TimeSystem();
  ~TimeSystem();

  void Initialize(SystemInitializer& initializer) override;
  
  // System Interface
  void Update() override;
  cstr GetName() override { return "TimeSystem"; }

  float GetTargetDt() const;

  void OnProjectLoaded(ObjectEvent* event);
  void OnProjectCogModified(Event* event);

  typedef InList<TimeSpace> TimeSpaceList;
  TimeSpaceList List;

  TimeConfig* mTimeConfig;

  float mEngineDt;
  double mEngineRuntime;

  HandleOf<Cog> mProjectCog;
  bool mLimitFrameRate;
  uint mFrameRate;

private:
  //Main system timer
  Timer mTimer;
};

}//namespace Zero
