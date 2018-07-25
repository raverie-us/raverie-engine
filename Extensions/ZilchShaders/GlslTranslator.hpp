///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------GlslBackendSettings
/// Shared settings for glsl translation
struct GlslBackendSettings
{
  GlslBackendSettings()
  {
    mLegacyMode = false;
    mSupportsBools = true;
  }
  bool mLegacyMode;
  bool mSupportsBools;
};

//-------------------------------------------------------------------BaseGlslTranslator
/// Common logic for all glsl translators
class BaseGlslTranslator : public ZilchShaderTranslator
{
public:
  String GetLanguageName() override;
  void SetupShaderLanguage() override;
  void WriteGeometryOutputVariableDeclaration(Zilch::LocalVariableNode*& node, ShaderType* variableShaderType, ZilchShaderTranslatorContext* context) override;
  void WriteMainForClass(Zilch::SyntaxNode* node, ShaderType* currentType, ShaderFunction* function, ZilchShaderTranslatorContext* context) override;

  virtual GlslBackendSettings GetSettings() { return GlslBackendSettings(); }

  static String mLanguageName;
  // @JoshD: Types that need to have the flat attribute auto-appended to them. This should be
  // moved to the actual shader type as an attribute or flag once shaders are refactored.
  HashSet<String> mFlatTypes;
};

//-------------------------------------------------------------------Glsl130Translator
/// Glsl version 130 specific translation
class Glsl130Translator : public BaseGlslTranslator
{
public:
  String GetFullLanguageString() override;
  int GetLanguageVersionNumber() override;
  String GetVersionString() override;
  bool SupportsFragmentType(ShaderType* shaderType) override;
};

//-------------------------------------------------------------------Glsl150Translator
/// Glsl version 150 specific translation
class Glsl150Translator : public BaseGlslTranslator
{
public:
  String GetFullLanguageString() override;
  int GetLanguageVersionNumber() override;
  String GetVersionString() override;
  bool SupportsFragmentType(ShaderType* shaderType) override;
};

//-------------------------------------------------------------------Glsl150Translator
/// Glsl version 300 Es specific translation
class Glsl300EsTranslator : public BaseGlslTranslator
{
public:
  String GetFullLanguageString() override;
  int GetLanguageVersionNumber() override;
  String GetVersionString() override;
  bool SupportsFragmentType(ShaderType* shaderType) override;
};

}//namespace Zero
