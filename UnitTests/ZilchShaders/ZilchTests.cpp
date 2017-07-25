///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

void CollectMainResults(SimpleZilchShaderGenerator& shaderGenerator, Zilch::ExecutableState* state, Zilch::BoundType* boundType, RenderResults& results)
{
  Zilch::ExceptionReport report;
  // Construct the shader's bound type
  Zilch::Handle preconstructedObject = state->AllocateDefaultConstructedHeapObject(boundType, report, Zilch::HeapFlags::NonReferenceCounted);

  // Find and execute Main
  Zilch::Function* mainFunction = boundType->FindFunction("Main", Array<Zilch::Type*>(), Zilch::Core::GetInstance().VoidType, Zilch::FindMemberOptions::None);
  if(mainFunction == nullptr)
  {
    Error("This should never happen (failed to find main)");
    return;
  }

  Zilch::Call mainCall(mainFunction, state);
  mainCall.SetHandle(Zilch::Call::This, preconstructedObject);
  mainCall.Invoke(report);

  RenderResult& result = results.mLanguageResult[RenderResults::mZilchKey];
  Array<String>& renderTargetNames = shaderGenerator.mSettings->mShaderDefinitionSettings.mRenderTargetNames;
  // Iterate over all of the provided render target names and collect their default values if they exist
  for(size_t i = 0; i < renderTargetNames.Size(); ++i)
  {
    // Find the property of this render target
    String renderTargetName = renderTargetNames[i];
    Zilch::Property* prop = boundType->FindProperty(renderTargetName, Zilch::FindMemberOptions::None);
    // If it doesn't exist (meaning the fragments/shader don't write to it) then just mark it as unused
    if(prop == nullptr)
    {
      results.mTargets[i] = false;
      continue;
    }

    // Otherwise mark this target as used
    results.mTargets[i] = true;

    // Get the result of the property (call the "Get" function on it).
    Zilch::Call call(prop->Get, state);
    call.SetHandle(Zilch::Call::This, preconstructedObject);
    call.Invoke(report);

    result.mData[i] = call.Get<Vec4>(Zilch::Call::Return);
  }
}

void ComputeZilchRenderResults(SimpleZilchShaderGenerator& shaderGenerator, StringParam zilchTypeName, RenderResults& results)
{
  // Build the zilch module for the shader library
  ZilchShaderModuleRef shaderDependencies = new ZilchShaderModule();
  shaderDependencies->PushBack(shaderGenerator.mFragmentLibraryRef);
  Zilch::Module zilchModule;
  shaderGenerator.mShaderProject.PopulateZilchModule(zilchModule, shaderDependencies);
  zilchModule.PushBack(shaderGenerator.mShaderLibraryRef->mZilchLibrary);
  
  // Link the module into an exe
  Zilch::ExecutableState* state = zilchModule.Link();
  // Find the type that we're trying to collect results for
  Zilch::BoundType* boundType = zilchModule.FindType(zilchTypeName);

  CollectMainResults(shaderGenerator, state, boundType, results);

  delete state;
}
