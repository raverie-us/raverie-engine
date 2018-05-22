///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Common/CommonStandard.hpp"

using namespace Zero;

ZeroGuiMain()
{
  CommonLibrary::Initialize();
  int result =  BrowserSubProcess::Execute();
  CommonLibrary::Shutdown();
  return result;
}
