///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

String ToString(const Ray& value, bool shortFormat)
{
  String start = ToString(value.Start, shortFormat);
  String dir = ToString(value.Direction, shortFormat);
  return String::Format("(%s), (%s)", start.c_str(), dir.c_str());
}

String ToString(const Segment& value, bool shortFormat)
{
  String start = ToString(value.Start, shortFormat);
  String end = ToString(value.End, shortFormat);
  return String::Format("(%s), (%s)", start.c_str(), end.c_str());
}

String ToString(const Aabb& value, bool shortFormat)
{
  String minStr = ToString(value.mMin, shortFormat);
  String maxStr = ToString(value.mMax, shortFormat);
  return String::Format("(%s), (%s)", minStr.c_str(), maxStr.c_str());
}

String ToString(const Sphere& value, bool shortFormat)
{
  String centerStr = ToString(value.mCenter, shortFormat);
  return String::Format("(%s), %g", centerStr.c_str(), value.mRadius);
}

String ToString(const Plane& value, bool shortFormat)
{
  String dataStr = ToString(value.mData, shortFormat);
  return String::Format("(%s)", dataStr.c_str());
}

String ToString(const Frustum& value, bool shortFormat)
{
  Vec3 center;
  value.GetCenter(center);

  String centerStr = ToString(center, shortFormat);
  String aabbStr = ToString(value.GetAabb(), shortFormat);
  return String::Format("(%s), (%s)", centerStr.c_str(), aabbStr.c_str());
}

}//namespace Zero
