///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------ZilchShaderProject
ZilchShaderProject::ZilchShaderProject(StringParam projectName)
{
  mProjectName = projectName;
  mCompiledSuccessfully = false;
}

void ZilchShaderProject::AddCodeFromString(StringParam code, StringParam codeLocation, void* userData)
{
  CodeEntry& entry = mCodeEntries.PushBack();
  entry.mCode = code;
  entry.mCodeLocation = codeLocation;
  entry.mUserData = userData;
}

void ZilchShaderProject::AddCodeFromFile(StringParam filePath, void* userData)
{
  File file;
  Status status;
  file.Open(filePath, FileMode::Read, FileAccessPattern::Sequential, FileShare::Read, &status);

  String code = ReadFileIntoString(filePath);
  AddCodeFromString(code, filePath, userData);
}

void ZilchShaderProject::Clear()
{
  mCodeEntries.Clear();
  UserData = nullptr;
  ComplexUserData.Clear();
}

ZilchShaderLibraryRef ZilchShaderProject::CompileAndTranslate(ZilchShaderModuleRef& dependencies, BaseShaderTranslator* translator, ZilchShaderSettingsRef& settings)
{
  // Add all of the dependencies to the zilch module
  Zilch::Module zilchDependencies;
  // Module is defaulting with an entry that is already being accounted for
  zilchDependencies.Clear();
  PopulateZilchModule(zilchDependencies, dependencies);
  
  // Add all of the source code to the zilch project
  Zilch::Project zilchProject;
  BuildZilchProject(zilchProject);
  // Listen for compilation errors on this zilch project (so we can forward them back up)
  ListenForZilchErrors(zilchProject);

  // Compile the source code into a syntax tree
  Zilch::Array<Zilch::UserToken> tokensOut;
  Zilch::SyntaxTree syntaxTree;
  Zilch::LibraryBuilder libraryBuilder(mProjectName);
  mCompiledSuccessfully = zilchProject.CompileCheckedSyntaxTree(syntaxTree, libraryBuilder, tokensOut, zilchDependencies, Zilch::EvaluationMode::Project);

  // If it failed to compile then don't build the library
  if(!mCompiledSuccessfully)
    return nullptr;

  // Otherwise make a new library ref
  ZilchShaderLibrary* library = new ZilchShaderLibrary();
  library->mDependencies = dependencies;
  // Flatten all dependents from the dependency parents
  library->FlattenModuleDependents();

  // Always have to run the code generator to create the zilch library
  // (so we can build an executable state and run it to collect default values).
  Zilch::CodeGenerator codeGenerator;
  library->mZilchLibrary = codeGenerator.Generate(syntaxTree, libraryBuilder);
   
  library->mIsUserCode = true;
  ZilchShaderLibraryRef libraryRef = library;

  // Add the zilch library that was just built to the zilch module so any
  // calls such as searching for a type will search the new library
  zilchDependencies.PushBack(library->mZilchLibrary);

  // First run the type collector (grabs type names along with their functions, members, and attributes)
  ZilchTypeCollector typeCollector;
  typeCollector.CollectTypes(syntaxTree, libraryRef, &zilchDependencies, this, settings);
  // If an error was triggered during type collecting then just return (likely because of using class instead of struct)
  if(this->mErrorTriggered)
    return nullptr;

  // Now actually translate the types of this library
  library->mTranslated = translator->Translate(syntaxTree, this, libraryRef);
  // If translation failed then return then don't return a valid library
  if(library->mTranslated == false)
    return nullptr;

  // For all types in this library, collect the default values of all properties (so they can be stored in meta or wherever)
  CollectLibraryDefaultValues(libraryRef, zilchDependencies);
  
  return libraryRef;
}

void ZilchShaderProject::BuildZilchProject(Zilch::Project& zilchProject)
{
  for(size_t i = 0; i < mCodeEntries.Size(); ++i)
    zilchProject.AddCodeFromString(mCodeEntries[i].mCode, mCodeEntries[i].mCodeLocation, mCodeEntries[i].mUserData);
  zilchProject.UserData = UserData;
  zilchProject.ComplexUserData = ComplexUserData;
}

