///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------SimpleZilchShaderGenerator
// This class is basically a helper wrapper around projects, libraries, the compositor, and the translator.
// This should give simple examples of how to use the zilch shader translation system.
class SimpleZilchShaderGenerator : public Zilch::EventHandler
{
public:
  SimpleZilchShaderGenerator(BaseShaderTranslator* translator);
  ~SimpleZilchShaderGenerator();

  // Load zilch script that define our settings for translation from a path.
  void LoadSettings(StringParam settingsDirectoryPath);
  // Setup all dependent libraries that we have and compile them into a shader library (only once)
  // for all subsequent fragment and shader operations. This is not done in the constructor so the 
  // user has a chance to connect events to this before the dependent libraries are built (in-case any errors happen).
  void SetupDependencies(StringParam extensionsDirectoryPath);
  // Clear all projects and reset the translator (clears the library translation)
  void ClearAll();

  // Fragment creation/compiling interface
  void AddFragmentCode(StringParam fragmentCode, StringParam fileName, void* userData);
  void CompileAndTranslateFragments();
  void ClearFragmentsProjectAndLibrary();

  // Shader creation/compiling interface
  void ComposeShader(ZilchShaderDefinition& shaderDef);
  void AddShaderCode(StringParam shaderCode, StringParam fileName, void* userData);
  void ClearShadersProjectAndLibrary();
  void CompileAndTranslateShaders();

  // Helpers to find the definition/zilch code of a give shader.
  ZilchShaderDefinition* FindShaderResults(StringParam shaderName);
  // Finds the zilch shader code for an entire shader (vertex + pixel)
  String FindZilchShaderString(StringParam shaderName);
  // Finds the zilch shader code for a certain fragment type (vertex or pixel)
  String FindZilchShaderString(StringParam shaderName, FragmentType::Enum fragmentType);
  
  
  
  // Basic handler to forward the errors from the underlying
  // project/translator to whoever is listening on this class.
  void OnForwardEvent(Zilch::EventData* e);

  // Helper to setup a lot of shared data on the translator
  void SetupTranslator(BaseShaderTranslator* translator);

  // The library of built-in shader primitives (everything on "Shader.", samplers, etc...)
  ZilchShaderLibraryRef mShaderIntrinsicsLibraryRef;

  ZilchShaderLibraryRef mExtensionsLibraryRef;

  // The individual fragments to be assembled into shaders
  ZilchShaderProject mFragmentProject;
  ZilchShaderLibraryRef mFragmentLibraryRef;

  // The actual shaders (typically built from shader definitions)
  ZilchShaderProject mShaderProject;
  ZilchShaderLibraryRef mShaderLibraryRef;

  // The shared settings to be used through all of the compositing/translation
  ZilchShaderSettingsRef mSettings;

  // Keep a mapping of shader name to definition so that we can lookup the corresponding vertex/pixel shaders for a given shader.
  typedef HashMap<String, ZilchShaderDefinition> ShaderDefinitionMap;
  ShaderDefinitionMap mShaderDefinitionMap;
  BaseShaderTranslator* mTranslator;
};

}//namespace Zero
