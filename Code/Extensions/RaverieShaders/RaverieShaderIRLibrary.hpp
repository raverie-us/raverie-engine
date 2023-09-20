// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class RaverieShaderIRLibrary;
class RaverieShaderIRModule;
typedef Raverie::Ref<RaverieShaderIRLibrary> RaverieShaderIRLibraryRef;
typedef Raverie::Ref<RaverieShaderIRModule> RaverieShaderIRModuleRef;

class RaverieSpirVFrontEnd;
class RaverieSpirVFrontEndContext;

typedef Pair<RaverieShaderIRType*, Raverie::Any> ConstantOpKeyType;

typedef void (*DefaultConstructorResolverFn)(RaverieSpirVFrontEnd* translator,
                                             Raverie::Type* resultType,
                                             RaverieSpirVFrontEndContext* context);
typedef void (*ConstructorCallResolverIRFn)(RaverieSpirVFrontEnd* translator,
                                            Raverie::FunctionCallNode* fnCallNode,
                                            Raverie::StaticTypeNode* staticTypeNode,
                                            RaverieSpirVFrontEndContext* context);
typedef void (*MemberAccessResolverIRFn)(RaverieSpirVFrontEnd* translator,
                                         Raverie::MemberAccessNode* memberAccessNode,
                                         RaverieSpirVFrontEndContext* context);
typedef void (*MemberFunctionResolverIRFn)(RaverieSpirVFrontEnd* translator,
                                           Raverie::FunctionCallNode* functionCallNode,
                                           Raverie::MemberAccessNode* memberAccessNode,
                                           RaverieSpirVFrontEndContext* context);
typedef void (*MemberPropertySetterResolverIRFn)(RaverieSpirVFrontEnd* translator,
                                                 Raverie::MemberAccessNode* memberAccessNode,
                                                 RaverieShaderIROp* resultValue,
                                                 RaverieSpirVFrontEndContext* context);
typedef void (*BinaryOpResolverIRFn)(RaverieSpirVFrontEnd* translator,
                                     Raverie::BinaryOperatorNode* binaryOpNode,
                                     RaverieSpirVFrontEndContext* context);
typedef void (*UnaryOpResolverIRFn)(RaverieSpirVFrontEnd* translator,
                                    Raverie::UnaryOperatorNode* binaryOpNode,
                                    RaverieSpirVFrontEndContext* context);
typedef void (*TypeCastResolverIRFn)(RaverieSpirVFrontEnd* translator,
                                     Raverie::TypeCastNode* binaryOpNode,
                                     RaverieSpirVFrontEndContext* context);
typedef void (*TemplateTypeIRResloverFn)(RaverieSpirVFrontEnd* translator, Raverie::BoundType* boundType);
typedef void (*ExpressionInitializerIRResolverFn)(RaverieSpirVFrontEnd* translator,
                                                  Raverie::ExpressionInitializerNode*& node,
                                                  RaverieSpirVFrontEndContext* context);

//-------------------------------------------------------------------FragmentSharedKey
/// Hash key used to lookup fields with the FragmentShared attribute. A shared field
/// is uniquely described by its type and name (additionally include any storage
/// class as this is technically part of the type, not sure if this is actually important).
struct FragmentSharedKey
{
  FragmentSharedKey();
  FragmentSharedKey(spv::StorageClass storageClass, RaverieShaderIRType* fieldType, StringParam varName);

  size_t Hash() const;
  bool operator==(const FragmentSharedKey& rhs) const;

  spv::StorageClass mStorageClass;
  RaverieShaderIRType* mFieldType;
  String mVariableName;

private:
  template <typename T>
  void HashCombine(size_t& seed, const T& value) const;
};

/// A collection of member/function/etc... resolvers for specific library
/// translations. Some examples include Math.Dot and samplers.
class TypeResolvers
{
public:
  TypeResolvers();

  /// Register a resolver for a field.
  void RegisterFieldResolver(Raverie::Field* field, MemberAccessResolverIRFn fieldResolver);
  /// Register a resolver to use when a field-specific resolver isn't found.
  /// Used to handle things like swizzles.
  void RegisterBackupFieldResolver(MemberAccessResolverIRFn backupResolver);
  /// Finds the resolver for the given field. Returns the backup resolver if no
  /// match is found.
  MemberAccessResolverIRFn FindFieldResolver(Raverie::Field* field);

  void RegisterConstructorResolver(Raverie::Function* raverieFunction, ConstructorCallResolverIRFn resolverFn);
  ConstructorCallResolverIRFn FindConstructorResolver(Raverie::Function* raverieFunction);

  /// Register a resolver for a function call.
  void RegisterFunctionResolver(Raverie::Function* function, MemberFunctionResolverIRFn functionResolver);
  MemberFunctionResolverIRFn FindFunctionResolver(Raverie::Function* function);

