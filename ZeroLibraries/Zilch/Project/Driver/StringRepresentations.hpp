#pragma once
using namespace Zilch;

inline String TestToString(Real val)
{
  return String::Format("[%g]", val);
}

inline String TestToString(const Real2& val)
{
  return String::Format("[%g,%g]", val.x, val.y);
}

inline String TestToString(const Real3& val)
{
  return String::Format("[%g,%g,%g]", val.x, val.y, val.z);
}

inline String TestToString(const Real4& val)
{
  return String::Format("[%g,%g,%g,%g]", val.x, val.y, val.z, val.w);
}

inline String TestToString(const Quaternion& val)
{
  return String::Format("[%g,%g,%g,%g]", val.x, val.y, val.z, val.w);
}

inline String TestToString(Integer val)
{
  return String::Format("[%d]", val);
}

inline String TestToString(StringParam val)
{
  return val;
}
