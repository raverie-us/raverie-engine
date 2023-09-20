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

struct TimerPrivateData
{
  Timer::TickType mStart;
  Timer::TickType mLast;
  Timer::TickType mCurrent;
};

void Timer::Reset()
{
  RaverieGetPrivateData(TimerPrivateData);
  self->mStart = GetTimeNanosecond();
  self->mCurrent = self->mStart;
  self->mLast = self->mStart;
}

void Timer::Update()
{
  RaverieGetPrivateData(TimerPrivateData);
  self->mLast = self->mCurrent;
  self->mCurrent = GetTimeNanosecond();
}

double Timer::Time() const
{
  RaverieGetPrivateData(TimerPrivateData);
  return double(self->mCurrent - self->mStart) * NanosecondToSecond;
}

double Timer::TimeDelta() const
{
  RaverieGetPrivateData(TimerPrivateData);
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
  RaverieGetPrivateData(TimerPrivateData);
  u64 current = GetTimeNanosecond();
  return double(current - self->mStart) * NanosecondToSecond;
}

Timer::TickType Timer::GetTickTime() const
{
  RaverieGetPrivateData(TimerPrivateData);
  return self->mCurrent;
}

double Timer::TicksToSeconds(TickType ticks) const
{
  return double(ticks) * NanosecondToSecond;
}

} // namespace Raverie
