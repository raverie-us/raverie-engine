// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class ZilchShaderIRLibrary;
class ZilchShaderIRModule;
typedef Zilch::Ref<ZilchShaderIRLibrary> ZilchShaderIRLibraryRef;
typedef Zilch::Ref<ZilchShaderIRModule> ZilchShaderIRModuleRef;

class ZilchSpirVFrontEnd;
class ZilchSpirVFrontEndContext;

typedef Pair<ZilchShaderIRType*, Zilch::Any> ConstantOpKeyType;

typedef void (*DefaultConstructorResolverFn)(ZilchSpirVFrontEnd* translator,
                                             Zilch::Type* resultType,
                                             ZilchSpirVFrontEndContext* context);
typedef void (*ConstructorCallResolverIRFn)(ZilchSpirVFrontEnd* translator,
                                            Zilch::FunctionCallNode* fnCallNode,
                                            Zilch::StaticTypeNode* staticTypeNode,
                                            ZilchSpirVFrontEndContext* context);
typedef void (*MemberAccessResolverIRFn)(ZilchSpirVFrontEnd* translator,
                                         Zilch::MemberAccessNode* memberAccessNode,
                                         ZilchSpirVFrontEndContext* context);
typedef void (*MemberFunctionResolverIRFn)(ZilchSpirVFrontEnd* translator,
                                           Zilch::FunctionCallNode* functionCallNode,
                                           Zilch::MemberAccessNode* memberAccessNode,
                                           ZilchSpirVFrontEndContext* context);
typedef void (*MemberPropertySetterResolverIRFn)(ZilchSpirVFrontEnd* translator,
                                                 Zilch::MemberAccessNode* memberAccessNode,
                                                 ZilchShaderIROp* resultValue,
                                                 ZilchSpirVFrontEndContext* context);
typedef void (*BinaryOpResolverIRFn)(ZilchSpirVFrontEnd* translator,
                                     Zilch::BinaryOperatorNode* binaryOpNode,
                                     ZilchSpirVFrontEndContext* context);
typedef void (*UnaryOpResolverIRFn)(ZilchSpirVFrontEnd* translator,
                                    Zilch::UnaryOperatorNode* binaryOpNode,
                                    ZilchSpirVFrontEndContext* context);
typedef void (*TypeCastResolverIRFn)(ZilchSpirVFrontEnd* translator,
                                     Zilch::TypeCastNode* binaryOpNode,
                                     ZilchSpirVFrontEndContext* context);
typedef void (*TemplateTypeIRResloverFn)(ZilchSpirVFrontEnd* translator, Zilch::BoundType* boundType);
typedef void (*ExpressionInitializerIRResolverFn)(ZilchSpirVFrontEnd* translator,
                                                  Zilch::ExpressionInitializerNode*& node,
                                                  ZilchSpirVFrontEndContext* context);

//-------------------------------------------------------------------FragmentSharedKey
/// Hash key used to lookup fields with the FragmentShared attribute. A shared field
/// is uniquely described by its type and name (additionally include any storage
/// class as this is technically part of the type, not sure if this is actually important).
struct FragmentSharedKey
{
  FragmentSharedKey();
  FragmentSharedKey(spv::StorageClass storageClass, ZilchShaderIRType* fieldType, StringParam varName);

  size_t Hash() const;
  bool operator==(const FragmentSharedKey& rhs) const;

  spv::StorageClass mStorageClass;
  ZilchShaderIRType* mFieldType;
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
  void RegisterFieldResolver(Zilch::Field* field, MemberAccessResolverIRFn fieldResolver);
  /// Register a resolver to use when a field-specific resolver isn't found.
  /// Used to handle things like swizzles.
  void RegisterBackupFieldResolver(MemberAccessResolverIRFn backupResolver);
  /// Finds the resolver for the given field. Returns the backup resolver if no
  /// match is found.
  MemberAccessResolverIRFn FindFieldResolver(Zilch::Field* field);

  void RegisterConstructorResolver(Zilch::Function* zilchFunction, ConstructorCallResolverIRFn resolverFn);
  ConstructorCallResolverIRFn FindConstructorResolver(Zilch::Function* zilchFunction);

  /// Register a resolver for a function call.
  void RegisterFunctionResolver(Zilch::Function* function, MemberFunctionResolverIRFn functionResolver);
  MemberFunctionResolverIRFn FindFunctionResolver(Zilch::Function* function);

