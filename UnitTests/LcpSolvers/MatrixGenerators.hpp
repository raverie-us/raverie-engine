///////////////////////////////////////////////////////////////////////////////
///
///  \file MatrixGenerators.hpp
///  Code to generate random positive definite matrices.
///  
///  Authors: Joshua Davis
///  Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "LcpStandard.hpp"
#include "Math/Random.hpp"

inline void GenerateRandomMat3(Mat3Ref A, real maxValue)
{
  Random random;
  for(uint y = 0; y < 3; ++y)
  {
    for(uint x = 0; x < 3; ++x)
      A(y,x) = random.FloatRange(-maxValue,maxValue);
  }
}

inline void GenerateRandomVec3(Vec3Ref v, real maxValue)
{
  Random random;
  for(uint x = 0; x < 3; ++x)
    v[x] = random.FloatRange(-maxValue,maxValue);
}

inline void GeneratePosDefMat3(Mat3Ref A, Vec3Ref expected, Vec3Ref b, real maxValue)
{
  //make a random semi-positive definite matrix
  GenerateRandomMat3(A,maxValue);
  A = A.Transposed() * A;

  //this will force A to become positive definite
  for(uint i = 0; i < 3; ++i)
    A(i,i) += real(.5);

  GenerateRandomVec3(expected,maxValue);

  b = Math::Transform(A,expected);
}

inline void GeneratePosDefBlockMat3(uint dim, BlockMat3Ref A, BlockVec3Ref expected, BlockVec3Ref b, real maxValue)
{
  A.SetSize(dim);
  expected.SetSize(dim);
  b.SetSize(dim);

  for(uint j = 0; j < dim; ++j)
  {
    for(uint i = 0; i < dim; ++i)
    {
      GenerateRandomMat3(A(j,i),maxValue);
    }
  }

  A = A.Transposed().Transform(A);

  for(uint i = 0; i < dim; ++i)
  {
    A(i,i)(0,0) += real(.5);
    A(i,i)(1,1) += real(.5);
    A(i,i)(2,2) += real(.5);
  }

  for(uint j = 0; j < dim; ++j)
    GenerateRandomVec3(expected[j],maxValue);

  A.Transform(expected,b);
}
