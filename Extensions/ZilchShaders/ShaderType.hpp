///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum6(ShaderVarType, Scalar, Vector, Matrix, Other, GeometryOutput, GeometryInput);

//-------------------------------------------------------------------ShaderTypeData
// A simple class to store information about a shader type. Currently this only
// stores information to distinguish between different math types and their sizes.
// Also now used for geometry shader translation and keeping track of input/output type sizes, types, etc...
class ShaderTypeData
{
public:
  ShaderTypeData();

  ShaderVarType::Enum mType;
  // The dimensions of a matrix. If this is not a matrix then these are zero.
  size_t mSizeX;
  size_t mSizeY;
  // A misc count used currently in geometry shaders
  size_t mCount;
  // Misc extra data used in geometry shaders. Currently used to store input/output primitive type data
  String mExtraData;
};

//-------------------------------------------------------------------ShaderField
// Stores information about the location (mainly zilch source file) for classes. Currently only used on ShaderType.
class ShaderLocation
{
public:
  ShaderLocation();

  // The path of the zilch source file given by the user when they add code to the shader project.
  String mZilchSourcePath;
  // User data provided when adding code to the shader project.
  const void* mUserData;
};

//-------------------------------------------------------------------ShaderFieldKey
// A typed string for looking up shader fields. This is a specific type to help avoid ambiguities on
// HashMap<String, ShaderField*> where it's hard to know what the string's value is. This type implies
// that the hashmap is not based upon the field's name, but rather it's key (name:string).
struct ShaderFieldKey
{
  ShaderFieldKey() {};
  ShaderFieldKey(ShaderField* shaderField);
  ShaderFieldKey(StringParam fieldName, StringParam fieldType);

  void Set(StringParam fieldName, StringParam fieldType);

  size_t Hash() const;
  bool operator==(const ShaderFieldKey& other) const;
  operator String() const;

  String mKey;
};

//-------------------------------------------------------------------ShaderField
// A member field from a zilch type (member variable). This contains both pre and post-translation information.
class ShaderField
{
public:
  ShaderField();
  // Is this a static field?
  bool IsStatic() const;
  // Is this field a shared field across multiple fragments?
  bool IsShared() const;
  // Find the shader type for this field.
  ShaderType* GetShaderType();

  // Does this field contain an attribute?
  bool ContainsAttribute(StringParam attributeName);

  // The zilch return type of this field. Used to translate and during compositing.
  String mZilchType;
  // As types can be changed/mangled during translation this might differ from the zilch type.
  String mShaderType;

  // The name of this field pre-translation.
  String mZilchName;
  // Most of the time the field's shader name will be the same as the zilch name. On rare occasions
  // the name needs to be mangled during translation to prevent conflicts of names
  // (statics or any type that is forced to be a static such as samplers).
  String mShaderName;

  // The translated default value of this field. Used to declare the default value in a pre-constructor function.
  String mShaderDefaultValueString;
  // The default value of this type loaded into a variant (so a property grid can use this value).
  Zilch::Any mDefaultValueVariant;

  // Any attributes that were on this field.
  ShaderAttributeList mAttributes;
  
  // Store if this field was static (instead of search the attributes each time).
  bool mIsStatic;
  // Is this field shared across multiple fragments. Currently only static and forced static fields (such as samplers) can be shared.
  bool mIsShared;

  CodeRangeMapping mPreInitRange;
  Zilch::CodeLocation mSourceLocation;

  // The type that owns this field
  ShaderType* mOwner;
};

//-------------------------------------------------------------------ShaderFunction
// A member function from a zilch type. This contains both pre and post-translation information.
class ShaderFunction
{
public:
  ShaderFunction();
  // Is this a static function?
  bool IsStatic() const;

  // Does this field contain an attribute?
  bool ContainsAttribute(StringParam attributeName);

  // The zilch return type of this function. Used to translate and during compositing.
  String mZilchReturnType;
  // As types can be changed/mangled during translation this might differ from the zilch return type.
  String mShaderReturnType;