  /// Register a resolver for a setter function call.
  void RegisterSetterResolver(Zilch::Function* function, MemberPropertySetterResolverIRFn functionResolver);
  void RegisterBackupSetterResolver(MemberPropertySetterResolverIRFn backupResolver);
  MemberPropertySetterResolverIRFn FindSetterResolver(Zilch::Function* function);

  HashMap<Zilch::Field*, MemberAccessResolverIRFn> mFieldResolvers;
  MemberAccessResolverIRFn mBackupFieldResolver;

  DefaultConstructorResolverFn mDefaultConstructorResolver;
  ConstructorCallResolverIRFn mBackupConstructorResolver;
  MemberPropertySetterResolverIRFn mBackupSetterResolver;
  /// Library translations for constructors of a type (e.g. Real3 splat
  /// constructor)
  HashMap<Zilch::Function*, ConstructorCallResolverIRFn> mConstructorResolvers;

  HashMap<Zilch::Function*, MemberFunctionResolverIRFn> mFunctionResolvers;
  HashMap<Zilch::Function*, MemberPropertySetterResolverIRFn> mSetterResolvers;
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
  void RegisterBinaryOpResolver(Zilch::Type* lhsType,
                                Zilch::Type* rhsType,
                                Zilch::Grammar::Enum op,
                                BinaryOpResolverIRFn resolver);
  BinaryOpResolverIRFn FindOpResolver(BinaryOperatorKey& opId);

  /// Unary operators
  void RegisterUnaryOpResolver(Zilch::Type* type, Zilch::Grammar::Enum op, UnaryOpResolverIRFn resolver);
  UnaryOpResolverIRFn FindOpResolver(UnaryOperatorKey& opId);

  /// Type Cast operators
  void RegisterTypeCastOpResolver(Zilch::Type* fromType, Zilch::Type* toType, TypeCastResolverIRFn resolver);
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
  ZilchShaderIROp* mInstance;
  /// A function that initializes this instance. Can be null.
  ZilchShaderIRFunction* mInitializerFunction;
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
  void Combine(Zilch::Member* dependency, const Zilch::CodeLocation& location, ShaderStage::Enum requiredStage);

  /// What stage this symbol requires.
  ShaderStage::Enum mRequiredStages;
  /// The first symbol that causes the stage requirement. Used to generate a
  /// call graph on error.
  Zilch::Member* mDependency;
  /// The location that the dependency is called.
  Zilch::CodeLocation mCallLocation;
};

/// A module represents a collection of libraries, typically used to
/// express what dependencies another library has. All libraries in this
/// module should be fully compiled (and hence locked) if they're in a module.
class ZilchShaderIRModule : public Array<ZilchShaderIRLibraryRef>
{
public:
  /// Find a type in any of the contained libraries
  ZilchShaderIRType* FindType(const String& typeName, bool checkDependencies = true);

  /// Find the global variable data associate with the given zilch field.
  GlobalVariableData* FindGlobalVariable(Zilch::Field* zilchField, bool checkDependencies = true);
  /// Find the global variable data associate with the given instance variable
  /// op.
  GlobalVariableData* FindGlobalVariable(ZilchShaderIROp* globalInstance, bool checkDependencies = true);

  /// Find the global variable data associated with a fragment shared variable.
  GlobalVariableData* FindFragmentSharedVariable(const FragmentSharedKey& key, bool checkDependencies = true);

  /// Find a resolver for a template type
  TemplateTypeIRResloverFn FindTemplateResolver(const TemplateTypeKey& templateKey, bool checkDependencies = true);

  TypeResolvers* FindTypeResolver(Zilch::Type* zilchType, bool checkDependencies = true);

  // Constructor library replacements
  ZilchShaderIRFunction* FindFunction(Zilch::Function* zilchFunction, bool checkDependencies = true);
  SpirVExtensionInstruction* FindExtensionInstruction(Zilch::Function* zilchFunction, bool checkDependencies = true);
  ZilchShaderExtensionImport* FindExtensionLibraryImport(SpirVExtensionLibrary* extensionLibrary,
                                                         bool checkDependencies = true);

