///////////////////////////////////////////////////////////////////////////////
///
/// \file Profiler.cpp
/// Implementation of the Profile Record, Manager, and Block class.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
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

void ProfileSystem::Add(cstr parentName, Record* record)
{
  Array<Record*>::range r = mRecordList.All();
  //if this object has a parent, then walk through the record list
  //to find the parent
  if(parentName != nullptr)
  {
    for(;!r.Empty();r.PopFront())
    {
      if(strcmp(r.Front()->mName,parentName)==0)
      {
        r.Front()->AddChild(record);
        break;
      }
    }
  }

  //always add this record to the record list
  mRecordList.PushBack(record);
}

ProfileTime ProfileSystem::GetTime()
{
  //TODO: Fix for multiple threads. (Thread local storage?)
  return mTimer.GetTickTime();
}

Record::Record(void)
{
  mName = nullptr;
  mParent = nullptr;
  mColor = 0xFFFFFFFF;
  Clear();
}

Record::Record(cstr name)
{ 
  Initialize(name, nullptr, 0xFFFFFFFF);
}

Record::Record(cstr name, cstr parentName, u32 color)
{
  Initialize(name, parentName, color);
}

void Record::Initialize(cstr name, cstr parentName, u32 color)
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
  mMaxTime =0;

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

  //Better way of smoothing?
  float newAvg = mSmoothAvg * 0.90f + 0.1f * mInstantAvg;
  mSmoothAvg = newAvg;

  RangeType range = GetChildren();
  while(!range.Empty())
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

  forRange(Record& record, GetChildren())
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

void Record::SetName(cstr name)
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
  mTotalTime+=time;
  //update the max time that was ever spent in this record.
  if(time > mMaxTime)
  {
    mMaxTime = time;
    //if(mInstantAvg != 0.0f && 3.0f * mInstantAvg < (float)time)
    //  DebugPrint("%s has an average of %g and spike with %g\n",mName,mInstantAvg,(float)time);
  }
}

ScopeTimer::ScopeTimer(Record* data)
{
  mData = data;
  mStartTime = ProfileSystem::Instance->GetTime();
}

ScopeTimer::~ScopeTimer()
{
  ProfileTime endTime = ProfileSystem::Instance->GetTime();
  mData->EnterRecord(endTime-mStartTime);
}


void PrintProfileGraph(Record* record, double total, int level)
{
  for(int i=0;i<level;++i)
    DebugPrint("\t");

  DebugPrint("%s : %3.2f\n", record->GetName(), ((double)record->GetTotalTime() / total) * 100.0);
  InListBaseLink<Profile::Record>::range records = record->GetChildren();
  double totalTime = 0.0f;
  while(!records.Empty())
  {
    Profile::Record& r = records.Front();
    PrintProfileGraph(&r, (double)record->GetTotalTime(), level+1);
    records.PopFront();
  }
}

void PrintProfileGraph()
{
  Array<Profile::Record*>::range records = Profile::ProfileSystem::Instance->GetRecords();
  Record* root = records.Front();
  PrintProfileGraph(root, (double)root->GetTotalTime(), 0);

}

}//namespace Profiler
}//namespace Zero