  // The name of the function (pre/post translation)
  String mZilchName;
  String mShaderName;

  // Comments typically captured from the zilch script to emit in the final shader.
  // Note: These don't contain the comment token "//" as this could be different per language being emitted to.
  Array<String> mComments;

  // Currently I've never needed the zilch signature of the function so this is not stored.
  //String mZilchSignature;
  // The signature of the function after translation.
  String mShaderSignature;

  // The code of this function post translation.
  String mShaderBodyCode;

  // Any attributes that were on this function.
  ShaderAttributeList mAttributes;

  // Store if this function was static (instead of search the attributes each time).
  bool mIsStatic;

  // The type that owns this function
  ShaderType* mOwner;

  // Where in the original zilch script this function came from. Not all of this information is needed, so maybe remove it later?
  Zilch::CodeLocation mSourceLocation;

  CodeRangeMapping mBodyRange;
  CodeRangeMapping mSignatureRange;
};

DeclareEnum4(FragmentType, Vertex, Pixel, Geometry, None);
// Various flags to be stored about a shader type
DeclareBitField6(ShaderTypeFlags, Intrinsic, NonCopyable, Native, HasMain, HideStruct, HideFunctions);



//-------------------------------------------------------------------ShaderType
// A zilch type from shader compilation. This type contains all the relevant functions and fields of the type
// needed for compositing and translation. This also contains both pre and post-translation information.
class ShaderType
{
public:
  ShaderType();
  ~ShaderType();

  // Find a shader function by key (typically the zilch::Function*). Returns null if not found.
  ShaderFunction* FindFunction(void* zilchFnKey);
  // Finds a shader function by key. If it doesn't exist then a new one is created.
  // This should typically not be used during translation as the type collector should generate all functions.
  ShaderFunction* FindOrCreateFunction(StringParam zilchFunctionName, void* zilchFnKey);
  // Creates a named function for final shader translation (these only exist in translation of the final shader type).
  ShaderFunction* CreateFinalShaderFunction(StringParam shaderFunctionName);
  // Find a field by the zilch variable name. Returns null if not found.
  ShaderField* FindField(StringParam zilchFieldName);
  // Finds a field by the zilch variable name. Returns null if not found.
  // This should typically not be used during translation as the type collector should generate all fields.
  // This is currently used in translation to create a dummy variable if no instance fields exist as glsl can't define a class with no fields.
  ShaderField* FindOrCreateField(StringParam zilchFieldName);

  // Does this field contain an attribute?
  bool ContainsAttribute(StringParam attributeName);

  // Add a shader type that this one depends on. Used to properly include all dependencies when building the final shader.
  void AddDependency(ShaderType* shaderType);

  // Get the meta data of this type.
  ShaderTypeData* GetTypeData();

  // Does this type represent an intrinsic type? An intrinsic type is one that doesn't produce a
  // struct in the shader language but directly maps to a built-in type in the shader language (such as sampler2d)
  bool GetIntrinsic() const;
  void SetIntrinsic(bool state);
  // Is this type non-copyable. If so it must be passed by reference to a function (again, samplers)
  bool IsNonCopyable() const;
  void SetNonCopyable(bool state);
  // Is this type backed by C++ not a zilch script? Native also implies to a certain extent that
  // the type is intrinsics (no struct should be written out, etc...)
  bool IsNative() const;
  void SetNative(bool state);
  // Does this type have a main function? Used to emit an error message if more than one main function exists.
  bool GetHasMain() const;
  void SetHasMain(bool state);

  bool GetHideFunctions();
  bool GetHideStruct();
  // Mark that this class should hide the struct and function emission when building the final shader string.
  void SetHideStructAndFunctions(bool state);
private:

  // Currently store this on the type. Later move it to some shared location?
  ShaderTypeData mTypeData;
public:  

