#include "Precompiled.hpp"
using namespace Zilch;

int Test05()
{
  int i = (Integer)0.0f;
  int j = (Integer)true;
  float real = 35.263f;

  while (i < 17)
  {
    ++i;

    if (i >= 3)
    {
      ++j;
      continue;
    }

    ++real;
    ++i;
    ++i;
  }

  while (i < 30)
  {
    ++i;

    if (i >= 22)
    {
      break;
    }

    ++i;
  }

  i += 62;

  bool flipFlop = true;

  do
  {
    flipFlop = !flipFlop;

    if (i > 19)
    {
      --i;
    }
    else if (flipFlop)
    {
      --i;
      ++j;
      --real;
    }
    else
    {
      continue;
    }

    ++j;
  }
  while (i != 16);
		
  for (int k = 1; k < 7 + j; ++k)
  {
    ++i;

    if (i % 3 == 0)
    {
      continue;
    }

    if (i % 2 == 0)
    {
      i += j;
    }
  }

  for (;;)
  {
    if (i > 9000 - j)
    {
      break;
    }

    ++i;
    i += i + (Integer)real;
  }

  return i + ~j / (Integer)real;
}
