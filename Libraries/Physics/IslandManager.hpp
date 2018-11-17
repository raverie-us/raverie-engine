///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// What kind of a constraint solver should be used. A few pre-defined types meant for comparing performance.
DeclareEnum4(PhysicsSolverType, Basic, Normal, GenericBasic, Threaded);
/// How should islands be built. Internal for testing (mostly legacy).
DeclareEnum3(PhysicsIslandType, Composites, Kinematics, ForcedOne);
/// What kind of pre-processing strategy should be used for merging islands.
DeclareEnum3(PhysicsIslandPreProcessingMode, None, ColliderCount, ConstraintCount);

namespace Physics
{

class Island;


///Builds, solves and debug draws islands.
class IslandManager
{
public:
  IslandManager(PhysicsSolverConfig* config);
  ~IslandManager();

  void SetSpace(PhysicsSpace* space);
  void SetSolverConfig(PhysicsSolverConfig* config);

  void BuildIslands(ColliderList& colliders);
  void PostProcessIslands();
  void Solve(real dt, bool allowSleeping, uint debugFlags);
  void SolvePositions(real dt);
  void Draw(uint flags);

  void RemoveCollider(Collider* collider);
  void Clear();

  ///Returns the Island that Contains the given collider. null if none exists.
  Island* GetObjectsIsland(const Collider* collider);


  template <typename Policy> void CreateCompactIslands(Policy policy, ColliderList& colliders);
  template <typename Policy, typename PreProcessing> void CreateCompactIslands(Policy policy, PreProcessing prePolicy, ColliderList& colliders);
  template <typename Policy> void CreateSingleIsland(Policy policy, ColliderList& colliders);

  IConstraintSolver* GetNewSolver();
  Island* CreateNewIsland();

  uint mIslandCount;
  typedef InList<Island,&Island::ManagerLink> IslandList;
  IslandList mIslands;
  bool mPostProcess;

  PhysicsSolverType::Enum mSolverType;
  PhysicsIslandType::Enum mIslandingType;
  PhysicsIslandPreProcessingMode::Enum mPreProcessingType;
  /// Contains settings for solving such as what kind of solver to use.
  /// Also Contains information the solver needs (such as iteration count).
  PhysicsSolverConfig* mPhysicsSolverConfig;

  PhysicsSpace* mSpace;
  bool mShareSolver;
  IConstraintSolver* mSharedSolver;
};

}//namespace Physics

}//namespace Zero
