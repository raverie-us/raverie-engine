#include "Precompiled.hpp"
using namespace Zilch;

int Test04()
{
  int i = 42 + CustomMath::Fib(6);
  float j = 3.2f;

  if (j > 2.0f)
  {
    i = 3;
  }
  else
  {
    i = 6;
  }

  if (((Integer)j) < i + CustomMath::Factorial(4))
  {
    i += 1;
  }
  else if (i * i > CustomMath::Factorial(3))
  {
    i += 2;
  }
  else
  {
    i += 3;
  }

  return i;
}
