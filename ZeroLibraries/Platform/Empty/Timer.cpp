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
struct TimerPrivateData
{
};

Timer::Timer()
{
  Error("Not implemented");
  ZeroConstructPrivateData(TimerPrivateData);
}

Timer::~Timer()
{
  ZeroGetPrivateData(TimerPrivateData);
  // Destruction logic
  ZeroDestructPrivateData(TimerPrivateData);
}

void Timer::Reset()
{
  ZeroGetPrivateData(TimerPrivateData);
}

void Timer::Update()
{
  ZeroGetPrivateData(TimerPrivateData);
}

double Timer::Time() const
{
  ZeroGetPrivateData(TimerPrivateData);
  return 0.0;
}

double Timer::TimeDelta() const
{
  ZeroGetPrivateData(TimerPrivateData);
  return 1.0 / 60.0;
}

double Timer::UpdateAndGetTime()
{
  ZeroGetPrivateData(TimerPrivateData);
  return 0.0;
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
  return 0.0;
}

Timer::TickType Timer::GetTickTime() const
{
  ZeroGetPrivateData(TimerPrivateData);
  return 0;
}

double Timer::TicksToSeconds(TickType ticks) const
{
  ZeroGetPrivateData(TimerPrivateData);
  return 0.0;
}

}//namespace Zero
