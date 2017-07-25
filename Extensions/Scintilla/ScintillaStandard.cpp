///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
  //ZilchDeclareRange(Range);

  ZilchDefineStaticLibrary(ScintillaMetaLibrary)
  {
    //ZilchInitializeRange(Range);

    ZilchInitializeType(TextEditor);

    //ZeroBindEnum(LayoutDirection); // METAREFACTOR
  }
}