  // The name of the type in zilch (pre-translation)
  String mZilchName;
  // The name of the type for the shader (post-translation. Some type names will end
  // up being mangled or altered slightly so this records the final name.
  String mShaderName;
  // If this is a built-in type then this will be the same as mZilchName. If this is a user defined
  // intrinsic type then the user has to define what kind of property this is. For example a user can
  // define Sampler2d in Zilch which becomes sampler2D in glsl but needs to be bound to the engine as "Texture".
  String mPropertyType;
  // The file name that this type came from in the original shader project it was compiled in.
  String mFileName;

  // The list of fields (member variables) that this type defines.
  // This include both static and instance fields.
  typedef Array<ShaderField*> FieldList;
  FieldList mFieldList;
  // A map of field name (zilch name) to the field instance for quick look-ups.
  typedef HashMap<String, ShaderField*> FieldMap;
  FieldMap mFieldMap;

  // The list of functions that this type defines.
  // This includes static and instance functions
  typedef Array<ShaderFunction*> FunctionList;
  FunctionList mFunctionList;
  // These are extra functions needed during final shader translation. They don't exist in zilch but are necessary in the final shader.
  // These are also kept track of in a list of functions for easy of forward declaring and so on.
  FunctionList mFinalShaderFunctions;
  // The map of function name (zilch name) to functions. As overloads of a function can be defined
  // this is a map to a list of all the overloads for that function name.
  typedef HashMap<String, FunctionList> FunctionNameMultiMap;
  FunctionNameMultiMap mFunctionNameMultiMap;
  // Most functions have some unique key that was generated from zilch. Typically this is the Zilch::Function* that is unique
  // for every overload of a function. This map allows quick look-ups from this key so no overload resolution is needed to
  // find the function. On rare occasions the key is not actually a Zilch::Function*
  // (Currently only implicitly default constructors as Zilch doesn't actually generate that function pointer.
  // Instead ZilchShaderSettings::GetDefaultConstructorKey() is used.)
  typedef HashMap<void*, ShaderFunction*> FunctionOverloadMap;
  FunctionOverloadMap mFunctionOverloadMap;
  
  // The names (Zilch names) of all other types this type depends on. This is used
  // to know what other types have to be pulled in when building a translation.
  // Dependencies are in the order they are first encountered to ensure determinism in translation order for unit tests.
  Array<ShaderType*> mDependencyList;
  // What attributes this type has.
  ShaderAttributeList mAttributes;

  // Some types (again...samplers) have to be declared specially (such as with the uniform attribute).
  // This is any qualifiers this type must have when being declared.
  String mForcedDeclarationQualifiers;
  // What is the qualifier used when this type is passed by reference. In glsl for instance,
  // there is: in, out, inout. Some types (samplers) can only be passed with in. This is used to override
  // the default value. If this is empty then the default reference qualifier for the language is used (on the name settings).
  String mReferenceTypeQualifier;

  // If this type declares any fields with any of the attributes of mUniformTag, mAttributeTag, or mVaryingTag then
  // those fields need to declare the global scoped variables with the correct qualifiers. This string stores these
  // variables separately from the function declarations so that they can be declared above everything else,
  // primarily so that built-in forced-static uniforms can be used in fragments.
  String mInputAndOutputVariableDeclarations;

  // What kind of fragment this type is. Currently this should only ever be: Vertex, Pixel, or None.
  FragmentType::Enum mFragmentType;
  // What library created this type. Note: Do not hold onto a type without keeping the library alive!!!
  ZilchShaderLibrary* mOwningLibrary;

  // Any input type that this type relies on. Currently this is used by geometry shaders to mark
  // their input stream type and the input stream type uses this to mark its template type.
  ShaderType* mInputType;
  // Same as mInputType but for outputs.
  ShaderType* mOutputType;

  Zilch::CodeLocation mSourceLocation;
  // Any user data that cant simply be represented by a pointer
  // Data can be written to the buffer and will be properly destructed
  // when this object is destroyed (must be read in the order it's written)
  mutable Zilch::DestructibleBuffer mComplexUserData;
  ShaderLocation mLocation;

  BitField<ShaderTypeFlags::Enum> mFlags;
};

}//namespace Zero
