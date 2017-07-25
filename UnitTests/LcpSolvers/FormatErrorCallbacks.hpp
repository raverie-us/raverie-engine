///////////////////////////////////////////////////////////////////////////////
///
///  \file FormatErrorCallbacks.hpp
///  Callbacks used in all tests to report when a solver fails to converge.
///  At the moment this doesn't report anything, just formats the data
///  so that it can be turned into a test later.
///  
///  Authors: Joshua Davis
///  Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "LcpStandard.hpp"

#include "String/StringBuilder.hpp"
#include "String/String.hpp"

inline void FormatMat3Test(Mat3Param A, Vec3Param b, Vec3Param x0, Vec3Param expected)
{
  Zero::StringBuilder builder;
  Mat3 m = A;
  builder.Append(Zero::String::Format("A = "
    "Mat3(real(%f),real(%f),real(%f),"
    "real(%f),real(%f),real(%f),"
    "real(%f),real(%f),real(%f));\n",
    m(0, 0), m(0, 1), m(0, 2), m(1, 0), m(1, 1), m(1, 2), m(2, 0), m(2, 1), m(2, 2)));

  builder.Append(Zero::String::Format("b = Vec3(real(%f), real(%f), real(%f));\n", b.x, b.y, b.z));

  builder.Append(Zero::String::Format("x0 = Vec3(real(%f), real(%f), real(%f));\n", x0.x, x0.y, x0.z));

  builder.Append(Zero::String::Format("expected = Vec3(real(%f), real(%f), real(%f));\n", expected.x, expected.y, expected.z));

  Zero::String out = builder.ToString();
  const cstr outCstr = out.c_str();
}

inline void FormatBlockMat3Test(BlockMat3Param A, BlockVec3Param b, BlockVec3Param x0, BlockVec3Param expected)
{
  Zero::StringBuilder builder;
  for(uint i = 0; i < A.GetSize(); ++i)
  {
    for(uint j = 0; j < A.GetSize(); ++j)
    {
      Mat3 m = A(i,j);
      builder.Append(Zero::String::Format("A(%d,%d) = "
        "Mat3(real(%f),real(%f),real(%f),"
        "real(%f),real(%f),real(%f),"
        "real(%f),real(%f),real(%f));\n",
        i, j, m(0, 0), m(0, 1), m(0, 2), m(1, 0), m(1, 1), m(1, 2), m(2, 0), m(2, 1), m(2, 2)));
    }
  }
  for(uint i = 0; i < b.GetSize(); ++i)
    builder.Append(Zero::String::Format("b[%d] = Vec3(real(%f), real(%f), real(%f));\n", i, b[i].x, b[i].y, b[i].z));

  for(uint i = 0; i < x0.GetSize(); ++i)
    builder.Append(Zero::String::Format("x0[%d] = Vec3(real(%f), real(%f), real(%f));\n", i, x0[i].x, x0[i].y, x0[i].z));

  for(uint i = 0; i < expected.GetSize(); ++i)
    builder.Append(Zero::String::Format("expected[%d] = Vec3(real(%f), real(%f), real(%f));\n", i, expected[i].x, expected[i].y, expected[i].z));

  Zero::String out = builder.ToString();
  const cstr outCstr = out.c_str();
}

struct SimpleUnitTestErrorCallback
{
  void operator()(Mat3Param A, Vec3Param b, Vec3Param x0, real convergence)
  {
    FormatMat3Test(A, b, x0, mExpected);
  }

  Vec3 mExpected;
};

struct BlockUnitTestErrorCallback
{
  void operator()(BlockMat3Param A, BlockVec3Param b, BlockVec3Param x0, real convergence)
  {
    FormatBlockMat3Test(A, b, x0, mExpected);
  }

  BlockVec3 mExpected;
};
