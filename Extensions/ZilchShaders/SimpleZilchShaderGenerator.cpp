///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------SimpleZilchShaderGenerator
SimpleZilchShaderGenerator::SimpleZilchShaderGenerator(BaseShaderTranslator* translator)
  : mFragmentProject("Fragments"), mShaderProject("Shaders")
{
  // Set the default render target names
  ZilchShaderSettings* settings = new ZilchShaderSettings();
  settings->mShaderDefinitionSettings.SetMaxSimultaneousRenderTargets(4);
  settings->mShaderDefinitionSettings.SetRenderTargetName("Target0", 0);
  settings->mShaderDefinitionSettings.SetRenderTargetName("Target1", 1);
  settings->mShaderDefinitionSettings.SetRenderTargetName("Target2", 2);
  settings->mShaderDefinitionSettings.SetRenderTargetName("Target3", 3);
  
  // Add some default built-in uniforms
  settings->mShaderDefinitionSettings.AddBuiltIn("Time", "Real");
  // Add some default vertex definitions (glsl attributes)
  settings->mShaderDefinitionSettings.AddVertexDefinition("Uv", "Real2");
  settings->mShaderDefinitionSettings.AddVertexDefinition("Normal", "Real3");
  settings->mShaderDefinitionSettings.AddVertexDefinition("Position", "Real3");

  settings->mNameSettings.mAllowedClassAttributes.Insert("Protected");
  settings->mNameSettings.mAllowedClassAttributes.Insert("CoreVertex");
  settings->mNameSettings.mAllowedClassAttributes.Insert("RenderPass");
  settings->mNameSettings.mAllowedClassAttributes.Insert("PostProcess");
  mSettings = settings;

  // If no translator was passed in then just default to glsl130 for now
  if(translator == nullptr)
    translator = new Glsl150Translator();

  mTranslator = translator;
  SetupTranslator(mTranslator);

  // Add event connections for errors on the fragment and shader projects
  EventConnect(&mFragmentProject, Events::TranslationError, &SimpleZilchShaderGenerator::OnForwardEvent, this);
  EventConnect(&mFragmentProject, Zilch::Events::CompilationError, &SimpleZilchShaderGenerator::OnForwardEvent, this);
  EventConnect(&mShaderProject, Events::TranslationError, &SimpleZilchShaderGenerator::OnForwardEvent, this);
  EventConnect(&mShaderProject, Zilch::Events::CompilationError, &SimpleZilchShaderGenerator::OnForwardEvent, this);
}

SimpleZilchShaderGenerator::~SimpleZilchShaderGenerator()
{
  delete mTranslator;
}

void SimpleZilchShaderGenerator::LoadSettings(StringParam settingsDirectoryPath)
{
  ZilchShaderSettingsLoader loader;
  EventConnect(&loader, Events::TranslationError, &SimpleZilchShaderGenerator::OnForwardEvent, this);
  EventConnect(&loader, Zilch::Events::CompilationError, &SimpleZilchShaderGenerator::OnForwardEvent, this);

  loader.LoadSettings(mSettings, settingsDirectoryPath);
}

void SimpleZilchShaderGenerator::SetupDependencies(StringParam extensionsDirectoryPath)
{
  // Translate the extensions project into a library

  // Create the core library and parse it
  ZilchShaderLibrary* coreLibrary = new ZilchShaderLibrary();
  coreLibrary->mZilchLibrary = Zilch::Core::GetInstance().GetLibrary();
  mTranslator->ParseNativeLibrary(coreLibrary);

  // Create and cache the shader intrinsics library (it only needs to ever be created once)
  ZilchShaderLibrary* shaderIntrinsicsLibrary = new ZilchShaderLibrary();
  shaderIntrinsicsLibrary->mZilchLibrary = Zilch::ShaderIntrinsicsLibrary::GetInstance().GetLibrary();
  mShaderIntrinsicsLibraryRef = shaderIntrinsicsLibrary;
  mTranslator->ParseNativeLibrary(shaderIntrinsicsLibrary);

  ZilchShaderModuleRef dependencies = new ZilchShaderModule();
  dependencies->PushBack(coreLibrary);
  dependencies->PushBack(mShaderIntrinsicsLibraryRef);

  // Load all of the extension zilch fragments
  ZilchShaderProject extensionProject("Extensions");
  EventConnect(&extensionProject, Events::TranslationError, &SimpleZilchShaderGenerator::OnForwardEvent, this);
  EventConnect(&extensionProject, Zilch::Events::CompilationError, &SimpleZilchShaderGenerator::OnForwardEvent, this);

  // This should really use the FileExtensionManager to find files of the correct extension but we can't see that right now.
  // These scripts are manually created as .zilchFrag so it should be fine for now.
  FileRange fileRange(extensionsDirectoryPath);
  for(; !fileRange.Empty(); fileRange.PopFront())
  {
    FileEntry entry = fileRange.frontEntry();
    String filePath = entry.GetFullPath();
    String fileExt = FilePath::GetExtension(filePath);
    if(fileExt == "zilchFrag")
      extensionProject.AddCodeFromFile(filePath, nullptr);
  }  

  mExtensionsLibraryRef = extensionProject.CompileAndTranslate(dependencies, mTranslator, mSettings);
}

