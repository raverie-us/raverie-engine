// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Platform/Timer.hpp"

namespace Zero
{
struct TimerPrivateData
{
};

Timer::Timer()
{
}

Timer::~Timer()
{
}

Timer::Timer(const Timer& rhs)
{
}

Timer& Timer::operator=(const Timer& rhs)
{
  return *this;
}

void Timer::Reset()
{
}

void Timer::Update()
{
}

double Timer::Time() const
{
  return 0.0;
}

double Timer::TimeDelta() const
{
  return 1.0 / 60.0;
}

double Timer::UpdateAndGetTime()
{
  return 1.0 / 60.0;
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
  return 0.0;
}

Timer::TickType Timer::GetTickTime() const
{
  return 0;
}

double Timer::TicksToSeconds(TickType ticks) const
{
  return (double)ticks;
}

} // namespace Zero
