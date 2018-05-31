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
}

Timer::~Timer()
{
  Error("Not implemented");
}

Timer::Timer(const Timer& rhs)
{
  Error("Not implemented");
}

Timer& Timer::operator=(const Timer& rhs)
{
  Error("Not implemented");
  return *this;
}

void Timer::Reset()
{
  Error("Not implemented");
}

void Timer::Update()
{
  Error("Not implemented");
}

double Timer::Time() const
{
  Error("Not implemented");
  return 0.0;
}

double Timer::TimeDelta() const
{
  Error("Not implemented");
  return 0.0;
}

double Timer::UpdateAndGetTime()
{
  Error("Not implemented");
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
  Error("Not implemented");
  return 0.0;
}

Timer::TickType Timer::GetTickTime() const
{
  Error("Not implemented");
  return 0;
}

double Timer::TicksToSeconds(TickType ticks) const
{
  Error("Not implemented");
  return 0.0;
}

}//namespace Zero
