///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------TypeResolvers
TypeResolvers::TypeResolvers()
{
  mBackupFieldResolver = nullptr;
  mDefaultConstructorResolver = nullptr;
  mBackupConstructorResolver = nullptr;
  mBackupSetterResolver = nullptr;
  mExpressionInitializerListResolver = nullptr;
}

void TypeResolvers::RegisterFieldResolver(Zilch::Field* field, MemberAccessResolverIRFn fieldResolver)
{
  ErrorIf(field == nullptr, "Cannot register a field resolver on a null field");
  mFieldResolvers.InsertOrError(field, fieldResolver);
}

void TypeResolvers::RegisterBackupFieldResolver(MemberAccessResolverIRFn backupResolver)
{
  ErrorIf(mBackupFieldResolver != nullptr, "Backup field resolver cannot be registered twice.");
  mBackupFieldResolver = backupResolver;
}

MemberAccessResolverIRFn TypeResolvers::FindFieldResolver(Zilch::Field* field)
{
  MemberAccessResolverIRFn resolver = mFieldResolvers.FindValue(field, nullptr);
  if(resolver == nullptr)
    resolver = mBackupFieldResolver;
  return resolver;
}

void TypeResolvers::RegisterConstructorResolver(Zilch::Function* zilchFunction, ConstructorCallResolverIRFn resolverFn)
{
  ErrorIf(zilchFunction == nullptr, "Trying to register a null function resolver");
  ErrorIf(mConstructorResolvers.ContainsKey(zilchFunction), "Constructor resolver was already registered");
  mConstructorResolvers[zilchFunction] = resolverFn;
}

ConstructorCallResolverIRFn TypeResolvers::FindConstructorResolver(Zilch::Function* zilchFunction)
{
  ConstructorCallResolverIRFn resolver = mConstructorResolvers.FindValue(zilchFunction, nullptr);
  if(resolver != nullptr)
    return resolver;
  return mBackupConstructorResolver;
}

void TypeResolvers::RegisterFunctionResolver(Zilch::Function* function, MemberFunctionResolverIRFn functionResolver)
{
  ErrorIf(function == nullptr, "Cannot register a function resolver on a null function");
  mFunctionResolvers.InsertOrError(function, functionResolver);
}

MemberFunctionResolverIRFn TypeResolvers::FindFunctionResolver(Zilch::Function* function)
{
  return mFunctionResolvers.FindValue(function, nullptr);
}

void TypeResolvers::RegisterSetterResolver(Zilch::Function* function, MemberPropertySetterResolverIRFn functionResolver)
{
  ErrorIf(function == nullptr, "Cannot register a function resolver on a null function");
  mSetterResolvers.InsertOrError(function, functionResolver);
}

void TypeResolvers::RegisterBackupSetterResolver(MemberPropertySetterResolverIRFn backupResolver)
{
  mBackupSetterResolver = backupResolver;
}

MemberPropertySetterResolverIRFn TypeResolvers::FindSetterResolver(Zilch::Function* function)
{
  MemberPropertySetterResolverIRFn resolver = mSetterResolvers.FindValue(function, nullptr);
  if(resolver == nullptr)
    resolver = mBackupSetterResolver;
  return resolver;
}

//-------------------------------------------------------------------OperationResolvers
void OperatorResolvers::RegisterBinaryOpResolver(Zilch::Type* lhsType, Zilch::Type* rhsType, Zilch::Grammar::Enum op, BinaryOpResolverIRFn resolver)
{
  BinaryOperatorKey opId(lhsType, rhsType, op);
  mBinaryOpResolvers.InsertOrError(opId, resolver);
}

BinaryOpResolverIRFn OperatorResolvers::FindOpResolver(BinaryOperatorKey& opId)
{
  return mBinaryOpResolvers.FindValue(opId, nullptr);
}

void OperatorResolvers::RegisterUnaryOpResolver(Zilch::Type* type, Zilch::Grammar::Enum op, UnaryOpResolverIRFn resolver)
{
  UnaryOperatorKey opId(type, op);
  mUnaryOpResolvers.InsertOrError(opId, resolver);
}

UnaryOpResolverIRFn OperatorResolvers::FindOpResolver(UnaryOperatorKey& opId)
{
  return mUnaryOpResolvers.FindValue(opId, nullptr);
}

void OperatorResolvers::RegisterTypeCastOpResolver(Zilch::Type* fromType, Zilch::Type* toType, TypeCastResolverIRFn resolver)
{
  TypeCastKey opId(fromType, toType);
  mTypeCastResolvers.InsertOrError(opId, resolver);
}

