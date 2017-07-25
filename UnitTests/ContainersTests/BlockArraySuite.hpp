#pragma once

#include "ContainerTestStandard.hpp"

namespace Math
{

std::ostream& operator<<(std::ostream& out, Vec3Param v)
{
  out << "[";
  for(uint i = 0; i < 3; ++i)
    out << v[i] << " ";
  out << "]";
  return out;
}

}

struct PolicyUint
{
  // Currently rely on the policy to define how to sort these types
  bool operator()(uint lhs, uint rhs)
  {
    return lhs < rhs;
  }

  static uint GetValue1()
  {
    return 42;
  }

  static uint GetValue2()
  {
    return 12;
  }

  static uint GetValueAt(uint index)
  {
    return index;
  }

  template <typename type1, typename type2>
  static void CheckEqual(CppUnitLite::TestResult& result_,const char * m_name,
                         const type1& expected, const type2& actual)
  {
    CHECK_EQUAL(expected,actual);
  }

  template <typename type1, typename type2>
  static void CheckArrayEqual(CppUnitLite::TestResult& result_,const char * m_name,
                              const type1& expected, const type2& actual, uint size)
  {
    CHECK_ARRAY_EQUAL(expected,actual,size);
  }
};

struct PolicyVec3
{
  // Currently rely on the policy to define how to sort these types
  bool operator()(Math::Vec3Param lhs, Math::Vec3Param rhs)
  {
    return lhs.x < rhs.x;
  }

  static Vec3 GetValue1()
  {
    return Vec3(42,43,44);
  }

  static Vec3 GetValue2()
  {
    return Vec3(12,13,14);
  }

  static Vec3 GetValueAt(uint index)
  {
    return Vec3((real)index,(real)index,(real)index);
  }

  template <typename type1, typename type2>
  static void CheckEqual(CppUnitLite::TestResult& result_,const char * m_name,
                         const type1& expected, const type2& actual)
  {
    CHECK_EQUAL(expected,actual);
  }

  template <typename type1, typename type2>
  static void CheckArrayEqual(CppUnitLite::TestResult& result_,const char * m_name,
                              const type1& expected, const type2& actual, uint size)
  {
    CHECK_ARRAY_EQUAL(expected,actual,size);
  }

  static void CheckEqual(CppUnitLite::TestResult& result_,const char * m_name,
    const Vec3& expected, const Vec3& actual)
  {
    CHECK_VEC3(expected,actual);
  }
};

template <typename valueType, uint shiftSize, typename Policy>
struct BlockArrayTests
{
  typedef Zero::PodBlockArray<valueType, shiftSize> BlockArrayType;
  typedef Zero::Array<valueType> ArrayType;

#define CheckEqual(expected, actual)\
  Policy::CheckEqual(result_,m_name,expected,actual);

#define CheckArrayEqual(expected, actual, size)\
  Policy::CheckArrayEqual(result_,m_name,expected,actual,size);

  static uint GetSize()
  {
    return (1 << shiftSize);
  }

  static uint GetLargeSize()
  {
    return GetSize() * 5;
  }

  static uint Get50PercentBucketSize()
  {
    return GetSize() / 2;
  }

  static uint Get101PercentBucketSize()
  {
    return GetSize() + 1;
  }

  static uint Get150PercentBucketSize()
  {
    return GetSize() + GetSize() / 2;
  }

  static uint Get201PercentBucketSize()
  {
    return GetSize() * 2 + 1;
  }

  static uint Get300PercentBucketSize()
  {
    return GetSize() * 3;
  }

  static void OperatorBrackets1(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = GetLargeSize();

    BlockArrayType actual;
    for(uint i = 0; i < size; ++i)
      actual.PushBack(Policy::GetValueAt(i));

    ArrayType expected;
    expected.Resize(size);
    for(uint i = 0; i < size; ++i)
      expected[i] = Policy::GetValueAt(i);

    CheckArrayEqual(expected,actual,size);
  }

  static void Size(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = GetLargeSize();

    BlockArrayType actual;
    for(uint i = 0; i < size; ++i)
      actual.PushBack(Policy::GetValueAt(i));

    CheckEqual(size,actual.Size());
  }