  /// Register a resolver for a setter function call.
  void RegisterSetterResolver(Raverie::Function* function, MemberPropertySetterResolverIRFn functionResolver);
  void RegisterBackupSetterResolver(MemberPropertySetterResolverIRFn backupResolver);
  MemberPropertySetterResolverIRFn FindSetterResolver(Raverie::Function* function);

  HashMap<Raverie::Field*, MemberAccessResolverIRFn> mFieldResolvers;
  MemberAccessResolverIRFn mBackupFieldResolver;

  DefaultConstructorResolverFn mDefaultConstructorResolver;
  ConstructorCallResolverIRFn mBackupConstructorResolver;
  MemberPropertySetterResolverIRFn mBackupSetterResolver;
  /// Library translations for constructors of a type (e.g. Real3 splat
  /// constructor)
  HashMap<Raverie::Function*, ConstructorCallResolverIRFn> mConstructorResolvers;

  HashMap<Raverie::Function*, MemberFunctionResolverIRFn> mFunctionResolvers;
  HashMap<Raverie::Function*, MemberPropertySetterResolverIRFn> mSetterResolvers;
  /// Some types need to override how expression initializers work (e.g. fixed
  /// array).
  ExpressionInitializerIRResolverFn mExpressionInitializerListResolver;
};

/// A collection of operators (unary, binary, type cast) resolvers for a
/// specific library.
class OperatorResolvers
{
public:
  /// Binary operators
  void RegisterBinaryOpResolver(Raverie::Type* lhsType,
                                Raverie::Type* rhsType,
                                Raverie::Grammar::Enum op,
                                BinaryOpResolverIRFn resolver);
  BinaryOpResolverIRFn FindOpResolver(BinaryOperatorKey& opId);

  /// Unary operators
  void RegisterUnaryOpResolver(Raverie::Type* type, Raverie::Grammar::Enum op, UnaryOpResolverIRFn resolver);
  UnaryOpResolverIRFn FindOpResolver(UnaryOperatorKey& opId);

  /// Type Cast operators
  void RegisterTypeCastOpResolver(Raverie::Type* fromType, Raverie::Type* toType, TypeCastResolverIRFn resolver);
  TypeCastResolverIRFn FindOpResolver(TypeCastKey& opId);

private:
  HashMap<BinaryOperatorKey, BinaryOpResolverIRFn> mBinaryOpResolvers;
  HashMap<UnaryOperatorKey, UnaryOpResolverIRFn> mUnaryOpResolvers;
  HashMap<TypeCastKey, TypeCastResolverIRFn> mTypeCastResolvers;
};

/// Data about global variables.
class GlobalVariableData
{
public:
  GlobalVariableData();
  ~GlobalVariableData();

  /// The instance of the global variable. This instance is owned by this class.
  RaverieShaderIROp* mInstance;
  /// A function that initializes this instance. Can be null.
  RaverieShaderIRFunction* mInitializerFunction;
};

/// Used to store if a symbol requires a certain stage (e.g. [RequiresPixel]).
/// If a symbol does have a requirement, this also stores the dependency that
/// causes this requirement as well as the location that references the
/// dependency (e.g. a the function call location).
struct StageRequirementsData
{
  StageRequirementsData();

  /// Merges the given dependency into this object. Only updates the dependency
  /// and location if this is the first time a non-empty stage requirement is
  /// being set.
  void Combine(Raverie::Member* dependency, const Raverie::CodeLocation& location, ShaderStage::Enum requiredStage);

  /// What stage this symbol requires.
  ShaderStage::Enum mRequiredStages;
  /// The first symbol that causes the stage requirement. Used to generate a
  /// call graph on error.
  Raverie::Member* mDependency;
  /// The location that the dependency is called.
  Raverie::CodeLocation mCallLocation;
};

/// A module represents a collection of libraries, typically used to
/// express what dependencies another library has. All libraries in this
/// module should be fully compiled (and hence locked) if they're in a module.
class RaverieShaderIRModule : public Array<RaverieShaderIRLibraryRef>
{
public:
  /// Find a type in any of the contained libraries
  RaverieShaderIRType* FindType(const String& typeName, bool checkDependencies = true);

  /// Find the global variable data associate with the given raverie field.
  GlobalVariableData* FindGlobalVariable(Raverie::Field* raverieField, bool checkDependencies = true);
  /// Find the global variable data associate with the given instance variable
  /// op.
  GlobalVariableData* FindGlobalVariable(RaverieShaderIROp* globalInstance, bool checkDependencies = true);

  /// Find the global variable data associated with a fragment shared variable.
  GlobalVariableData* FindFragmentSharedVariable(const FragmentSharedKey& key, bool checkDependencies = true);

