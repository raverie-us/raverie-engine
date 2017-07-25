///////////////////////////////////////////////////////////////////////////////
///
///  \file TestBlockMath.hpp
///  Test macros for block vectors and matrices.
///  
///  Authors: Joshua Davis
///  Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace CppUnitLite
{

///Specialized CHECK_ARRAY_CLOSE that assumes arrays of size 3 and compares
///values exactly
#define CHECK_BLOCK_VEC3(expected,actual,dim)\
  for(uint i = 0; i < dim; ++i) \
    CHECK_ARRAY_EQUAL(expected[i],actual[i],3)

///Specialized CHECK_ARRAY_CLOSE that assumes arrays of size 3
#define CHECK_BLOCK_VEC3_CLOSE(expected,actual,dim,kEpsilon)\
  for(uint i = 0; i < dim; ++i) \
    CHECK_ARRAY_CLOSE(expected[i],actual[i],3,kEpsilon)

#define CHECK_BLOCK_MAT3(expected,actual,dim)\
  for(uint i = 0; i < dim; ++i) \
    for(uint j = 0; j < dim; ++j) \
      CHECK_MAT3(expected(i,j),actual(i,j))

#define CHECK_BLOCK_MAT3_CLOSE(expected,actual,dim,kEpsilon)\
  for(uint i = 0; i < dim; ++i) \
    for(uint j = 0; j < dim; ++j) \
      CHECK_MAT3_CLOSE(expected(i,j),actual(i,j),kEpsilon)

}