  static void Clear(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = GetLargeSize();

    BlockArrayType actual;
    for(uint i = 0; i < size; ++i)
      actual.PushBack(Policy::GetValueAt(i));
    actual.Clear();

    CheckEqual(0,actual.Size());
  }

  static void Resize1(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = GetLargeSize();

    BlockArrayType actual;
    actual.Resize(size);

    CheckEqual(size,actual.Size());
  }

  static void Resize2(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = GetLargeSize();

    BlockArrayType actual;
    actual.Resize(size);
    for(uint i = 0; i < size; ++i)
      actual[i] = Policy::GetValueAt(i);

    CheckEqual(size,actual.Size());
  }

  static void Resize3(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get150PercentBucketSize();

    BlockArrayType actual;
    actual.Resize(size,Policy::GetValue1());

    ArrayType expected;
    expected.Resize(size,Policy::GetValue1());

    CheckArrayEqual(expected,actual,size);
  }

  static void Resize4(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size1 = Get150PercentBucketSize();
    uint size2 = Get300PercentBucketSize();

    BlockArrayType actual;
    actual.Resize(size1,Policy::GetValue1());
    actual.Resize(size2,Policy::GetValue2());

    ArrayType expected;
    expected.Resize(size1,Policy::GetValue1());
    expected.Resize(size2,Policy::GetValue2());

    CheckArrayEqual(expected,actual,size2);
  }

  static void Resize5(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size1 = Get300PercentBucketSize();
    uint size2 = Get150PercentBucketSize();

    BlockArrayType actual;
    for(uint i = 0; i < size1; ++i)
      actual.PushBack(Policy::GetValueAt(i));
    actual.Resize(size2);

    CheckEqual(size2,actual.Size());
    CheckEqual(Policy::GetValueAt(size2 - 1),actual.Back());
  }

  static void Resize6(CppUnitLite::TestResult& result_,const char * m_name)
  {
    BlockArrayType actual;
    actual.Resize(0);

    CheckEqual(0,actual.Size());
  }

  static void Resize7(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get300PercentBucketSize();

    BlockArrayType actual;
    actual.Resize(size);
    actual.Resize(size);

    CheckEqual(size,actual.Size());
  }

  static void Resize8(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get300PercentBucketSize();

    BlockArrayType actual;
    actual.Resize(size,Policy::GetValue1());
    actual.Resize(size,Policy::GetValue1());

    CheckEqual(size,actual.Size());
  }

  static void Resize9(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size1 = Get300PercentBucketSize();
    uint size2 = Get150PercentBucketSize();

    BlockArrayType actual;
    actual.Resize(size1,Policy::GetValue1());
    actual.Resize(size2,Policy::GetValue2());

    ArrayType expected;
    expected.Resize(size1,Policy::GetValue1());
    expected.Resize(size2,Policy::GetValue2());

    CheckArrayEqual(expected,actual,size2);
  }

  static void Resize10(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size1 = Get101PercentBucketSize();
    uint size2 = Get150PercentBucketSize();

    BlockArrayType actual;
    actual.Resize(size1,Policy::GetValue1());
    actual.Resize(size2,Policy::GetValue2());

    ArrayType expected;
    expected.Resize(size1,Policy::GetValue1());
    expected.Resize(size2,Policy::GetValue2());

    CheckArrayEqual(expected,actual,size2);
  }

  static void Resize11(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get300PercentBucketSize();

    BlockArrayType actual;
    actual.Resize(size,Policy::GetValue1());

    ArrayType expected;
    expected.Resize(size,Policy::GetValue1());

    CheckArrayEqual(expected,actual,size);
  }

  static void Resize12(CppUnitLite::TestResult& result_, const char * m_name)
  {
    uint size1 = Get50PercentBucketSize();
    uint size2 = Get300PercentBucketSize() + 1;

    BlockArrayType actual;
    actual.Resize(size1, Policy::GetValue1());
    actual.Resize(size2, Policy::GetValue2());

    ArrayType expected;
    expected.Resize(size1, Policy::GetValue1());
    expected.Resize(size2, Policy::GetValue2());

    CheckArrayEqual(expected, actual, size2);
  }

