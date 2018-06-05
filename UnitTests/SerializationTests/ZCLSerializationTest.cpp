#include "CppUnitLite2/CppUnitLite2.h"

#include "Geometry/GeometryStandard.hpp"
#include "Serialization/SerializationStandard.hpp"

#include "SerializationTestPattern.hpp"

using namespace Zero;
using namespace Zilch;

struct ContainerTests
{
  HashMap<String, String> Dictionary;
  HashSet<String> Keys;
  PodArray<int> Integers;
  Array<String> Log;
  Array<SimpleObject> Objects;

  void TestAgainst(ContainerTests& other)
  {
    GETACTIVETEST();
    CHECK(other.Dictionary==Dictionary); 
    CHECK(other.Keys==Keys);
    CHECK(other.Integers==Integers);
    CHECK(other.Log==Log);
    CHECK(other.Objects==Objects);
  }

  void Serialize( Serializer& stream )
  {
    SerializeName( Dictionary );
    SerializeName( Keys );
    SerializeName( Integers );
    SerializeName( Log );
    SerializeName( Objects );
  }

};

void Fill(ContainerTests& control)
{
  control.Integers.PushBack( 0 );
  control.Integers.PushBack( 1 );
  control.Integers.PushBack( 2 );
  control.Integers.PushBack( 3 );
  control.Integers.PushBack( 4 );

  control.Dictionary["A"] = "Apple";
  control.Dictionary["B"] = "Bat";
  control.Dictionary["C"] = "Cog";
  control.Dictionary["D"] = "Dog";

  control.Keys.Insert("Boom");
  control.Keys.Insert("Pow");
  control.Keys.Insert("Splat");
  control.Keys.Insert("Thud");

  control.Objects.PushBack(SimpleObject(1,2.0f));
  control.Objects.PushBack(SimpleObject(3,4.0f));
  control.Objects.PushBack(SimpleObject(5,6.0f));
}

typedef  FileBased<ContainerTests, TextSaver, DataTreeLoader> ZCLTextFileDataTree;

TEST_F(ZCLTextFileDataTree, ZCLTextFileDataTree)
{
  //Run();
}

typedef  FileBased<ContainerTests, TextSaver, DataTreeLoader> ZCLTextFile;

TEST_F(ZCLTextFile, ZCLTextFile)
{
  //Run();
}

//typedef  FileBased<ContainerTests, BinaryFileSaver, BinaryFileLoader> ZCLBinaryFile;
//
//TEST_F(ZCLBinaryFile, ZCLBinaryFile)
//{
//  Run();
//}
//
//typedef  BufferBased<ContainerTests, BinaryBufferSaver, BinaryBufferLoader> ZCLBinaryBuffer;
//
//TEST_F(ZCLBinaryBuffer, ZCLBinaryBuffer)
//{
//  Run();
//}
