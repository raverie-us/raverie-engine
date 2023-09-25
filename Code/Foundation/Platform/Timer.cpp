// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Platform/Timer.hpp"
#include <inttypes.h>
namespace Raverie
{

u64 SecondToNanosecond = 1000000000;
double NanosecondToSecond = 1.0 / double(SecondToNanosecond);

u64 GetTimeNanosecond()
{
  timespec time = {0, 0};
  clock_gettime(CLOCK_MONOTONIC, &time);
  return (u64(time.tv_sec) * SecondToNanosecond + time.tv_nsec);
}

Timer::Timer()
{
  Reset();
}

Timer::Timer(const Timer& rhs)
{
  // Our entire structure is mem-copyable
  memcpy(this, &rhs, sizeof(*this));
}

Timer::~Timer()
{
}

void Timer::Reset()
{
  mStart = GetTimeNanosecond();
  mCurrent = mStart;
  mLast = mStart;
}

void Timer::Update()
{
  mLast = mCurrent;
  mCurrent = GetTimeNanosecond();
}

double Timer::Time() const
{
  return double(mCurrent - mStart) * NanosecondToSecond;
}

double Timer::TimeDelta() const
{
  return double(mCurrent - mLast) * NanosecondToSecond;
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
  u64 current = GetTimeNanosecond();
  return double(current - mStart) * NanosecondToSecond;
}

Timer::TickType Timer::GetTickTime() const
{
  return mCurrent;
}

double Timer::TicksToSeconds(TickType ticks) const
{
  return double(ticks) * NanosecondToSecond;
}

} // namespace Raverie
