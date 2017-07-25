#pragma once

//#define Check_VecSim4(expected,actual) \
//  CHECK_EQUAL(0xf,_mm_movemask_ps(Equal(expected,actual)));
//
//#define Check_VecSim3(expected,actual) \
//  CHECK_EQUAL(0x7,_mm_movemask_ps(Equal(expected,actual)) & 0x7);

#define CheckBool_SimVec4(mask)\
  CHECK(_mm_movemask_ps(mask) == 0xf);

#define CheckBool_SimVec3(mask)\
  CHECK((_mm_movemask_ps(mask) & 0x7) == 0x7);

#define Check_SimVec4(expected,actual)\
  __declspec(align(16))float testExpected[4];\
  __declspec(align(16))float testActual[4];\
  Store(expected,testExpected);\
  Store(actual,testActual);\
  CHECK_ARRAY_EQUAL(testExpected,testActual,4);

#define Check_SimVec3(expected,actual)\
  __declspec(align(16))float testExpected[4];\
  __declspec(align(16))float testActual[4];\
  Store(expected,testExpected);\
  Store(actual,testActual);\
  CHECK_ARRAY_EQUAL(testExpected,testActual,3);

#define Check_SimVec4_Close(expected,actual,epsilon)\
  __declspec(align(16))float testExpected[4];\
  __declspec(align(16))float testActual[4];\
  Store(expected,testExpected);\
  Store(actual,testActual);\
  CHECK_ARRAY_CLOSE(testExpected,testActual,4,epsilon);

#define Check_SimVec3_Close(expected,actual,epsilon)\
  __declspec(align(16))float testExpected[4];\
  __declspec(align(16))float testActual[4];\
  Store(expected,testExpected);\
  Store(actual,testActual);\
  CHECK_ARRAY_CLOSE(testExpected,testActual,3,epsilon);

#define CheckBool_SimMat4(mask)\
  CheckBool_SimVec4(mask.columns[0]);\
  CheckBool_SimVec4(mask.columns[1]);\
  CheckBool_SimVec4(mask.columns[2]);\
  CheckBool_SimVec4(mask.columns[3]);

#define Check_MatSim4_Close(expected,actual,epsilon)\
  __declspec(align(16))float testExpected[4];\
  __declspec(align(16))float testActual[4];\
  Store(expected.columns[0],testExpected);\
  Store(actual.columns[0],testActual);\
  CHECK_ARRAY_CLOSE(testExpected,testActual,4,epsilon);\
  Store(expected.columns[1],testExpected);\
  Store(actual.columns[1],testActual);\
  CHECK_ARRAY_CLOSE(testExpected,testActual,4,epsilon);\
  Store(expected.columns[2],testExpected);\
  Store(actual.columns[2],testActual);\
  CHECK_ARRAY_CLOSE(testExpected,testActual,4,epsilon);\
  Store(expected.columns[3],testExpected);\
  Store(actual.columns[3],testActual);\
  CHECK_ARRAY_CLOSE(testExpected,testActual,4,epsilon);


#define CheckBool_SimMat3(mask)\
  CheckBool_SimVec3(mask.columns[0]);\
  CheckBool_SimVec3(mask.columns[1]);\
  CheckBool_SimVec3(mask.columns[2]);

#define Check_MatSim3_Close(expected,actual,epsilon)\
  __declspec(align(16))float testExpected[4];\
  __declspec(align(16))float testActual[4];\
  Store(expected.columns[0],testExpected);\
  Store(actual.columns[0],testActual);\
  CHECK_ARRAY_CLOSE(testExpected,testActual,3,epsilon);\
  Store(expected.columns[1],testExpected);\
  Store(actual.columns[1],testActual);\
  CHECK_ARRAY_CLOSE(testExpected,testActual,3,epsilon);\
  Store(expected.columns[2],testExpected);\
  Store(actual.columns[2],testActual);\
  CHECK_ARRAY_CLOSE(testExpected,testActual,3,epsilon);
