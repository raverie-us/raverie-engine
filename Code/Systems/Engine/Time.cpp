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
  RaverieBindFieldProperty(mMinFrameRate);
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
  SerializeNameDefault(mMinFrameRate, 10.0f);
  SerializeNameDefault(mTimeScale, 1.0f);
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

  const float maxDt = mMinFrameRate <= 0.0f ? 1.0f : 1.0f / mMinFrameRate;
  mScaledClampedDt = Math::Clamp(dt, 0.0000001f, maxDt);
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

RaverieDefineType(TimeSystem, builder, type)
{
}

void TimeSystem::Initialize(SystemInitializer& initializer)
{
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

} // namespace Raverie
