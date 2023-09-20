// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieShaderIRProject::RaverieShaderIRProject(StringParam projectName)
{
  mProjectName = projectName;
  mCompiledSuccessfully = false;
}

void RaverieShaderIRProject::AddCodeFromString(StringParam code, StringParam codeLocation, void* userData)
{
  CodeEntry& entry = mCodeEntries.PushBack();
  entry.mCode = code;
  entry.mCodeLocation = codeLocation;
  entry.mUserData = userData;
}

void RaverieShaderIRProject::AddCodeFromFile(StringParam filePath, void* userData)
{
  File file;
  Status status;
  file.Open(filePath, FileMode::Read, FileAccessPattern::Sequential, FileShare::Read, &status);

  String code = ReadFileIntoString(filePath);
  AddCodeFromString(code, filePath, userData);
}

void RaverieShaderIRProject::Clear()
{
  mCodeEntries.Clear();
  UserData = nullptr;
  ComplexUserData.Clear();
}

bool RaverieShaderIRProject::CompileTree(Raverie::Module& raverieDependencies,
                                       Raverie::LibraryRef& raverieLibrary,
                                       Raverie::SyntaxTree& syntaxTree,
                                       Raverie::Array<Raverie::UserToken>& tokensOut)
{
  // Add all of the source code to the raverie project
  Raverie::Project raverieProject;
  BuildRaverieProject(raverieProject);
  // Listen for compilation errors on this raverie project (so we can forward them
  // back up)
  ListenForRaverieErrors(raverieProject);
  ListenForTypeParsed(raverieProject);

  // Compile the source code into a syntax tree
  Raverie::LibraryBuilder libraryBuilder(mProjectName);
  bool compiledSuccessfully = raverieProject.CompileCheckedSyntaxTree(
      syntaxTree, libraryBuilder, tokensOut, raverieDependencies, Raverie::EvaluationMode::Project);

  // If it failed to compile then don't build the library
  if (!compiledSuccessfully)
    return compiledSuccessfully;

  // Always have to run the code generator to create the raverie library
  // (so we can build an executable state and run it to collect default values).
  Raverie::CodeGenerator codeGenerator;
  raverieLibrary = codeGenerator.Generate(syntaxTree, libraryBuilder);
  return compiledSuccessfully;
}

RaverieShaderIRLibraryRef RaverieShaderIRProject::CompileAndTranslate(RaverieShaderIRModuleRef& dependencies,
                                                                  BaseShaderIRTranslator* translator)
{
  // Reset the error state
  mErrorTriggered = false;

  // Add all of the dependencies to the raverie module
  Raverie::Module raverieDependencies;
  // Module is defaulting with an entry that is already being accounted for
  raverieDependencies.Clear();
  PopulateRaverieModule(raverieDependencies, dependencies);

  // Add all of the source code to the raverie project
  Raverie::Project raverieProject;
  BuildRaverieProject(raverieProject);
  // Listen for compilation errors on this raverie project (so we can forward them
  // back up)
  ListenForRaverieErrors(raverieProject);
  ListenForTypeParsed(raverieProject);

  // Compile the source code into a syntax tree
  Raverie::Array<Raverie::UserToken> tokensOut;
  Raverie::SyntaxTree syntaxTree;
  Raverie::LibraryBuilder libraryBuilder(mProjectName);
  mCompiledSuccessfully = raverieProject.CompileCheckedSyntaxTree(
      syntaxTree, libraryBuilder, tokensOut, raverieDependencies, Raverie::EvaluationMode::Project);

  // If it failed to compile then don't build the library
  if (!mCompiledSuccessfully)
    return nullptr;

  // Otherwise make a new library ref
  RaverieShaderIRLibrary* library = new RaverieShaderIRLibrary();
  library->mDependencies = dependencies;
  // Flatten all dependents from the dependency parents
  library->FlattenModuleDependents();

  // Always have to run the code generator to create the raverie library
  // (so we can build an executable state and run it to collect default values).
  Raverie::CodeGenerator codeGenerator;
  library->mRaverieLibrary = codeGenerator.Generate(syntaxTree, libraryBuilder);

  RaverieShaderIRLibraryRef libraryRef = library;

  // Add the raverie library that was just built to the raverie module so any
  // calls such as searching for a type will search the new library
  raverieDependencies.PushBack(library->mRaverieLibrary);

  // Now actually translate the types of this library
  library->mTranslated = translator->Translate(syntaxTree, this, libraryRef);
  // If translation failed then return then don't return a valid library
  if (library->mTranslated == false)
    return nullptr;

  // For all types in this library, collect the default values of all properties
  // (so they can be stored in meta or wherever)
  CollectLibraryDefaultValues(libraryRef, raverieDependencies);

  return libraryRef;
}

// RaverieShaderLibraryRef CompileAndTranslate(RaverieShaderModuleRef& dependencies,
// BaseShaderTranslator* translator, RaverieShaderSettingsRef& settings, bool test
// = false);
void RaverieShaderIRProject::BuildRaverieProject(Raverie::Project& raverieProject)
{
  for (size_t i = 0; i < mCodeEntries.Size(); ++i)
    raverieProject.AddCodeFromString(mCodeEntries[i].mCode, mCodeEntries[i].mCodeLocation, mCodeEntries[i].mUserData);
  raverieProject.UserData = UserData;
  raverieProject.ComplexUserData = ComplexUserData;
}

