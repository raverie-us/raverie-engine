// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

ZilchShaderIRProject::ZilchShaderIRProject(StringParam projectName)
{
  mProjectName = projectName;
  mCompiledSuccessfully = false;
}

void ZilchShaderIRProject::AddCodeFromString(StringParam code,
                                             StringParam codeLocation,
                                             void* userData)
{
  CodeEntry& entry = mCodeEntries.PushBack();
  entry.mCode = code;
  entry.mCodeLocation = codeLocation;
  entry.mUserData = userData;
}

void ZilchShaderIRProject::AddCodeFromFile(StringParam filePath, void* userData)
{
  File file;
  Status status;
  file.Open(filePath,
            FileMode::Read,
            FileAccessPattern::Sequential,
            FileShare::Read,
            &status);

  String code = ReadFileIntoString(filePath);
  AddCodeFromString(code, filePath, userData);
}

void ZilchShaderIRProject::Clear()
{
  mCodeEntries.Clear();
  UserData = nullptr;
  ComplexUserData.Clear();
}

bool ZilchShaderIRProject::CompileTree(
    Zilch::Module& zilchDependencies,
    Zilch::LibraryRef& zilchLibrary,
    Zilch::SyntaxTree& syntaxTree,
    Zilch::Array<Zilch::UserToken>& tokensOut)
{
  // Add all of the source code to the zilch project
  Zilch::Project zilchProject;
  BuildZilchProject(zilchProject);
  // Listen for compilation errors on this zilch project (so we can forward them
  // back up)
  ListenForZilchErrors(zilchProject);
  ListenForTypeParsed(zilchProject);

  // Compile the source code into a syntax tree
  Zilch::LibraryBuilder libraryBuilder(mProjectName);
  bool compiledSuccessfully =
      zilchProject.CompileCheckedSyntaxTree(syntaxTree,
                                            libraryBuilder,
                                            tokensOut,
                                            zilchDependencies,
                                            Zilch::EvaluationMode::Project);

  // If it failed to compile then don't build the library
  if (!compiledSuccessfully)
    return compiledSuccessfully;

  // Always have to run the code generator to create the zilch library
  // (so we can build an executable state and run it to collect default values).
  Zilch::CodeGenerator codeGenerator;
  zilchLibrary = codeGenerator.Generate(syntaxTree, libraryBuilder);
  return compiledSuccessfully;
}

ZilchShaderIRLibraryRef ZilchShaderIRProject::CompileAndTranslate(
    ZilchShaderIRModuleRef& dependencies, BaseShaderIRTranslator* translator)
{
  // Reset the error state
  mErrorTriggered = false;

  // Add all of the dependencies to the zilch module
  Zilch::Module zilchDependencies;
  // Module is defaulting with an entry that is already being accounted for
  zilchDependencies.Clear();
  PopulateZilchModule(zilchDependencies, dependencies);

  // Add all of the source code to the zilch project
  Zilch::Project zilchProject;
  BuildZilchProject(zilchProject);
  // Listen for compilation errors on this zilch project (so we can forward them
  // back up)
  ListenForZilchErrors(zilchProject);
  ListenForTypeParsed(zilchProject);

  // Compile the source code into a syntax tree
  Zilch::Array<Zilch::UserToken> tokensOut;
  Zilch::SyntaxTree syntaxTree;
  Zilch::LibraryBuilder libraryBuilder(mProjectName);
  mCompiledSuccessfully =
      zilchProject.CompileCheckedSyntaxTree(syntaxTree,
                                            libraryBuilder,
                                            tokensOut,
                                            zilchDependencies,
                                            Zilch::EvaluationMode::Project);

  // If it failed to compile then don't build the library
  if (!mCompiledSuccessfully)
    return nullptr;

  // Otherwise make a new library ref
  ZilchShaderIRLibrary* library = new ZilchShaderIRLibrary();
  library->mDependencies = dependencies;
  // Flatten all dependents from the dependency parents
  library->FlattenModuleDependents();

  // Always have to run the code generator to create the zilch library
  // (so we can build an executable state and run it to collect default values).
  Zilch::CodeGenerator codeGenerator;
  library->mZilchLibrary = codeGenerator.Generate(syntaxTree, libraryBuilder);

  ZilchShaderIRLibraryRef libraryRef = library;

  // Add the zilch library that was just built to the zilch module so any
  // calls such as searching for a type will search the new library
  zilchDependencies.PushBack(library->mZilchLibrary);

  // Now actually translate the types of this library
  library->mTranslated = translator->Translate(syntaxTree, this, libraryRef);
  // If translation failed then return then don't return a valid library
  if (library->mTranslated == false)
    return nullptr;

  // For all types in this library, collect the default values of all properties
  // (so they can be stored in meta or wherever)
  CollectLibraryDefaultValues(libraryRef, zilchDependencies);

  return libraryRef;
}

// ZilchShaderLibraryRef CompileAndTranslate(ZilchShaderModuleRef& dependencies,
// BaseShaderTranslator* translator, ZilchShaderSettingsRef& settings, bool test
// = false);
void ZilchShaderIRProject::BuildZilchProject(Zilch::Project& zilchProject)
{
  for (size_t i = 0; i < mCodeEntries.Size(); ++i)
    zilchProject.AddCodeFromString(mCodeEntries[i].mCode,
                                   mCodeEntries[i].mCodeLocation,
                                   mCodeEntries[i].mUserData);
  zilchProject.UserData = UserData;
  zilchProject.ComplexUserData = ComplexUserData;
}

