// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace Events
{
DeclareEvent(SystemLogicUpdate);
DeclareEvent(FrameUpdate);
// DeclareEvent(GraphicsFrameUpdate);
DeclareEvent(LogicUpdate);
DeclareEvent(PreviewUpdate);
DeclareEvent(EngineUpdate);
DeclareEvent(EngineDebuggerUpdate);
DeclareEvent(EngineShutdown);
DeclareEvent(ActionFrameUpdate);
DeclareEvent(ActionLogicUpdate);
} // namespace Events

extern const float cFixedDt;

class TimeSystem;
class TimeConfig;
System* CreateTimeSystem();

/// Update event Contains current time and delta time.
class UpdateEvent : public Event
{
public:
  RaverieDeclareType(UpdateEvent, TypeCopyMode::ReferenceType);

  UpdateEvent(float dt, float rDt, float timePassed, float realTimePassed);

  /// The amount of time that passed between this frame and the last (scaled by
  /// TimeSpace.TimeScale).
  float Dt;
  /// The real amount of time that passed between this frame and the last
  /// (unscaled and unclamped).
  float RealDt;
  /// The amount of time that has passed since this space was created
  /// (TimeSpace.TimeScale affects this value).
  float TimePassed;
  /// The real amount of time that has passed since this space was created
  /// (unscaled and unclamped).
  float RealTimePassed;
};

/// Time space component controls time for a Space.
class TimeSpace : public Component
{
public:
  RaverieDeclareType(TimeSpace, TypeCopyMode::ReferenceType);

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

  /// Gets the current frame time, or zero if the time space is paused.
  /// This dt will include frame time.
  float GetDtOrZero();

  /// Returns true if either the space is paused, or the game is globally paused
  bool GetGloballyPaused();

  /// Scale the speed of time for interesting effects like bullet time or
  /// fast paced gameplay. TimeScale is clamped to be positive.
  float GetTimeScale();
  void SetTimeScale(float timeScale);

  /// The minimum frame rate we send to the game.
  /// If this value is set too low and the user does anything to pause their
  /// system or the game (example grabbing the window) then a large frame time
  /// will be sent out and physics objects will jump very far (causing tunneling
  /// and random bounces)
  float mMinFrameRate;

  /// Scale the speed of time for interesting effects like bullet time or fast
  /// paced gameplay
  float mTimeScale;

  /// If the time space is paused then we cease sending out logic update events
  /// When paused, the Dt will remain at whatever it was (it will NOT be set to
  /// 0)
  bool mPaused;

  /// The current frame we are on (starts at 0 and counts up for every frame
  /// that is run) This value counts up regardless of if the space is paused
  int mFrame;

  /// The real amount of time that has passed since this space was created
  /// (unscaled and unclamped).
  float mRealTimePassed;
  /// The amount of time that has passed since this space was created
  /// (TimeSpace.TimeScale affects this value).
  float mScaledClampedTimePassed;

  /// The real amount of time that passed between this frame and the last
  /// (unscaled and unclamped).
  float mRealDt;
  /// The amount of time that passed between this frame and the last (scaled by
  /// TimeSpace.TimeScale).
  float mScaledClampedDt;

  /// Causes the engine to update multiple times before rendering a frame.
  uint mStepCount;

  // Internals
  Link<TimeSpace> link;
  TimeSystem* mTimeSystem;
};

/// Time system updates all time spaces.
class TimeSystem : public System
{
public:
  RaverieDeclareType(TimeSystem, TypeCopyMode::ReferenceType);

  TimeSystem();
  ~TimeSystem();

  void Initialize(SystemInitializer& initializer) override;

  // System Interface
  void Update(bool debugger) override;
  cstr GetName() override
  {
    return "TimeSystem";
  }

  typedef InList<TimeSpace> TimeSpaceList;
  TimeSpaceList List;

  TimeConfig* mTimeConfig;

  float mEngineDt;
  double mEngineRuntime;

  HandleOf<Cog> mProjectCog;

private:
  // Main system timer
  Timer mTimer;
};

} // namespace Raverie
