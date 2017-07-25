///////////////////////////////////////////////////////////////////////////////
///
/// \file SimConversion.hpp
/// Declaration of the functionality for the SimVec.
/// 
/// Authors: Joshua Davis
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Math
{

namespace Simd
{

SimInline SimVec LoadVec3(Vec3Param v)
{
  return Set3(v.x,v.y,v.z);
}

SimInline SimVec LoadVec4(Vec4Param v)
{
  return Load(v.array);
}

SimInline SimMat4 LoadMat4(Mat4Param m)
{
  return LoadMat4x4(m.array);
}

SimInline void UnLoadVec3(SimVecParam simVec, Vec3Ref v)
{
  float temp[4];
  UnAlignedStore(simVec,temp);
  v = Vec3(temp);
}

SimInline void UnLoadVec4(SimVecParam simVec, Vec4Ref v)
{
  Store(simVec,v.array);
}

SimInline void UnLoadMat4(SimMat4Param simMat, Mat4Ref m)
{
  StoreMat4x4(m.array,simMat);
}

}//Simd

}//Math
