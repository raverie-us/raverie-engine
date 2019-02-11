// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

int WINAPI wWinMain(HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    PWSTR pCmdLine,
                    int nCmdShow)
{
  using namespace Zero;
  int argc = 0;
  wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);

  if (argv)
  {
    for (int i = 0; i < argc; ++i)
      gCommandLineArguments.PushBack(Narrow(argv[i]));
  }

  return PlatformMain(gCommandLineArguments);
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   PSTR pCmdLine,
                   int nCmdShow)
{
  using namespace Zero;
  int argc = 0;
  wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);

  if (argv)
  {
    for (int i = 0; i < argc; ++i)
      gCommandLineArguments.PushBack(Narrow(argv[i]));
  }

  return PlatformMain(gCommandLineArguments);
}

int main(int argc, char* argv[])
{
  using namespace Zero;
  CommandLineToStringArray(gCommandLineArguments, argv, argc);
  return PlatformMain(gCommandLineArguments);
}
