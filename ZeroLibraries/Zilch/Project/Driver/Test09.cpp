#include "Precompiled.hpp"

void NativeCallMe5Times(void (*fn)())
{
  for (size_t i = 0; i < 5; ++i)
  {
    fn();
  }
}

static int Counter = 0;

void RunMe()
{
  ++Counter;
}

int Test09()
{
  NativeCallMe5Times(RunMe);
  return Counter;
}
