///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Nathan Carlson
/// Copyright 2012-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

#include <cstdlib>

namespace Zero
{

ZilchDefineType(GjkDebug, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultConstructor);

  ZeroBindDependency(Collider);

  ZilchBindGetterSetterProperty(OtherObject);
  ZilchBindFieldProperty(mMaxExpands);
  ZilchBindFieldProperty(mDt);
  // ZilchBindFieldProperty(mShowCSO);
  // ZilchBindFieldProperty(mSubdivisions)->Add(new EditorRange(12.0f, 48.0f, 1.0f));
  // ZilchBindFieldProperty(mOpacityCSO)->Add(new EditorRange(0.0f, 255.0f, 1.0f));
  // ZilchBindFieldProperty(mOpacitySimplex)->Add(new EditorRange(0.0f, 255.0f, 1.0f));

  // ZilchBindMethod(InitEpa)->SetHidden(false);
  // ZilchBindMethod(AddPoint)->SetHidden(false);
  ZilchBindMethodProperty(StepEpa);
  ZilchBindMethodProperty(ToggleAnimation);
}

GjkDebug::GjkDebug(void)
  : mShowCSO(true)
  , mSubdivisions(24.0f)
  , mOpacityCSO(50.0f)
  , mOpacitySimplex(50.0f)
  , mAnimate(false)
  , mMaxExpands(20)
  , mDt(1.0)
{
}

void GjkDebug::InitEpa(void)
{
  // mEpa.Init(Vec3(0.3f, 0, 0), Vec3(-0.3f, 0, 0.3), Vec3(-0.3f, 0, -0.3f), Vec3(0, 0.3f, 0));
}

void GjkDebug::AddPoint(void)
{
  // mEpa.Expand(RandomPoint());
}

void GjkDebug::StepEpa(void)
{
  ++mMaxExpands;
  // static unsigned step = 0;
  // if (step == 0)
  // {
  //   mEpa.DebugPoint(RandomPoint());
  //   ++step;
  // }
  // else if (step < 4)
  // {
  //   if (mEpa.DebugStep())
  //     ++step;
  //   else
  //     step = 0;
  // }

  // if (step == 4) step = 0;
}

void GjkDebug::ToggleAnimation(void)
{
  mAnimate = !mAnimate;
}

