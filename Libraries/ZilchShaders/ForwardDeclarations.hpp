///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zilch
{
class SyntaxTree;
}

namespace Zero
{

class ShaderCodeBuilder;
class ShaderCompilationErrors;
class ZilchShaderSettings;
class TranslationErrorEvent;
class ZilchShaderSpirVSettings;

typedef Zilch::Ref<ZilchShaderSettings> ZilchShaderSettingsRef;
typedef Zilch::Ref<ZilchShaderSpirVSettings> ZilchShaderSpirVSettingsRef;

}//namespace Zero
