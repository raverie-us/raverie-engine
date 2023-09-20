// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
DefineEvent(SystemLogicUpdate);
DefineEvent(FrameUpdate);
DefineEvent(GraphicsFrameUpdate);
DefineEvent(LogicUpdate);
DefineEvent(PreviewUpdate);
DefineEvent(EngineUpdate);
DefineEvent(EngineDebuggerUpdate);
DefineEvent(EngineShutdown);
DefineEvent(ActionFrameUpdate);
DefineEvent(ActionLogicUpdate);
} // namespace Events

const float cFixedDt = (1.0f / 60.0f);

System* CreateTimeSystem()
{
  return new TimeSystem();
}

RaverieDefineType(UpdateEvent, builder, type)
{
  RaverieBindDocumented();
  RaverieBindFieldProperty(Dt);
  RaverieBindFieldProperty(TimePassed);
  RaverieBindFieldProperty(RealTimePassed);
}

UpdateEvent::UpdateEvent(float dt, float realDt, float timePassed, float realTimePassed) : Dt(dt), RealDt(realDt), TimePassed(timePassed), RealTimePassed(realTimePassed)
{
}

RaverieDefineType(TimeSpace, builder, type)
{
  RaverieBindComponent();
  type->AddAttribute(ObjectAttributes::cCore);
  RaverieBindDocumented();
  RaverieBindSetup(SetupMode::DefaultSerialization);

  RaverieBindEvent(Events::LogicUpdate, UpdateEvent);
  RaverieBindEvent(Events::FrameUpdate, UpdateEvent);
  RaverieBindEvent(Events::PreviewUpdate, UpdateEvent);

  RaverieBindDependency(Space);

  RaverieBindMethod(TogglePause);
  RaverieBindMethod(Step);
  RaverieBindGetter(GloballyPaused);
  RaverieBindGetter(DtOrZero);
  RaverieBindGetterSetterProperty(TimeMode);
  RaverieBindFieldProperty(mMinDt);
  RaverieBindFieldProperty(mMaxDt);
  RaverieBindGetterSetterProperty(TimeScale);
  RaverieBindFieldProperty(mPaused);
  RaverieBindFieldGetterAs(mScaledClampedDt, "Dt");
  RaverieBindFieldGetter(mRealDt);
  RaverieBindFieldAs(mScaledClampedTimePassed, "TimePassed");
  RaverieBindField(mRealTimePassed);
  RaverieBindField(mFrame);
  RaverieBindFieldProperty(mStepCount);
}

TimeSpace::TimeSpace()
{
  mTimeSystem = NULL;

  mPaused = false;
  mFrame = 0;

  mRealTimePassed = 0.0;
  mScaledClampedTimePassed = 0.0;

  mRealDt = cFixedDt;
  mScaledClampedDt = cFixedDt;
}

TimeSpace::~TimeSpace()
{
  mTimeSystem->List.Erase(this);
}

void TimeSpace::Serialize(Serializer& stream)
{
  SerializeNameDefault(mMaxDt, 0.3f);
  SerializeNameDefault(mMinDt, 0.000001f);
  SerializeNameDefault(mTimeScale, 1.0f);
  SerializeEnumNameDefault(TimeMode, mTimeMode, TimeMode::FixedFrametime);
  SerializeNameDefault(mStepCount, (uint)1);
}

void TimeSpace::Initialize(CogInitializer& initializer)
{
  mTimeSystem = Z::gEngine->has(TimeSystem);
  mTimeSystem->List.PushBack(this);
}

float TimeSpace::GetDtOrZero()
{
  if (GetGloballyPaused())
    return 0.0f;
  return mScaledClampedDt;
}

bool TimeSpace::GetGloballyPaused()
{
  if (GameSession* gameSession = this->GetGameSession())
    return mPaused || gameSession->mPaused;
  return mPaused;
}

float TimeSpace::GetTimeScale()
{
  return mTimeScale;
}

void TimeSpace::SetTimeScale(float timeScale)
{
  mTimeScale = Math::Max(timeScale, 0.0f);
}

void TimeSpace::Update(float dt)
{
  Space* space = GetSpace();

  // If this is a preview space and we're sending out update (should include any
  // special preview update) we don't want notifications to happen which we're
  // previewing, as its very annoying to the user
  NotificationCallback notify = nullptr;
  if (space->IsPreviewMode())
    notify = IgnoreDoNotify;
  TemporaryDoNotifyOverride doNotifyOverride(notify);
  Array<NotificationCallback>& cb = Z::gNotifyCallbackStack;
  space->CheckForChangedObjects();

  // push on id of this space for the debug drawer so anyone who debug
  // draws will by default be in the correct space.
  // Debug::DefaultConfig config;
  // config.SpaceId(this->GetOwner()->GetId().Id);
  Debug::ActiveDrawSpace drawSpace(GetOwner()->GetId().Id);

  mRealDt = dt;

  if (mTimeMode == TimeMode::FixedFrametime)
  {
    mScaledClampedDt = 1.0f / mTimeSystem->mFrameRate;
  }
  else
  {
    if (dt > mMaxDt)
      mScaledClampedDt = mMaxDt;
    else if (dt < mMinDt)
      mScaledClampedDt = mMinDt;
    else
      mScaledClampedDt = dt;
  }

  mScaledClampedDt *= mTimeScale;

  // We don't want the engine to lock down, so cap it at 60 steps per update
  mStepCount = Math::Clamp(mStepCount, 1u, 60u);

  for (uint i = 0; i < mStepCount; ++i)
  {
    ++mFrame;

    mRealTimePassed += mRealDt;
    mScaledClampedTimePassed += mScaledClampedDt;

    EventDispatcher* dispatcher = GetOwner()->GetDispatcher();
    UpdateEvent updateEvent(mScaledClampedDt, mRealDt, mScaledClampedTimePassed, mRealTimePassed);

    {
      ProfileScopeTree("FrameUpdate", "TimeSystem", Color::PaleGoldenrod);
      dispatcher->Dispatch(Events::FrameUpdate, &updateEvent);
    }

    {
      ProfileScopeTree("ActionFrameUpdateEvent", "TimeSystem", Color::BlueViolet);
      dispatcher->Dispatch(Events::ActionFrameUpdate, &updateEvent);
    }

    if (space->IsPreviewMode())
    {
      ProfileScopeTree("PreviewUpdateEvent", "TimeSystem", Color::Gainsboro);
      dispatcher->Dispatch(Events::PreviewUpdate, &updateEvent);
    }

    if (!GetGloballyPaused())
      Step();

    {
      // ProfileScopeTree("GraphicsFrameUpdate", "TimeSystem", Color::SkyBlue);
      // dispatcher->Dispatch(Events::GraphicsFrameUpdate, &updateEvent);
    }
  }
}

