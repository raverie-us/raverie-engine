///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zilch
{

//------------------------------------------------------------------------Shader
class Shader
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  static void AddInlineShaderCode(StringParam language, StringParam shaderCode);
  static void AddInlineShaderCode(StringParam language, int minVersion, int maxVersion, StringParam shaderCode);
  static bool IsLanguage(StringParam language);
  static bool IsLanguage(StringParam language, int minVersion, int maxVersion);
};

}//namespace Zilch
