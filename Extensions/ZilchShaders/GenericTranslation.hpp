///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// This function sets up a lot of generic translation functions for, mostly for the math library.
// If a specific translation needs to be different for another language then it can be overridden afterwards.
void SetupGenericLibraryTranslation(ZilchShaderTranslator* translator);

}//namespace Zero
