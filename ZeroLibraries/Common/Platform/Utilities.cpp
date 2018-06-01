///////////////////////////////////////////////////////////////////////////////
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
void SystemOpenFile(cstr file, uint verb, cstr parameters, cstr workingDirectory)
{
  Status status;
  SystemOpenFile(status, file, verb, parameters, workingDirectory);
}

void SystemOpenNetworkFile(cstr file, uint verb, cstr parameters, cstr workingDirectory)
{
  Status status;
  SystemOpenNetworkFile(status, file, verb, parameters, workingDirectory);
}

}// namespace Zero
