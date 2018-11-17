///////////////////////////////////////////////////////////////////////////////
///
/// \file Utilties.cpp
/// Declaration of the Os High precision Timer class.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Platform/Timer.hpp"

namespace Zero
{

u64 SecondToNanosecond = 1000000000;
double NanosecondToSecond = 1.0 / double(SecondToNanosecond);

u64 GetTimeNanosecond()
{
// Apple OSes do not implement 'clock_gettime'
#ifdef __APPLE__
  // https://developer.apple.com/library/mac/qa/qa1398/_index.html
  uint64_t time = mach_absolute_time();
  // 'AbsoluteTime' is guaranteed to be a 64 bit wide value, however it's a struct, not a primitive 64 bit value
  Nanoseconds nanoseconds = AbsoluteToNanoseconds(*(AbsoluteTime*)&time);
  return *(uint64_t *)&nanoseconds;
#else
  timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  return ( u64(time.tv_sec) * SecondToNanosecond + time.tv_nsec);
#endif
}

Timer::Timer()
{
  Reset();
}

Timer::~Timer()
{
}

struct TimerPrivateData
{
  Timer::TickType mStart;
  Timer::TickType mLast;
  Timer::TickType mCurrent;
};


void Timer::Reset()
{
  ZeroGetPrivateData(TimerPrivateData);
  self->mStart = GetTimeNanosecond();
  self->mCurrent = self->mStart;
  self->mLast = self->mStart;
}

void Timer::Update()
{
  ZeroGetPrivateData(TimerPrivateData);
  self->mLast = self->mCurrent;
  self->mCurrent = GetTimeNanosecond();
}

double Timer::Time() const
{
  ZeroGetPrivateData(TimerPrivateData);
  return double(self->mCurrent - self->mStart) * NanosecondToSecond;
}

double Timer::TimeDelta() const
{
  ZeroGetPrivateData(TimerPrivateData);
  return double(self->mCurrent - self->mLast) * NanosecondToSecond;
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
  u64 current = GetTimeNanosecond();
  return double(current - self->mStart) * NanosecondToSecond;
}

Timer::TickType Timer::GetTickTime() const
{
  ZeroGetPrivateData(TimerPrivateData);
  return self->mCurrent;
}

double Timer::TicksToSeconds(TickType ticks) const
{
  return double(ticks) * NanosecondToSecond;
}

} // namespace Zero
