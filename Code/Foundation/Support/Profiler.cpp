// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Profiler.hpp"
#include "Platform/Timer.hpp"
#include "Utility/Misc.hpp"

namespace Zero
{

namespace Profile
{

uint Record::sSampleIndex = 0;

ProfileSystem* ProfileSystem::Instance = nullptr;
void ProfileSystem::Initialize()
{
  Instance = new ProfileSystem();
  Instance->mIsRecording = false;
}

void ProfileSystem::Shutdown()
{
  SafeDelete(Instance);
}

float ProfileSystem::GetTimeInSeconds(ProfileTime time)
{
  return (float)mTimer.TicksToSeconds(time);
}

void ProfileSystem::Add(Record* record)
{
  mRecordList.PushBack(record);
}

void ProfileSystem::Add(StringParam parentName, Record* record)
{
  // if this object has a parent, then walk through the record list
  // to find the parent
  if (!parentName.Empty())
  {
    Array<Record*>::range r = mRecordList.All();
    for (; !r.Empty(); r.PopFront())
    {
      if (r.Front()->mName == parentName)
      {
        r.Front()->AddChild(record);
        break;
      }
    }
  }

  // always add this record to the record list
  mRecordList.PushBack(record);
}

ProfileTime ProfileSystem::GetTime()
{
  // TODO: Fix for multiple threads. (Thread local storage?)
  return mTimer.GetTickTime();
}

void ProfileSystem::BeginTracing()
{
  if (mIsRecording)
  {
    return;
  }
  ZPrint("Tracing begun\n");
  mTraceEventsLock.Lock();
  mTraceEvents.Clear();
  mTraceEvents.Reserve(10000);
  mTraceEventsLock.Unlock();
  mIsRecording = true;
}

void ProfileSystem::EndTracing(Array<TraceEvent>& output)
{
  if (!mIsRecording)
  {
    ZPrint("Cannot end tracing since it was never started\n");
    return;
  }
  mIsRecording = false;
  mTraceEventsLock.Lock();
  output.Swap(mTraceEvents);
  mTraceEvents.Clear();
  mTraceEventsLock.Unlock();
  ZPrint("Tracing ended\n");
}

Record::Record(void)
{
  mParent = nullptr;
  mColor = 0xFFFFFFFF;
  Clear();
}

Record::Record(StringParam name)
{
  Initialize(name, String(), 0xFFFFFFFF);
}

Record::Record(StringParam name, StringParam parentName, u32 color)
{
  Initialize(name, parentName, color);
}

void Record::Initialize(StringParam name, StringParam parentName, u32 color)
{
  mColor = color;
  mParent = nullptr;
  mName = name;
  ProfileSystem::Instance->Add(parentName, this);
  Clear();
}

void Record::Clear()
{
  mTotalTime = 0;
  mMaxTime = 0;

  mSmoothAvg = 0.0f;
  mOldTicks = 0;

  for (uint i = 0; i < cSampleCount; ++i)
    mSamples[i] = 0.0f;
  mRunningSample = 0.0f;
}

void Record::Update()
{
  ProfileTime delta = mTotalTime - mOldTicks;
  mInstantAvg = (float)delta;

  // Better way of smoothing?
  float newAvg = mSmoothAvg * 0.90f + 0.1f * mInstantAvg;
  mSmoothAvg = newAvg;

  RangeType range = GetChildren();
  while (!range.Empty())
  {
    range.Front().Update();
    range.PopFront();
  }

  mOldTicks = mTotalTime;

  mRunningSample += mInstantAvg;
  ++mRunningCount;
}

void Record::AverageRunningSample()
{
  mSamples[sSampleIndex] = mRunningSample / (float)(mRunningCount != 0 ? mRunningCount : 1);
  mRunningSample = 0.0f;
  mRunningCount = 0;

  forRange (Record& record, GetChildren())
    record.AverageRunningSample();
}

float Record::Average()
{
  return mInstantAvg;
}

float Record::SmoothAverage()
{
  return mSmoothAvg;
}

void Record::SetName(StringParam name)
{
  mName = name;
}

void Record::AddChild(Record* record)
{
  record->mParent = this;
  mChildren.PushBack(record);
}

void Record::EnterRecord(ProfileTime time)
{
  ++mHits;
  mTotalTime += time;
  // update the max time that was ever spent in this record.
  if (time > mMaxTime)
  {
    mMaxTime = time;
    // if(mInstantAvg != 0.0f && 3.0f * mInstantAvg < (float)time)
    //  DebugPrint("%s has an average of %g and spike with
    //  %g\n",mName,mInstantAvg,(float)time);
  }
}

ScopeTimer::ScopeTimer(Record* data, String args)
{
  mData = data;
  mStartTime = ProfileSystem::Instance->GetTime();
  mArgs = args;
}

ScopeTimer::~ScopeTimer()
{
  ProfileSystem* system = ProfileSystem::Instance;
  ProfileTime endTime = system->GetTime();
  ProfileTime duration = endTime - mStartTime;
  mData->EnterRecord(duration);

  if (system->mIsRecording && duration != 0)
  {
    TraceEvent event;
    if (mData->mParent)
      event.mCategory = mData->mParent->mName;
    event.mName = mData->mName;
    event.mArgs = mArgs;
    event.mThreadId = Thread::GetCurrentThreadId();
    event.mTimestamp = mStartTime;
    event.mDuration = duration;

    system->mTraceEventsLock.Lock();
    system->mTraceEvents.PushBack(event);
    system->mTraceEventsLock.Unlock();
  }
}

void PrintProfileGraph(Record* record, double total, int level)
{
  for (int i = 0; i < level; ++i)
    DebugPrint("\t");

  DebugPrint("%s : %3.2f\n", record->GetName().c_str(), ((double)record->GetTotalTime() / total) * 100.0);
  InListBaseLink<Profile::Record>::range records = record->GetChildren();

  while (!records.Empty())
  {
    Profile::Record& r = records.Front();
    PrintProfileGraph(&r, (double)record->GetTotalTime(), level + 1);
    records.PopFront();
  }
}

void PrintProfileGraph()
{
  Array<Profile::Record*>::range records = Profile::ProfileSystem::Instance->GetRecords();
  Record* root = records.Front();
  PrintProfileGraph(root, (double)root->GetTotalTime(), 0);
}

} // namespace Profile
} // namespace Zero
