///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------FunctionCallResolverWrapper
typedef void(*FunctionCallResolverFn)(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context);
typedef void(*ConstructorCallResolverFn)(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::StaticTypeNode* staticTypeNode, ZilchShaderTranslatorContext* context);
// A structure to wrap various methods to resolve a function call. The 3 main methods of resolving a function call are:
// 1. Off a member access. This can be either an instance or static function call, such as this.Member or Math.Dot
// 2. Off a static type node. Any function call off a static type node should be a constructor, that is Type().
// 3. Sometimes it's too big of a hassle to register a callback function for each member access available and it is easier
//    to just build up a replacement string. This is the fallback for both options 1 and 2 as well as just one way to translate.
//    An example of this is registering each default constructor for vector types.
struct FunctionCallResolverWrapper
{
  FunctionCallResolverWrapper();
  FunctionCallResolverWrapper(FunctionCallResolverFn fnCall);
  FunctionCallResolverWrapper(ConstructorCallResolverFn constructorCall);
  FunctionCallResolverWrapper(StringParam str);

  void Translate(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context);
  void Translate(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::StaticTypeNode* staticTypeNode, ZilchShaderTranslatorContext* context);
  void Translate(ZilchShaderTranslator* translator, ZilchShaderTranslatorContext* context);

  FunctionCallResolverFn mFnCall;
  ConstructorCallResolverFn mConstructorCallResolver;
  String mBackupString;
};

//-------------------------------------------------------------------MemberAccessResolverWrapper
// A structure to wrap resolving a member access. This often times is changing the way a member
// is accessed to another simple string, such as changing "Math.Dot" -> "dot". Sometimes this
// does a larger replacement such as changing "Vec3.Count" -> "3".
typedef void(*MemberAccessResolverFn)(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context);
struct MemberAccessResolverWrapper
{
  MemberAccessResolverWrapper();
  MemberAccessResolverWrapper(MemberAccessResolverFn resolverFn);
  MemberAccessResolverWrapper(StringParam replacementStr);

  void Translate(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context);

  MemberAccessResolverFn mResolverFn;
  String mNameReplacementString;
};

//-------------------------------------------------------------------MemberAccessResolverWrapper
// Some binary operators don't exist in certain languages as actual operators. Instead they are
// implemented as a function call. For that reason (and any others) this wrapper allows you to
// replace the translation of a binary operator between two types.
typedef void(*BinaryOpResolver)(ZilchShaderTranslator* translator, Zilch::BinaryOperatorNode* node, ZilchShaderTranslatorContext* context);
struct BinaryOpResolverWrapper
{
  BinaryOpResolverWrapper();
  BinaryOpResolverWrapper(BinaryOpResolver resolverFn);
  BinaryOpResolverWrapper(StringParam replacementFnCallStr);

  void Translate(ZilchShaderTranslator* translator, Zilch::BinaryOperatorNode* node, ZilchShaderTranslatorContext* context);

  BinaryOpResolver mResolverFn;
  String mReplacementFnCallStr;
};

typedef void(*NativeLibraryParser)(ZilchShaderTranslator* translator, ZilchShaderLibrary* shaderLibrary);
// Function pointer to allow the user to detect a new template type and register any callbacks needed for it.
typedef bool(*TemplateTypeResolver)(ZilchShaderTranslator* translator, Zilch::Type* type, Zilch::SyntaxNode* nodeLocation, String& resultTypeName);
// Function pointer to allow the user to register a resolver for an initializer node for a certain type.
typedef void(*InitializerResolver)(ZilchShaderTranslator* translator, Zilch::Type* type, Zilch::ExpressionInitializerNode* initializerNode, ZilchShaderTranslatorContext* context);

//-------------------------------------------------------------------LibraryTranslator

/// This class is in charge of resolving various library functions that aren't simple to resolve.
/// This mostly is translating the math library of zero/zilch into the correct shader language's functions.
/// The translation that needs to be done here is to rename functions (just the function call), replace functions
/// (either to not be functions or to not be member functions), replace member names,
/// replace members (to functions or even hard-coded values), replace binary operators (glsl doesn't have < between vectors), and so on...
/// The primary method this is done is via function* maps (so getting the pointer to Math.Dot(Real3, Real3) for instance).
class LibraryTranslator
{
public:
  LibraryTranslator();

  // Clear out all the cached lookups for the next run.
  void Reset();

  void RegisterNativeLibraryParser(NativeLibraryParser nativeLibraryParser);
  void ParseNativeLibrary(ZilchShaderTranslator* translator, ZilchShaderLibrary* shaderLibrary);

  // Various methods of registering a resolver for a function call node.
  void RegisterFunctionCallResolver(Zilch::Function* fn, const FunctionCallResolverWrapper& wrapper);
  void RegisterFunctionCallResolver(Zilch::Function* fn, FunctionCallResolverFn resolver);
  void RegisterFunctionCallResolver(Zilch::Function* fn, ConstructorCallResolverFn resolver);
  void RegisterFunctionCallResolver(Zilch::Function* fn, StringParam replacementStr);
  // Sometimes it's a hassle to register a bunch of function* callbacks for constructors when all we're going
  // to do is the same simple translation. In that case allow a backup resolver by type for all constructors
  // that aren't already registered. This should be used sparingly, such as simple vector types.
  // If used too much then any extra constructors that get added will be accidentally translated when they should be errors.
  void RegisterBackupConstructorResolver(Zilch::Type* type, ConstructorCallResolverFn resolver);
  // Resolve a function call. This could either be off a MemberAccessNode (instance or static fn call) or a StaticTypeNode (constructor call).
  bool ResolveFunctionCall(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, ZilchShaderTranslatorContext* context);
  // If the user declared a variable with no explicit constructor call, then zilch will normally 
  // call the default constructor for this type (eg. var a : Integer2 is actually performing var a : Integer2 = Integer2()).
  // In this case (and this case only) we need to invoke the default constructor for the given type.
  bool ResolveDefaultConstructor(ZilchShaderTranslator* translator, Zilch::Type* type, Zilch::Function* fn, ZilchShaderTranslatorContext* context);

