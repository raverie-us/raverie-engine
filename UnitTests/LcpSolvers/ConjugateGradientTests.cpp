///////////////////////////////////////////////////////////////////////////////
///
///  \file ConjugateGradientTests.cpp
///  Unit tests for Conjugate Gradient Solver
///  
///  Authors: Joshua Davis
///  Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"

#include "LcpStandard.hpp"
#include "Math/ConjugateGradient.hpp"
#include "Math/SimpleCgPolicies.hpp"

#include "MatrixGenerators.hpp"
#include "BlockMatrixSuite.hpp"
#include "Matrix3Suite.hpp"
#include "FormatErrorCallbacks.hpp"
#include "TestContainers.hpp"

void GenerateBadMat3Case()
{
  real maxValue = real(500.0);

  for(uint i = 0; i < 99999/*uint(-1)*/; ++i)
  {
    Vec3 b;
    Vec3 expected;
    Mat3 A;

    GeneratePosDefMat3(A,expected,b,maxValue);

    Vec3 x0;
    x0.ZeroOut();

    FormatMat3Test(A,b,x0,expected);

    Math::ConjugateGradientSolver cg;
    cg.mErrorTolerance = 0.0001f;
    cg.mMaxIterations = 30;
    Math::SimpleCgPolicy policy;
    SimpleUnitTestErrorCallback errCallback;
    errCallback.mExpected = expected;
    cg.Solve(A,b,x0,policy,errCallback);
  }
}

void GenerateBadSimpleBlockCase(uint dim)
{
  real maxValue = real(10.0);
  
  for(uint iteration = 0; iteration < 9999/*uint(-1)*/; ++iteration)
  {
    BlockVec3 b;
    BlockVec3 expected;
    BlockMat3 A;

    GeneratePosDefBlockMat3(dim,A,expected,b,maxValue);
  
    BlockVec3 x0;
    x0.SetSize(dim);
  
    Math::ConjugateGradientSolver cg;
    cg.mErrorTolerance = real(.001);
    cg.mMaxIterations = 100;
    Math::BlockCgPolicy policy;
    BlockUnitTestErrorCallback errCallback;
    errCallback.mExpected = expected;

    FormatBlockMat3Test(A,b,x0,expected);

    cg.Solve(A,b,x0,policy,errCallback);
  }
}

TestNormalMatrices(Math::ConjugateGradientSolver,Math::SimpleCgPolicy,CG,50);
TestBlockMatrices(Math::ConjugateGradientSolver,Math::BlockCgPolicy,CG,100);
RandomTestNormalMatrices(Math::ConjugateGradientSolver,Math::SimpleCgPolicy,CG,50,1000);
RandomTestBlockMatrices(Math::ConjugateGradientSolver,Math::BlockCgPolicy,CG,100,12,1000);

//----------------------------------------------------------- CG_SparseMatrix_Test1
//TEST(CG_SparseMatrix_Test1)
//{
//  Math::ConjugateGradientSolver cg;
//
//  SparseVector b;
//  b.Set(0,Vec3(real(24.94),real(7.81),real(19.27)));
//  b.Set(1,Vec3(real(-1.2),real(3.1),real(9.0)));
//
//  SparseVector x0;
//  SparseBlockMatrix A;
//  A.Set(0,0,Mat3(real(9.5),real(2.2),real(5.4),
//    real(2.2),real(4.2),real(3.1),
//    real(5.4),real(3.1),real(7.7)));
//  A.Set(1,1,Mat3().SetIdentity());
//
//  SparceBlockCgPolicy policy;
//  cg.Solve(A,b,x0,policy);
//
//  SparseVector expected;
//  expected.Set(0,Vec3(real(2.0),real(0.0),real(1.1)));
//  expected.Set(1,Vec3(real(-1.2),real(3.1),real(9.0)));
//
//  CHECK_VEC3_CLOSE(expected.Get(0), x0.Get(0), real(0.001));
//  CHECK_VEC3_CLOSE(expected.Get(1), x0.Get(1), real(0.001));
//}

//hilbert matrices are incredibly ill-conditioned, meaning that as the hilber matrix gets larger, it is nearly impossible
//to converge to a "correct" answer. The reason is that with minor changes to b it wildly changes x.
void GenerateHilbertTest(uint dim, Math::BlockMatrix3& A, Math::BlockVector3& b, Math::BlockVector3& expected)
{
  A.SetSize(dim);
  for(uint j = 0; j < dim; ++j)
  {
    for(uint i = 0; i < dim; ++i)
    {
      Mat3 m;
      for(uint r = 0; r < 3; ++r)
      {
        for(uint c = 0; c < 3; ++c)
          m(r,c) = real(1.0) / (real(i * 3 + c + 1) + real(j * 3 + r + 1) - 1);
      }
      A(j,i) = m;
    }
  }

  expected.SetSize(dim);
  for(uint i = 0; i < dim; ++i)
    expected[i] = Vec3(real(i + 1),real(i + 2),real(i + 3));

  b.SetSize(dim);
  A.Transform(expected,b);
}
