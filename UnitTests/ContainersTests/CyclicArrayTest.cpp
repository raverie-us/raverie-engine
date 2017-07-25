///////////////////////////////////////////////////////////////////////////////
///
///  \file CyclicArrayTest.cpp
///  Unit tests for the cyclic array.
///  
///  Authors: Joshua Claeys
///  Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "ContainerTestStandard.hpp"
#include "CppUnitLite2/CppUnitLite2.h"

typedef Zero::CyclicArray<int> CyclicArray;
typedef Zero::Array<int> Array;

//----------------------------------------------------------- Zero (Static Data)
TEST(Push_1)
{
  CyclicArray actual(1);
  CHECK_EQUAL(0, actual.Size());
  actual.Push(2);
  actual.Push(1);
  actual.Push(0);

  Array expected(1);
  expected[0] = 0;

  CHECK_EQUAL(1, actual.Size());
  CHECK_EQUAL(1, actual.GetMaxSize());
  CHECK_ARRAY_EQUAL(expected, actual, 1);
  CHECK_EQUAL(true, Equal(expected.All(), actual.All()));
}

TEST(Push_2)
{
  CyclicArray actual(3);
  actual.Push(2);
  actual.Push(1);
  actual.Push(0);

  Array expected(3);
  for(uint i = 0; i < 3; ++i)
    expected[i] = i;

  CHECK_EQUAL(3, actual.Size());
  CHECK_EQUAL(3, actual.GetMaxSize());
  CHECK_ARRAY_EQUAL(expected, actual, 3);
  CHECK_EQUAL(true, Equal(expected.All(), actual.All()));
}

TEST(Push_3)
{
  CyclicArray actual(3);
  actual.Push(2);
  actual.Push(1);
  actual.Push(0);

  actual.Clear();
  actual.Push(3);
  actual.Push(4);

  Array expected(2);
  expected[0] = 4;
  expected[1] = 3;

  CHECK_EQUAL(2, actual.Size());
  CHECK_EQUAL(3, actual.GetMaxSize());
  CHECK_ARRAY_EQUAL(expected, actual, 2);
  CHECK_EQUAL(true, Equal(expected.All(), actual.All()));
}

TEST(Push_4)
{
  CyclicArray actual(10);
  for(uint i = 0; i < 15; ++i)
    actual.Push(14 - i);

  Array expected(10);
  for(uint i = 0; i < 10; ++i)
    expected[i] = i;

  CHECK_EQUAL(10, actual.Size());
  CHECK_EQUAL(10, actual.GetMaxSize());
  CHECK_ARRAY_EQUAL(expected, actual, 10);
  CHECK_EQUAL(true, Equal(expected.All(), actual.All()));
}

TEST(ChangeMaxSize_1)
{
  CyclicArray actual(3);
  actual.Push(2);
  actual.Push(1);
  actual.Push(0);
  actual.SetMaxSize(5);

  Array expected(3);
  for(uint i = 0; i < 3; ++i)
    expected[i] = i;

  CHECK_EQUAL(3, actual.Size());
  CHECK_EQUAL(5, actual.GetMaxSize());
  CHECK_ARRAY_EQUAL(expected, actual, 3);
  CHECK_EQUAL(true, Equal(expected.All(), actual.All()));
}

TEST(ChangeMaxSize_2)
{
  CyclicArray actual(3);
  actual.Push(1);
  actual.Push(0);
  actual.SetMaxSize(5);

  Array expected(2);
  for(uint i = 0; i < 2; ++i)
    expected[i] = i;

  CHECK_EQUAL(2, actual.Size());
  CHECK_EQUAL(5, actual.GetMaxSize());
  CHECK_ARRAY_EQUAL(expected, actual, 2);
  CHECK_EQUAL(true, Equal(expected.All(), actual.All()));
}

TEST(ChangeMaxSize_3)
{
  CyclicArray actual(10);
  for(uint i = 0; i < 14; ++i)
    actual.Push(i);
  CHECK_EQUAL(10, actual.Size());
  CHECK_EQUAL(10, actual.GetMaxSize());
  actual.SetMaxSize(5);

  Array expected(5);
  expected[0] = 13;
  expected[1] = 12;
  expected[2] = 11;
  expected[3] = 10;
  expected[4] = 9;

  CHECK_EQUAL(5, actual.Size());
  CHECK_EQUAL(5, actual.GetMaxSize());
  CHECK_ARRAY_EQUAL(expected, actual, 5);
  CHECK_EQUAL(true, Equal(expected.All(), actual.All()));
}

TEST(FrontBack)
{
  CyclicArray actual(10);
  for(uint i = 0; i < 14; ++i)
    actual.Push(i);
  actual.SetMaxSize(5);
  actual.PopFront();

  CHECK_EQUAL(12, actual.Front());
  CHECK_EQUAL(9, actual.Back());
}

TEST(PopFront)
{
  CyclicArray actual(10);
  for(uint i = 0; i < 14; ++i)
    actual.Push(i);
  CHECK_EQUAL(10, actual.Size());
  CHECK_EQUAL(10, actual.GetMaxSize());
  actual.SetMaxSize(5);
  actual.PopFront();
  CHECK_EQUAL(4, actual.Size());
  CHECK_EQUAL(5, actual.GetMaxSize());

  Array expected(4);
  expected[0] = 12;
  expected[1] = 11;
  expected[2] = 10;
  expected[3] = 9;

  CHECK_ARRAY_EQUAL(expected, actual, 4);
  CHECK_EQUAL(true, Equal(expected.All(), actual.All()));
}

TEST(PopBack)
{
  CyclicArray actual(10);
  for(uint i = 0; i < 14; ++i)
    actual.Push(i);
  actual.SetMaxSize(5);
  actual.PopBack();

  Array expected(4);
  expected[0] = 13;
  expected[1] = 12;
  expected[2] = 11;
  expected[3] = 10;

  CHECK_ARRAY_EQUAL(expected, actual, 4);
  CHECK_EQUAL(true, Equal(expected.All(), actual.All()));
}

TEST(Clear)
{
  CyclicArray actual(10);
  for(uint i = 0; i < 14; ++i)
    actual.Push(i);
  actual.SetMaxSize(5);
  actual.PopBack();
  actual.Clear();

  CHECK_EQUAL(0, actual.Size());
}

TEST(MaxSize_1)
{
  CyclicArray actual(0);

  CHECK_EQUAL(0, actual.Size());
  CHECK_EQUAL(0, actual.GetMaxSize());
}

TEST(MaxSize_2)
{
  CyclicArray actual(10);

  CHECK_EQUAL(0, actual.Size());
  CHECK_EQUAL(10, actual.GetMaxSize());
}