void ZilchShaderIRProject::PopulateZilchModule(
    Zilch::Module& zilchDependencies, ZilchShaderIRModuleRef& dependencies)
{
  // Handle having no dependencies
  if (dependencies == nullptr)
    return;

  // The shader dependencies contains a whole bunch of top-level dependencies.
  // First add all of these to a stack of modules we need to walk.
  Array<ZilchShaderIRLibrary*> dependencyStack;
  for (size_t i = 0; i < dependencies->Size(); ++i)
    dependencyStack.PushBack((*dependencies)[i]);

  // Now we need to iterate over all dependencies of dependencies but in a
  // breadth first order. This is not a "proper" dependency walker and can run
  // into errors in diamond situations, but I'm ignoring this for now.
  HashSet<Zilch::Library*> visitedZilchDependencies;
  for (size_t i = 0; i < dependencyStack.Size(); ++i)
  {
    // Add the zilch dependency of this library to the zilch module
    ZilchShaderIRLibrary* dependencyLibrary = dependencyStack[i];
    zilchDependencies.PushBack(dependencyLibrary->mZilchLibrary);

    // @JoshD - Confirm this is correct
    visitedZilchDependencies.Insert(dependencyLibrary->mZilchLibrary);

    // If this shader library doesn't have any dependencies then stop
    if (dependencyLibrary->mDependencies == nullptr)
      continue;

    // Otherwise walk all dependent shader libraries
    for (size_t subIndex = 0;
         subIndex < dependencyLibrary->mDependencies->Size();
         ++subIndex)
    {
      ZilchShaderIRLibrary* subLibrary =
          (*dependencyLibrary->mDependencies)[subIndex];
      Zilch::Library* subZilchLibrary = subLibrary->mZilchLibrary;

      // Basic error Checking
      if (subLibrary == nullptr || subZilchLibrary == nullptr)
      {
        Error("Cannot have a null library as a dependency");
        continue;
      }
      ErrorIf(subLibrary->mTranslated == false,
              "Dependency was not already compiled somehow");

      // If we've already walked this library then ignore it
      if (visitedZilchDependencies.Contains(subZilchLibrary))
        continue;

      // Mark that we've now visited this zilch library and add it to the zilch
      // module
      visitedZilchDependencies.Insert(subZilchLibrary);
      dependencyStack.PushBack(subLibrary);
    }
  }
}

void ZilchShaderIRProject::CollectLibraryDefaultValues(
    ZilchShaderIRLibraryRef libraryRef, Zilch::Module& zilchModule)
{
  ZilchShaderIRLibrary* library = libraryRef;
  // Link the module together to get an executable state we can run (to find
  // default values)
  Zilch::ExecutableState* state = zilchModule.Link();

  // Iterate over all the types in this library
  AutoDeclare(range, library->mTypes.All());
  for (; !range.Empty(); range.PopFront())
  {
    // Find the zilch type from the shader type
    ZilchShaderIRType* shaderType = range.Front().second;
    ShaderIRTypeMeta* typeMeta = shaderType->mMeta;

    // Only get property values for fragment types (not helpers)
    if (typeMeta == nullptr || typeMeta->mFragmentType == FragmentType::None)
      continue;

    Zilch::BoundType* zilchType = zilchModule.FindType(typeMeta->mZilchName);

    // Default construct this type
    Zilch::ExceptionReport report;
    Zilch::Handle preconstructedObject =
        state->AllocateDefaultConstructedHeapObject(
            zilchType, report, Zilch::HeapFlags::NonReferenceCounted);

    for (size_t i = 0; i < typeMeta->mFields.Size(); ++i)
    {
      ShaderIRFieldMeta* fieldMeta = typeMeta->mFields[i];

      // Check if this is a static field
      bool isStatic = fieldMeta->ContainsAttribute(Zilch::StaticAttribute);

      // Find the zilch property, properly setting the options
      // depending on if this is a static field or not
      Zilch::FindMemberOptions::Enum options = Zilch::FindMemberOptions::None;
      if (isStatic)
        options = Zilch::FindMemberOptions::Static;

      Zilch::Property* zilchProperty =
          zilchType->FindProperty(fieldMeta->mZilchName, options);
      // Validate that the property exists in zilch. This might not exist if the
      // property is entirely generated in shaders (such as the dummy)
      if (zilchProperty == nullptr)
        continue;

      // Invoke the property's Get function so we can find the return value
      Zilch::Call call(zilchProperty->Get, state);
      // Set the 'this' handle if the field is an instance field
      if (!isStatic)
        call.SetHandle(Zilch::Call::This, preconstructedObject);
      call.Invoke(report);

      if (report.HasThrownExceptions())
      {
        Error("Getting property default value from pre-constructed object "
              "failed");
        break;
      }

      // Extract the return value of the property's Get call and store it as an
      // Zilch::Any on our ShaderType
      fieldMeta->mDefaultValueVariant =
          Zilch::Any(call.GetReturnUnchecked(), zilchProperty->GetTypeOrNull());
    }
  }
  delete state;
}

} // namespace Zero
