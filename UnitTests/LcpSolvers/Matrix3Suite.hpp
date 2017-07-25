///////////////////////////////////////////////////////////////////////////////
///
///  \file Matrix3Suite.hpp
///  A collection of Matrix 3 tests for all LCP solvers. Makes it so I don't
///  have to copy-paste these tests all over the place and it's easy to add
///  new tests to all LCP solvers.
///  
///  Authors: Joshua Davis
///  Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "LcpStandard.hpp"

inline void Mat3Test1(Mat3Ref A,Vec3Ref b,Vec3Ref expected,Vec3Ref x0)
{
  A = Mat3(real(177573.187500),real(57317.097656),real(-103210.789063),real(57317.097656),real(239940.984375),real(-41279.476563),real(-103210.789063),real(-41279.476563),real(231855.078125));
  b = Vec3(real(42245376.000000), real(5216578.500000), real(-93206272.000000));
  x0 = Vec3(real(0.000000), real(0.000000), real(0.000000));
  expected = Vec3(real(21.256142), real(-52.476578), real(-401.882996));
}

inline void Mat3Test2(Mat3Ref A,Vec3Ref b,Vec3Ref expected,Vec3Ref x0)
{
  A = Mat3(real(34.685570),real(-14.240574),real(0.083067),real(-14.240574),real(10.295238),real(-1.905839),real(0.083067),real(-1.905839),real(28.342457));
  b = Vec3(real(132.243332), real(-75.510170), real(108.149208));
  x0 = Vec3(real(0.000000), real(0.000000), real(0.000000));
  expected = Vec3(real(2.466048), real(-3.258919), real(3.589434));
}

inline void Mat3Test3(Mat3Ref A,Vec3Ref b,Vec3Ref expected,Vec3Ref x0)
{
  A = Mat3(real(29.375662),real(11.995889),real(0.624546),real(11.995889),real(61.459858),real(-13.194157),real(0.624546),real(-13.194157),real(7.865818));
  b = Vec3(real(-21.721247), real(247.572113), real(-83.606689));
  x0 = Vec3(real(0.000000), real(0.000000), real(0.000000));
  expected = Vec3(real(-2.027192), real(3.401440), real(-4.762566));
}

inline void Mat3Test4(Mat3Ref A,Vec3Ref b,Vec3Ref expected,Vec3Ref x0)
{
  A = Mat3(real(14.588331),real(-11.506293),real(-15.435483),real(-11.506293),real(17.290642),real(3.481298),real(-15.435483),real(3.481298),real(30.365955));
  b = Vec3(real(-8.723238), real(-33.988770), real(60.082607));
  x0 = Vec3(real(0.000000), real(0.000000), real(0.000000));
  expected = Vec3(real(-1.544389), real(-3.310190), real(1.573077));
}

inline void Mat3Test5(Mat3Ref A,Vec3Ref b,Vec3Ref expected,Vec3Ref x0)
{
  A = Mat3(real(11.132988),real(-5.765055),real(13.691436),real(-5.765055),real(10.339767),real(-8.569814),real(13.691436),real(-8.569814),real(19.197058));
  b = Vec3(real(-97.663353), real(82.067528), real(-136.028229));
  x0 = Vec3(real(0.000000), real(0.000000), real(0.000000));
  expected = Vec3(real(-1.321909), real(3.346812), real(-4.649037));
}

struct MatrixTestConfig
{
  MatrixTestConfig()
  {;
    mMaxIterations = 100;
    mTolerance = real(.0000001);
    mErrorThreshold = real(0.1);
  }

  uint mMaxIterations;
  real mTolerance;
  real mErrorThreshold;
};

template <typename SolverType, typename PolicyType, typename TestFunctor>
void TestMat3Template(CppUnitLite::TestResult& result_,const char * m_name, MatrixTestConfig& config, TestFunctor functor)
{
  Vec3 b;
  Vec3 expected;
  Mat3 A;
  Vec3 x0;

  functor(A,b,expected,x0);

  SolverType solver;
  PolicyType policy;
  solver.mMaxIterations = config.mMaxIterations;
  solver.mErrorTolerance = config.mTolerance;

  solver.Solve(A,b,x0,policy);

  CHECK_VEC3_CLOSE(expected, x0, config.mErrorThreshold);
}



#define CreateMat3Test(SolverType, PolicyType, testName, testToRun, maxIterations) \
  TEST(testName##_BlockMatSuite_##testToRun)                                       \
  {                                                                                \
    MatrixTestConfig config;                                                       \
    config.mMaxIterations = maxIterations;                                         \
    TestMat3Template<SolverType,PolicyType>(result_,m_name,config,testToRun);      \
  }

#define TestNormalMatrices(SolverType, PolicyType, testName, maxIterations) \
  CreateMat3Test(SolverType, PolicyType,testName,Mat3Test1,maxIterations)   \
  CreateMat3Test(SolverType, PolicyType,testName,Mat3Test2,maxIterations)   \
  CreateMat3Test(SolverType, PolicyType,testName,Mat3Test3,maxIterations)   \
  CreateMat3Test(SolverType, PolicyType,testName,Mat3Test4,maxIterations)   \
  CreateMat3Test(SolverType, PolicyType,testName,Mat3Test5,maxIterations)


#define RandomTestNormalMatrices(SolverType, PolicyType, testName, maxIterations, testsToRun) \
  TEST(testName##_Mat3_Random)                                                                \
  {                                                                                           \
    for(uint i = 0; i < testsToRun; ++i)                                                      \
    {                                                                                         \
      SolverType solver;                                                                      \
      Vec3 b,expected;                                                                        \
      Mat3 A;                                                                                 \
      Vec3 x0(0,0,0);                                                                         \
      GeneratePosDefMat3(A,expected,b,real(1.0));                                             \
      solver.mMaxIterations = maxIterations;                                                  \
      PolicyType policy;                                                                      \
                                                                                              \
      FormatMat3Test(A,b,x0,expected);                                                        \
                                                                                              \
      solver.Solve(A,b,x0,policy);                                                            \
                                                                                              \
      CHECK_VEC3_CLOSE(expected, x0, real(0.1));                                              \
    }                                                                                         \
  }