void TimeSpace::TogglePause()
{
  mPaused = !mPaused;
}

void TimeSpace::SetPaused(bool state)
{
  mPaused = state;
}

void TimeSpace::Step()
{
  EventDispatcher* dispatcher = GetOwner()->GetDispatcher();
  UpdateEvent updateEvent(mScaledClampedDt, mRealDt, mScaledClampedTimePassed, mRealTimePassed);

  {
    ProfileScopeTree("SystemLogicUpdate", "TimeSystem", Color::RoyalBlue);
    dispatcher->Dispatch(Events::SystemLogicUpdate, &updateEvent);
  }

  {
    ProfileScopeTree("LogicUpdate", "TimeSystem", Color::Gainsboro);
    dispatcher->Dispatch(Events::LogicUpdate, &updateEvent);
  }

  {
    ProfileScopeTree("ActionLogicUpdateEvent", "TimeSystem", Color::BlanchedAlmond);
    dispatcher->Dispatch(Events::ActionLogicUpdate, &updateEvent);
  }
}

TimeMode::Enum TimeSpace::GetTimeMode() const
{
  return mTimeMode;
}

void TimeSpace::SetTimeMode(TimeMode::Enum value)
{
  mTimeMode = value;
}

RaverieDefineType(TimeSystem, builder, type)
{
}

void TimeSystem::Initialize(SystemInitializer& initializer)
{
  ConnectThisTo(Z::gEngine, Events::ProjectLoaded, OnProjectLoaded);
  mLimitFrameRate = true;
  mFrameRate = 60;
}

TimeSystem::TimeSystem()
{
  mEngineDt = 0.0f;
  mEngineRuntime = 0.0;
}

TimeSystem::~TimeSystem()
{
}

void TimeSystem::Update(bool debugger)
{
  mTimer.Update();
  float dt = (float)mTimer.TimeDelta();

  // The frame rate is normally limited by graphics vertical sync
  // but on some systems the vertical sync is disabled by the driver or the
  // user. Instead of wastefully drawing at maximum speed this try to sleep for
  // the the rest of the frame. This reduces heat and power use on laptops.
  if (mLimitFrameRate)
  {
    ProfileScopeTree("Limiter", "Engine", Color::Green);
    const int limitError = 1;
    const int limitframeTimeMs = int(1.0f / float(mFrameRate) * 1000.0f);
    int frameTime = int(dt * 1000.0f) + limitError;
    if (frameTime < limitframeTimeMs)
    {
      // Compute amount of time left in the frame
      int left = limitframeTimeMs - frameTime;
      Os::Sleep(left);

      // Spin for the last ~1ms
      float limitTimeSeconds = 1.0f / mFrameRate;
      while (dt < limitTimeSeconds)
      {
        mTimer.Update();
        dt += (float)mTimer.TimeDelta();
      }
    }
  }

  ProfileScopeTree("TimeSystem", "Engine", Color::Orange);

  mEngineDt = dt;
  mEngineRuntime = mTimer.Time();

  if (!debugger)
  {
    // Update every space in the engine
    TimeSpaceList::range range = List.All();
    for (; !range.Empty(); range.PopFront())
    {
      TimeSpace& timeSpace = range.Front();
      timeSpace.Update(dt);
    }
  }

  UpdateEvent uiUpdate(dt, 0, 0, 0);
  GetDispatcher()->Dispatch("UiUpdate", &uiUpdate);
}

float TimeSystem::GetTargetDt() const
{
  return 1.0f / mFrameRate;
}

void TimeSystem::OnProjectLoaded(ObjectEvent* event)
{
  if (mProjectCog.IsNotNull())
    DisconnectAll(mProjectCog, this);

  mProjectCog = (Cog*)event->GetSource();
  ConnectThisTo(*mProjectCog, Events::ObjectModified, OnProjectCogModified);
  OnProjectCogModified(event);
}

void TimeSystem::OnProjectCogModified(Event* event)
{
  if (FrameRateSettings* frameRateSettings = mProjectCog.has(FrameRateSettings))
  {
    mLimitFrameRate = frameRateSettings->mLimitFrameRate;
    mFrameRate = frameRateSettings->mFrameRate;
  }
  else
  {
    mLimitFrameRate = true;
    mFrameRate = 60;
  }
}

} // namespace Raverie
