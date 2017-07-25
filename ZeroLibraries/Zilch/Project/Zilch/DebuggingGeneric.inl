/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

namespace Zilch
{
  //***************************************************************************
  bool DebugErrorHandler(ErrorSignaler::ErrorData& errorData)
  {
    // Just print out the error
    printf("%s(%d) : %s %s\n", errorData.File, errorData.Line, errorData.Message, errorData.Expression);

    // Returning true will cause a break point
    return true;
  }
}