  static void Empty(CppUnitLite::TestResult& result_,const char * m_name)
  {
    BlockArrayType actual;
    CheckEqual(true,actual.Empty());
  }

  static void PopBack1(CppUnitLite::TestResult& result_,const char * m_name)
  {
    BlockArrayType actual;
    actual.PushBack(Policy::GetValue1());
    actual.PopBack();

    CheckEqual(true,actual.Empty());
  }

  static void PopBack2(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get300PercentBucketSize();
    BlockArrayType actual;
    for(uint i = 0; i < size; ++i)
      actual.PushBack(Policy::GetValueAt(i));
    for(uint i = 0; i < size; ++i)
      actual.PopBack();

    CheckEqual(true,actual.Empty());
  }

  static void PopBack3(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size1 = Get300PercentBucketSize();
    uint size2 = Get150PercentBucketSize();
    BlockArrayType actual;
    for(uint i = 0; i < size1; ++i)
      actual.PushBack(Policy::GetValueAt(i));
    for(uint i = 0; i < size2; ++i)
      actual.PopBack();

    CheckEqual(size2,actual.Size());
    CheckEqual(Policy::GetValueAt(size2 - 1),actual.Back());
  }

  static void PushBack(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get300PercentBucketSize();

    BlockArrayType actual;
    for(uint i = 0; i < size; ++i)
      actual.PushBack() = Policy::GetValueAt(i);

    ArrayType expected;
    for(uint i = 0; i < size; ++i)
      expected.PushBack() = Policy::GetValueAt(i);

    CheckArrayEqual(expected,actual,size);
  }

  static void Front1(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get150PercentBucketSize();

    BlockArrayType actual;
    for(uint i = 0; i < size; ++i)
      actual.PushBack() = Policy::GetValueAt(i);

    CheckEqual(Policy::GetValueAt(0),actual.Front());
  }

  static void Front2(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get150PercentBucketSize();

    BlockArrayType actual;
    for(uint i = 0; i < size; ++i)
      actual.PushBack() = Policy::GetValueAt(i);
    actual[0] = Policy::GetValue1();

    CheckEqual(Policy::GetValue1(),actual.Front());
  }

  static void Copy1(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get201PercentBucketSize();

    BlockArrayType toCopy;
    toCopy.Resize(size,Policy::GetValue1());

    BlockArrayType actual;
    actual.Copy(toCopy);

    ArrayType expected;
    expected.Resize(size,Policy::GetValue1());

    CheckArrayEqual(expected,actual,size);
  }

  static void Copy2(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get201PercentBucketSize();

    BlockArrayType toCopy;

    BlockArrayType actual;
    actual.Copy(toCopy);

    ArrayType expected;

    CheckArrayEqual(expected,actual,0);
  }

  static void Copy3(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get50PercentBucketSize();

    BlockArrayType toCopy;
    toCopy.Resize(size,Policy::GetValue1());

    BlockArrayType actual;
    actual.Copy(toCopy);

    ArrayType expected;

    CheckArrayEqual(expected,actual,0);
  }

  static void Copy4(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get201PercentBucketSize();

    BlockArrayType toCopy;

    BlockArrayType actual;
    actual.Resize(size,Policy::GetValue1());
    actual.Copy(toCopy);

    ArrayType expected;

    CheckArrayEqual(expected,actual,0);
  }

  static void CopyConstructor(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get201PercentBucketSize();

    BlockArrayType toCopy;
    toCopy.Resize(size,Policy::GetValue1());

    BlockArrayType actual(toCopy);

    ArrayType expected;
    expected.Resize(size,Policy::GetValue1());

    CheckArrayEqual(expected,actual,size);
  }

  static void Constructor1(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get201PercentBucketSize();

    BlockArrayType actual(size);
    for(uint i = 0; i < size; ++i)
      actual[i] = Policy::GetValueAt(i);

    ArrayType expected(size);
    for(uint i = 0; i < size; ++i)
      expected[i] = Policy::GetValueAt(i);

    CheckArrayEqual(expected,actual,size);
  }

