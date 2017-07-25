#include "Precompiled.hpp"

int CustomMath::Fib(int n)
{
  if (n <= 1)
  {
    return n;
  }

  return CustomMath::Fib(n - 1) + CustomMath::Fib(n - 2);
}

int CustomMath::Factorial(int n)
{
  if (n <= 1)
    return 1;

  return CustomMath::Factorial(n - 1) * n;
}

int CustomMath::Double(int n)
{
  return n * 2;
}

int CustomMath::Triple(int n)
{
  return n * 3;
}
