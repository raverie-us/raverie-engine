///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Translator.hpp"

namespace Zero
{

//-------------------------------------------------------------------BaseGlslTranslator
/// Common logic for all glsl translators
class BaseHlslTranslator : public ZilchShaderTranslator
{
  String GetLanguageName() override;
  void SetupShaderLanguage() override;
  void WriteGeometryOutputVariableDeclaration(Zilch::LocalVariableNode*& node, ShaderType* variableShaderType, ZilchShaderTranslatorContext* context) override;
  void WriteMainForClass(Zilch::SyntaxNode* node, ShaderType* currentType, ShaderFunction* function, ZilchShaderTranslatorContext* context) override;

  static String mLanguageName;
};

//-------------------------------------------------------------------Glsl130Translator
/// Dx11 hlsl translator
class Hlsl11Translator : public BaseHlslTranslator
{
public:
  String GetFullLanguageString() override;
  int GetLanguageVersionNumber() override;
  String GetVersionString() override;
  bool SupportsFragmentType(ShaderType* shaderType) override;
};

}//namespace Zero