  // A member access could either be a field, property, or function. In either case we can resolve either with a function pointer or a string.
  void RegisterMemberAccessResolver(Zilch::Field* field, const MemberAccessResolverWrapper& wrapper);
  void RegisterMemberAccessResolver(Zilch::Field* field, MemberAccessResolverFn resolverFn);
  void RegisterMemberAccessResolver(Zilch::Field* field, StringParam replacementStr);
  void RegisterMemberAccessResolver(Zilch::Property* prop, const MemberAccessResolverWrapper& wrapper);
  void RegisterMemberAccessResolver(Zilch::Property* prop, MemberAccessResolverFn resolverFn);
  void RegisterMemberAccessResolver(Zilch::Property* prop, StringParam replacementStr);
  void RegisterMemberAccessResolver(Zilch::Function* function, const MemberAccessResolverWrapper& wrapper);
  void RegisterMemberAccessResolver(Zilch::Function* function, MemberAccessResolverFn resolverFn);
  void RegisterMemberAccessResolver(Zilch::Function* function, StringParam replacementStr);
  // A backup method for resolving a member access on a type. This is primarily to prevent registering an
  // excessive number of callbacks for some types (such as all of the splats on vector types).
  // As with function calls, this should be used sparingly, and ideally in a such a way access of non-expected members still results in errors.
  void RegisterMemberAccessBackupResolver(Zilch::Type* type, MemberAccessResolverFn resolverFn);
  // Resolve a member access node. Note that this only resolves the left side of the access, that is Math.Dot(a, b) can only resolve the "Math.Dot" portion.
  bool ResolveMemberAccess(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context);

  // Most binary operators are translated as normal, however there are some special cases such as Real3 <= Real3 -> lessThanEqual(Real3, Real3)
  void RegisterBinaryOpResolver(Zilch::Type* typeA, Zilch::Grammar::Enum op, Zilch::Type* typeB, const BinaryOpResolverWrapper wrapper);
  void RegisterBinaryOpResolver(Zilch::Type* typeA, Zilch::Grammar::Enum op, Zilch::Type* typeB, StringParam replacementFnCallStr);
  void RegisterBinaryOpResolver(Zilch::Type* typeA, Zilch::Grammar::Enum op, Zilch::Type* typeB, BinaryOpResolver resolverFn);
  void RegisterBinaryOpResolver(StringParam key, const BinaryOpResolverWrapper wrapper);
  void RegisterBinaryOpResolver(StringParam key, StringParam replacementFnCallStr);
  void RegisterBinaryOpResolver(StringParam key, BinaryOpResolver resolverFn);
  bool ResolveBinaryOp(ZilchShaderTranslator* translator, Zilch::BinaryOperatorNode* node, ZilchShaderTranslatorContext* context);

  // When we detect a template type we can choose to register it as a type as well as map any of its functions out.
  // Currently this is used when we see an array type for the first time so we can resolve the
  // name (Array[Real]->float[]) and map some of its functions (Get(index) -> [index]).
  void RegisterTemplateTypeResolver(TemplateTypeResolver resolver);
 
  // We want to register a resolver for a specific type's initializer node. This is used to more easily
  // translate certain things such as the array's initializer list to the language's specific initializer list.
  void RegisterInitializerNodeResolver(Zilch::Type* type, InitializerResolver resolver);
  bool ResolveInitializerNode(ZilchShaderTranslator* translator, Zilch::Type* type, Zilch::ExpressionInitializerNode* initializerNode, ZilchShaderTranslatorContext* context);

  String GetFunctionDescription(StringParam fnName, Zilch::Function* fn);

  bool IsUserLibrary(Zilch::Library* library);

private:
  NativeLibraryParser mNativeLibraryParser;
  // Function Resolvers
  HashMap<Zilch::Function*, FunctionCallResolverWrapper> mFunctionCallResolvers;
  HashMap<Zilch::Type*, FunctionCallResolverWrapper> mBackupConstructorResolvers;
  
  // Member Access Resolvers
  HashMap<Zilch::Field*, MemberAccessResolverWrapper> mMemberAccessFieldResolvers;
  HashMap<Zilch::Property*, MemberAccessResolverWrapper> mMemberAccessPropertyResolvers;
  HashMap<Zilch::Function*, MemberAccessResolverWrapper> mMemberAccessFunctionResolvers;
  HashMap<Zilch::Type*, MemberAccessResolverWrapper> mBackupMemberAccessResolvers;

  HashMap<String, BinaryOpResolverWrapper> mBinaryOpResolvers;
public:
  TemplateTypeResolver mTemplateTypeResolver;
  HashSet<String> mRegisteredTemplates;
  HashMap<Zilch::Type*, InitializerResolver> mIntializerResolvers;

public:

  /// The errors from the translator. This is so we can invoke errors for the user.
  ZilchShaderTranslator* mTranslator;
};

}//namespace Zero
