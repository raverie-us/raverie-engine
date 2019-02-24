// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

int CallMain()
{
  using namespace Zero;
  int argc = 0;
  wchar_t** wideArgv = CommandLineToArgvW(GetCommandLineW(), &argc);

  Array<char*> argv;
  Array<String> utf8Argv;
  argv.Reserve(static_cast<size_t>(argc));

  if (wideArgv)
  {
    for (int i = 0; i < argc; ++i)
    {
      String str = Narrow(wideArgv[i]);
      utf8Argv.PushBack(str);
      argv.PushBack(const_cast<char*>(str.c_str()));
    }
  }
  else
  {
    argc = 0;
  }
  LocalFree(wideArgv);
  return main(argc, argv.Data());
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
  return CallMain();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
{
  return CallMain();
}
