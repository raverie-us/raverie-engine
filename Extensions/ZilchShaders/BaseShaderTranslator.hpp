///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------BaseShaderTranslator
// Base shader translator class so other translators can be implemented.
// @JoshD: Make this interface better!
class BaseShaderTranslator
{
public:
  virtual ~BaseShaderTranslator() {};

  // Tell the translator what settings to use for translation (Names, render targets, etc...)
  virtual void SetSettings(ZilchShaderSettingsRef& settings) = 0;
  virtual void Setup() = 0;

  // A string to describe the target language. Used currently just for unit tests.
  virtual String GetFullLanguageString() = 0;
  // Parse a native and generate type information (mainly shader types) to populate the ShaderLibrary with.
  virtual void ParseNativeLibrary(ZilchShaderLibrary* shaderLibrary) = 0;
  // Translate a given project (and it's syntax tree) into the passed in library. Each ShaderType in the
  // library will contain translated pieces of the target language. These pieces can be put together into a final shader with "BuildFinalShader".
  virtual bool Translate(Zilch::SyntaxTree& syntaxTree, ZilchShaderProject* project, ZilchShaderLibraryRef& libraryRef) = 0;
  // Determines if this shader type is supported for translation (such as geometry shaders).
  virtual bool SupportsFragmentType(ShaderType* shaderType) = 0;
  // Given a ShaderType, build the string for the shader that results from calling the given type's [Main] function.
  // This can also build range mappings from the final shader back to each zilch script to help display the source of errors,
  // but this is off by default for performance reasons. When an error occurs a shader type can be re-translated to this time
  // with line number mappings (errors are exceptional cases after all). Also for debugging this can choose to only translate
  // the type and not all dependencies.
  virtual void BuildFinalShader(ShaderType* type, ShaderTypeTranslation& shaderResult, bool generatedCodeRangeMappings = false, bool walkDependencies = true) = 0;
  // Returns the language version of the translator. Used to conditionally write code.
  virtual int GetLanguageVersionNumber() = 0;
  // Returns the name of the language that this translator is for
  virtual String GetLanguageName() = 0;

  // An intrusive reference count for memory handling
  ZilchRefLink(BaseShaderTranslator);
};

}//namespace Zero
