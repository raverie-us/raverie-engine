///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#ifdef USESSE
#include "Physics/Joints/ConstraintFragmentsSse.hpp"
#include "Math/SimVectors.hpp"
#include "Math/SimMatrix3.hpp"
#endif
//////////////////////////////////////////////////////////////////////////
///C: dot(p2 - p1,n) = 0
///C: dot(c2 + r2 - c1 - r1,n) = 0
///Cdot: dot(v2 + cross(w2,r2) - v1 - cross(w1,r1),n) = 0
///Cdot: dot(v2,n) + dot(cross(w2,r2),n) - dot(v1,n) - dot(cross(w1,r1),n)
///Cdot: dot(v2,n) + dot(cross(r2,n),w2) - dot(v1,n) - dot(cross(r1,n),w1)
///J: [-n, -cross(r1,n), n, cross(r2,n)]
///Identity used:    a x b * c =     a * b x c     = c x a * b
///          dot(cross(a,b),c) = dot(a,cross(b,c)) = dot(cross(c,a),b)
//////////////////////////////////////////////////////////////////////////

namespace Zero
{

namespace Physics
{

const real debugLineLength = real(2.0f);

Contact::Contact()
{
  mManifold = nullptr;
  mContactManager = nullptr;
  mFlags.Clear();
  mFlags.SetFlag(ContactFlags::Active);
}

Contact::~Contact()
{
  //contacts now own the manifold
  delete mManifold;
}

bool Contact::GetOnIsland() const
{
  return mFlags.IsSet(ContactFlags::OnIsland);
}

void Contact::SetOnIsland(bool onIsland)
{
  mFlags.SetState(ContactFlags::OnIsland, onIsland);
}

bool Contact::GetGhost() const
{
  return mFlags.IsSet(ContactFlags::Ghost);
}

void Contact::SetGhost(bool ghost)
{
  mFlags.SetState(ContactFlags::Ghost, ghost);
}

bool Contact::GetSkipResolution() const
{
  return mFlags.IsSet(ContactFlags::SkipsResolution);
}

void Contact::SetSkipResolution(bool skip)
{
  mFlags.SetState(ContactFlags::SkipsResolution, skip);
}

bool Contact::GetValid() const
{
  return mFlags.IsSet(ContactFlags::Valid);
}

void Contact::SetValid(bool valid)
{
  mFlags.SetState(ContactFlags::Valid,valid);
}

bool Contact::GetActive() const
{
  return mFlags.IsSet(ContactFlags::Active);
}

void Contact::SetActive(bool active)
{
  mFlags.SetState(ContactFlags::Active,active);
}

bool Contact::GetIsNew() const
{
  return mFlags.IsSet(ContactFlags::NewContact);
}

bool Contact::GetSendsEvents() const
{
  return mManifold->GetSendsMessages();
}

void Contact::SetPair(ColliderPair& pair)
{
  mEdges[0].mCollider = pair.Top;
  mEdges[0].mOther = pair.Bot;
  mEdges[0].mContact = this;

  // need to make all "seamless objects" grouped
  // together at the beginning of the inlist

  if(pair.Bot && pair.Bot->mState.IsSet(ColliderFlags::Seamless))
    pair.Top->mContactEdges.PushFront(&mEdges[0]);
  else
    pair.Top->mContactEdges.PushBack(&mEdges[0]);


  if(pair.Bot)
  {
    mEdges[1].mCollider = pair.Bot;
    mEdges[1].mOther = pair.Top;
    mEdges[1].mContact = this;

    if(pair.Top->mState.IsSet(ColliderFlags::Seamless))
      pair.Bot->mContactEdges.PushFront(&mEdges[1]);
    else
      pair.Bot->mContactEdges.PushBack(&mEdges[1]);
  }

  SetValid(true);
}

void Contact::UnLinkPair()
{
  if(GetOnIsland() && !GetGhost() && GetActive())
    InList<Contact, &Contact::SolverLink>::Unlink(this);

  Collider::ContactEdgeList::Unlink(&mEdges[0]);
  Collider::ContactEdgeList::Unlink(&mEdges[1]);
}

void Contact::Destroy(bool sendImmediately)
{
  mContactManager->Remove(this, sendImmediately);
}

void Contact::SetManifold(Manifold* manifold)
{
  mManifold = manifold;
  SetValid(true);

  mFlags.SetFlag(ContactFlags::NewContact);

  UpdateManifoldInternal(manifold);
}

void Contact::UpdateManifold(Manifold* manifold)
{
  mFlags.ClearFlag(ContactFlags::NewContact);
  UpdateManifoldInternal(manifold);
  mManifold->AddPoints(manifold->Contacts, manifold->ContactCount);
}

void Contact::UpdateManifoldInternal(Manifold* manifold)
{
  if(mManifold->Objects[0] != manifold->Objects[0])
    manifold->SwapPair();
  mManifold->SetPair(manifold->Objects);

  SetValid(true);
}

Manifold* Contact::GetManifold()
{
  return mManifold;
}

uint Contact::GetContactCount() const
{
  return mManifold->ContactCount;
}

void Contact::UpdateAtoms()
{

}

uint Contact::MoleculeCount() const
{
  return GetContactCount() * 3;
}

void ComputeContactLimits(Contact* contact, MoleculeWalker& fragments)
{
  real frictionRatio = contact->mManifold->DynamicFriction / contact->GetContactCount();
  real frictionMax = frictionRatio * fragments[0].mImpulse;

  fragments[0].SetLimit(real(0.0), Math::PositiveMax());
  fragments[1].SetLimit(-frictionMax, frictionMax);
  fragments[2].SetLimit(-frictionMax, frictionMax);
}

void ComputeContactFragments(Contact* contact, MoleculeWalker& fragments, uint atomCount, MoleculeData& data, real restitutionBias)
{
  real baumgarte = contact->GetLinearBaumgarte();
  JointMass masses;
  JointHelpers::GetMasses(contact->GetCollider(0),contact->GetCollider(1),masses);

  real frictionRatio = contact->mManifold->DynamicFriction / contact->GetContactCount();
  real frictionMax = frictionRatio * fragments[0].mImpulse;

  real mass;
  LinearAxisFragment(data.mAnchors, data.LinearAxes[0], fragments[0]);
  mass = fragments[0].mJacobian.ComputeMass(masses);
  fragments[0].mMass = (mass == real(0.0) ? mass : real(1.0) / mass);
  
  LinearAxisFragment(data.mAnchors, data.LinearAxes[1], fragments[1]);
  mass = fragments[1].mJacobian.ComputeMass(masses);
  fragments[1].mMass = (mass == real(0.0) ? mass : real(1.0) / mass);

  LinearAxisFragment(data.mAnchors, data.LinearAxes[2], fragments[2]);
  mass = fragments[2].mJacobian.ComputeMass(masses);
  fragments[2].mMass = (mass == real(0.0) ? mass : real(1.0) / mass);

  //the normal constraint needs a special fragment that accumulates baumgarte and restitution
  //separately so that a baumgarte of zero doesn't remove restitution
  ContactNormalFragment(fragments[0], baumgarte, restitutionBias);
  //there's no baumgarte on the friction constraints
  //(the error is also zero so nothing would be solved anyways, but this makes it clearer)
  RigidConstraintFragment(fragments[1], real(0.0));
  RigidConstraintFragment(fragments[2], real(0.0));

  ComputeContactLimits(contact, fragments);
}

void Contact::ComputeMolecules(MoleculeWalker& fragments)
{
  uint contactCount = GetContactCount();

  real velocityThreshold = mSolver->mSolverConfig->mVelocityRestitutionThreshold;
  real baumgarte = GetLinearBaumgarte();

  for(uint i = 0; i < contactCount; ++i)
  {
    ManifoldPoint& contact = mManifold->Contacts[i];

    fragments[0].mImpulse = contact.AccumulatedImpulse[0];
    real slop = mSolver->mSolverConfig->mContactBlock.GetSlop();
    fragments[0].mError = -Math::Max(mManifold->Contacts[i].Penetration - slop, real(0.0));
    //compute restitution as a bias term based upon the separating velocity
    real relativeVelocity = mManifold->GetSeparatingVelocity(i);
    real restitutionBias = real(0.0);
    //only apply restitution if we are above a threshold,
    //otherwise instabilities can happen when we're near a resting state
    if(relativeVelocity > velocityThreshold)
      restitutionBias = mManifold->Restitution * relativeVelocity;

    fragments[1].mImpulse = contact.AccumulatedImpulse[1];
    fragments[1].mError = 0;

    fragments[2].mImpulse = contact.AccumulatedImpulse[2];
    fragments[2].mError = 0;

    AnchorAtom mAnchors;
    mAnchors.mBodyR[0] = mManifold->Contacts[i].BodyPoints[0];
    mAnchors.mBodyR[1] = mManifold->Contacts[i].BodyPoints[1];

    MoleculeData fragmentData;
    fragmentData.SetUp(&mAnchors, nullptr, this);
    fragmentData.LinearAxes[0] = contact.Normal;

    mManifold->GetTangents(mSolver->mSolverConfig, i, fragmentData.LinearAxes[1], fragmentData.LinearAxes[2]);
    fragmentData.SetAngularIdentity();

    ComputeContactFragments(this, fragments, 3, fragmentData, restitutionBias);

    fragments += 3;
  }
}

void Contact::WarmStart(MoleculeWalker& fragments)
{
  WarmStartFragment(this, fragments, MoleculeCount());
}

void Contact::Solve(MoleculeWalker& fragments)
{
  uint contactCount = GetContactCount();
  JointVelocity velocities;
  JointHelpers::GetVelocities(GetCollider(0), GetCollider(1), velocities);
  JointMass masses;
  JointHelpers::GetMasses(GetCollider(0), GetCollider(1), masses);

  for(uint i = 0; i < contactCount; ++i)
  {
    real lambda;
    
    lambda = ComputeLambda(fragments[0], velocities);
    JointHelpers::ApplyConstraintImpulse(masses, velocities, fragments[0].mJacobian, lambda);

    ComputeContactLimits(this, fragments);

    lambda = ComputeLambda(fragments[1], velocities);
    JointHelpers::ApplyConstraintImpulse(masses, velocities, fragments[1].mJacobian, lambda);

    lambda = ComputeLambda(fragments[2], velocities);
    JointHelpers::ApplyConstraintImpulse(masses, velocities, fragments[2].mJacobian, lambda);

    fragments += 3;
  }

  JointHelpers::CommitVelocities(GetCollider(0), GetCollider(1), velocities);
}

void Contact::SolveSse(MoleculeWalker& fragments)
{
#ifdef USESSE
  uint contactCount = GetContactCount();
  JointVelocity velocities;
  JointHelpers::GetVelocities(GetCollider(0), GetCollider(1), velocities);
  JointMass masses;
  JointHelpers::GetMasses(GetCollider(0), GetCollider(1), masses);

  //load velocities into simd registers
  SimVec v0 = Simd::UnAlignedLoad(velocities.Linear[0].array);
  SimVec v1 = Simd::UnAlignedLoad(velocities.Linear[1].array);
  SimVec w0 = Simd::UnAlignedLoad(velocities.Angular[0].array);
  SimVec w1 = Simd::UnAlignedLoad(velocities.Angular[1].array);

  //load mass and inertia into simd registers
  SimVec m0 = Simd::UnAlignedLoad(masses.mInvMass[0].GetInvMasses().array);
  SimVec m1 = Simd::UnAlignedLoad(masses.mInvMass[1].GetInvMasses().array);
  Mat3Param invM0 = masses.InverseInertia[0];
  Mat3Param invM1 = masses.InverseInertia[1];
  SimMat3 i0 = Simd::SetMat3(invM0.m00, invM0.m01, invM0.m02,
                             invM0.m10, invM0.m11, invM0.m12,
                             invM0.m20, invM0.m21, invM0.m22);
  SimMat3 i1 = Simd::SetMat3(invM1.m00, invM1.m01, invM1.m02,
                             invM1.m10, invM1.m11, invM1.m12,
                             invM1.m20, invM1.m21, invM1.m22);

  for(uint i = 0; i < contactCount; ++i)
  {
    SolveConstraintSse(v0, w0, v1, w1, m0, i0, m1, i1, fragments[0]);

    ComputeContactLimits(this, fragments);
    SolveConstraintSse(v0, w0, v1, w1, m0, i0, m1, i1, fragments[1]);
    SolveConstraintSse(v0, w0, v1, w1, m0, i0, m1, i1, fragments[2]);

    fragments += 3;
  }

  Vec4_16 temp;
  Simd::Store(v0, temp.array);
  velocities.Linear[0].Set(temp.x, temp.y, temp.z);
  Simd::Store(v1, temp.array);
  velocities.Linear[1].Set(temp.x, temp.y, temp.z);
  Simd::Store(w0, temp.array);
  velocities.Angular[0].Set(temp.x, temp.y, temp.z);
  Simd::Store(w1, temp.array);
  velocities.Angular[1].Set(temp.x, temp.y, temp.z);

  JointHelpers::CommitVelocities(GetCollider(0), GetCollider(1), velocities);
#endif
}

void Contact::Commit(MoleculeWalker& fragments)
{
  uint contactCount = GetContactCount();
  for(uint i = 0; i < contactCount; ++i)
  {
    ManifoldPoint& contact = mManifold->Contacts[i];
    contact.AccumulatedImpulse[0] = fragments[0].mImpulse;
    contact.AccumulatedImpulse[1] = fragments[1].mImpulse;
    contact.AccumulatedImpulse[2] = fragments[2].mImpulse;

    fragments += 3;
  }
}

uint Contact::PositionMoleculeCount() const
{
  return GetContactCount();
}

void Contact::ComputePositionMolecules(MoleculeWalker& fragments)
{
  uint contactCount = GetContactCount();

  Collider* collider0 = GetCollider(0);
  Collider* collider1 = GetCollider(1);

  JointMass masses;
  JointHelpers::GetMasses(collider0, collider1, masses);

  Vec3 cm0, cm1;
  if(collider0->GetActiveBody())
    cm0 = collider0->GetActiveBody()->mCenterOfMass;
  else
    cm0 = collider0->GetWorldTranslation();
  if(collider1->GetActiveBody())
    cm1 = collider1->GetActiveBody()->mCenterOfMass;
  else
    cm1 = collider1->GetWorldTranslation();

  for(uint i = 0; i < contactCount; ++i)
  {
    ManifoldPoint& contactPoint = mManifold->Contacts[i];
    ConstraintMolecule& mol = fragments[0];

    mol.mImpulse = contactPoint.AccumulatedImpulse[0];
    real slop = mSolver->mSolverConfig->mContactBlock.GetSlop();
    mol.mError = -Math::Max(contactPoint.Penetration - slop, real(0.0));


    AnchorAtom mAnchors;
    mAnchors.mBodyR[0] = contactPoint.BodyPoints[0];
    mAnchors.mBodyR[1] = contactPoint.BodyPoints[1];
    WorldAnchorAtom worldAnchors;
    worldAnchors.mWorldR[0] = contactPoint.WorldPoints[0] - cm0;
    worldAnchors.mWorldR[1] = contactPoint.WorldPoints[1] - cm1;


    real mass;
    mol.mJacobian.Set(-contactPoint.Normal,-Math::Cross(worldAnchors[0], contactPoint.Normal),
                       contactPoint.Normal, Math::Cross(worldAnchors[1], contactPoint.Normal));
    mass = mol.mJacobian.ComputeMass(masses);
    mol.mMass = (mass == real(0.0) ? mass : real(1.0) / mass);

    RigidConstraintFragment(mol, real(0.0));


    mol.SetLimit(real(0.0), Math::PositiveMax());


    fragments += 1;
  }
}

void Contact::DebugDraw()
{
  uint contactCount = GetContactCount();
  for(uint i = 0; i < contactCount; ++i)
  {
    ManifoldPoint& contact = mManifold->Contacts[i];
    gDebugDraw->Add(Debug::Sphere(contact.WorldPoints[0], real(0.2)));
    gDebugDraw->Add(Debug::Sphere(contact.WorldPoints[1], real(0.2)));
    gDebugDraw->Add(Debug::Line(contact.WorldPoints[0], contact.WorldPoints[0] + contact.Normal));
  }
}

uint Contact::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = real(0.0);
  return Joint::LinearAxis;
}

bool Contact::GetShouldBaumgarteBeUsed() const
{
  PhysicsSolverConfig* config = mSolver->mSolverConfig;
  ConstraintConfigBlock& block = config->GetContactBlock();
  if(block.GetPositionCorrectionType() == ConstraintPositionCorrection::Inherit)
  {
    return config->GetPositionCorrectionType() == PhysicsSolverPositionCorrection::Baumgarte;
  }
  return block.GetPositionCorrectionType() == ConstraintPositionCorrection::Baumgarte;
}

real Contact::GetLinearBaumgarte() const
{
  PhysicsSolverConfig* config = mSolver->mSolverConfig;
  ConstraintConfigBlock& block = config->GetContactBlock();

  if(GetShouldBaumgarteBeUsed())
    return block.mLinearBaumgarte;
  return 0;
}

real Contact::GetLinearErrorCorrection() const
{
  // It's technically not necessary to check if we use post-stabilization here as contacts never
  // have this function called unless the solver is set to use post-stabilization
  PhysicsSolverConfig* config = mSolver->mSolverConfig;
  ConstraintConfigBlock& block = config->GetContactBlock();
  return block.mLinearErrorCorrection;
}

Collider* Contact::GetCollider(uint index) const
{
  return mEdges[index].mCollider;
}

}//namespace Physics

}//namespace Zero
