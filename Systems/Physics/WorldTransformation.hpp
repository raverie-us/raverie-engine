///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

///This represents the world transformation for physics to use. This is also
///used to collapse the body to world transformations for hierarchies. Also, 
///used to remove the sheering from these transformations. Also Contains
///functionality to transform normals and points between body and world space.
class WorldTransformation
{
public:
  WorldTransformation();

  void SetTransform(Transform* transform);

  void ReadTransform(PhysicsNode* owner);
  ///Pass in nullptr if this is the root (or effective root with kinematics).
  void ComputeTransformation(WorldTransformation* parent, PhysicsNode* owner);

  ///Returns the world scale of this object
  Vec3 GetWorldScale() const;
  ///Returns the world rotation of this object
  Mat3 GetWorldRotation() const;
  ///Returns the world translation of this object
  Vec3 GetWorldTranslation() const;

  Vec3 GetPublishedTranslation() const;

  //Returns the local scale of this object
  Vec3 GetLocalScale() const;
  ///Returns the local rotation of this object
  Mat3 GetLocalRotation() const;
  ///Returns the local translation of this object
  Vec3 GetLocalTranslation() const;

  //Returns the shearless world matrix
  Mat4 GetWorldMatrix() const;

  void SetScale(Vec3Param scale);
  void SetRotation(Mat3Param rotation);
  void SetTranslation(Vec3Param translation);

  ///Transforms a point from body to world space (applies translation)
  Vec3 TransformPoint(Vec3Param point) const;
  ///Transforms a normal from body to world space (ignores translation)
  Vec3 TransformNormal(Vec3Param normal) const;
  ///Transforms a surface normal from body to world space using the inverse
  ///transform to transform a normal.
  Vec3 TransformSurfaceNormal(Vec3Param direction) const;
  ///Transforms a point from world to body space (applies translation)
  Vec3 InverseTransformPoint(Vec3Param point) const;
  ///Transforms a normal from world to body space (ignores translation)
  Vec3 InverseTransformNormal(Vec3Param normal) const;
  ///World to body space for a surface normal vector.
  Vec3 InverseTransformSurfaceNormal(Vec3Param direction) const;

  void ComputeOldValues();
  Vec3 GetOldTranslation();
  Mat3 GetOldRotation();

  bool IsUniformlyScaled();

private:
  ///The entire matrix (including sheers) to get from body to world space
  Mat4 mBodyToWorld;

  //Below is the transformation stripped from sheers. The individual elements
  //are stored separately because many requests are made to get an individual
  //element. Performing transformations from the individual elements is not
  //significantly more expensive.
  Vec3 mWorldTranslation;
  Vec3 mWorldScale;
  Mat3 mWorldRotation;

  Vec3 mWorldOffset;
public:

  Vec3 mLocalTranslation;
  Vec3 mLocalScale;
  Mat3 mLocalRotation;
  //quick fix item for kinematics
  Vec3 mOldTranslation;
  Mat3 mOldRotation;

  Transform* mTransform;
};

}//namespace Zero
