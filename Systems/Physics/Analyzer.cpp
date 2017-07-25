///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Physics
{

BroadPhaseFrameData::BroadPhaseFrameData()
{
  Reset();
}

void BroadPhaseFrameData::Reset()
{
  PossibleCollisionsReturned = 0;
  ActualCollisions = 0;
  TimeTaken = 0;
}

Statistics::Statistics()
{
  PossibleCollisionsReturned = 0;
  ActualCollisions = 0;
  Iterations = 0;
  CollisionsMissed = 0;
}

///Adds the frame results to the total results.
void Statistics::Update(const BroadPhaseFrameData& result)
{
  PossibleCollisionsReturned += result.PossibleCollisionsReturned;
  ActualCollisions += result.ActualCollisions;

  ++Iterations;
}

///Prints the results out to a file.
void Statistics::PrintResults()
{
  DebugPrintFilter(Filter::PhysicsFilter, "%s Results:", Name.c_str());


  DebugPrintFilter(Filter::PhysicsFilter, "| Accuracy:  %.2f%%", (real(ActualCollisions) / real(PossibleCollisionsReturned)) / real(100.0));
  DebugPrintFilter(Filter::PhysicsFilter, "| Collision Time Taken:  %.2fms", Profile::ProfileSystem::Instance->GetTimeInSeconds(UpdateTime.GetTotalTime()));
  DebugPrintFilter(Filter::PhysicsFilter, "| Update Time Taken:  %.2fms", UpdateTime.GetTotalTime());
  DebugPrintFilter(Filter::PhysicsFilter, "| Ray Cast Time Taken:  %.2fms", RayCastTime.GetTotalTime());
  DebugPrintFilter(Filter::PhysicsFilter, "| Insertion Time Taken:  %.2fms", InsertionTime.GetTotalTime());
  DebugPrintFilter(Filter::PhysicsFilter, "| Removal Time Taken:  %.2fms", RemovalTime.GetTotalTime());
  DebugPrintFilter(Filter::PhysicsFilter, "| Construction Time Taken:  %.2fms", ConstructionTime.GetTotalTime());
  if(CollisionsMissed)
  {
    DebugPrintFilter(Filter::PhysicsFilter, "|");
    DebugPrintFilter(Filter::PhysicsFilter, "| !--COLLISIONS MISSED--!");
    DebugPrintFilter(Filter::PhysicsFilter, "| %u collision(s) was picked up by another", CollisionsMissed);
    DebugPrintFilter(Filter::PhysicsFilter, "| broadphase that this did not catch.");
  }
}

void Analyzer::AnalyzePerformance(uint type, StatisticsVec& statistics)
{
  //Return if there is nothing to analyze
  if(statistics.Empty())
    return;

  switch(type)
  {
  case BroadPhase::Dynamic:
    AnalyzeDynamic(statistics);
    break;
  case BroadPhase::Static:
    AnalyzeStatic(statistics);
    break;
  };

  //Clean up after this analysis.
  //Spikes.Clear();
}

void Analyzer::AnalyzeDynamic(StatisticsVec& statistics)
{
  //Set the default best
  Statistics* bestInerstionTime = statistics[0];
  Statistics* bestRemovalTime = statistics[0];
  Statistics* bestUpdateTime = statistics[0];
  Statistics* bestCollisionTestTime = statistics[0];
  Statistics* bestRayCastTime = statistics[0];

  //Check for better 
  for(uint i = 0; i < statistics.Size(); ++i)
  {
    //Insertion

  }
}

void Analyzer::AnalyzeStatic(StatisticsVec& statistics)
{

}

void Analyzer::ReportSpike(const char* type, real ms)
{

}

real Analyzer::CalculateScore(Statistics& stats)
{
  return 0.0;
}

void Analyzer::PrintResults()
{

}

}//namespace Physics

}//namespace Zero
