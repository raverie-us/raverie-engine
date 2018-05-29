///////////////////////////////////////////////////////////////////////////////
///
///	Authors: Joshua Davis
///	Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
#include "UnitTestCommon.hpp"

using Zero::Aabb;

void GetAabbPoints(const Aabb& aabb, Vec3 points[8])
{
  Vec3 center, halfExtent;
  aabb.GetCenterAndHalfExtents(center, halfExtent);
  points[0] = center + halfExtent * Vec3(+1, +1, +1);
  points[1] = center + halfExtent * Vec3(-1, +1, +1);
  points[2] = center + halfExtent * Vec3(+1, -1, +1);
  points[3] = center + halfExtent * Vec3(-1, -1, +1);
  points[4] = center + halfExtent * Vec3(+1, +1, -1);
  points[5] = center + halfExtent * Vec3(-1, +1, -1);
  points[6] = center + halfExtent * Vec3(+1, -1, -1);
  points[7] = center + halfExtent * Vec3(-1, -1, -1);
}

// Verifies that transforming all 8 points of the aabb gives the same result as the optimized transform function
void TestAabbTransform(CppUnitLite::TestResult& result_, const char * m_name, const Matrix4& transform, Aabb& inputAabb, real epsilon)
{
  Vec3 points[8];
  GetAabbPoints(inputAabb, points);

  Aabb resultAabb = inputAabb;
  resultAabb = resultAabb.TransformAabb(transform);

  for(size_t i = 0; i < 8; ++i)
    points[i] = Math::MultiplyPoint(transform, points[i]);

  Aabb expectedAabb;
  expectedAabb.Compute(points, 8);

  CHECK_VEC3_CLOSE(expectedAabb.mMin, resultAabb.mMin, epsilon);
  CHECK_VEC3_CLOSE(expectedAabb.mMax, resultAabb.mMax, epsilon);
}

// Verifies that inverse transforming all 8 points of the aabb gives the same result as the optimized inverse transform function
void TestAabbInverseTransform(CppUnitLite::TestResult& result_, const char * m_name, const Matrix4& transform, Aabb& inputAabb, real epsilon)
{
  Vec3 points[8];
  GetAabbPoints(inputAabb, points);

  Matrix4 inverseTransform = transform.Inverted();
  Aabb resultAabb = Aabb::InverseTransform(inputAabb, transform);
  //resultAabb = inputAabb.TransformAabb(inverseTransform);

  for(size_t i = 0; i < 8; ++i)
    points[i] = Math::MultiplyPoint(inverseTransform, points[i]);

  Aabb expectedAabb;
  expectedAabb.Compute(points, 8);

  CHECK_VEC3_CLOSE(expectedAabb.mMin, resultAabb.mMin, epsilon);
  CHECK_VEC3_CLOSE(expectedAabb.mMax, resultAabb.mMax, epsilon);
}

TEST(AabbTransform_1)
{
  Vector3 scale(Vector3(2, 2, 2));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(1, 0, 0)).Normalized(), 0);
  Vector3 translation(Vector3(0, 0, 0));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbTransform(result_, m_name, transform, aabb, 0.0001f);
}

TEST(AabbTransform_2)
{
  Vector3 scale(Vector3(1, 1, 1));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(1, 0, 0)).Normalized(), 0);
  Vector3 translation(Vector3(1, 2, 3));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbTransform(result_, m_name, transform, aabb, 0.0001f);
}

TEST(AabbTransform_3)
{
  // Rotate about (1, 0, 0) by 45 degrees
  Vector3 scale(Vector3(1, 1, 1));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(1, 0, 0)).Normalized(), 0.785398f);
  Vector3 translation(Vector3(0, 0, 0));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbTransform(result_, m_name, transform, aabb, 0.0001f);
}

// Scale by 2, Rotate about (1, 1, 0) by 45 degrees, translate by (0, 1, 0)
TEST(AabbTransform_4)
{
  Vector3 scale(Vector3(2, 2, 2));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(1, 1, 0)).Normalized(), 0.785398f);
  Vector3 translation(Vector3(0, 1, 0));

  Aabb aabb(Vector3(0, 0, 0), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbTransform(result_, m_name, transform, aabb, 0.0001f);
}

TEST(AabbTransform_5)
{
  Vector3 scale(Vector3(1, 1, 1));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(0, 1, 0)).Normalized(), 0.785398f);
  Vector3 translation(Vector3(0, 0, 0));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbTransform(result_, m_name, transform, aabb, 0.0001f);
}

TEST(AabbTransform_6)
{
  Vector3 scale(Vector3(1, 1, 1));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(0, 0, 1)).Normalized(), 0.785398f);
  Vector3 translation(Vector3(0, 0, 0));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbTransform(result_, m_name, transform, aabb, 0.0001f);
}

