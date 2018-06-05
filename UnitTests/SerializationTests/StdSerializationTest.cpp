#include "CppUnitLite2/CppUnitLite2.h"
#include "Geometry/GeometryStandard.hpp"
#include "Serialization/SerializationStandard.hpp"

using namespace Zero;

typedef std::string string;
#include <vector>
#include <list>
#include <map>

#include "SerializationTestPattern.hpp"

struct StdTests
{
public:
  StdTests()
  {

  }

  std::vector<SimpleObject> Objects;
  std::vector<int> IntArray;
  std::set<string> Keys;
  typedef std::pair<string, string> dictionpair;
  typedef std::map<string, string> dictionarytype;
  dictionarytype Dictionary;

  void TestAgainst(StdTests& other)
  {
    GETACTIVETEST();
    CHECK(other.IntArray==IntArray);
    CHECK(other.Dictionary==Dictionary); 
    CHECK(other.Keys==Keys); 
    CHECK(other.Objects==Objects); 
  }

  void Serialize( Serializer& stream )
  {
    // METAREFACTOR
    //SerializeName( IntArray );
    //SerializeName( Dictionary );
    //SerializeName( Keys );
    //SerializeName( Objects );
  }

};

void Fill(StdTests& control)
{
  control.IntArray.push_back( 0 );
  control.IntArray.push_back( 1 );
  control.IntArray.push_back( 2 );
  control.IntArray.push_back( 3 );
  control.IntArray.push_back( 4 );

  control.Dictionary["A"] = "Apple";
  control.Dictionary["B"] = "Bat";
  control.Dictionary["C"] = "Cog";
  control.Dictionary["D"] = "Dog";

  control.Keys.insert("Boom");
  control.Keys.insert("Pow");
  control.Keys.insert("Splat");
  control.Keys.insert("Thud");

  control.Objects.push_back(SimpleObject(1,2.0f));
  control.Objects.push_back(SimpleObject(3,4.0f));
  control.Objects.push_back(SimpleObject(5,6.0f));
}

typedef  FileBased<StdTests, TextSaver, DataTreeLoader> StdConTextFileDataTree;

TEST_F(StdConTextFileDataTree, StdConTextFileDataTree)
{
  //Run();
}

typedef  FileBased<StdTests, TextSaver, DataTreeLoader> StdConTextFile;

TEST_F(StdConTextFile, StdConTextFile)
{
  //Run();
}

//typedef  FileBased<StdTests, BinaryFileSaver, BinaryFileLoader> StdConBinaryFile;
//
//TEST_F(StdConBinaryFile, StdConBinaryFile)
//{
//  Run();
//}
//
//typedef  BufferBased<StdTests, BinaryBufferSaver, BinaryBufferLoader> StdConBinaryBuffer;
//
//TEST_F(StdConBinaryBuffer, StdConBinaryBuffer)
//{
//  Run();
//}