TypeCastResolverIRFn OperatorResolvers::FindOpResolver(TypeCastKey& opId)
{
  return mTypeCastResolvers.FindValue(opId, nullptr);
}

//-------------------------------------------------------------------GlobalVariableData
GlobalVariableData::GlobalVariableData()
{
  mInstance = nullptr;
  mInitializerFunction = nullptr;
}

GlobalVariableData::~GlobalVariableData()
{
  delete mInstance;
}

//-------------------------------------------------------------------StageRequirementsData
StageRequirementsData::StageRequirementsData()
{
  mRequiredStages = (ShaderStage::Enum)ShaderStage::None;
  mDependency = nullptr;
}

void StageRequirementsData::Combine(Zilch::Member* dependency, const Zilch::CodeLocation& location, ShaderStage::Enum requiredStage)
{
  // Only set the dependency and call location on the first non-empty stage requirement.
  if(mDependency == nullptr && requiredStage != ShaderStage::None)
  {
    mDependency = dependency;
    mCallLocation = location;
  }
  mRequiredStages |= requiredStage;
}

//-------------------------------------------------------------------ZilchShaderIRModule
ZilchShaderIRType* ZilchShaderIRModule::FindType(const String& typeName, bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderIRLibrary* library = (*this)[i];
    ZilchShaderIRType* type = library->FindType(typeName, checkDependencies);
    if(type != nullptr)
      return type;
  }
  return nullptr;
}

GlobalVariableData* ZilchShaderIRModule::FindGlobalVariable(Zilch::Field* zilchField, bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderIRLibrary* library = (*this)[i];
    GlobalVariableData* result = library->FindGlobalVariable(zilchField, checkDependencies);
    if(result != nullptr)
      return result;
  }
  return nullptr;
}

GlobalVariableData* ZilchShaderIRModule::FindGlobalVariable(ZilchShaderIROp* globalInstance, bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderIRLibrary* library = (*this)[i];
    GlobalVariableData* result = library->FindGlobalVariable(globalInstance, checkDependencies);
    if(result != nullptr)
      return result;
  }
  return nullptr;
}

TemplateTypeIRResloverFn ZilchShaderIRModule::FindTemplateResolver(const TemplateTypeKey& templateKey, bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderIRLibrary* library = (*this)[i];
    TemplateTypeIRResloverFn resolver = library->FindTemplateResolver(templateKey, checkDependencies);
    if(resolver != nullptr)
      return resolver;
  }
  return nullptr;
}

TypeResolvers* ZilchShaderIRModule::FindTypeResolver(Zilch::Type* zilchType, bool checkDependencies)
{
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderIRLibrary* library = (*this)[i];
    TypeResolvers* result = library->FindTypeResolver(zilchType, checkDependencies);
    if(result != nullptr)
      return result;
  }
  return nullptr;
}

ZilchShaderIRFunction* ZilchShaderIRModule::FindFunction(Zilch::Function* zilchFunction, bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderIRLibrary* library = (*this)[i];
    ZilchShaderIRFunction* irFunction = library->FindFunction(zilchFunction, checkDependencies);
    if(irFunction != nullptr)
      return irFunction;
  }
  return nullptr;
}

SpirVExtensionInstruction* ZilchShaderIRModule::FindExtensionInstruction(Zilch::Function* zilchFunction, bool checkDependencies)
{
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderIRLibrary* library = (*this)[i];
    SpirVExtensionInstruction* result = library->FindExtensionInstruction(zilchFunction, checkDependencies);
    if(result != nullptr)
      return result;
  }
  return nullptr;
}

ZilchShaderExtensionImport* ZilchShaderIRModule::FindExtensionLibraryImport(SpirVExtensionLibrary* extensionLibrary, bool checkDependencies)
{
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderIRLibrary* library = (*this)[i];
    ZilchShaderExtensionImport* result = library->FindExtensionLibraryImport(extensionLibrary, checkDependencies);
    if(result != nullptr)
      return result;
  }
  return nullptr;
}

ZilchShaderIRConstantLiteral* ZilchShaderIRModule::FindConstantLiteral(Zilch::Any& literalValue, bool checkDependencies)
{
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderIRLibrary* library = (*this)[i];
    ZilchShaderIRConstantLiteral* result = library->FindConstantLiteral(literalValue, checkDependencies);
    if(result != nullptr)
      return result;
  }
  return nullptr;
}