  /// Find a resolver for a template type
  TemplateTypeIRResloverFn FindTemplateResolver(const TemplateTypeKey& templateKey, bool checkDependencies = true);

  TypeResolvers* FindTypeResolver(Raverie::Type* raverieType, bool checkDependencies = true);

  // Constructor library replacements
  RaverieShaderIRFunction* FindFunction(Raverie::Function* raverieFunction, bool checkDependencies = true);
  SpirVExtensionInstruction* FindExtensionInstruction(Raverie::Function* raverieFunction, bool checkDependencies = true);
  RaverieShaderExtensionImport* FindExtensionLibraryImport(SpirVExtensionLibrary* extensionLibrary,
                                                         bool checkDependencies = true);

  RaverieShaderIRConstantLiteral* FindConstantLiteral(Raverie::Any& literalValue, bool checkDependencies = true);
  RaverieShaderIROp* FindConstantOp(ConstantOpKeyType& key, bool checkDependencies = true);
  RaverieShaderIROp* FindEnumConstantOp(void* key, bool checkDependencies = true);
  RaverieShaderIROp* FindSpecializationConstantOp(void* key, bool checkDependencies = true);

  // An intrusive reference count for memory handling
  RaverieRefLink(RaverieShaderIRModule);

private:
  friend RaverieShaderIRLibrary;

  // Helper template to make finding operators easier
  template <typename OpIdType, typename OpResolverType>
  OpResolverType FindOperatorResolverTemplate(OpIdType& opId, bool checkDependencies);
};

/// A library built during shader translation. Mostly an internal type that
/// stores all necessary lookup information to build and generate a spir-v
/// shader. Also contains what types were created during translation.
class RaverieShaderIRLibrary
{
public:
  RaverieShaderIRLibrary();
  ~RaverieShaderIRLibrary();

  void AddType(StringParam typeName, RaverieShaderIRType* shaderType);
  RaverieShaderIRType* FindType(const String& typeName, bool checkDependencies = true);
  RaverieShaderIRType* FindType(Raverie::Type* raverieType, bool checkDependencies = true);

  /// Find the global variable data associate with the given raverie field.
  GlobalVariableData* FindGlobalVariable(Raverie::Field* raverieField, bool checkDependencies = true);
  /// Find the global variable data associate with the given instance variable
  /// op.
  GlobalVariableData* FindGlobalVariable(RaverieShaderIROp* globalInstance, bool checkDependencies = true);

  /// Find the global variable data associated with a fragment shared variable.
  GlobalVariableData* FindFragmentSharedVariable(const FragmentSharedKey& key, bool checkDependencies = true);

  // Resolvers for template types (e.g. FixedArray)
  void RegisterTemplateResolver(const TemplateTypeKey& templateKey, TemplateTypeIRResloverFn resolver);
  TemplateTypeIRResloverFn FindTemplateResolver(const TemplateTypeKey& templateKey, bool checkDependencies = true);

  /// Pulls all reverse dependencies from all the dependent modules into this
  /// library (flattens the list)
  void FlattenModuleDependents();
  /// Fills out a list of all types that depend on the given type
  void GetAllDependents(RaverieShaderIRType* shaderType, HashSet<RaverieShaderIRType*>& finalDependents);

  // Library replacements on a type
  TypeResolvers* FindTypeResolver(Raverie::Type* raverieType, bool checkDependencies = true);

  // Operator resolvers
  BinaryOpResolverIRFn FindOperatorResolver(BinaryOperatorKey& opId, bool checkDependencies = true);
  UnaryOpResolverIRFn FindOperatorResolver(UnaryOperatorKey& opId, bool checkDependencies = true);
  TypeCastResolverIRFn FindOperatorResolver(TypeCastKey& opId, bool checkDependencies = true);

  MemberAccessResolverIRFn FindFieldResolver(Raverie::Type* raverieType,
                                             Raverie::Field* raverieField,
                                             bool checkDependencies = true);
  MemberFunctionResolverIRFn FindFunctionResolver(Raverie::Type* raverieType,
                                                  Raverie::Function* raverieFunction,
                                                  bool checkDependencies = true);
  MemberPropertySetterResolverIRFn FindSetterResolver(Raverie::Type* raverieType,
                                                      Raverie::Function* raverieFunction,
                                                      bool checkDependencies = true);
  ConstructorCallResolverIRFn FindConstructorResolver(Raverie::Type* raverieType,
                                                      Raverie::Function* raverieFunction,
                                                      bool checkDependencies = true);

