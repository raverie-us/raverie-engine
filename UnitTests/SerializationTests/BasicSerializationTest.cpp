#include "CppUnitLite2/CppUnitLite2.h"

#include "Geometry/GeometryStandard.hpp"
#include "Serialization/SerializationStandard.hpp"

//using namespace Zero;

typedef std::string string;
#define floatvalue 3.1459f
#define uintvalue 42
#define intvalue -42
#define stringvalue "[Test String]"
#define doublevalue 9865.34123

#include "SerializationTestPattern.hpp"

DeclareEnum3(TestEnum, A, B, C);

struct BasicTests
{
  BasicTests()
  {
    integer = 0;
    uinteger = 0;
    flag = false;
    flag2 = false;
    realnumber = 0;
    bigNumber = 0;
    //text = "";
  }

  int integer;
  unsigned uinteger;
  float realnumber;
  double bigNumber;
  //string text;
  SimpleObject object;
  bool flag;
  bool flag2;
  //TestEnum::Type testEnum;


  void TestAgainst(BasicTests& other)
  {
    GETACTIVETEST();
    CHECK_EQUAL(other.integer, integer);
    CHECK_EQUAL(other.uinteger, uinteger);
    CHECK_EQUAL(other.realnumber, realnumber);		
    CHECK_CLOSE(other.bigNumber, bigNumber, 0.001);
    //CHECK_EQUAL(other.text, text);
    CHECK(other.object == object);
    CHECK_EQUAL(other.flag, flag);
    CHECK_EQUAL(other.flag2, flag2);
    //CHECK_EQUAL(other.testEnum, testEnum);

  }

  void Serialize(Zero::Serializer& stream )
  {
    SerializeName( integer );
    SerializeName( uinteger );
    SerializeName( realnumber );
    SerializeName( bigNumber );
    //SerializeName( text );
    SerializeName( object );
    SerializeName( flag );
    SerializeName( flag2 );

    // METAREFACTOR
    //Zero::EnumMap testEnumMap = Zero::EnumMap(TestEnum::Names, TestEnum::Values);
    //stream.EnumField("TestEnum", "test", testEnum, &testEnumMap);
  }
};


void Fill(BasicTests& control)
{
  control.integer = intvalue;
  control.uinteger = uintvalue;
  control.flag = true;
  control.flag2 = false;
  control.realnumber = floatvalue;
  control.bigNumber = doublevalue;
  //control.text = stringvalue;
  control.object.a = intvalue;
  control.object.b = floatvalue;
  //control.testEnum = TestEnum::B;
}

typedef  FileBased<BasicTests, Zero::TextSaver, Zero::DataTreeLoader> BasicTextFileDataTree;

TEST_F(BasicTextFileDataTree, BasicTextFileDataTree)
{
  Run();
}

typedef  FileBased<BasicTests, Zero::TextSaver, Zero::DataTreeLoader> BasicTextFile;

TEST_F(BasicTextFile, BasicTextFile)
{
  Run();
}

//typedef  FileBased<BasicTests, Zero::BinaryFileSaver, Zero::BinaryFileLoader> BasicBinaryFile;
//
//TEST_F(BasicBinaryFile, BasicBinaryFile)
//{
//  //Run();
//}
//
//typedef  BufferBased<BasicTests, Zero::BinaryBufferSaver, Zero::BinaryBufferLoader> BasicBinaryBuffer;
//
//TEST_F(BasicBinaryBuffer, BasicBinaryBuffer)
//{
//  //Run();
//}