void GjkDebug::DebugDraw(void)
{
  // if (mAnimate)
  //   mEpa.Expand(RandomPoint());

  // mEpa.Draw();
  // return;

  if (!mOtherObject.IsValid() || !mOtherObject.has(Collider))
    return;

  // Origin lines
  gDebugDraw->Add(Debug::Line(Vec3(0.5f, 0, 0), Vec3(-0.5f, 0, 0)).Color(Color::Red));
  gDebugDraw->Add(Debug::Line(Vec3(0, 0.5f, 0), Vec3(0, -0.5f, 0)).Color(Color::Green));
  gDebugDraw->Add(Debug::Line(Vec3(0, 0, 0.5f), Vec3(0, 0, -0.5f)).Color(Color::Blue));

  // ComputeCSO();

  // // CSO points
  // // for (unsigned i = 0; i < mSupports.Size(); ++i)
  // //   gDebugDraw->Add(Debug::Sphere(mSupports[i], 0.01f).Color(Color::Green));

  // // CSO faces
  // for (unsigned i = 0; i < mCSO.Size(); ++i)
  // {
  //   Face &face = mCSO[i];
  //   Vec3 p0 = face.vertices[0];
  //   Vec3 p1 = face.vertices[1];
  //   Vec3 p2 = face.vertices[2];
  //   gDebugDraw->Add(Debug::Triangle(p0, p1, p2).Color(Color::Orange).Border(true).Alpha((unsigned)mOpacityCSO));
  // }

  // Intersection test
  Intersection::Manifold manifold;
  Intersection::Gjk gjk;
  Intersection::Type type;

  Intersection::SupportShape shapeA = GetOwner()->has(Collider)->GetSupportShape(true);
  Intersection::SupportShape shapeB = mOtherObject.has(Collider)->GetSupportShape(true);

  // real dt = 2.0855958f;
  Transform* transformA = GetOwner()->has(Transform);
  Transform* transformB = mOtherObject.has(Transform);

  RigidBody* bodyA = GetOwner()->has(RigidBody);
  RigidBody* bodyB = mOtherObject.has(RigidBody);

  Vec3 posA = transformA->GetTranslation();
  Vec3 posB = transformB->GetTranslation();

  Quat rotA = transformA->GetRotation();
  Quat rotB = transformB->GetRotation();

  Vec3 velA = bodyA->GetVelocity();
  Vec3 velB = bodyB->GetVelocity();

  Vec3 angVelA = bodyA->GetAngularVelocity();
  Vec3 angVelB = bodyB->GetAngularVelocity();

  angVelA = Math::Multiply(rotA, angVelA);
  angVelB = Math::Multiply(rotB, angVelB);

  real rotAngleA = angVelA.Length();
  real rotAngleB = angVelB.Length();

  if (rotAngleA != 0.0f)
    angVelA.Normalize();
  if (rotAngleB != 0.0f)
    angVelB.Normalize();

  Quat dRotA = ToQuaternion(angVelA, rotAngleA * mDt);
  Quat dRotB = ToQuaternion(angVelB, rotAngleB * mDt);

  shapeA.SetDeltaPosition(velA * mDt);
  shapeB.SetDeltaPosition(velB * mDt);

  shapeA.SetDeltaRotation(dRotA);
  shapeB.SetDeltaRotation(dRotB);

  type = gjk.TestDebug(&shapeA, &shapeB, &manifold, mMaxExpands);

  gjk.DrawDebug(0);
  if (type != Intersection::None)
  {
    gDebugDraw->Add(Debug::Sphere(manifold.Points[0].Points[0], 0.01f).Color(Color::Red).OnTop(true));
    gDebugDraw->Add(Debug::Sphere(manifold.Points[0].Points[1], 0.01f).Color(Color::Red).OnTop(true));

    gDebugDraw->Add(Debug::Line(manifold.Points[0].Points[0], manifold.Points[0].Points[0] + manifold.Normal).Color(Color::Red).OnTop(true));
  }
  else
  {
    Intersection::Simplex simplex = gjk.GetSimplex();
    Array<Intersection::CSOVertex> points = simplex.GetPoints();
    // if (points.Size() < 4) return;

    ByteColor simplexColor = Color::Blue;
    // DebugPrint("%d\n", points.Size());
    switch (points.Size())
    {
      case 1:
        gDebugDraw->Add(Debug::Sphere(points[0].cso, 0.01f).Color(simplexColor));
      break;
      case 2:
        gDebugDraw->Add(Debug::Line(points[0].cso, points[1].cso).Color(simplexColor));
      break;
      case 3:
        gDebugDraw->Add(Debug::Triangle(points[0].cso, points[1].cso, points[2].cso).Color(simplexColor).Border(true).Alpha((unsigned)mOpacitySimplex));
      break;
      case 4:
        gDebugDraw->Add(Debug::Triangle(points[0].cso, points[1].cso, points[2].cso).Color(simplexColor).Border(true).Alpha((unsigned)mOpacitySimplex));
        gDebugDraw->Add(Debug::Triangle(points[0].cso, points[2].cso, points[3].cso).Color(simplexColor).Border(true).Alpha((unsigned)mOpacitySimplex));
        gDebugDraw->Add(Debug::Triangle(points[0].cso, points[3].cso, points[1].cso).Color(simplexColor).Border(true).Alpha((unsigned)mOpacitySimplex));
        gDebugDraw->Add(Debug::Triangle(points[1].cso, points[2].cso, points[3].cso).Color(simplexColor).Border(true).Alpha((unsigned)mOpacitySimplex));
      break;
    }
  }

}

Cog * GjkDebug::GetOtherObject(void)
{
  return mOtherObject;
}

void GjkDebug::SetOtherObject(Cog *cog)
{
  if (cog == nullptr)
    mOtherObject = CogId();
  else
    mOtherObject = cog->GetId();
}