  RaverieShaderIRFunction* FindFunction(Raverie::Function* raverieFunction, bool checkDependencies = true);
  SpirVExtensionInstruction* FindExtensionInstruction(Raverie::Function* raverieFunction, bool checkDependencies = true);
  RaverieShaderExtensionImport* FindExtensionLibraryImport(SpirVExtensionLibrary* extensionLibrary,
                                                         bool checkDependencies = true);

  RaverieShaderIRConstantLiteral* FindConstantLiteral(Raverie::Any& literalValue, bool checkDependencies = true);

  RaverieShaderIROp* FindConstantOp(ConstantOpKeyType& key, bool checkDependencies = true);
  RaverieShaderIROp* FindEnumConstantOp(void* key, bool checkDependencies = true);
  /// Finds a specialization constant given a key. The key is normally a raverie
  /// field, but can also be special global keys (like languageId).
  RaverieShaderIROp* FindSpecializationConstantOp(void* key, bool checkDependencies = true);

  Raverie::LibraryRef mRaverieLibrary;
  RaverieShaderIRModuleRef mDependencies;
  // An intrusive reference count for memory handling
  RaverieRefLink(RaverieShaderIRLibrary);

  // Data owned by this library. Needed for memory management.
  PodBlockArray<RaverieShaderIRType*> mOwnedTypes;
  PodBlockArray<RaverieShaderIRFunction*> mOwnedFunctions;
  PodBlockArray<ShaderIRFieldMeta*> mOwnedFieldMeta;
  PodBlockArray<ShaderIRFunctionMeta*> mOwnedFunctionMeta;
  PodBlockArray<ShaderIRTypeMeta*> mOwnedTypeMeta;
  PodBlockArray<RaverieShaderIROp*> mOwnedSpecializationConstants;
  PodBlockArray<GlobalVariableData*> mOwnedGlobals;

  /// Map of all types by name (Raverie type name). Contains all types generated
  /// (including function pointer types, Real*, etc...), not just bound types.
  /// A type is a bound type is the meta is non-null.
  HashMap<String, RaverieShaderIRType*> mTypes;

  /// All constant literals.
  HashMap<Raverie::Any, RaverieShaderIRConstantLiteral*> mConstantLiterals;
  /// Map of all constants by type/value
  HashMap<ConstantOpKeyType, RaverieShaderIROp*> mConstantOps;
  /// Lookup for enums based upon their raverie property.
  HashMap<void*, RaverieShaderIROp*> mEnumContants;
  /// Map of all specialization constant by key (typically raverie field)
  HashMap<void*, RaverieShaderIROp*> mSpecializationConstantMap;
  /// All globals declared in this library. Currently used for samplers.
  HashMap<Raverie::Field*, GlobalVariableData*> mRaverieFieldToGlobalVariable;
  /// Maps ops of global variables to their variable data.
  /// Used for the backend to find the initializer function for globals.
  HashMap<RaverieShaderIROp*, GlobalVariableData*> mVariableOpLookupMap;
  /// Lookup map for global variables with the FragmentShared attribute.
  /// Needed to find if a shared variable has already been created.
  HashMap<FragmentSharedKey, GlobalVariableData*> mFragmentSharedGlobalVariables;

  /// Map of all raverie functions to shader functions
  HashMap<Raverie::Function*, RaverieShaderIRFunction*> mFunctions;
  HashMap<Raverie::Type*, TypeResolvers> mTypeResolvers;
  /// Operator resolvers for the current library
  OperatorResolvers mOperatorResolvers;

  Array<SpirVExtensionLibrary*> mExtensionLibraries;
  HashMap<Raverie::Function*, SpirVExtensionInstruction*> mExtensionInstructions;
  HashMap<SpirVExtensionLibrary*, RaverieShaderExtensionImport*> mExtensionLibraryImports;
  HashMap<TemplateTypeKey, TemplateTypeIRResloverFn> mTemplateResolvers;

  /// Stores cached information about symbols in this library that have
  /// certain stage requirements. Used to detect and emit errors.
  HashMap<void*, StageRequirementsData> mStageRequirementsData;

  bool mTranslated;

  // A multi-map of all types to their dependents for this library (flattened to
  // include all module dependency data)
  typedef HashMap<RaverieShaderIRType*, HashSet<RaverieShaderIRType*>> TypeDependentMultiMap;
  TypeDependentMultiMap mTypeDependents;

private:
  // Helper template to make finding operators easier
  template <typename OpIdType, typename OpResolverType>
  OpResolverType FindOperatorResolverTemplate(OpIdType& opId, bool checkDependencies);
};

// Due to a bug in ANGLE, we can't rely on WebGL GLSL overloads on samplers.
// To fix this, we append the hash of the function signature to the name
// (different per overload).
String GetOverloadedName(StringParam functionName, Raverie::Function* raverieFunction);

} // namespace Raverie
