#include "Precompiled.hpp"
using namespace Zilch;

class Robot
{
public:
  Integer Nuts;
  Integer Bolts;

  Robot()
  {
    this->Nuts = 0;
    this->Bolts = 0;

    this->Bolts += 1;
  }

  void ImplicitReturn()
  {
    this->Nuts += 4;
    this->Bolts -= 1;
  }

  void ExplicitReturn()
  {
    this->Nuts -= 1;
    this->Bolts += 3;
    return;
  }

  Integer AllPathsReturn()
  {
    if (this->Nuts > this->Bolts)
    {
      this->Nuts += 7;
      this->Bolts -= 2;
      return 2;
    }
    else
    {
      this->Nuts += 5;
      this->Bolts -= 6;
      return 1;
    }
  }
};

Integer Test10()
{
  Robot* bot = new Robot();

  int result = 33;

  bot->ExplicitReturn();
  bot->ImplicitReturn();
  result += bot->AllPathsReturn();

  int retVal = result + bot->Nuts * bot->Bolts;
  delete bot;
  return retVal;
}