TEST(AabbTransform_7)
{
  Vector3 scale(Vector3(1, 1, 1));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(1, 0, 0)).Normalized(), 3.14159f);
  Vector3 translation(Vector3(0, 0, 0));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbTransform(result_, m_name, transform, aabb, 0.0001f);
}

TEST(AabbTransform_8)
{
  Vector3 scale(Vector3(-1, 2, -3));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(0, 0, 1)).Normalized(), 0.785398f);
  Vector3 translation(Vector3(1, 1, 1));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbTransform(result_, m_name, transform, aabb, 0.0001f);
}

TEST(AabbTransform_9)
{
  Vector3 scale(Vector3(-1, 2, -3));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(1, 0, 1)).Normalized(), 0.785398f);
  Vector3 translation(Vector3(2, -3, 4));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbTransform(result_, m_name, transform, aabb, 0.0001f);
}

TEST(AabbInverseTransform_1)
{
  Vector3 scale(Vector3(2, 2, 2));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(1, 0, 0)).Normalized(), 0);
  Vector3 translation(Vector3(0, 0, 0));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbInverseTransform(result_, m_name, transform, aabb, 0.0001f);
}

TEST(AabbInverseTransform_2)
{
  Vector3 scale(Vector3(1, 1, 1));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(1, 0, 0)).Normalized(), 0);
  Vector3 translation(Vector3(1, 2, 3));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbInverseTransform(result_, m_name, transform, aabb, 0.0001f);
}

TEST(AabbInverseTransform_3)
{
  // Rotate about (1, 0, 0) by 45 degrees
  Vector3 scale(Vector3(1, 1, 1));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(1, 0, 0)).Normalized(), 0.785398f);
  Vector3 translation(Vector3(0, 0, 0));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbInverseTransform(result_, m_name, transform, aabb, 0.0001f);
}

// Scale by 2, Rotate about (1, 1, 0) by 45 degrees, translate by (0, 1, 0)
TEST(AabbInverseTransform_4)
{
  Vector3 scale(Vector3(2, 2, 2));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(1, 1, 0)).Normalized(), 0.785398f);
  Vector3 translation(Vector3(0, 1, 0));

  Aabb aabb(Vector3(0, 0, 0), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbInverseTransform(result_, m_name, transform, aabb, 0.0001f);
}

TEST(AabbInverseTransform_5)
{
  Vector3 scale(Vector3(1, 1, 1));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(0, 1, 0)).Normalized(), 0.785398f);
  Vector3 translation(Vector3(0, 0, 0));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbInverseTransform(result_, m_name, transform, aabb, 0.0001f);
}

TEST(AabbInverseTransform_6)
{
  Vector3 scale(Vector3(1, 1, 1));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(0, 0, 1)).Normalized(), 0.785398f);
  Vector3 translation(Vector3(0, 0, 0));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbInverseTransform(result_, m_name, transform, aabb, 0.0001f);
}

TEST(AabbInverseTransform_7)
{
  Vector3 scale(Vector3(1, 1, 1));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(1, 0, 0)).Normalized(), 3.14159f);
  Vector3 translation(Vector3(0, 0, 0));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbInverseTransform(result_, m_name, transform, aabb, 0.0001f);
}

TEST(AabbInverseTransform_8)
{
  Vector3 scale(Vector3(-1, 2, -3));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(0, 0, 1)).Normalized(), 0.785398f);
  Vector3 translation(Vector3(1, 1, 1));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbInverseTransform(result_, m_name, transform, aabb, 0.0001f);
}

TEST(AabbInverseTransform_9)
{
  Vector3 scale(Vector3(-1, 2, -3));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(1, 0, 1)).Normalized(), 0.785398f);
  Vector3 translation(Vector3(2, -3, 4));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform = BuildTransform(translation, rotation, scale);
  TestAabbInverseTransform(result_, m_name, transform, aabb, 0.0001f);
}

TEST(AabbInverseTransform_10)
{
  Vector3 scale(Vector3(-1, 2, -3));
  Matrix3 rotation = Math::ToMatrix3(Vector3(Vector3(1, 0, 1)).Normalized(), 0.785398f);
  Vector3 translation(Vector3(2, -3, 4));

  Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));
  Matrix4 transform0 = BuildTransform(translation, rotation, scale);

  scale = (Vector3(2, 2, 2));
  rotation = Math::ToMatrix3(Vector3(Vector3(1, 1, 0)).Normalized(), 0.785398f);
  translation = (Vector3(0, 1, 0));
  Matrix4 transform1 = BuildTransform(translation, rotation, scale);

  Matrix4 fullTransform = Math::Multiply(transform1, transform0);

  TestAabbInverseTransform(result_, m_name, fullTransform, aabb, 0.0001f);
}