void ZilchShaderProject::PopulateZilchModule(Zilch::Module& zilchDependencies, ZilchShaderModuleRef& dependencies)
{
  // Handle having no dependencies
  if(dependencies == nullptr)
    return;

  // The shader dependencies contains a whole bunch of top-level dependencies.
  // First add all of these to a stack of modules we need to walk.
  Array<ZilchShaderLibrary*> dependencyStack;
  for(size_t i = 0; i < dependencies->Size(); ++i)
    dependencyStack.PushBack((*dependencies)[i]);

  // Now we need to iterate over all dependencies of dependencies but in a breadth first order. This is not a "proper"
  // dependency walker and can run into errors in diamond situations, but I'm ignoring this for now.
  HashSet<Zilch::Library*> visitedZilchDependencies;
  for(size_t i = 0; i < dependencyStack.Size(); ++i)
  {
    // Add the zilch dependency of this library to the zilch module
    ZilchShaderLibrary* dependencyLibrary = dependencyStack[i];
    zilchDependencies.PushBack(dependencyLibrary->mZilchLibrary);

    // @JoshD - Confirm this is correct
    visitedZilchDependencies.Insert(dependencyLibrary->mZilchLibrary);

    // If this shader library doesn't have any dependencies then stop
    if(dependencyLibrary->mDependencies == nullptr)
      continue;

    // Otherwise walk all dependent shader libraries
    for(size_t subIndex = 0; subIndex < dependencyLibrary->mDependencies->Size(); ++subIndex)
    {
      ZilchShaderLibrary* subLibrary = (*dependencyLibrary->mDependencies)[subIndex];
      Zilch::Library* subZilchLibrary = subLibrary->mZilchLibrary;

      // Basic error Checking
      if(subLibrary == nullptr || subZilchLibrary == nullptr)
      {
        Error("Cannot have a null library as a dependency");
        continue;
      }
      ErrorIf(subLibrary->mIsUserCode == true && subLibrary->mTranslated == false, "Dependency was not already compiled somehow");

      // If we've already walked this library then ignore it
      if(visitedZilchDependencies.Contains(subZilchLibrary))
        continue;

      // Mark that we've now visited this zilch library and add it to the zilch module
      visitedZilchDependencies.Insert(subZilchLibrary);
      dependencyStack.PushBack(subLibrary);
    }
  }
}

void ZilchShaderProject::CollectLibraryDefaultValues(ZilchShaderLibraryRef libraryRef, Zilch::Module& zilchModule)
{
  ZilchShaderLibrary* library = libraryRef;
  // Link the module together to get an executable state we can run (to find default values)
  Zilch::ExecutableState* state = zilchModule.Link();
  
  // Iterate over all the types in this library
  AutoDeclare(range, library->mTypes.All());
  for(; !range.Empty(); range.PopFront())
  {
    // Find the zilch type from the shader type
    ShaderType* shaderType = range.Front().second;
    // Only get property values for fragment types (not helpers)
    if(shaderType->mFragmentType == FragmentType::None)
      continue;

    Zilch::BoundType* zilchType = zilchModule.FindType(shaderType->mZilchName);

    // Default construct this type
    Zilch::ExceptionReport report;
    Zilch::Handle preconstructedObject = state->AllocateDefaultConstructedHeapObject(zilchType, report, Zilch::HeapFlags::NonReferenceCounted);

    // Iterate over all properties on the type
    forRange(Zilch::Property* zilchProperty, zilchType->AllProperties.All())
    {
      // Skip static properties for now. We don't actually ever need (at the moment)
      // the default value for static properties and there is a minor bug in Zilch when
      // going through a getter where it assumes the getter is on an instance field.
      if(zilchProperty->IsStatic)
        continue;

      // Find the corresponding shader field for the zilch property
      ShaderField* shaderField = shaderType->FindField(zilchProperty->Name);
      if(shaderField == nullptr)
        continue;

      // Invoke the property's Get function so we can find the return value
      Zilch::Call call(zilchProperty->Get, state);
      call.SetHandle(Zilch::Call::This, preconstructedObject);
      call.Invoke(report);

      if (report.HasThrownExceptions())
      {
        Error("Getting property default value from pre-constructed object failed");
        break;
      }

      // Extract the return value of the property's Get call and store it as an Zilch::Any on our ShaderType
      shaderField->mDefaultValueVariant = Zilch::Any(call.GetReturnUnchecked(), zilchProperty->GetTypeOrNull());
    }

    // Make sure to destruct the object
    preconstructedObject.Delete();
  }
  delete state;
}

}//namespace Zero
