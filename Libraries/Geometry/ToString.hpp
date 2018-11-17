///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

String ToString(const Ray& value, bool shortFormat = false);
String ToString(const Segment& value, bool shortFormat = false);
String ToString(const Aabb& value,   bool shortFormat = false);
String ToString(const Sphere& value, bool shortFormat = false);
String ToString(const Plane& value, bool shortFormat = false);
String ToString(const Frustum& value, bool shortFormat = false);

}//namespace Zero