void RaverieShaderIRProject::PopulateRaverieModule(Raverie::Module& raverieDependencies, RaverieShaderIRModuleRef& dependencies)
{
  // Handle having no dependencies
  if (dependencies == nullptr)
    return;

  // The shader dependencies contains a whole bunch of top-level dependencies.
  // First add all of these to a stack of modules we need to walk.
  Array<RaverieShaderIRLibrary*> dependencyStack;
  for (size_t i = 0; i < dependencies->Size(); ++i)
    dependencyStack.PushBack((*dependencies)[i]);

  // Now we need to iterate over all dependencies of dependencies but in a
  // breadth first order. This is not a "proper" dependency walker and can run
  // into errors in diamond situations, but I'm ignoring this for now.
  HashSet<Raverie::Library*> visitedRaverieDependencies;
  for (size_t i = 0; i < dependencyStack.Size(); ++i)
  {
    // Add the raverie dependency of this library to the raverie module
    RaverieShaderIRLibrary* dependencyLibrary = dependencyStack[i];
    raverieDependencies.PushBack(dependencyLibrary->mRaverieLibrary);

    // @JoshD - Confirm this is correct
    visitedRaverieDependencies.Insert(dependencyLibrary->mRaverieLibrary);

    // If this shader library doesn't have any dependencies then stop
    if (dependencyLibrary->mDependencies == nullptr)
      continue;

    // Otherwise walk all dependent shader libraries
    for (size_t subIndex = 0; subIndex < dependencyLibrary->mDependencies->Size(); ++subIndex)
    {
      RaverieShaderIRLibrary* subLibrary = (*dependencyLibrary->mDependencies)[subIndex];
      Raverie::Library* subRaverieLibrary = subLibrary->mRaverieLibrary;

      // Basic error Checking
      if (subLibrary == nullptr || subRaverieLibrary == nullptr)
      {
        Error("Cannot have a null library as a dependency");
        continue;
      }
      ErrorIf(subLibrary->mTranslated == false, "Dependency was not already compiled somehow");

      // If we've already walked this library then ignore it
      if (visitedRaverieDependencies.Contains(subRaverieLibrary))
        continue;

      // Mark that we've now visited this raverie library and add it to the raverie
      // module
      visitedRaverieDependencies.Insert(subRaverieLibrary);
      dependencyStack.PushBack(subLibrary);
    }
  }
}

void RaverieShaderIRProject::CollectLibraryDefaultValues(RaverieShaderIRLibraryRef libraryRef, Raverie::Module& raverieModule)
{
  RaverieShaderIRLibrary* library = libraryRef;
  // Link the module together to get an executable state we can run (to find
  // default values)
  Raverie::ExecutableState* state = raverieModule.Link();

  // Iterate over all the types in this library
  AutoDeclare(range, library->mTypes.All());
  for (; !range.Empty(); range.PopFront())
  {
    // Find the raverie type from the shader type
    RaverieShaderIRType* shaderType = range.Front().second;
    ShaderIRTypeMeta* typeMeta = shaderType->mMeta;

    // Only get property values for fragment types (not helpers)
    if (typeMeta == nullptr || typeMeta->mFragmentType == FragmentType::None)
      continue;

    Raverie::BoundType* raverieType = raverieModule.FindType(typeMeta->mRaverieName);

    // Default construct this type
    Raverie::ExceptionReport report;
    Raverie::Handle preconstructedObject =
        state->AllocateDefaultConstructedHeapObject(raverieType, report, Raverie::HeapFlags::NonReferenceCounted);

    for (size_t i = 0; i < typeMeta->mFields.Size(); ++i)
    {
      ShaderIRFieldMeta* fieldMeta = typeMeta->mFields[i];

      // Check if this is a static field
      bool isStatic = fieldMeta->ContainsAttribute(Raverie::StaticAttribute);

      // Find the raverie property, properly setting the options
      // depending on if this is a static field or not
      Raverie::FindMemberOptions::Enum options = Raverie::FindMemberOptions::None;
      if (isStatic)
        options = Raverie::FindMemberOptions::Static;

      Raverie::Property* raverieProperty = raverieType->FindProperty(fieldMeta->mRaverieName, options);
      // Validate that the property exists in raverie. This might not exist if the
      // property is entirely generated in shaders (such as the dummy)
      if (raverieProperty == nullptr)
        continue;

      // Invoke the property's Get function so we can find the return value
      Raverie::Call call(raverieProperty->Get, state);
      // Set the 'this' handle if the field is an instance field
      if (!isStatic)
        call.SetHandle(Raverie::Call::This, preconstructedObject);
      call.Invoke(report);

      if (report.HasThrownExceptions())
      {
        Error("Getting property default value from pre-constructed object "
              "failed");
        break;
      }

      // Extract the return value of the property's Get call and store it as an
      // Raverie::Any on our ShaderType
      fieldMeta->mDefaultValueVariant = Raverie::Any(call.GetReturnUnchecked(), raverieProperty->GetTypeOrNull());
    }
  }
  delete state;
}

} // namespace Raverie