  static void Constructor2(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get201PercentBucketSize();

    BlockArrayType actual(size,Policy::GetValue1());

    ArrayType expected(size,Policy::GetValue1());

    CheckArrayEqual(expected,actual,size);
  }

  static void OperatorEquals(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get201PercentBucketSize();

    BlockArrayType toCopy;
    toCopy.Resize(size,Policy::GetValue1());

    BlockArrayType actual;
    actual = toCopy;

    ArrayType expected;
    expected.Resize(size,Policy::GetValue1());

    CheckArrayEqual(expected,actual,size);
  }

  static void OperatorEqualEqual1(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get201PercentBucketSize();

    BlockArrayType test1;
    test1.Resize(size,Policy::GetValue1());

    BlockArrayType test2;
    test2.Resize(size,Policy::GetValue1());

    CheckEqual(test1 == test2,true);
  }

  static void OperatorEqualEqual2(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get201PercentBucketSize();

    BlockArrayType test1;
    test1.Resize(size,Policy::GetValue1());

    BlockArrayType test2;
    test2.Resize(size,Policy::GetValue2());

    CheckEqual(test1 == test2,false);
  }

  static void OperatorEqualEqual3(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get201PercentBucketSize();

    BlockArrayType test1;
    test1.Resize(size,Policy::GetValue1());

    BlockArrayType test2;
    test2.Resize(size - 1,Policy::GetValue2());

    CheckEqual(test1 == test2,false);
  }

  static void OperatorEqualEqual4(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get201PercentBucketSize();

    BlockArrayType test1;
    test1.Resize(size,Policy::GetValue1());

    BlockArrayType test2;
    test2.Resize(size - 1,Policy::GetValue1());
    test2.PushBack(Policy::GetValue1());

    CheckEqual(test1 == test2,true);
  }

  static void OperatorEqualEqual5(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get201PercentBucketSize();

    BlockArrayType test1;
    test1.Resize(size,Policy::GetValue1());

    BlockArrayType test2;
    test2.Resize(size,Policy::GetValue1());
    test2[size - 1] = Policy::GetValue2();

    CheckEqual(test1 == test2,false);
  }

  static void ChangeCapacity1(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get201PercentBucketSize();

    BlockArrayType blockArray;
    blockArray.ChangeCapacity(1);

    CheckEqual(true,blockArray.mCapacity == 1 << shiftSize);
  }

  static void ChangeCapacity2(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get201PercentBucketSize();

    BlockArrayType blockArray;
    blockArray.PushBack(Policy::GetValue1());
    blockArray.ChangeCapacity(blockArray.mCapacity + 1);

    CheckEqual(true, blockArray.mCapacity == (1 << shiftSize) * 2);
  }

  static void ChangeCapacity3(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get201PercentBucketSize();

    BlockArrayType blockArray;
    blockArray.PushBack(Policy::GetValue1());
    blockArray.ChangeCapacity(0);

    CheckEqual(true, blockArray.mCapacity == 1 << shiftSize);
  }

  static void SubRange(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get300PercentBucketSize();

    BlockArrayType actualArray;
    for(uint i = 0; i < size; ++i)
      actualArray.PushBack(Policy::GetValueAt(i));

    ArrayType expectedArray;
    for(uint i = 0; i < size; ++i)
      expectedArray.PushBack(Policy::GetValueAt(i));

    uint start = (1 << shiftSize) - 1;
    uint end = size - 1;

    BlockArrayType::range actual = actualArray.SubRange(start, end - start);
    ArrayType::range expected = expectedArray.SubRange(start, end - start);

    CheckEqual(true, Equal(actual,expected));
  }

  static void Iterator1(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get300PercentBucketSize();

    BlockArrayType actualArray;
    for(uint i = 0; i < size; ++i)
      actualArray.PushBack(Policy::GetValueAt(i));

    ArrayType expectedArray;
    for(uint i = 0; i < size; ++i)
      expectedArray.PushBack(Policy::GetValueAt(i));

    CheckEqual(true, *actualArray.Begin() == *expectedArray.Begin());
  }

