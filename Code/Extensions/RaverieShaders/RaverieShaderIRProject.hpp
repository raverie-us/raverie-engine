// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class BaseShaderIRTranslator;

/// A project full of raverie code to be compiled into a library.
class RaverieShaderIRProject : public ShaderCompilationErrors
{
public:
  RaverieShaderIRProject(StringParam projectName);

  void AddCodeFromString(StringParam code, StringParam codeLocation, void* userData = nullptr);
  void AddCodeFromFile(StringParam filePath, void* userData = nullptr);

  /// Clear the code from this library
  void Clear();

  bool CompileTree(Raverie::Module& raverieDependencies, Raverie::LibraryRef& raverieLibrary, Raverie::SyntaxTree& syntaxTree, Raverie::Array<Raverie::UserToken>& tokensOut);
  /// Compiles and translates this project into a library.
  RaverieShaderIRLibraryRef CompileAndTranslate(RaverieShaderIRModuleRef& dependencies, BaseShaderIRTranslator* translator);

  struct CodeEntry
  {
    String mCode;
    String mCodeLocation;
    void* mUserData;
  };

  // Private Interface
  void BuildRaverieProject(Raverie::Project& raverieProject);
  void PopulateRaverieModule(Raverie::Module& raverieDependencies, RaverieShaderIRModuleRef& dependencies);
  void CollectLibraryDefaultValues(RaverieShaderIRLibraryRef libraryRef, Raverie::Module& raverieModule);

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
  mutable Raverie::DestructibleBuffer ComplexUserData;
};

} // namespace Raverie
