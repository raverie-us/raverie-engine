///////////////////////////////////////////////////////////////////////////////
///
/// \file Timer.hpp
/// Declaration of the Os High precision Timer class.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "PrivateImplementation.hpp"
#include <limits>

namespace Zero
{

/// Time in milliseconds
/// (Intentionally signed to allow for negative time values)
typedef s64 TimeMs;

/// Constants
static const TimeMs cOneSecondTimeMs = TimeMs(1000);
static const TimeMs cInfiniteTimeMs  = std::numeric_limits<TimeMs>::max();

/// Converts a rate (hertz) to an interval (milliseconds)
#define RATE_TO_INTERVAL(Rate) TimeMs((double(1) / double(Rate)) * cOneSecondTimeMs)

/// Returns the duration between start and end
ZeroShared inline TimeMs GetDuration(TimeMs start, TimeMs end)
{
  return end - start;
}

/// Converts milliseconds (TimeMs) to seconds (float)
ZeroShared inline float TimeMsToFloatSeconds(TimeMs milliseconds)
{
  return float(milliseconds) / float(1000);
}

/// Converts seconds (float) to milliseconds (TimeMs)
ZeroShared inline TimeMs FloatSecondsToTimeMs(float seconds)
{
  return TimeMs(seconds * float(1000));
}

/// High precision timer class
class ZeroShared Timer
{
public:
  /// Tick type
  typedef unsigned long long TickType;

  /// Constructor
  Timer();

  /// Destructor
  ~Timer();

  /// Resets the time to zero
  void Reset();
  /// Updates the clock
  void Update();
  /// Gets the time in seconds since the last reset
  double Time() const;
  /// Gets the time in seconds between the last update and the update before it
  double TimeDelta() const;
  /// Updates the clock and gets the time in seconds since the last reset
  double UpdateAndGetTime();

  /// Gets the time in milliseconds since the last reset
  TimeMs TimeMilliseconds() const;
  /// Gets the time in milliseconds between the last update and the update before it
  TimeMs TimeDeltaMilliseconds() const;
  /// Updates the clock and gets the time in milliseconds since the last reset
  TimeMs UpdateAndGetTimeMilliseconds();

  /// Gets the time from the last update
  double TimeNoUpdate() const;
  /// Gets the tick time
  TickType GetTickTime() const;
  /// Gets the time in seconds from a tick count
  double TicksToSeconds(TickType ticks) const;

private:
  ZeroDeclarePrivateData(Timer, 50);
};

} // namespace Zero