ZilchShaderIROp* ZilchShaderIRModule::FindConstantOp(ConstantOpKeyType& key, bool checkDependencies)
{
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderIRLibrary* library = (*this)[i];
    ZilchShaderIROp* result = library->FindConstantOp(key, checkDependencies);
    if(result != nullptr)
      return result;
  }
  return nullptr;
}

ZilchShaderIROp* ZilchShaderIRModule::FindEnumConstantOp(void* key, bool checkDependencies)
{
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderIRLibrary* library = (*this)[i];
    ZilchShaderIROp* result = library->FindEnumConstantOp(key, checkDependencies);
    if(result != nullptr)
      return result;
  }
  return nullptr;
}

ZilchShaderIROp* ZilchShaderIRModule::FindSpecializationConstantOp(void* key, bool checkDependencies)
{
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderIRLibrary* library = (*this)[i];
    ZilchShaderIROp* result = library->FindSpecializationConstantOp(key, checkDependencies);
    if(result != nullptr)
      return result;
  }
  return nullptr;
}

template <typename OpIdType, typename OpResolverType>
OpResolverType ZilchShaderIRModule::FindOperatorResolverTemplate(OpIdType& opId, bool checkDependencies)
{
  for(size_t i = 0; i < Size(); ++i)
  {
    ZilchShaderIRLibrary* library = (*this)[i];
    OpResolverType result = library->FindOperatorResolver(opId, checkDependencies);
    if(result != nullptr)
      return result;
  }
  return nullptr;
}

//-------------------------------------------------------------------ZilchShaderIRLibrary
ZilchShaderIRLibrary::ZilchShaderIRLibrary()
{
  mTranslated = false;
}

ZilchShaderIRLibrary::~ZilchShaderIRLibrary()
{
  DeleteObjectsIn(mOwnedTypes.All());
  DeleteObjectsIn(mOwnedFunctions.All());
  DeleteObjectsIn(mOwnedFieldMeta.All());
  DeleteObjectsIn(mOwnedFunctionMeta.All());
  DeleteObjectsIn(mOwnedTypeMeta.All());
  DeleteObjectsIn(mOwnedSpecializationConstants.All());

  DeleteObjectsIn(mExtensionInstructions.Values());
  DeleteObjectsIn(mExtensionLibraries.All());
  DeleteObjectsIn(mConstantLiterals.Values());
  DeleteObjectsIn(mConstantOps.Values());
  DeleteObjectsIn(mExtensionLibraryImports.Values());
  DeleteObjectsIn(mZilchFieldToGlobalVariable.Values());
}

void ZilchShaderIRLibrary::AddType(StringParam typeName, ZilchShaderIRType* shaderType)
{
  mTypes.InsertOrError(typeName, shaderType);
  mOwnedTypes.PushBack(shaderType);
}