  static void Iterator2(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get300PercentBucketSize();

    BlockArrayType array1;
    for(uint i = 0; i < size; ++i)
      array1.PushBack(Policy::GetValueAt(i));

    BlockArrayType array2;
    for(uint i = 0; i < size; ++i)
      array2.PushBack(Policy::GetValueAt(i));

    CheckEqual(false, array1.Begin() == array2.Begin());
  }

  static void Sort(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get300PercentBucketSize();

    BlockArrayType actualArray;
    for(uint i = 0; i < size; ++i)
      actualArray.PushBack(Policy::GetValueAt(i));

    ArrayType expectedArray;
    for(uint i = 0; i < size; ++i)
      expectedArray.PushBack(Policy::GetValueAt(i));

    Zero::PodArray<valueType> podArray;
    for(uint i = 0; i < size; ++i)
      podArray.PushBack(Policy::GetValueAt(i));

    Zero::Sort(actualArray.All(), Policy());
    Zero::Sort(expectedArray.All(), Policy());

    CheckArrayEqual(expectedArray,actualArray,size);
  }

  static void RandomAccess1(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = 15;
    uint indices[15] = {8,2,0,4,9,1,5,6,7,3,12,14,10,13,11};

    BlockArrayType actualArray;
    for(uint i = 0; i < size; ++i)
      actualArray.PushBack(Policy::GetValueAt(i));

    ArrayType expectedArray;
    for(uint i = 0; i < size; ++i)
      expectedArray.PushBack(Policy::GetValueAt(i));

    for(uint i = 0; i < size; ++i)
      CheckEqual(expectedArray[indices[i]],actualArray[indices[i]]);
  }

  static void RandomAccess2(CppUnitLite::TestResult& result_,const char * m_name)
  {
    uint size = Get300PercentBucketSize();
    Zero::Array<uint> tempArray;
    for(uint i = 0; i < size; ++i)
      tempArray.PushBack(i);

    Zero::Array<uint> indices;
    while(!tempArray.Empty())
    {
      uint index = std::rand() % tempArray.Size();
      indices.PushBack(tempArray[index]);
      tempArray.EraseAt(index);
    }

    BlockArrayType actualArray;
    for(uint i = 0; i < size; ++i)
      actualArray.PushBack(Policy::GetValueAt(i));

    ArrayType expectedArray;
    for(uint i = 0; i < size; ++i)
      expectedArray.PushBack(Policy::GetValueAt(i));

    for(uint i = 0; i < size; ++i)
      CheckEqual(expectedArray[indices[i]],actualArray[indices[i]]);
  }

#undef CheckEqual

#undef CheckArrayEqual
};

#define CreateBlockArrayTest(valueType,shiftSize,Policy,testName) \
  TEST(BlockArray_##valueType##_##shiftSize##_##testName) \
  {\
    BlockArrayTests<valueType,shiftSize,Policy>::testName(result_,m_name);\
  }

#define CreateBlockArrayTests(valueType, shiftSize, Policy) \
  CreateBlockArrayTest(valueType,shiftSize,Policy,OperatorBrackets1); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Size); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Clear); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Resize1); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Resize2); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Resize3); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Resize4); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Resize5); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Resize6); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Resize7); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Resize8); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Resize9); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Resize10); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Resize11); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Resize12); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Empty); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,PopBack1); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,PopBack2); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,PopBack3); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,PushBack); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Front1); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Front2); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Copy1); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Copy2); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Copy3); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Copy4); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,CopyConstructor); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Constructor1); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Constructor2); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,OperatorEquals); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,OperatorEqualEqual1); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,OperatorEqualEqual2); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,OperatorEqualEqual3); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,OperatorEqualEqual4); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,OperatorEqualEqual5); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,ChangeCapacity1); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,ChangeCapacity2); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,ChangeCapacity3); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,SubRange); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Iterator1); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Iterator2); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,Sort); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,RandomAccess1); \
  CreateBlockArrayTest(valueType,shiftSize,Policy,RandomAccess2); \

