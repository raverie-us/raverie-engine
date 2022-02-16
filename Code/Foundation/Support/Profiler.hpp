// MIT Licensed (see LICENSE.md).
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

class TraceEvent
{
public:
  String mCategory;
  String mName;
  String mArgs;
  size_t mThreadId;
  ProfileTime mTimestamp;
  ProfileTime mDuration;
};

/// System to manage all of the profile records.
class ProfileSystem
{
public:
  friend class ScopeTimer;
  static ProfileSystem* Instance;
  static void Initialize();
  static void Shutdown();
  void Add(Record* record);
  void Add(StringParam parentName, Record* record);
  float GetTimeInSeconds(ProfileTime time);
  ProfileTime GetTime();
  void BeginTracing();
  void EndTracing(Array<TraceEvent>& output);
  Array<Record*>::range GetRecords()
  {
    return mRecordList.All();
  }

private:
  Atomic<bool> mIsRecording;
  SpinLock mTraceEventsLock;
  Array<TraceEvent> mTraceEvents;
  Array<Record*> mRecordList;
  Timer mTimer;
};

/// Stores a timed record for a given name. This record may have a parent
/// and it may also have children.
class Record : public LinkBase
{
public:
  friend class ProfileSystem;
  friend class ScopeTimer;
  Record();
  Record(StringParam name);
  Record(StringParam name, StringParam parentName, u32 color = 0);

  void Initialize(StringParam name, StringParam parentName, u32 color = 0);

  // Display information
  void SetName(StringParam name);
  StringParam GetName()
  {
    return mName;
  };
  u32 GetColor()
  {
    return mColor;
  }
  void SetColor(u32 newColor)
  {
    mColor = newColor;
  };

  typedef InListBaseLink<Record>::range RangeType;
  RangeType GetChildren()
  {
    return mChildren.All();
  }
  ProfileTime GetTotalTime()
  {
    return mTotalTime;
  };

  // Running Average
  void Update();
  float SmoothAverage();
  float Average();

  /// Used when this record should be updated with a new elapsed entry.
  /// Updates the hit count, total time and max time of this record.
  void EnterRecord(ProfileTime time);
  void Clear();

  Record(const Record&)
  {
  }
  void operator=(const Record&)
  {
  }

  static const uint cSampleCount = 512;
  static uint sSampleIndex;
  float mSamples[cSampleCount];
  float mRunningSample;
  uint mRunningCount;

  void AverageRunningSample();

private:
  // Display information
  u32 mColor;
  String mName;

  // General measurement
  u32 mHits;
  ProfileTime mTotalTime;
  ProfileTime mMaxTime;

  // Running Average
  float mSmoothAvg;
  float mInstantAvg;
  ProfileTime mOldTicks;

  // Graph
  void AddChild(Record* record);
  Record* mParent;
  InListBaseLink<Record> mChildren;
};

/// A timer that keeps a record for the given variable scope.
class ScopeTimer
{
public:
  ScopeTimer(Record* data, String args = String());
  ~ScopeTimer();

  Record* mData;
  ProfileTime mStartTime;
  String mArgs;
};

void PrintProfileGraph();

} // namespace Profile
} // namespace Zero

#define ZPROFILE_ENABLED 1

#if ZPROFILE_ENABLED

#  define ProfileScopeFunction()                                                                                       \
    static Zero::Profile::Record __LocalRecord(__FUNCTION__);                                                          \
    Zero::Profile::ScopeTimer __ScopedBlock(&__LocalRecord);

#  define ProfileScopeFunctionArgs(args)                                                                               \
    static Zero::Profile::Record __LocalRecord(__FUNCTION__);                                                          \
    Zero::Profile::ScopeTimer __ScopedBlock(&__LocalRecord, args);

#  define ProfileScope(name)                                                                                           \
    static Zero::Profile::Record __LocalRecord(name);                                                                  \
    Zero::Profile::ScopeTimer __ScopedBlock(&__LocalRecord);

#  define ProfileScopeArgs(name, args)                                                                                 \
    static Zero::Profile::Record __LocalRecord(name);                                                                  \
    Zero::Profile::ScopeTimer __ScopedBlock(&__LocalRecord, args);

#  define ProfileScopeTree(name, parentName, color)                                                                    \
    static Zero::Profile::Record __LocalRecord(name, parentName, color);                                               \
    Zero::Profile::ScopeTimer __ScopedBlock(&__LocalRecord);

#  define ProfileScopeTreeArgs(name, args, parentName, color)                                                          \
    static Zero::Profile::Record __LocalRecord(name, parentName, color);                                               \
    Zero::Profile::ScopeTimer __ScopedBlock(&__LocalRecord, args);

#  define ProfileScopeRecord(recordName) Zero::Profile::ScopeTimer __ScopedBlock(&recordName);

#else

#endif