  ZilchShaderIRConstantLiteral* FindConstantLiteral(Zilch::Any& literalValue, bool checkDependencies = true);
  ZilchShaderIROp* FindConstantOp(ConstantOpKeyType& key, bool checkDependencies = true);
  ZilchShaderIROp* FindEnumConstantOp(void* key, bool checkDependencies = true);
  ZilchShaderIROp* FindSpecializationConstantOp(void* key, bool checkDependencies = true);

  // An intrusive reference count for memory handling
  ZilchRefLink(ZilchShaderIRModule);

private:
  friend ZilchShaderIRLibrary;

  // Helper template to make finding operators easier
  template <typename OpIdType, typename OpResolverType>
  OpResolverType FindOperatorResolverTemplate(OpIdType& opId, bool checkDependencies);
};

/// A library built during shader translation. Mostly an internal type that
/// stores all necessary lookup information to build and generate a spir-v
/// shader. Also contains what types were created during translation.
class ZilchShaderIRLibrary
{
public:
  ZilchShaderIRLibrary();
  ~ZilchShaderIRLibrary();

  void AddType(StringParam typeName, ZilchShaderIRType* shaderType);
  ZilchShaderIRType* FindType(const String& typeName, bool checkDependencies = true);
  ZilchShaderIRType* FindType(Zilch::Type* zilchType, bool checkDependencies = true);

  /// Find the global variable data associate with the given zilch field.
  GlobalVariableData* FindGlobalVariable(Zilch::Field* zilchField, bool checkDependencies = true);
  /// Find the global variable data associate with the given instance variable
  /// op.
  GlobalVariableData* FindGlobalVariable(ZilchShaderIROp* globalInstance, bool checkDependencies = true);

  /// Find the global variable data associated with a fragment shared variable.
  GlobalVariableData* FindFragmentSharedVariable(const FragmentSharedKey& key, bool checkDependencies = true);

  // Resolvers for template types (e.g. FixedArray)
  void RegisterTemplateResolver(const TemplateTypeKey& templateKey, TemplateTypeIRResloverFn resolver);
  TemplateTypeIRResloverFn FindTemplateResolver(const TemplateTypeKey& templateKey, bool checkDependencies = true);

  /// Pulls all reverse dependencies from all the dependent modules into this
  /// library (flattens the list)
  void FlattenModuleDependents();
  /// Fills out a list of all types that depend on the given type
  void GetAllDependents(ZilchShaderIRType* shaderType, HashSet<ZilchShaderIRType*>& finalDependents);

  // Library replacements on a type
  TypeResolvers* FindTypeResolver(Zilch::Type* zilchType, bool checkDependencies = true);

  // Operator resolvers
  BinaryOpResolverIRFn FindOperatorResolver(BinaryOperatorKey& opId, bool checkDependencies = true);
  UnaryOpResolverIRFn FindOperatorResolver(UnaryOperatorKey& opId, bool checkDependencies = true);
  TypeCastResolverIRFn FindOperatorResolver(TypeCastKey& opId, bool checkDependencies = true);

  MemberAccessResolverIRFn FindFieldResolver(Zilch::Type* zilchType,
                                             Zilch::Field* zilchField,
                                             bool checkDependencies = true);
  MemberFunctionResolverIRFn FindFunctionResolver(Zilch::Type* zilchType,
                                                  Zilch::Function* zilchFunction,
                                                  bool checkDependencies = true);
  MemberPropertySetterResolverIRFn FindSetterResolver(Zilch::Type* zilchType,
                                                      Zilch::Function* zilchFunction,
                                                      bool checkDependencies = true);
  ConstructorCallResolverIRFn FindConstructorResolver(Zilch::Type* zilchType,
                                                      Zilch::Function* zilchFunction,
                                                      bool checkDependencies = true);

  ZilchShaderIRFunction* FindFunction(Zilch::Function* zilchFunction, bool checkDependencies = true);
  SpirVExtensionInstruction* FindExtensionInstruction(Zilch::Function* zilchFunction, bool checkDependencies = true);
  ZilchShaderExtensionImport* FindExtensionLibraryImport(SpirVExtensionLibrary* extensionLibrary,
                                                         bool checkDependencies = true);

  ZilchShaderIRConstantLiteral* FindConstantLiteral(Zilch::Any& literalValue, bool checkDependencies = true);