void GjkDebug::ComputeCSO(void)
{
  mSupports.Clear();
  mCSO.Clear();
  // if (!mOtherObject.IsValid() || !mOtherObject.has(Collider))
  // {
  //   DoNotify("No Collider", "OtherObject is not set or OtherObject does not own a Collider.", "Warning");
  //   return;
  // }

  unsigned subdivisions = (unsigned)mSubdivisions;
  float epsilon = 0.001f;
  float alphaLimit = Math::cPi - epsilon;
  float betaLimit = Math::cPi * 2 - epsilon;

  float delta = Math::cPi / subdivisions;
  for (float alpha = delta; alpha < alphaLimit; alpha += delta)
  {
    float alpha2 = (alpha + delta < alphaLimit) ? alpha + delta : delta;
    float sinAlpha1 = Math::Sin(alpha);
    float CosAlpha1 = Math::Cos(alpha);
    float sinAlpha2 = Math::Sin(alpha2);
    float CosAlpha2 = Math::Cos(alpha2);

    for (float beta = 0; beta < betaLimit; beta += delta)
    {
      float beta2 = (beta + delta < betaLimit) ? beta + delta : 0;
      float sinBeta1 = Math::Sin(beta);
      float CosBeta1 = Math::Cos(beta);
      float sinBeta2 = Math::Sin(beta2);
      float CosBeta2 = Math::Cos(beta2);

      Vec3 p0(sinAlpha1 * sinBeta1, CosAlpha1, sinAlpha1 * CosBeta1);
      Vec3 p1(sinAlpha2 * sinBeta1, CosAlpha2, sinAlpha2 * CosBeta1);
      Vec3 p2(sinAlpha2 * sinBeta2, CosAlpha2, sinAlpha2 * CosBeta2);
      Vec3 p3(sinAlpha1 * sinBeta2, CosAlpha1, sinAlpha1 * CosBeta2);
      p0 = ComputeSupport(p0);
      p1 = ComputeSupport(p1);
      p2 = ComputeSupport(p2);
      p3 = ComputeSupport(p3);

      mSupports.PushBack(p0);

      if (alpha2 > delta)
      {
        mCSO.PushBack(Face(p0, p2, p1));
        mCSO.PushBack(Face(p0, p3, p2));
      }
    }
  }

  Vec3 top = ComputeSupport(Vec3(0, 1, 0));
  Vec3 bottom = ComputeSupport(Vec3(0, -1, 0));

  unsigned collumns = subdivisions * 2;
  for (unsigned i = 0; i < collumns; ++i)
  {
    unsigned i2 = (i + 1) % (collumns);
    Vec3 p0 = mSupports[i];
    Vec3 p1 = mSupports[i2];

    unsigned j = mSupports.Size() - (collumns - i);
    unsigned j2 = mSupports.Size() - (collumns - i2);
    Vec3 p2 = mSupports[j];
    Vec3 p3 = mSupports[j2];

    mCSO.PushBack(Face(top, p1, p0));
    mCSO.PushBack(Face(p2, p3, bottom));
  }

  mSupports.PushBack(top);
  mSupports.PushBack(bottom);
}

Vec3 GjkDebug::ComputeSupport(Vec3 supportVector)
{
  Vec3 supportA, supportB;
  GetOwner()->has(Collider)->Support(supportVector, &supportA);
  mOtherObject.has(Collider)->Support(-supportVector, &supportB);
  return supportA - supportB;
}

Vec3 GjkDebug::RandomPoint(void)
{
  float alpha = std::rand() * Math::cPi / RAND_MAX;
  float beta = std::rand() * Math::cPi / RAND_MAX * 2;
  float sinAlpha1 = Math::Sin(alpha);
  float CosAlpha1 = Math::Cos(alpha);
  float sinBeta1 = Math::Sin(beta);
  float CosBeta1 = Math::Cos(beta);
  Vec3 dir(sinAlpha1 * sinBeta1, CosAlpha1, sinAlpha1 * CosBeta1);
  Vec3 point, center;
  GetOwner()->has(Collider)->GetSupportShape().Support(dir, &point);
  GetOwner()->has(Collider)->GetSupportShape().GetCenter(&center);
  return point - center;
}

} // namespace Zero
