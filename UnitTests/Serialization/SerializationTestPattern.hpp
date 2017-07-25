

#include "Platform/PlatformStandard.hpp"
#include "Serialization/Serialization.hpp"
#include "Utility/Status.hpp"

struct SimpleObject
{
  int a;
  float b;

  SimpleObject()
  {
    a = 0;
    b = 0;
  }

  SimpleObject(int pa, float pb)
  {
    a = pa;
    b = pb;
  }

  bool operator==(const SimpleObject& other) const
  {
    if(a != other.a) return false;
    if(b != other.b) return false;
    return true;
  } 

  void Serialize( Zero::Serializer& stream )
  {
    SerializeName( a );
    SerializeName( b );
  }
};


template<typename typeToTest, typename saverType, typename loaderType>
class FileBased
{
public:
  void Run()
  {
    GETACTIVETEST();

    std::string testFilename = "TestFile";
    testFilename+=m_name;
    testFilename+=".txt";

    {
      typeToTest output;
      Fill( output );

      Zero::Status status;
      saverType saver;
      saver.Open(status, testFilename.c_str());
      //saver.SerializeField("TestObject", output);
      saver.Start("TestObject", nullptr, Zero::StructureType::Object);
      output.Serialize(saver);
      saver.End("TestObject", Zero::StructureType::Object);
      saver.Close();

    }

    {
      typeToTest control;
      Fill( control ); 
      typeToTest readFromFile;

      loaderType loader;
      Zero::Status status;
      loader.OpenFile(status, testFilename.c_str());
      loader.Start("TestObject", nullptr, Zero::StructureType::Object);
      readFromFile.Serialize(loader);
      loader.End("TestObject", Zero::StructureType::Object);
      //loader.SerializeField("TestObject", readFromFile);
      loader.Close();

      readFromFile.TestAgainst( control );
    }

    //std::remove( testFilename.c_str() );
  }
};


template<typename typeToTest, typename saverType, typename loaderType>
class BufferBased
{
public:
  void Run()
  {

    byte* data = NULL;
    size_t size = 0;

    {
      typeToTest output;
      Fill( output ); 

      saverType saver;
      saver.Open();
      saver.SerializeField("TestObject", output);

      size = saver.GetSize();
      data = new byte[size];
      saver.ExtractInto(data, size);
    }

    {
      typeToTest control;
      Fill( control );

      typeToTest readFromBinary;

      loaderType loader;
      loader.SetBuffer(data, size);
      loader.SerializeField("TestObject", readFromBinary);

      readFromBinary.TestAgainst( control );
    }


    delete [] data;
  }
};