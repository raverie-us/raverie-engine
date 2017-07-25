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

class BaseShaderTranslator;
class NameSettings;
class ShaderCodeBuilder;
class ShaderCompilationErrors;
class ShaderField;
class ShaderFunction;
class ShaderType;
class ShaderTypeTranslation;
class TranslatorLibrary;
class ZilchShaderLibrary;
class ZilchShaderModule;
class ZilchShaderProject;
class ZilchShaderSettings;
class ZilchShaderTranslator;
class ZilchShaderTranslatorContext;
class ZilchTypeCollector;

typedef Zilch::Ref<ZilchShaderLibrary> ZilchShaderLibraryRef;
typedef Zilch::Ref<ZilchShaderModule> ZilchShaderModuleRef;
typedef Zilch::Ref<ZilchShaderSettings> ZilchShaderSettingsRef;
typedef Zilch::Ref<BaseShaderTranslator> BaseShaderTranslatorRef;

}//namespace Zero
