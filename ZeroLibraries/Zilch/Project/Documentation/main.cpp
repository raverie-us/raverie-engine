#include <vld.h>

#define main CompilingZilchScriptMain
#include "CompilingZilchScript.inl"
#undef main

#define main RunningZilchScriptMain
#include "RunningZilchScript.inl"
#undef main

#include "Wallaby.inl"
#include "WallabyBinding.inl"

int main(void)
{
  {
    int result = CompilingZilchScriptMain();
    ErrorIf(result != 0);
  }
  {
    int result = RunningZilchScriptMain();
    ErrorIf(result != 0);
  }

  {
    ZilchSetup setup;
    LibraryRef wallaby = Wallaby::GetLibrary();
  }

  system("pause");
  return 0;
}
