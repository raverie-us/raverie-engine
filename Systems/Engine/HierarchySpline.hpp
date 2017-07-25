///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class UpdateEvent;
class HierarchyEvent;

//-------------------------------------------------------------------HierarchySpline
/// A spline that builds its control points from all child cogs that have Transforms.
class HierarchySpline : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  HierarchySpline();

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void DebugDraw();

  /// The internal spline data
  Spline* GetSpline() const;
  /// Does the spline loop back on itself?
  bool GetClosed() const;
  void SetClosed(bool closed);
  /// The kind of spline (Linear, BSpline, CatmulRom).
  /// Determines how the control points affect the curve.
  SplineType::Enum GetSplineType() const;
  void SetSplineType(SplineType::Enum splineType);
  /// The max number of units that a line segment is
  /// allowed to deviate from the curve.
  real GetError() const;
  void SetError(real error);
  /// Should the spline draw every frame? Mainly used for debugging purposes.
  bool GetDebugDrawSpline() const;
  void SetDebugDrawSpline(bool debugDrawSpline);
  /// What color should that spline be drawn with
  Vec4 GetSplineColor() const;
  void SetSplineColor(Vec4 splineColor);
  
  /// Rebuild the baked points if there are any changes to the spline's control
  /// points. This should never need to be manually called.
  void RebuildIfModified();
  /// Forcibly rebuilds the baked points for the spline
  void ForceRebuild();
  /// The total arc-length of the curve. Use to normalize the curve if you wish.
  real GetTotalDistance() const;
  /// Samples the curve at a given arc-length distance.
  SplineSampleData SampleDistance(real distance);

  
private:
  void OnQuerySpline(SplineEvent* e);
  void OnFrameUpdate(UpdateEvent* e);
  void OnChildAttached(HierarchyEvent* e);
  void OnChildDetached(HierarchyEvent* e);
  void OnMarkModified(Event* e);
  
  void ConnectChildEvents(Cog* child);
  void GetChildrenConnections();
  

  bool mDebugDrawSpline;
  Vec4 mSplineColor;
  bool mIsModified;

  HandleOf<Spline> mSpline;
};

}//namespace Zero

