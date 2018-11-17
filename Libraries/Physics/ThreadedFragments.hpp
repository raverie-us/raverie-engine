///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{

template <typename JointType>
struct ConstraintBatch
{
  ConstraintBatch() { ConstraintCount = 0; }
  ~ConstraintBatch() { Joints.Clear(); }
  uint ConstraintCount;
  typedef InList<JointType,&JointType::SolverLink> JointList;
  JointList Joints;

  IntrusiveLink(ConstraintBatch<JointType>,link);
};

template <typename JointType>
struct ConstraintPhase
{
  ConstraintPhase() { BatchCount = 0; }
  ~ConstraintPhase() { DeleteObjectsIn(Batches); }
  uint BatchCount;

  typedef ConstraintBatch<JointType> JointBatch;
  typedef InList<JointBatch> JointBatches;
  JointBatches Batches;

  IntrusiveLink(ConstraintPhase<JointType>,link);
};

template <typename JointType>
struct ConstraintGroup
{
  ConstraintGroup() { PhaseCount = 0; }
  ~ConstraintGroup() { Clear(); }
  void Clear()
  {
    while(!Phases.Empty())
    {
      PhaseType* phase = &Phases.Front();
      Phases.PopFront();
    
      delete phase;
    }
  }
  uint PhaseCount;
  typedef ConstraintPhase<JointType> PhaseType;
  typedef InList<PhaseType> PhaseTypeList;
  PhaseTypeList Phases;
};

template <typename ListType, typename Functor>
void GroupOperationFragment(ConstraintGroup<typename ListType::value_type>& group, Functor operation)
{
  typedef ConstraintGroup<typename ListType::value_type> JointGroup;
  typedef ConstraintPhase<typename ListType::value_type> JointPhase;

  typename JointGroup::PhaseTypeList::range jointRange = group.Phases.All();
  for(; !jointRange.Empty(); jointRange.PopFront())
  {
    JointPhase& phase = jointRange.Front();
    typename JointPhase::JointBatches::range range = phase.Batches.All();
    for(; !range.Empty(); range.PopFront())
      operation(range.Front().Joints);
  }
}

template <typename ListType, typename ParamType, typename Functor>
void GroupOperationParamFragment(ConstraintGroup<typename ListType::value_type>& group, ParamType& param, Functor operation)
{
  typedef ConstraintGroup<typename ListType::value_type> JointGroup;
  typedef ConstraintPhase<typename ListType::value_type> JointPhase;

  typename JointGroup::PhaseTypeList::range jointRange = group.Phases.All();
  for(; !jointRange.Empty(); jointRange.PopFront())
  {
    JointPhase& phase = jointRange.Front();
    typename JointPhase::JointBatches::range range = phase.Batches.All();
    for(; !range.Empty(); range.PopFront())
      operation(range.Front().Joints,param);
  }
}

template <typename ListType, typename ParamType1, typename ParamType2, typename Functor>
void GroupOperationTwoParamFragment(ConstraintGroup<typename ListType::value_type>& group, ParamType1& param1, ParamType2& param2, Functor operation)
{
  typedef ConstraintGroup<typename ListType::value_type> JointGroup;
  typedef ConstraintPhase<typename ListType::value_type> JointPhase;

  typename JointGroup::PhaseTypeList::range jointRange = group.Phases.All();
  for(; !jointRange.Empty(); jointRange.PopFront())
  {
    JointPhase& phase = jointRange.Front();
    typename JointPhase::JointBatches::range range = phase.Batches.All();
    for(; !range.Empty(); range.PopFront())
      operation(range.Front().Joints,param1,param2);
  }
}

template <typename ListType>
void SplitConstraints(ListType& joints, ConstraintGroup<typename ListType::value_type>& phases)
{
  typedef ConstraintPhase<typename ListType::value_type> PhaseType;
  typedef ConstraintBatch<typename ListType::value_type> BatchType;

  HashSet<uint> bodySet;
  uint batchSize = 32;
  uint batchesPerPhase = 2;

  PhaseType* phase = nullptr;
  BatchType* batch = nullptr;

  bool newPhaseStarted = false;

  while(!joints.Empty())
  {
    if(newPhaseStarted == false)
    {
      phase = new PhaseType();
      phases.Phases.PushBack(phase);
      ++phases.PhaseCount;
      batch = new BatchType();
      phase->Batches.PushBack(batch);
      ++phase->BatchCount;
      bodySet.Clear();
    }
    newPhaseStarted = false;

    typename ListType::range range = joints.All();
    while(!range.Empty())
    {
      typename ListType::pointer joint = &(range.Front());
      range.PopFront();

      //get the id of the two bodies involved in this joint
      uint idA = joint->GetCollider(0)->mId;
      uint idB = joint->GetCollider(1)->mId;

      //if either of the bodies have been used in this phase, then skip this joint
      if(!bodySet.Find(idA).Empty() || !bodySet.Find(idB).Empty())
        continue;

      //if adding this joint would make the batch too large, make a new batch and add the old to the phase
      uint ConstraintCount = joint->MoleculeCount();
      if(ConstraintCount + batch->ConstraintCount > batchSize)
      {
        //if this will be too many batches for this phase, then make a new phase
        if(phase->BatchCount >= batchesPerPhase)
        {
          phase = new PhaseType();
          phases.Phases.PushBack(phase);
          ++phases.PhaseCount;
          //since this is a new phase, all bodies are valid again
          bodySet.Clear();
          newPhaseStarted = true;
        }

        batch = new BatchType();
        phase->Batches.PushBack(batch);
        ++phase->BatchCount;
      }

      //mark both of these bodies as being used for this phase
      bodySet.Insert(idA);
      bodySet.Insert(idB);

      //put the joint in this batch
      ListType::Unlink(joint);

      batch->Joints.PushBack(joint);
      batch->ConstraintCount += ConstraintCount;
    }
  }
}

template <typename ListType>
void CollectJoints(ListType& inList, ListType& outList)
{
  outList.Splice(outList.End(), inList.All());
}

}//namespace Physics

}//namespace Zero
