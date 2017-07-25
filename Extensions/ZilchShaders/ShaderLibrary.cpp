///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------ZilchShaderModule
ShaderType* ZilchShaderModule::FindType(StringParam name, bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderLibrary* library = (*this)[i];
    ShaderType* type = library->FindType(name, checkDependencies);
    if(type != nullptr)
      return type;
  }
  return nullptr;
}

ShaderType* ZilchShaderModule::FindType(Zilch::BoundType* boundType, bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderLibrary* library = (*this)[i];
    ShaderType* type = library->FindType(boundType, checkDependencies);
    if(type != nullptr)
      return type;
  }
  return nullptr;
}

ShaderFunction* ZilchShaderModule::FindImplements(Zilch::Function* fnKey)
{
  // Find if any library implements this function
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderLibrary* library = (*this)[i];
    ShaderFunction* function = library->FindImplements(fnKey);
    if(function != nullptr)
      return function;
  }
  return nullptr;
}

ShaderFunction* ZilchShaderModule::FindExtension(Zilch::Function* fnKey)
{
  // Find if any library extends with this function
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderLibrary* library = (*this)[i];
    ShaderFunction* function = library->FindExtension(fnKey);
    if(function != nullptr)
      return function;
  }
  return nullptr;
}

//-------------------------------------------------------------------ZilchShaderLibrary
ZilchShaderLibrary::ZilchShaderLibrary()
{
  mIsUserCode = false;
  mTranslated = false;
}

ZilchShaderLibrary::~ZilchShaderLibrary()
{
  // Delete all of the types we created
  AutoDeclare(typeRange, mTypes.Values());
  for(; !typeRange.Empty(); typeRange.PopFront())
  {
    ShaderType* type = typeRange.Front();
    delete type;
  }
}

ShaderType* ZilchShaderLibrary::FindType(StringParam name, bool checkDependencies)
{
  // Try to find the type in this library
  ShaderType* type = mTypes.FindValue(name, nullptr);
  if(type != nullptr)
    return type;

  // If we failed to find the type but we don't check dependencies then return that we can't find it
  if(!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if(mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindType(name);
}

ShaderType* ZilchShaderLibrary::FindType(Zilch::BoundType* boundType, bool checkDependencies)
{
  // Try to find the type in this library
  ShaderType* type = mZilchToShaderTypes.FindValue(boundType, nullptr);
  if(type != nullptr)
    return type;

  // If we failed to find the type but we don't check dependencies then return that we can't find it
  if(!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if(mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindType(boundType);
}

ShaderType* ZilchShaderLibrary::CreateType(StringParam name)
{
  if(mTypes.ContainsKey(name))
  {
    Error("Library already contains type '%s", name.c_str());
    return mTypes[name];
  }
  
  // Create the type and store the zilch information
  ShaderType* type = new ShaderType();
  type->mZilchName = name;
  type->mOwningLibrary = this;

  mTypes[type->mZilchName] = type;
  return type;
}

ShaderType* ZilchShaderLibrary::CreateType(Zilch::BoundType* boundType)
{
  String typeName = boundType->ToString();
  ShaderType* shaderType = CreateType(typeName);
  mZilchToShaderTypes[boundType] = shaderType;
  return shaderType;
}

ShaderType* ZilchShaderLibrary::CreateNativeType(StringParam zilchName, StringParam shaderName, StringRange refType, ShaderVarType::Enum varType)
{
  Zilch::Type* zilchType = mZilchLibrary->BoundTypes.FindValue(zilchName, nullptr);
  ErrorIf(zilchType == nullptr, "Cannot create a native type with no backing zilch type. Zilch type '%s' doesn't exist.", zilchName.c_str());

  ShaderType* shaderType = CreateType(zilchName);
  shaderType->mShaderName = shaderName;
  shaderType->mReferenceTypeQualifier = refType;
  shaderType->mPropertyType = zilchName;
  shaderType->SetNative(true);
  shaderType->SetHideStructAndFunctions(true);
  shaderType->mOwningLibrary = this;
  ShaderTypeData* typeData = shaderType->GetTypeData();
  typeData->mType = varType;
  mZilchToShaderTypes[zilchType] = shaderType;
  return shaderType;
}

ShaderFunction* ZilchShaderLibrary::FindImplements(Zilch::Function* fnKey)
{
  // If this library implements the given function then return the shader function that does so
  ShaderFunction* implementsFunction = mImplements.FindValue(fnKey, nullptr);
  // Otherwise check all of the dependencies
  if(implementsFunction == nullptr && mDependencies != nullptr)
    return mDependencies->FindImplements(fnKey);
  return implementsFunction;
}

ShaderFunction* ZilchShaderLibrary::FindExtension(Zilch::Function* fnKey)
{
  // If this library implements the given function then return the shader function that does so
  ShaderFunction* extensionFunction = mExtensions.FindValue(fnKey, nullptr);
  // Otherwise check all of the dependencies
  if(extensionFunction == nullptr && mDependencies != nullptr)
    return mDependencies->FindExtension(fnKey);
  return extensionFunction;
}

void ZilchShaderLibrary::FlattenModuleDependents()
{
  // Bring all dependents from our dependency modules into ourself
  ZilchShaderModule* module = mDependencies;
  for(size_t i = 0; i < module->Size(); ++i)
  {
    ZilchShaderLibrary* parentLibrary = (*module)[i];

    // For each type in the parent library's reverse dependencies, copy all dependents into the current library
    AutoDeclare(pairRange, parentLibrary->mTypeDependents.All());
    for(; !pairRange.Empty(); pairRange.PopFront())
    {
      AutoDeclare(pair, pairRange.Front());
      HashSet<ShaderType*>& parentLibraryDependents = pair.second;
      HashSet<ShaderType*>& currentLibraryDependents = mTypeDependents[pair.first];
      
      // Copy all dependents into the current library
      HashSet<ShaderType*>::range dependentRange = parentLibraryDependents.All();
      for(; !dependentRange.Empty(); dependentRange.PopFront())
        currentLibraryDependents.Insert(dependentRange.Front());
    }
  }
}

void ZilchShaderLibrary::GetAllDependents(ShaderType* shaderType, HashSet<ShaderType*>& finalDependents)
{
  // Find if the current shader type has any dependents (things that depend 
  // on it). If it doesn't then there's nothing more to do.
  HashSet<ShaderType*>* currentTypeDependents = mTypeDependents.FindPointer(shaderType);
  if(currentTypeDependents == nullptr)
    return;

  // Iterate over all dependent types
  HashSet<ShaderType*>::range range = currentTypeDependents->All();
  for(; !range.Empty(); range.PopFront())
  {
    // Don't visit a dependent that's already been visited
    ShaderType* dependent = range.Front();
    if(finalDependents.Contains(dependent))
      continue;

    // Mark that we've visited this dependent and get all of its dependents recursively
    finalDependents.Insert(dependent);
    GetAllDependents(dependent, finalDependents);
  }
}

}//namespace Zero
