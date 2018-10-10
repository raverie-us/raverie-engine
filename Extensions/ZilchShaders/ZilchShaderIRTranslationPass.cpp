///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "ZilchShaderIRTranslationPass.hpp"

#include "libspirv.h"

namespace Zero
{

//-------------------------------------------------------------------ZilchShaderIRTranslationPass
String ZilchShaderIRTranslationPass::SpirvDiagnosticToString(spv_diagnostic& diagnostic)
{
  if(diagnostic == nullptr)
    return String();
  
  // Note: This is basically a copy of the internal spirv diagnostic printf function.

  StringBuilder builder;
  if(diagnostic->isTextSource) {
    // NOTE: This is a text position
    // NOTE: add 1 to the line as editors start at line 1, we are counting new
    // line characters to start at line 0
    builder << "error: " << ToString(diagnostic->position.line + 1) << ": "
      << ToString(diagnostic->position.column + 1) << ": " << ToString(diagnostic->error)
      << "\n";
  }
  else
  {
    // NOTE: Assume this is a binary position
    builder << "error: ";
    if(diagnostic->position.index > 0)
      builder << ToString(diagnostic->position.index) << ": ";
    builder << diagnostic->error << "\n";
  }
  return builder.ToString();
}

}//namespace Zero
