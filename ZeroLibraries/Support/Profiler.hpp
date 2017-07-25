///////////////////////////////////////////////////////////////////////////////
///
/// \file Profiler.hpp
/// Declaration of the Profile Record, Manager, and Block class.
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Utility/Typedefs.hpp"
#include "Containers/Array.hpp"
#include "Containers/InList.hpp"
#include "Platform/Timer.hpp"

namespace Zero
{

namespace Profile
{

// Profile Time in MS
typedef u64 ProfileTime;
class Record;

/// System to manage all of the profile records.
class ProfileSystem
{
public:
  static ProfileSystem* Instance;
  static void Initialize();
  static void Shutdown();
  void Add(Record* record);
  void Add(cstr parentName, Record* record);
  float GetTimeInSeconds(ProfileTime time);
  ProfileTime GetTime();
  Array<Record*>::range GetRecords(){ return mRecordList.All(); }
private:
  Array<Record*> mRecordList;
  Timer mTimer;
};

/// Stores a timed record for a given name. This record may have a parent
/// and it may also have children.
class Record : public LinkBase
{
public:
  friend class ProfileSystem;
  Record();
  Record(cstr name);
  Record(cstr name, cstr parentName, u32 color = 0);

  void Initialize(cstr name, cstr parentName, u32 color = 0);

  //Display information
  void SetName(cstr name);
  cstr GetName(){return mName;};
  u32 GetColor(){return mColor;}
  void SetColor(u32 newColor){mColor = newColor;};

  typedef InListBaseLink<Record>::range RangeType;
  RangeType GetChildren(){return mChildren.All();}
  ProfileTime GetTotalTime(){return mTotalTime;};

  //Running Average
  void Update();
  float SmoothAverage();
  float Average();

  /// Used when this record should be updated with a new elapsed entry.
  /// Updates the hit count, total time and max time of this record.
  void EnterRecord(ProfileTime time);
  void Clear();

  Record(const Record&){}
  void operator=(const Record&){}

  static const uint cSampleCount = 512;
  static uint sSampleIndex;
  float mSamples[cSampleCount];
  float mRunningSample;
  uint mRunningCount;

  void AverageRunningSample();

private:

  //Display information
  u32 mColor;
  cstr mName;

  //General measurement
  u32 mHits;
  ProfileTime mTotalTime;
  ProfileTime mMaxTime;

  //Running Average
  float mSmoothAvg;
  float mInstantAvg;
  ProfileTime mOldTicks;

  //Graph
  void AddChild(Record* record);
  Record* mParent;
  InListBaseLink<Record> mChildren;
};

/// A timer that keeps a record for the given variable scope.
class ScopeTimer
{
public:
  ScopeTimer(Record* data);
  ~ScopeTimer();

  Record* mData;
  ProfileTime mStartTime;
};

void PrintProfileGraph();

}//namespace Profile
}//namespace Zero

#define ZPROFILE_ENABLED 1

#if ZPROFILE_ENABLED

#define ProfileScopeFunction() \
  static Zero::Profile::Record __LocalRecord(__FUNCTION__); \
  Zero::Profile::ScopeTimer __ScopedBlock(&__LocalRecord);

#define ProfileScope(name) \
  static Zero::Profile::Record __LocalRecord(name); \
  Zero::Profile::ScopeTimer __ScopedBlock(&__LocalRecord);

#define ProfileScopeTree(name, parentName, color) \
  static Zero::Profile::Record __LocalRecord(name, parentName, color); \
  Zero::Profile::ScopeTimer __ScopedBlock(&__LocalRecord);

#define ProfileScopeRecord(recordName) \
  Zero::Profile::ScopeTimer __ScopedBlock(&recordName);

#else


#endif