void SimpleZilchShaderGenerator::ClearAll()
{
  ClearFragmentsProjectAndLibrary();
  ClearShadersProjectAndLibrary();
}

void SimpleZilchShaderGenerator::AddFragmentCode(StringParam fragmentCode, StringParam fileName, void* userData)
{
  mFragmentProject.AddCodeFromString(fragmentCode, fileName, userData);
}

void SimpleZilchShaderGenerator::CompileAndTranslateFragments()
{
  // Make sure that our dependencies have been built
  if(mShaderIntrinsicsLibraryRef == nullptr || mExtensionsLibraryRef == nullptr)
  {
    Error("Cannot compile fragments before dependency libraries are setup. Call SetupDependencies first.");
    return;
  }

  // Create the dependencies for the fragment library
  ZilchShaderModuleRef fragmentDependencies = new ZilchShaderModule();
  fragmentDependencies->PushBack(mExtensionsLibraryRef);

  // Compile and translate the fragments project into a library
  mFragmentLibraryRef = mFragmentProject.CompileAndTranslate(fragmentDependencies, mTranslator, mSettings);
}

void SimpleZilchShaderGenerator::ClearFragmentsProjectAndLibrary()
{
  mFragmentProject.Clear();
  mFragmentLibraryRef = nullptr;
}

void SimpleZilchShaderGenerator::ComposeShader(ZilchShaderDefinition& shaderDef)
{
  // Check and make sure the fragment library has been built
  if(mFragmentLibraryRef == nullptr)
  {
    Error("Cannot compile shaders before the fragment library is built.");
    return;
  }

  // Compose the given fragment definitions into vertex and pixel shaders
  ZilchCompositor compositor;
  compositor.BuildCompositedShader(mTranslator, mFragmentLibraryRef, shaderDef, mSettings);

  // Grab the vertex and pixel info and add the zilch code to our shader project
  ZilchFragmentInfo& vertexInfo = shaderDef.mShaderData[FragmentType::Vertex];
  ZilchFragmentInfo& geometryInfo = shaderDef.mShaderData[FragmentType::Geometry];
  ZilchFragmentInfo& pixelInfo = shaderDef.mShaderData[FragmentType::Pixel];
  AddShaderCode(vertexInfo.mZilchCode, vertexInfo.mZilchClassName, nullptr);
  AddShaderCode(geometryInfo.mZilchCode, geometryInfo.mZilchClassName, nullptr);
  AddShaderCode(pixelInfo.mZilchCode, pixelInfo.mZilchClassName, nullptr);
  
  // Store this definition (so we can look up the vertex/pixel shader names later)
  mShaderDefinitionMap[shaderDef.mShaderName] = shaderDef;
}

void SimpleZilchShaderGenerator::AddShaderCode(StringParam shaderCode, StringParam fileName, void* userData)
{
  mShaderProject.AddCodeFromString(shaderCode, fileName, userData);
}

void SimpleZilchShaderGenerator::ClearShadersProjectAndLibrary()
{
  mShaderProject.Clear();
  mShaderLibraryRef = nullptr;
  mShaderDefinitionMap.Clear();
}

void SimpleZilchShaderGenerator::CompileAndTranslateShaders()
{
  // Create the dependencies for the shader library
  ZilchShaderModuleRef shaderDependencies = new ZilchShaderModule();
  shaderDependencies->PushBack(mFragmentLibraryRef);

  // Compile and translate the shaders project into a library
  mShaderLibraryRef = mShaderProject.CompileAndTranslate(shaderDependencies, mTranslator, mSettings);
}

ZilchShaderDefinition* SimpleZilchShaderGenerator::FindShaderResults(StringParam shaderName)
{
  return mShaderDefinitionMap.FindPointer(shaderName);
}

String SimpleZilchShaderGenerator::FindZilchShaderString(StringParam shaderName)
{
  String vertexShaderStr = FindZilchShaderString(shaderName, FragmentType::Vertex);
  String geometryShaderStr = FindZilchShaderString(shaderName, FragmentType::Geometry);
  String pixelShaderStr = FindZilchShaderString(shaderName, FragmentType::Pixel);
  return BuildString(vertexShaderStr, geometryShaderStr, pixelShaderStr);
}

String SimpleZilchShaderGenerator::FindZilchShaderString(StringParam shaderName, FragmentType::Enum fragmentType)
{
  ZilchShaderDefinition* shaderDef = mShaderDefinitionMap.FindPointer(shaderName);
  if(shaderDef == nullptr)
    return String();

  return shaderDef->mShaderData[fragmentType].mZilchCode;
}

void SimpleZilchShaderGenerator::OnForwardEvent(Zilch::EventData* e)
{
  EventSend(this, e->EventName, e);
}

void SimpleZilchShaderGenerator::SetupTranslator(BaseShaderTranslator* translator)
{
  translator->SetSettings(mSettings);
  ZilchShaderTranslator* glslTranslator = (ZilchShaderTranslator*)translator;
  glslTranslator->Setup();
}

}//namespace Zero
