///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

void Socket::Close()
{
  // Ignore any errors that are returned
  Status status;
  Close(status);
}

}//namespace Zero
