///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------ZilchShaderProject
// A project full of zilch code to be compiled into a library.
class ZilchShaderProject : public ShaderCompilationErrors
{
public:
  ZilchShaderProject(StringParam projectName);

  void AddCodeFromString(StringParam code, StringParam codeLocation, void* userData = nullptr);
  void AddCodeFromFile(StringParam filePath, void* userData = nullptr);

  // Clear the code from this library
  void Clear();

  // Compiles and translates this project into a library.
  ZilchShaderLibraryRef CompileAndTranslate(ZilchShaderModuleRef& dependencies, BaseShaderTranslator* translator, ZilchShaderSettingsRef& settings);

  struct CodeEntry
  {
    String mCode;
    String mCodeLocation;
    void* mUserData;
  };

  // Private Interface
  void BuildZilchProject(Zilch::Project& zilchProject);
  void PopulateZilchModule(Zilch::Module& zilchDependencies, ZilchShaderModuleRef& dependencies);
  void CollectLibraryDefaultValues(ZilchShaderLibraryRef libraryRef, Zilch::Module& zilchModule);

  Array<CodeEntry> mCodeEntries;
  String mProjectName;
  // Has this project been successfully compiled into a library? (Not really used at the moment...)
  bool mCompiledSuccessfully;

  // A pointer to any data the user wants to attach
  mutable const void* UserData;

  // Any user data that cant simply be represented by a pointer
  // Data can be written to the buffer and will be properly destructed
  // when this object is destroyed (must be read in the order it's written)
  mutable Zilch::DestructibleBuffer ComplexUserData;
};

}//namespace Zero
