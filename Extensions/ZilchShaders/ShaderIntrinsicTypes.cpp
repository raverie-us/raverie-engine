///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

// Bind some extra types to zilch for simulating shaders
namespace Zilch
{

using namespace Zero;

//------------------------------------------------------------------------Shader
void Shader::AddInlineShaderCode(StringParam language, StringParam shaderCode)
{
  //pretend to add code here...
}

void Shader::AddInlineShaderCode(StringParam language, int minVersion, int maxVersion, StringParam shaderCode)
{

}

bool Shader::IsLanguage(StringParam language)
{
  return false;
}

bool Shader::IsLanguage(StringParam language, int minVersion, int maxVersion)
{
  return false;
}

ZilchDefineType(Shader, builder, type)
{
  ZilchFullBindMethod(builder, type, &Shader::AddInlineShaderCode, (void(*)(StringParam, StringParam)), "AddInlineShaderCode", "language, shaderCode");
  ZilchFullBindMethod(builder, type, &Shader::AddInlineShaderCode, (void(*)(StringParam, int, int, StringParam)), "AddInlineShaderCode", "language, minVersion, maxVersion, shaderCode");
  ZilchFullBindMethod(builder, type, &Shader::IsLanguage, (bool(*)(StringParam)), "IsLanguage", "language");
  ZilchFullBindMethod(builder, type, &Shader::IsLanguage, (bool(*)(StringParam, int, int)), "IsLanguage", "language, minVersion, maxVersion");
}

}//namespace Zilch