ZilchShaderIRType* ZilchShaderIRLibrary::FindType(const String& typeName, bool checkDependencies)
{
  // Try to find the type in this library
  ZilchShaderIRType* type = mTypes.FindValue(typeName, nullptr);
  if(type != nullptr)
    return type;

  // If we failed to find the type but we don't check dependencies then return that we can't find it
  if(!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if(mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindType(typeName);
}

ZilchShaderIRType* ZilchShaderIRLibrary::FindType(Zilch::Type* zilchType, bool checkDependencies)
{
  Zilch::BoundType* boundType = Zilch::BoundType::GetBoundType(zilchType);
  return FindType(boundType->Name, checkDependencies);
}

GlobalVariableData* ZilchShaderIRLibrary::FindGlobalVariable(Zilch::Field* zilchField, bool checkDependencies)
{
  GlobalVariableData* result = mZilchFieldToGlobalVariable.FindValue(zilchField, nullptr);
  if(result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return that we can't find it
  if(!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if(mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindGlobalVariable(zilchField, checkDependencies);
}

GlobalVariableData* ZilchShaderIRLibrary::FindGlobalVariable(ZilchShaderIROp* globalInstance, bool checkDependencies)
{
  Zilch::Field* zilchField = mGlobalVariableToZilchField.FindValue(globalInstance, nullptr);
  GlobalVariableData* result = mZilchFieldToGlobalVariable.FindValue(zilchField, nullptr);
  if(result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return that we can't find it
  if(!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if(mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindGlobalVariable(zilchField, checkDependencies);
}

void ZilchShaderIRLibrary::RegisterTemplateResolver(const TemplateTypeKey& templateKey, TemplateTypeIRResloverFn resolver)
{
  mTemplateResolvers.InsertOrError(templateKey, resolver);
}

TemplateTypeIRResloverFn ZilchShaderIRLibrary::FindTemplateResolver(const TemplateTypeKey& templateKey, bool checkDependencies)
{
  // Try to find the type in this library
  TemplateTypeIRResloverFn resolver = mTemplateResolvers.FindValue(templateKey, nullptr);
  if(resolver != nullptr)
    return resolver;

  // If we failed to find the type but we don't check dependencies then return that we can't find it
  if(!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if(mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindTemplateResolver(templateKey);
}

void ZilchShaderIRLibrary::FlattenModuleDependents()
{
  // Bring all dependents from our dependency modules into ourself
  ZilchShaderIRModule* module = mDependencies;
  for(size_t i = 0; i < module->Size(); ++i)
  {
    ZilchShaderIRLibrary* parentLibrary = (*module)[i];

    // For each type in the parent library's reverse dependencies, copy all dependents into the current library
    AutoDeclare(pairRange, parentLibrary->mTypeDependents.All());
    for(; !pairRange.Empty(); pairRange.PopFront())
    {
      AutoDeclare(pair, pairRange.Front());
      HashSet<ZilchShaderIRType*>& parentLibraryDependents = pair.second;
      HashSet<ZilchShaderIRType*>& currentLibraryDependents = mTypeDependents[pair.first];

      // Copy all dependents into the current library
      HashSet<ZilchShaderIRType*>::range dependentRange = parentLibraryDependents.All();
      for(; !dependentRange.Empty(); dependentRange.PopFront())
        currentLibraryDependents.Insert(dependentRange.Front());
    }
  }
}

void ZilchShaderIRLibrary::GetAllDependents(ZilchShaderIRType* shaderType, HashSet<ZilchShaderIRType*>& finalDependents)
{
  // Find if the current shader type has any dependents (things that depend 
  // on it). If it doesn't then there's nothing more to do.
  HashSet<ZilchShaderIRType*>* currentTypeDependents = mTypeDependents.FindPointer(shaderType);
  if(currentTypeDependents == nullptr)
    return;

  // Iterate over all dependent types
  HashSet<ZilchShaderIRType*>::range range = currentTypeDependents->All();
  for(; !range.Empty(); range.PopFront())
  {
    // Don't visit a dependent that's already been visited
    ZilchShaderIRType* dependent = range.Front();
    if(finalDependents.Contains(dependent))
      continue;

    // Mark that we've visited this dependent and get all of its dependents recursively
    finalDependents.Insert(dependent);
    GetAllDependents(dependent, finalDependents);
  }
}

TypeResolvers* ZilchShaderIRLibrary::FindTypeResolver(Zilch::Type* zilchType, bool checkDependencies)
{
  TypeResolvers* result = mTypeResolvers.FindPointer(zilchType);
  if(result != nullptr)
    return result;
  
  // If we failed to find the type but we don't check dependencies then return that we can't find it
  if(!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if(mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindTypeResolver(zilchType, checkDependencies);
}

BinaryOpResolverIRFn ZilchShaderIRLibrary::FindOperatorResolver(BinaryOperatorKey& opId, bool checkDependencies)
{
  return FindOperatorResolverTemplate<BinaryOperatorKey, BinaryOpResolverIRFn>(opId, checkDependencies);
}

UnaryOpResolverIRFn ZilchShaderIRLibrary::FindOperatorResolver(UnaryOperatorKey& opId, bool checkDependencies)
{
  return FindOperatorResolverTemplate<UnaryOperatorKey, UnaryOpResolverIRFn>(opId, checkDependencies);
}

TypeCastResolverIRFn ZilchShaderIRLibrary::FindOperatorResolver(TypeCastKey& opId, bool checkDependencies)
{
  return FindOperatorResolverTemplate<TypeCastKey, TypeCastResolverIRFn>(opId, checkDependencies);
}

MemberAccessResolverIRFn ZilchShaderIRLibrary::FindFieldResolver(Zilch::Type* zilchType, Zilch::Field* zilchField, bool checkDependencies)
{
  TypeResolvers* typeResolver = FindTypeResolver(zilchType, checkDependencies);
  if(typeResolver != nullptr)
    return typeResolver->FindFieldResolver(zilchField);
  return nullptr;
}

MemberFunctionResolverIRFn ZilchShaderIRLibrary::FindFunctionResolver(Zilch::Type* zilchType, Zilch::Function* zilchFunction, bool checkDependencies)
{
  TypeResolvers* typeResolver = FindTypeResolver(zilchType, checkDependencies);
  if(typeResolver != nullptr)
    return typeResolver->FindFunctionResolver(zilchFunction);
  return nullptr;
}

MemberPropertySetterResolverIRFn ZilchShaderIRLibrary::FindSetterResolver(Zilch::Type* zilchType, Zilch::Function* zilchFunction, bool checkDependencies)
{
  TypeResolvers* typeResolver = FindTypeResolver(zilchType, checkDependencies);
  if(typeResolver != nullptr)
    return typeResolver->FindSetterResolver(zilchFunction);
  return nullptr;
}

ConstructorCallResolverIRFn ZilchShaderIRLibrary::FindConstructorResolver(Zilch::Type* zilchType, Zilch::Function* zilchFunction, bool checkDependencies)
{
  TypeResolvers* typeResolver = FindTypeResolver(zilchType, checkDependencies);
  if(typeResolver != nullptr)
    return typeResolver->FindConstructorResolver(zilchFunction);
  return nullptr;
}

ZilchShaderIRFunction* ZilchShaderIRLibrary::FindFunction(Zilch::Function* zilchFunction, bool checkDependencies)
{
  // Try to find the type in this library
  ZilchShaderIRFunction* irFunction = mFunctions.FindValue(zilchFunction, nullptr);
  if(irFunction != nullptr)
    return irFunction;

  // If we failed to find the type but we don't check dependencies then return that we can't find it
  if(!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if(mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindFunction(zilchFunction, checkDependencies);
}

SpirVExtensionInstruction* ZilchShaderIRLibrary::FindExtensionInstruction(Zilch::Function* zilchFunction, bool checkDependencies)
{
  SpirVExtensionInstruction* result = mExtensionInstructions.FindValue(zilchFunction, nullptr);
  if(result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return that we can't find it
  if(!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if(mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindExtensionInstruction(zilchFunction, checkDependencies);
}

ZilchShaderExtensionImport* ZilchShaderIRLibrary::FindExtensionLibraryImport(SpirVExtensionLibrary* extensionLibrary, bool checkDependencies)
{
  ZilchShaderExtensionImport* result = mExtensionLibraryImports.FindValue(extensionLibrary, nullptr);
  if(result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return that we can't find it
  if(!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if(mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindExtensionLibraryImport(extensionLibrary, checkDependencies);
}

ZilchShaderIRConstantLiteral* ZilchShaderIRLibrary::FindConstantLiteral(Zilch::Any& literalValue, bool checkDependencies)
{
  // Try to find the type in this library
  ZilchShaderIRConstantLiteral* result = mConstantLiterals.FindValue(literalValue, nullptr);
  if(result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return that we can't find it
  if(!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if(mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindConstantLiteral(literalValue, checkDependencies);
}

ZilchShaderIROp* ZilchShaderIRLibrary::FindConstantOp(ConstantOpKeyType& key, bool checkDependencies)
{
  // Try to find the type in this library
  ZilchShaderIROp* result = mConstantOps.FindValue(key, nullptr);
  if(result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return that we can't find it
  if(!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if(mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindConstantOp(key, checkDependencies);
}

ZilchShaderIROp* ZilchShaderIRLibrary::FindEnumConstantOp(void* key, bool checkDependencies)
{
  // Try to find the type in this library
  ZilchShaderIROp* result = mEnumContants.FindValue(key, nullptr);
  if(result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return that we can't find it
  if(!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if(mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindEnumConstantOp(key, checkDependencies);
}

ZilchShaderIROp* ZilchShaderIRLibrary::FindSpecializationConstantOp(void* key, bool checkDependencies)
{
  // Try to find the type in this library
  ZilchShaderIROp* result = mSpecializationConstantMap.FindValue(key, nullptr);
  if(result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return that we can't find it
  if(!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if(mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindSpecializationConstantOp(key, checkDependencies);
}

template <typename OpIdType, typename OpResolverType>
OpResolverType ZilchShaderIRLibrary::FindOperatorResolverTemplate(OpIdType& opId, bool checkDependencies)
{
  OpResolverType result = mOperatorResolvers.FindOpResolver(opId);
  if(result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return that we can't find it
  if(!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if(mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindOperatorResolverTemplate<OpIdType, OpResolverType>(opId, checkDependencies);
}

}//public Zero
