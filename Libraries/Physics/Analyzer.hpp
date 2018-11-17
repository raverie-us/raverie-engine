///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

///Tracks down which object, if any, was not sent to be checked 
///for collision when it should have (assuming at least one broad phase caught it).
#define OBJECT_TRACKING 1

namespace Zero
{

namespace Physics
{

//The result of a single frame
struct BroadPhaseFrameData
{
  BroadPhaseFrameData(void);

  ///Clears out all data for the next frame(sets to 0).
  void Reset(void);

  ///Information about the results of the broad phase.
  uint PossibleCollisionsReturned;
  uint ActualCollisions;
  real TimeTaken;
};

//The statistics for a single broad phases entire life.
struct Statistics
{
  Statistics(void);

  ///Adds the frame results to the total results.
  void Update(const BroadPhaseFrameData& result);

  ///Prints the results out to a file.
  void PrintResults(void);

  ///The name of the broad phase
  String Name;
  ///The broad phase type, used to help determine what is a more optimal
  ///broad phase for certain situations.
  uint mType;

  ///Collisions data
  uint PossibleCollisionsReturned;
  uint ActualCollisions;
  uint CollisionsMissed;

  ///Iterations
  uint Iterations;

  ///Insertion / Removal.
  Profile::Record InsertionTime;
  Profile::Record RemovalTime;
  ///Time taken to update the broad phase (generally dynamic broad phases).
  Profile::Record UpdateTime;
  ///Time taken to test objects / get possible pairs.
  Profile::Record CollisionTime;
  ///Time taken for construction.
  Profile::Record ConstructionTime;
  ///Time taken for ray casts.
  Profile::Record RayCastTime;
};

class Analyzer
{
public:
  Analyzer(void);
  ~Analyzer(void);
  
  typedef Array<Statistics*> StatisticsVec;

  void AnalyzePerformance(uint type, StatisticsVec& statistics);
  void AnalyzeDynamic(StatisticsVec& statistics);
  void AnalyzeStatic(StatisticsVec& statistics);
  void ReportSpike(const char* type, real ms);
  real CalculateScore(Statistics& stats);
  void PrintResults();
};

}//namespace Physics

}//namespace Zero
