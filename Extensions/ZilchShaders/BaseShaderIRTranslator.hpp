///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class BaseShaderIRTranslator
{
public:
  virtual ~BaseShaderIRTranslator() {};

  // Tell the translator what settings to use for translation (Names, render targets, etc...)
  virtual void SetSettings(ZilchShaderSpirVSettingsRef& settings) = 0;
  virtual void Setup() = 0;

  // Translate a given project (and it's syntax tree) into the passed in library. Each ShaderType in the
  // library will contain translated pieces of the target language. These pieces can be put together into a final shader with "BuildFinalShader".
  virtual bool Translate(Zilch::SyntaxTree& syntaxTree, ZilchShaderIRProject* project, ZilchShaderIRLibrary* library) = 0;

  // An intrusive reference count for memory handling
  ZilchRefLink(BaseShaderIRTranslator);
};

}//namespace Zero
