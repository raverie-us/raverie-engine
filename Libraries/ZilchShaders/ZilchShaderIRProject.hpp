// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

class BaseShaderIRTranslator;

/// A project full of zilch code to be compiled into a library.
class ZilchShaderIRProject : public ShaderCompilationErrors
{
public:
  ZilchShaderIRProject(StringParam projectName);

  void AddCodeFromString(StringParam code, StringParam codeLocation, void* userData = nullptr);
  void AddCodeFromFile(StringParam filePath, void* userData = nullptr);

  /// Clear the code from this library
  void Clear();

  bool CompileTree(Zilch::Module& zilchDependencies,
                   Zilch::LibraryRef& zilchLibrary,
                   Zilch::SyntaxTree& syntaxTree,
                   Zilch::Array<Zilch::UserToken>& tokensOut);
  /// Compiles and translates this project into a library.
  ZilchShaderIRLibraryRef CompileAndTranslate(ZilchShaderIRModuleRef& dependencies, BaseShaderIRTranslator* translator);

  struct CodeEntry
  {
    String mCode;
    String mCodeLocation;
    void* mUserData;
  };

  // Private Interface
  void BuildZilchProject(Zilch::Project& zilchProject);
  void PopulateZilchModule(Zilch::Module& zilchDependencies, ZilchShaderIRModuleRef& dependencies);
  void CollectLibraryDefaultValues(ZilchShaderIRLibraryRef libraryRef, Zilch::Module& zilchModule);

  Array<CodeEntry> mCodeEntries;
  String mProjectName;
  /// Has this project been successfully compiled into a library? (Not really
  /// used at the moment...)
  bool mCompiledSuccessfully;

  /// A pointer to any data the user wants to attach
  mutable const void* UserData;

  /// Any user data that cant simply be represented by a pointer
  /// Data can be written to the buffer and will be properly destructed
  /// when this object is destroyed (must be read in the order it's written)
  mutable Zilch::DestructibleBuffer ComplexUserData;
};

} // namespace Zero
