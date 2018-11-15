#include "Precompiled.hpp"

int Test07()
{
  return CustomMath::Fib(CustomMath::Factorial(4)) + CustomMath::Factorial(CustomMath::Fib(3));
  //return CustomMath::Double(CustomMath::Triple(20));
}
