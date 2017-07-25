///////////////////////////////////////////////////////////////////////////////
///
/// \file TimerBlock.hpp
/// Simple timer block used for profiling.
///
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
namespace Zero
{

class TimerBlock
{
public:
  String Name;
  TimerBlock(StringParam name)
    :Name(name)
  {
  }
  
  ~TimerBlock()
  {
    float time = (float)timer.UpdateAndGetTime();
    ZPrintFilter(Filter::DefaultFilter, "%s %.2fs\n", Name.c_str(), time);
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

}//namespace Zero
