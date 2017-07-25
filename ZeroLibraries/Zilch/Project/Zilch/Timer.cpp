/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  const long long Timer::TicksPerSecond = CLOCKS_PER_SEC;

  //***************************************************************************
  Timer::Timer() :
    TotalTicks(0),
    LastClock(Zero::Time::Clock())
  {
  }

  //***************************************************************************
  long long Timer::GetAndUpdateTicks()
  {
    // Get the current clock tick count (it may have wrapped around / overflowed!)
    long long currentClock = Zero::Time::Clock();

    // If the current value we sampled is greater than the last value, there's a good chance
    // that we didn't wrap around. The only time this could fail is if this function only gets
    // called once every 500 hours or so, giving it time to wrap back up to its last value
    if (currentClock >= this->LastClock)
    {
      // Just Append the difference in ticks
      long long difference = currentClock - this->LastClock;
      this->TotalTicks += difference;

      // Because of wrap around, we need this timer to be updated periodicially (on a regular basis)
      // We'll basically throw an assert/warning if the user only calls this once an hour or longer
      const long long SecondsPerMinute = 60;
      const long long MinutesPerHour = 60;
      const long long HoursPerDay = 24;
      ErrorIf(difference > Timer::TicksPerSecond * SecondsPerMinute * MinutesPerHour * HoursPerDay,
        "The timer should be called at least once a day to prevent wrap around issues");
    }
    else
    {
      // Otherwise, we probably just wrapped around so we need to figure out how much we added
      // The amount that was added is the current clock value minus the bottom value (since it wrapped around)
      // plus the difference between the last clock value and the max, where it would have wrapped
      // We add 1 because the wrap itself actually represents an increment of 1
      this->TotalTicks += currentClock - numeric_limits<clock_t>::min();
      this->TotalTicks += numeric_limits<clock_t>::max() - this->LastClock;
      this->TotalTicks += 1;
    }

    // Store the last clock, used for resolving time
    this->LastClock = currentClock;

    // Return the ticks we've acounted for
    return this->TotalTicks;
  }
  
  //***************************************************************************
  void Timer::Reset()
  {
    // Just clear the tick count
    this->TotalTicks = 0;
  }
}
