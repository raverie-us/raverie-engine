// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

//-------------------------------------------------------------------FragmentSharedKey
FragmentSharedKey::FragmentSharedKey()
{
  mStorageClass = spv::StorageClassFunction;
  mFieldType = nullptr;
}

FragmentSharedKey::FragmentSharedKey(spv::StorageClass storageClass, RaverieShaderIRType* fieldType, StringParam varName)
{
  mStorageClass = storageClass;
  mFieldType = fieldType;
  mVariableName = varName;
}

size_t FragmentSharedKey::Hash() const
{
  size_t hash = 0;
  HashCombine(hash, (int)mStorageClass);
  HashCombine(hash, mFieldType);
  HashCombine(hash, mVariableName);
  return hash;
}

bool FragmentSharedKey::operator==(const FragmentSharedKey& rhs) const
{
  bool isEqual =
      (mStorageClass == rhs.mStorageClass) && (mFieldType == rhs.mFieldType) && (mVariableName == rhs.mVariableName);
  return isEqual;
}

template <typename T>
void FragmentSharedKey::HashCombine(size_t& seed, const T& value) const
{
  // Code from boost
  // Reciprocal of the golden ratio helps spread entropy
  //     and handles duplicates.
  // See Mike Seymour in magic-numbers-in-boosthash-combine:
  //     https://stackoverflow.com/questions/4948780
  seed ^= HashPolicy<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

TypeResolvers::TypeResolvers()
{
  mBackupFieldResolver = nullptr;
  mDefaultConstructorResolver = nullptr;
  mBackupConstructorResolver = nullptr;
  mBackupSetterResolver = nullptr;
  mExpressionInitializerListResolver = nullptr;
}

void TypeResolvers::RegisterFieldResolver(Raverie::Field* field, MemberAccessResolverIRFn fieldResolver)
{
  ErrorIf(field == nullptr, "Cannot register a field resolver on a null field");
  mFieldResolvers.InsertOrError(field, fieldResolver);
}

void TypeResolvers::RegisterBackupFieldResolver(MemberAccessResolverIRFn backupResolver)
{
  ErrorIf(mBackupFieldResolver != nullptr, "Backup field resolver cannot be registered twice.");
  mBackupFieldResolver = backupResolver;
}

MemberAccessResolverIRFn TypeResolvers::FindFieldResolver(Raverie::Field* field)
{
  MemberAccessResolverIRFn resolver = mFieldResolvers.FindValue(field, nullptr);
  if (resolver == nullptr)
    resolver = mBackupFieldResolver;
  return resolver;
}

void TypeResolvers::RegisterConstructorResolver(Raverie::Function* raverieFunction, ConstructorCallResolverIRFn resolverFn)
{
  ErrorIf(raverieFunction == nullptr, "Trying to register a null function resolver");
  ErrorIf(mConstructorResolvers.ContainsKey(raverieFunction), "Constructor resolver was already registered");
  mConstructorResolvers[raverieFunction] = resolverFn;
}

ConstructorCallResolverIRFn TypeResolvers::FindConstructorResolver(Raverie::Function* raverieFunction)
{
  ConstructorCallResolverIRFn resolver = mConstructorResolvers.FindValue(raverieFunction, nullptr);
  if (resolver != nullptr)
    return resolver;
  return mBackupConstructorResolver;
}

void TypeResolvers::RegisterFunctionResolver(Raverie::Function* function, MemberFunctionResolverIRFn functionResolver)
{
  ErrorIf(function == nullptr, "Cannot register a function resolver on a null function");
  mFunctionResolvers.InsertOrError(function, functionResolver);
}

MemberFunctionResolverIRFn TypeResolvers::FindFunctionResolver(Raverie::Function* function)
{
  return mFunctionResolvers.FindValue(function, nullptr);
}

void TypeResolvers::RegisterSetterResolver(Raverie::Function* function, MemberPropertySetterResolverIRFn functionResolver)
{
  ErrorIf(function == nullptr, "Cannot register a function resolver on a null function");
  mSetterResolvers.InsertOrError(function, functionResolver);
}

void TypeResolvers::RegisterBackupSetterResolver(MemberPropertySetterResolverIRFn backupResolver)
{
  mBackupSetterResolver = backupResolver;
}

MemberPropertySetterResolverIRFn TypeResolvers::FindSetterResolver(Raverie::Function* function)
{
  MemberPropertySetterResolverIRFn resolver = mSetterResolvers.FindValue(function, nullptr);
  if (resolver == nullptr)
    resolver = mBackupSetterResolver;
  return resolver;
}

void OperatorResolvers::RegisterBinaryOpResolver(Raverie::Type* lhsType,
                                                 Raverie::Type* rhsType,
                                                 Raverie::Grammar::Enum op,
                                                 BinaryOpResolverIRFn resolver)
{
  BinaryOperatorKey opId(lhsType, rhsType, op);
  mBinaryOpResolvers.InsertOrError(opId, resolver);
}

BinaryOpResolverIRFn OperatorResolvers::FindOpResolver(BinaryOperatorKey& opId)
{
  return mBinaryOpResolvers.FindValue(opId, nullptr);
}

void OperatorResolvers::RegisterUnaryOpResolver(Raverie::Type* type,
                                                Raverie::Grammar::Enum op,
                                                UnaryOpResolverIRFn resolver)
{
  UnaryOperatorKey opId(type, op);
  mUnaryOpResolvers.InsertOrError(opId, resolver);
}

UnaryOpResolverIRFn OperatorResolvers::FindOpResolver(UnaryOperatorKey& opId)
{
  return mUnaryOpResolvers.FindValue(opId, nullptr);
}

void OperatorResolvers::RegisterTypeCastOpResolver(Raverie::Type* fromType,
                                                   Raverie::Type* toType,
                                                   TypeCastResolverIRFn resolver)
{
  TypeCastKey opId(fromType, toType);
  mTypeCastResolvers.InsertOrError(opId, resolver);
}

TypeCastResolverIRFn OperatorResolvers::FindOpResolver(TypeCastKey& opId)
{
  return mTypeCastResolvers.FindValue(opId, nullptr);
}

GlobalVariableData::GlobalVariableData()
{
  mInstance = nullptr;
  mInitializerFunction = nullptr;
}

GlobalVariableData::~GlobalVariableData()
{
  delete mInstance;
}

StageRequirementsData::StageRequirementsData()
{
  mRequiredStages = (ShaderStage::Enum)ShaderStage::None;
  mDependency = nullptr;
}

void StageRequirementsData::Combine(Raverie::Member* dependency,
                                    const Raverie::CodeLocation& location,
                                    ShaderStage::Enum requiredStage)
{
  // Only set the dependency and call location on the first non-empty stage
  // requirement.
  if (mDependency == nullptr && requiredStage != ShaderStage::None)
  {
    mDependency = dependency;
    mCallLocation = location;
  }
  mRequiredStages |= requiredStage;
}

RaverieShaderIRType* RaverieShaderIRModule::FindType(const String& typeName, bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for (size_t i = 0; i < Size(); ++i)
  {
    RaverieShaderIRLibrary* library = (*this)[i];
    RaverieShaderIRType* type = library->FindType(typeName, checkDependencies);
    if (type != nullptr)
      return type;
  }
  return nullptr;
}

GlobalVariableData* RaverieShaderIRModule::FindGlobalVariable(Raverie::Field* raverieField, bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for (size_t i = 0; i < Size(); ++i)
  {
    RaverieShaderIRLibrary* library = (*this)[i];
    GlobalVariableData* result = library->FindGlobalVariable(raverieField, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

GlobalVariableData* RaverieShaderIRModule::FindGlobalVariable(RaverieShaderIROp* globalInstance, bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for (size_t i = 0; i < Size(); ++i)
  {
    RaverieShaderIRLibrary* library = (*this)[i];
    GlobalVariableData* result = library->FindGlobalVariable(globalInstance, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

GlobalVariableData* RaverieShaderIRModule::FindFragmentSharedVariable(const FragmentSharedKey& key,
                                                                    bool checkDependencies)
{
  // Check each library, if any library finds the shared variable then return it
  for (size_t i = 0; i < Size(); ++i)
  {
    RaverieShaderIRLibrary* library = (*this)[i];
    GlobalVariableData* result = library->FindFragmentSharedVariable(key, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

TemplateTypeIRResloverFn RaverieShaderIRModule::FindTemplateResolver(const TemplateTypeKey& templateKey,
                                                                   bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for (size_t i = 0; i < Size(); ++i)
  {
    RaverieShaderIRLibrary* library = (*this)[i];
    TemplateTypeIRResloverFn resolver = library->FindTemplateResolver(templateKey, checkDependencies);
    if (resolver != nullptr)
      return resolver;
  }
  return nullptr;
}

TypeResolvers* RaverieShaderIRModule::FindTypeResolver(Raverie::Type* raverieType, bool checkDependencies)
{
  for (size_t i = 0; i < Size(); ++i)
  {
    RaverieShaderIRLibrary* library = (*this)[i];
    TypeResolvers* result = library->FindTypeResolver(raverieType, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

RaverieShaderIRFunction* RaverieShaderIRModule::FindFunction(Raverie::Function* raverieFunction, bool checkDependencies)
{
  // Check each library, if any library finds the type then return that type
  for (size_t i = 0; i < Size(); ++i)
  {
    RaverieShaderIRLibrary* library = (*this)[i];
    RaverieShaderIRFunction* irFunction = library->FindFunction(raverieFunction, checkDependencies);
    if (irFunction != nullptr)
      return irFunction;
  }
  return nullptr;
}

SpirVExtensionInstruction* RaverieShaderIRModule::FindExtensionInstruction(Raverie::Function* raverieFunction,
                                                                         bool checkDependencies)
{
  for (size_t i = 0; i < Size(); ++i)
  {
    RaverieShaderIRLibrary* library = (*this)[i];
    SpirVExtensionInstruction* result = library->FindExtensionInstruction(raverieFunction, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

RaverieShaderExtensionImport* RaverieShaderIRModule::FindExtensionLibraryImport(SpirVExtensionLibrary* extensionLibrary,
                                                                            bool checkDependencies)
{
  for (size_t i = 0; i < Size(); ++i)
  {
    RaverieShaderIRLibrary* library = (*this)[i];
    RaverieShaderExtensionImport* result = library->FindExtensionLibraryImport(extensionLibrary, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

RaverieShaderIRConstantLiteral* RaverieShaderIRModule::FindConstantLiteral(Raverie::Any& literalValue, bool checkDependencies)
{
  for (size_t i = 0; i < Size(); ++i)
  {
    RaverieShaderIRLibrary* library = (*this)[i];
    RaverieShaderIRConstantLiteral* result = library->FindConstantLiteral(literalValue, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

RaverieShaderIROp* RaverieShaderIRModule::FindConstantOp(ConstantOpKeyType& key, bool checkDependencies)
{
  for (size_t i = 0; i < Size(); ++i)
  {
    RaverieShaderIRLibrary* library = (*this)[i];
    RaverieShaderIROp* result = library->FindConstantOp(key, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

RaverieShaderIROp* RaverieShaderIRModule::FindEnumConstantOp(void* key, bool checkDependencies)
{
  for (size_t i = 0; i < Size(); ++i)
  {
    RaverieShaderIRLibrary* library = (*this)[i];
    RaverieShaderIROp* result = library->FindEnumConstantOp(key, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

RaverieShaderIROp* RaverieShaderIRModule::FindSpecializationConstantOp(void* key, bool checkDependencies)
{
  for (size_t i = 0; i < Size(); ++i)
  {
    RaverieShaderIRLibrary* library = (*this)[i];
    RaverieShaderIROp* result = library->FindSpecializationConstantOp(key, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

template <typename OpIdType, typename OpResolverType>
OpResolverType RaverieShaderIRModule::FindOperatorResolverTemplate(OpIdType& opId, bool checkDependencies)
{
  for (size_t i = 0; i < Size(); ++i)
  {
    RaverieShaderIRLibrary* library = (*this)[i];
    OpResolverType result = library->FindOperatorResolver(opId, checkDependencies);
    if (result != nullptr)
      return result;
  }
  return nullptr;
}

RaverieShaderIRLibrary::RaverieShaderIRLibrary()
{
  mTranslated = false;
}

RaverieShaderIRLibrary::~RaverieShaderIRLibrary()
{
  DeleteObjectsIn(mOwnedTypes.All());
  DeleteObjectsIn(mOwnedFunctions.All());
  DeleteObjectsIn(mOwnedFieldMeta.All());
  DeleteObjectsIn(mOwnedFunctionMeta.All());
  DeleteObjectsIn(mOwnedTypeMeta.All());
  DeleteObjectsIn(mOwnedSpecializationConstants.All());
  DeleteObjectsIn(mOwnedGlobals.All());

  DeleteObjectsIn(mExtensionInstructions.Values());
  DeleteObjectsIn(mExtensionLibraries.All());
  DeleteObjectsIn(mConstantLiterals.Values());
  DeleteObjectsIn(mConstantOps.Values());
  DeleteObjectsIn(mExtensionLibraryImports.Values());
}

void RaverieShaderIRLibrary::AddType(StringParam typeName, RaverieShaderIRType* shaderType)
{
  mTypes.InsertOrError(typeName, shaderType);
  mOwnedTypes.PushBack(shaderType);
}

RaverieShaderIRType* RaverieShaderIRLibrary::FindType(const String& typeName, bool checkDependencies)
{
  // Try to find the type in this library
  RaverieShaderIRType* type = mTypes.FindValue(typeName, nullptr);
  if (type != nullptr)
    return type;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindType(typeName);
}

RaverieShaderIRType* RaverieShaderIRLibrary::FindType(Raverie::Type* raverieType, bool checkDependencies)
{
  Raverie::BoundType* boundType = Raverie::BoundType::GetBoundType(raverieType);
  return FindType(boundType->Name, checkDependencies);
}

GlobalVariableData* RaverieShaderIRLibrary::FindGlobalVariable(Raverie::Field* raverieField, bool checkDependencies)
{
  GlobalVariableData* result = mRaverieFieldToGlobalVariable.FindValue(raverieField, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindGlobalVariable(raverieField, checkDependencies);
}

GlobalVariableData* RaverieShaderIRLibrary::FindGlobalVariable(RaverieShaderIROp* globalInstance, bool checkDependencies)
{
  GlobalVariableData* result = mVariableOpLookupMap.FindValue(globalInstance, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindGlobalVariable(globalInstance, checkDependencies);
}

GlobalVariableData* RaverieShaderIRLibrary::FindFragmentSharedVariable(const FragmentSharedKey& key,
                                                                     bool checkDependencies)
{
  GlobalVariableData* result = mFragmentSharedGlobalVariables.FindValue(key, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the variable but we don't check dependencies then return that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindFragmentSharedVariable(key, checkDependencies);
}

void RaverieShaderIRLibrary::RegisterTemplateResolver(const TemplateTypeKey& templateKey,
                                                    TemplateTypeIRResloverFn resolver)
{
  mTemplateResolvers.InsertOrError(templateKey, resolver);
}

TemplateTypeIRResloverFn RaverieShaderIRLibrary::FindTemplateResolver(const TemplateTypeKey& templateKey,
                                                                    bool checkDependencies)
{
  // Try to find the type in this library
  TemplateTypeIRResloverFn resolver = mTemplateResolvers.FindValue(templateKey, nullptr);
  if (resolver != nullptr)
    return resolver;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindTemplateResolver(templateKey);
}

void RaverieShaderIRLibrary::FlattenModuleDependents()
{
  // Bring all dependents from our dependency modules into ourself
  RaverieShaderIRModule* module = mDependencies;
  for (size_t i = 0; i < module->Size(); ++i)
  {
    RaverieShaderIRLibrary* parentLibrary = (*module)[i];

    // For each type in the parent library's reverse dependencies, copy all
    // dependents into the current library
    AutoDeclare(pairRange, parentLibrary->mTypeDependents.All());
    for (; !pairRange.Empty(); pairRange.PopFront())
    {
      AutoDeclare(pair, pairRange.Front());
      HashSet<RaverieShaderIRType*>& parentLibraryDependents = pair.second;
      HashSet<RaverieShaderIRType*>& currentLibraryDependents = mTypeDependents[pair.first];

      // Copy all dependents into the current library
      HashSet<RaverieShaderIRType*>::range dependentRange = parentLibraryDependents.All();
      for (; !dependentRange.Empty(); dependentRange.PopFront())
        currentLibraryDependents.Insert(dependentRange.Front());
    }
  }
}

void RaverieShaderIRLibrary::GetAllDependents(RaverieShaderIRType* shaderType, HashSet<RaverieShaderIRType*>& finalDependents)
{
  // Find if the current shader type has any dependents (things that depend
  // on it). If it doesn't then there's nothing more to do.
  HashSet<RaverieShaderIRType*>* currentTypeDependents = mTypeDependents.FindPointer(shaderType);
  if (currentTypeDependents == nullptr)
    return;

  // Iterate over all dependent types
  HashSet<RaverieShaderIRType*>::range range = currentTypeDependents->All();
  for (; !range.Empty(); range.PopFront())
  {
    // Don't visit a dependent that's already been visited
    RaverieShaderIRType* dependent = range.Front();
    if (finalDependents.Contains(dependent))
      continue;

    // Mark that we've visited this dependent and get all of its dependents
    // recursively
    finalDependents.Insert(dependent);
    GetAllDependents(dependent, finalDependents);
  }
}

TypeResolvers* RaverieShaderIRLibrary::FindTypeResolver(Raverie::Type* raverieType, bool checkDependencies)
{
  TypeResolvers* result = mTypeResolvers.FindPointer(raverieType);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindTypeResolver(raverieType, checkDependencies);
}

BinaryOpResolverIRFn RaverieShaderIRLibrary::FindOperatorResolver(BinaryOperatorKey& opId, bool checkDependencies)
{
  return FindOperatorResolverTemplate<BinaryOperatorKey, BinaryOpResolverIRFn>(opId, checkDependencies);
}

UnaryOpResolverIRFn RaverieShaderIRLibrary::FindOperatorResolver(UnaryOperatorKey& opId, bool checkDependencies)
{
  return FindOperatorResolverTemplate<UnaryOperatorKey, UnaryOpResolverIRFn>(opId, checkDependencies);
}

TypeCastResolverIRFn RaverieShaderIRLibrary::FindOperatorResolver(TypeCastKey& opId, bool checkDependencies)
{
  return FindOperatorResolverTemplate<TypeCastKey, TypeCastResolverIRFn>(opId, checkDependencies);
}

MemberAccessResolverIRFn RaverieShaderIRLibrary::FindFieldResolver(Raverie::Type* raverieType,
                                                                 Raverie::Field* raverieField,
                                                                 bool checkDependencies)
{
  TypeResolvers* typeResolver = FindTypeResolver(raverieType, checkDependencies);
  if (typeResolver != nullptr)
    return typeResolver->FindFieldResolver(raverieField);
  return nullptr;
}

MemberFunctionResolverIRFn RaverieShaderIRLibrary::FindFunctionResolver(Raverie::Type* raverieType,
                                                                      Raverie::Function* raverieFunction,
                                                                      bool checkDependencies)
{
  TypeResolvers* typeResolver = FindTypeResolver(raverieType, checkDependencies);
  if (typeResolver != nullptr)
    return typeResolver->FindFunctionResolver(raverieFunction);
  return nullptr;
}

MemberPropertySetterResolverIRFn RaverieShaderIRLibrary::FindSetterResolver(Raverie::Type* raverieType,
                                                                          Raverie::Function* raverieFunction,
                                                                          bool checkDependencies)
{
  TypeResolvers* typeResolver = FindTypeResolver(raverieType, checkDependencies);
  if (typeResolver != nullptr)
    return typeResolver->FindSetterResolver(raverieFunction);
  return nullptr;
}

ConstructorCallResolverIRFn RaverieShaderIRLibrary::FindConstructorResolver(Raverie::Type* raverieType,
                                                                          Raverie::Function* raverieFunction,
                                                                          bool checkDependencies)
{
  TypeResolvers* typeResolver = FindTypeResolver(raverieType, checkDependencies);
  if (typeResolver != nullptr)
    return typeResolver->FindConstructorResolver(raverieFunction);
  return nullptr;
}

RaverieShaderIRFunction* RaverieShaderIRLibrary::FindFunction(Raverie::Function* raverieFunction, bool checkDependencies)
{
  // Try to find the type in this library
  RaverieShaderIRFunction* irFunction = mFunctions.FindValue(raverieFunction, nullptr);
  if (irFunction != nullptr)
    return irFunction;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindFunction(raverieFunction, checkDependencies);
}

SpirVExtensionInstruction* RaverieShaderIRLibrary::FindExtensionInstruction(Raverie::Function* raverieFunction,
                                                                          bool checkDependencies)
{
  SpirVExtensionInstruction* result = mExtensionInstructions.FindValue(raverieFunction, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindExtensionInstruction(raverieFunction, checkDependencies);
}

RaverieShaderExtensionImport* RaverieShaderIRLibrary::FindExtensionLibraryImport(SpirVExtensionLibrary* extensionLibrary,
                                                                             bool checkDependencies)
{
  RaverieShaderExtensionImport* result = mExtensionLibraryImports.FindValue(extensionLibrary, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindExtensionLibraryImport(extensionLibrary, checkDependencies);
}

RaverieShaderIRConstantLiteral* RaverieShaderIRLibrary::FindConstantLiteral(Raverie::Any& literalValue,
                                                                        bool checkDependencies)
{
  // Try to find the type in this library
  RaverieShaderIRConstantLiteral* result = mConstantLiterals.FindValue(literalValue, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindConstantLiteral(literalValue, checkDependencies);
}

RaverieShaderIROp* RaverieShaderIRLibrary::FindConstantOp(ConstantOpKeyType& key, bool checkDependencies)
{
  // Try to find the type in this library
  RaverieShaderIROp* result = mConstantOps.FindValue(key, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindConstantOp(key, checkDependencies);
}

RaverieShaderIROp* RaverieShaderIRLibrary::FindEnumConstantOp(void* key, bool checkDependencies)
{
  // Try to find the type in this library
  RaverieShaderIROp* result = mEnumContants.FindValue(key, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindEnumConstantOp(key, checkDependencies);
}

RaverieShaderIROp* RaverieShaderIRLibrary::FindSpecializationConstantOp(void* key, bool checkDependencies)
{
  // Try to find the type in this library
  RaverieShaderIROp* result = mSpecializationConstantMap.FindValue(key, nullptr);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindSpecializationConstantOp(key, checkDependencies);
}

template <typename OpIdType, typename OpResolverType>
OpResolverType RaverieShaderIRLibrary::FindOperatorResolverTemplate(OpIdType& opId, bool checkDependencies)
{
  OpResolverType result = mOperatorResolvers.FindOpResolver(opId);
  if (result != nullptr)
    return result;

  // If we failed to find the type but we don't check dependencies then return
  // that we can't find it
  if (!checkDependencies)
    return nullptr;

  // Otherwise check all of our dependencies (if we have any)
  if (mDependencies == nullptr)
    return nullptr;
  return mDependencies->FindOperatorResolverTemplate<OpIdType, OpResolverType>(opId, checkDependencies);
}

String GetOverloadedName(StringParam functionName, Raverie::Function* raverieFunction)
{
  Raverie::GuidType thisHash = raverieFunction->This ? raverieFunction->This->ResultType->Hash() : 0;
  return BuildString(functionName, ToString(raverieFunction->FunctionType->Hash() ^ thisHash));
}

} // namespace Raverie