  ZilchShaderIROp* FindConstantOp(ConstantOpKeyType& key, bool checkDependencies = true);
  ZilchShaderIROp* FindEnumConstantOp(void* key, bool checkDependencies = true);
  /// Finds a specialization constant given a key. The key is normally a zilch
  /// field, but can also be special global keys (like languageId).
  ZilchShaderIROp* FindSpecializationConstantOp(void* key, bool checkDependencies = true);

  Zilch::LibraryRef mZilchLibrary;
  ZilchShaderIRModuleRef mDependencies;
  // An intrusive reference count for memory handling
  ZilchRefLink(ZilchShaderIRLibrary);

  // Data owned by this library. Needed for memory management.
  PodBlockArray<ZilchShaderIRType*> mOwnedTypes;
  PodBlockArray<ZilchShaderIRFunction*> mOwnedFunctions;
  PodBlockArray<ShaderIRFieldMeta*> mOwnedFieldMeta;
  PodBlockArray<ShaderIRFunctionMeta*> mOwnedFunctionMeta;
  PodBlockArray<ShaderIRTypeMeta*> mOwnedTypeMeta;
  PodBlockArray<ZilchShaderIROp*> mOwnedSpecializationConstants;
  PodBlockArray<GlobalVariableData*> mOwnedGlobals;

  /// Map of all types by name (Zilch type name). Contains all types generated
  /// (including function pointer types, Real*, etc...), not just bound types.
  /// A type is a bound type is the meta is non-null.
  HashMap<String, ZilchShaderIRType*> mTypes;

  /// All constant literals.
  HashMap<Zilch::Any, ZilchShaderIRConstantLiteral*> mConstantLiterals;
  /// Map of all constants by type/value
  HashMap<ConstantOpKeyType, ZilchShaderIROp*> mConstantOps;
  /// Lookup for enums based upon their zilch property.
  HashMap<void*, ZilchShaderIROp*> mEnumContants;
  /// Map of all specialization constant by key (typically zilch field)
  HashMap<void*, ZilchShaderIROp*> mSpecializationConstantMap;
  /// All globals declared in this library. Currently used for samplers.
  HashMap<Zilch::Field*, GlobalVariableData*> mZilchFieldToGlobalVariable;
  /// Maps ops of global variables to their variable data.
  /// Used for the backend to find the initializer function for globals.
  HashMap<ZilchShaderIROp*, GlobalVariableData*> mVariableOpLookupMap;
  /// Lookup map for global variables with the FragmentShared attribute.
  /// Needed to find if a shared variable has already been created.
  HashMap<FragmentSharedKey, GlobalVariableData*> mFragmentSharedGlobalVariables;

  /// Map of all zilch functions to shader functions
  HashMap<Zilch::Function*, ZilchShaderIRFunction*> mFunctions;
  HashMap<Zilch::Type*, TypeResolvers> mTypeResolvers;
  /// Operator resolvers for the current library
  OperatorResolvers mOperatorResolvers;

  Array<SpirVExtensionLibrary*> mExtensionLibraries;
  HashMap<Zilch::Function*, SpirVExtensionInstruction*> mExtensionInstructions;
  HashMap<SpirVExtensionLibrary*, ZilchShaderExtensionImport*> mExtensionLibraryImports;
  HashMap<TemplateTypeKey, TemplateTypeIRResloverFn> mTemplateResolvers;

  /// Stores cached information about symbols in this library that have
  /// certain stage requirements. Used to detect and emit errors.
  HashMap<void*, StageRequirementsData> mStageRequirementsData;

  bool mTranslated;

  // A multi-map of all types to their dependents for this library (flattened to
  // include all module dependency data)
  typedef HashMap<ZilchShaderIRType*, HashSet<ZilchShaderIRType*>> TypeDependentMultiMap;
  TypeDependentMultiMap mTypeDependents;

private:
  // Helper template to make finding operators easier
  template <typename OpIdType, typename OpResolverType>
  OpResolverType FindOperatorResolverTemplate(OpIdType& opId, bool checkDependencies);
};

// Due to a bug in ANGLE, we can't rely on WebGL GLSL overloads on samplers.
// To fix this, we append the hash of the function signature to the name
// (different per overload).
String GetOverloadedName(StringParam functionName, Zilch::Function* zilchFunction);

} // namespace Zero
