#include "Precompiled.hpp"
using namespace Zilch;

Array<Real3> GetTemporaryArray();

Integer Test12()
{
  Array<Real3> array;
  array.PushBack(Real3(53.4f, 1, -45));
  array.Resize(5);

  // Because Real3 doesn't have a zeroing constructor
  memset(array.Data() + 1, 0, sizeof(Real3) * 4);

  array[1] = array[0];
  array[2] += array[0];
  array[3] = array[0] * 5;
  array[4] = Real3(array[2].y);
  
  array[(Integer)array[4].x] += GetTemporaryArray()[1];

  HashMap<String, Integer> map;
  map["Hello"] = 5;
  map["World"] = 9;
  map["Hello"] += 100;

  Real total = 0.0f;

  ZilchForEach(Real3& val, array.All())
  {
    total += val.x + val.y + val.z;
  }

  total += map["Hello"] + map["World"];

  return (Integer)total;
}


Array<Real3> GetTemporaryArray()
{
  Array<Real3> array;
  array.PushBack(Real3(0, 1, 2));
  array.PushBack(Real3(3, 4, 5));
  array.PushBack(Real3(6, 7, 8));
  return array;
}
