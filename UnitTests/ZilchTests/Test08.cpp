#include "Precompiled.hpp"
using namespace Zilch;

class Beer
{
public:
  static String GetBottlePhrase(int count)
  {
    if (count == 1)
    {
      return " bottle";
    }
    else
    {
      return " bottles";
    }
  }

  static void MakeSong(StringBuilderExtended& builder)
  {
    for (int i = 99; i > 0;)
    {
      String phrase1 = GetBottlePhrase(i);

      builder.Write(i);
      builder.Write(phrase1);
      builder.WriteLine(" of beer on the wall,");

      builder.Write(i);
      builder.Write(phrase1);
      builder.WriteLine(" of beer,");

      builder.WriteLine("Take one down,");
      builder.WriteLine("Pass it around,");

      --i;
      String phrase2 = GetBottlePhrase(i);

      builder.Write(i);
      builder.Write(phrase2);
      builder.WriteLine(" of beer on the wall.");
      builder.WriteLine();
    }
  }
};

String Test08()
{
  StringBuilderExtended* builder = new StringBuilderExtended();

  Beer::MakeSong(*builder);

  String result = builder->ToString();

  delete builder;

  return result;
}
