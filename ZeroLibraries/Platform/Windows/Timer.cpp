///////////////////////////////////////////////////////////////////////////////
///
/// \file Timer.cpp
/// Implementation of the Os Timer class.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

struct TimerPrivateData
{
  long long mStartCount;
  long long mCurrentTickCount;
  long long mCurrentTickDelta;
  double mFrequency;
  double mCurTimeDelta;
  double mCurTime;
};

Timer::Timer()
{
  ZeroConstructPrivateData(TimerPrivateData);

  LARGE_INTEGER  Frequency;
  QueryPerformanceFrequency(&Frequency);
  //mInvFrequency = 1.0 / (double)Frequency.QuadPart;
  self->mFrequency = (double)Frequency.QuadPart;
  Reset();
}

Timer::~Timer()
{
  ZeroDestructPrivateData(TimerPrivateData);
}

void Timer::Reset()
{
  ZeroGetPrivateData(TimerPrivateData);
  LARGE_INTEGER Count;
  QueryPerformanceCounter(&Count);
  self->mStartCount = Count.QuadPart;

  self->mCurrentTickCount = 0;
  self->mCurrentTickDelta = 0;
  self->mCurTimeDelta = 0;
  self->mCurTime = 0;
}

void Timer::Update()
{
  ZeroGetPrivateData(TimerPrivateData);
  TickType lastTickCount = self->mCurrentTickCount;
  LARGE_INTEGER CurrCount;
  QueryPerformanceCounter(&CurrCount);
  self->mCurrentTickCount = (CurrCount.QuadPart - self->mStartCount);
  self->mCurrentTickDelta = self->mCurrentTickCount - lastTickCount;

  //mCurTime =  mCurrentTickCount * mInvFrequency;
  self->mCurTime =  self->mCurrentTickCount / self->mFrequency;
  //mCurTimeDelta = mCurrentTickDelta * mInvFrequency;
  self->mCurTimeDelta = self->mCurrentTickDelta / self->mFrequency;
}

double Timer::Time() const
{
  ZeroGetPrivateData(TimerPrivateData);
  return self->mCurTime;
}

double Timer::TimeDelta() const
{
  ZeroGetPrivateData(TimerPrivateData);
  return self->mCurTimeDelta;
}

double Timer::UpdateAndGetTime()
{
  Update();
  return Time();
}

TimeMs Timer::TimeMilliseconds() const
{
  return TimeMs(Time() * double(1000));
}

TimeMs Timer::TimeDeltaMilliseconds() const
{
  return TimeMs(TimeDelta() * double(1000));
}

TimeMs Timer::UpdateAndGetTimeMilliseconds()
{
  return TimeMs(UpdateAndGetTime() * double(1000));
}

double Timer::TimeNoUpdate() const
{
  ZeroGetPrivateData(TimerPrivateData);
  LARGE_INTEGER CurrCount;
  QueryPerformanceCounter(&CurrCount);
  TickType TempTicks  = (CurrCount.QuadPart - self->mStartCount);
  //return (double)(TempTicks * mInvFrequency);
  return (double)(TempTicks / self->mFrequency);
}

Timer::TickType Timer::GetTickTime() const
{
  ZeroGetPrivateData(TimerPrivateData);
  LARGE_INTEGER CurrCount;
  QueryPerformanceCounter(&CurrCount);
  return CurrCount.QuadPart - self->mStartCount;
}

double Timer::TicksToSeconds(TickType ticks) const
{
  ZeroGetPrivateData(TimerPrivateData);
  //return (double)(ticks* mInvFrequency);
  return (double)(ticks / self->mFrequency);
}

} // namespace Zero
