// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

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

} // namespace Zero
