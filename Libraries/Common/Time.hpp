///////////////////////////////////////////////////////////////////////////////
///
/// \file Time.hpp
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <time.h>

namespace Zero
{

typedef time_t TimeType;

// Structure to mimic the C-standard tm struct
// Only change is that the year is 0 based instead of 1900
struct CalendarDateTime
{
  int Seconds;
  int Minutes;
  int Hour;
  int Day;
  int Month;
  // Years from year 0
  int Year;
  // Days since Sunday
  int Weekday;
  // Days since January 1
  int Yearday;
  int IsDaylightSavings;
};

class Time
{
public:

  static TimeType GetTime();
  static TimeType Clock();
  static TimeType GenerateSeed();

  static CalendarDateTime GetLocalTime(const TimeType& timer);
  static TimeType CalendarDateTimeToTimeType(const CalendarDateTime& time);
  static TimeType ClocksPerSecond();

  static const char* GetMonthString(int month)
  {
    const char* cDates[] = {"January", "February", "March", "April", "May",
                             "June", "July", "August", "September",
                             "October", "November", "December"};

    return cDates[month];
  }
};

} // namespace Zero
