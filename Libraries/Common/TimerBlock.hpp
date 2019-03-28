// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"
namespace Zero
{

class TimerBlock
{
public:
  String Name;
  TimerBlock(StringParam name) : Name(name)
  {
  }

  ~TimerBlock()
  {
    float time = (float)timer.UpdateAndGetTime();
    ZPrintFilter(Filter::DefaultFilter, "%s %.2fs\n", Name.c_str(), time);
  }
  Timer timer;
};

class PreciseTimerBlock
{
public:
  String Name;
  PreciseTimerBlock(StringParam name) : Name(name)
  {
  }

  ~PreciseTimerBlock()
  {
    double time = timer.UpdateAndGetTime();
    ZPrintFilter(Filter::DefaultFilter, "%s %gs\n", Name.c_str(), time);
  }
  Timer timer;
};

#define DetectHitch(maxTime) HitchDetector hitchDetector((float)(maxTime));

class HitchDetector
{
public:
  HitchDetector(float maxTime) : mMaxTime(maxTime)
  {
  }
  ~HitchDetector()
  {
    float time = (float)mTimer.UpdateAndGetTime();

    if (time > mMaxTime)
    {
      Os::DebugBreak();
    }
  }

private:
  float mMaxTime;
  Timer mTimer;
};

} // namespace Zero
