///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------ZilchShaderModule
// A module is a collection of libraries, typically used as dependencies for building a project into a library.
class ZilchShaderModule : public Array<ZilchShaderLibraryRef>
{
public:
  // Find a type in any of the contained libraries
  ShaderType* FindType(StringParam name, bool checkDependencies = true);
  ShaderType* FindType(Zilch::BoundType* boundType , bool checkDependencies = true);

  // Find if any libraries implemented this function
  ShaderFunction* FindImplements(Zilch::Function* fnKey);
  // Find if any libraries extended this function
  ShaderFunction* FindExtension(Zilch::Function* fnKey);

  // An intrusive reference count for memory handling
  ZilchRefLink(ZilchShaderModule);
};

//-------------------------------------------------------------------ZilchShaderLibrary
// A library is a collection of types that was built/translated from zilch.
// A library also contains references to all of its dependent libraries.
class ZilchShaderLibrary
{
public:
  ZilchShaderLibrary();
  ~ZilchShaderLibrary();

  // Finds a type in this library. If check dependencies is true then the search will recurse through all of the dependency libraries.
  ShaderType* FindType(StringParam name, bool checkDependencies = true);
  ShaderType* FindType(Zilch::BoundType* boundType , bool checkDependencies = true);

  // Creates a type entry for this library. Only meant for internal use!
  ShaderType* CreateType(StringParam name);
  ShaderType* CreateType(Zilch::BoundType* boundType);
  ShaderType* CreateNativeType(StringParam zilchName, StringParam shaderName, StringRange refType = "", ShaderVarType::Enum varType = ShaderVarType::Other);

  // Given a zilch function, find if there are any shader functions that implemented it (checking all dependent libraries as well).
  ShaderFunction* FindImplements(Zilch::Function* fnKey);
  // Given a zilch function, find if there are any shader extension functions (checking all dependent libraries as well).
  ShaderFunction* FindExtension(Zilch::Function* fnKey);

  // Pulls all reverse dependencies from all the dependent modules into this library (flattens the list)
  void FlattenModuleDependents();
  // Fills out a list of all types that depend on the given type
  void GetAllDependents(ShaderType* shaderType, HashSet<ShaderType*>& finalDependents);
  
  // An intrusive reference count for memory handling
  ZilchRefLink(ZilchShaderLibrary);

  // All of the types contained in this library
  typedef HashMap<String, ShaderType*> TypeMap;
  TypeMap mTypes;

  // Was this library already successfully translated?
  bool mTranslated;
  
  // Is a library generated from user code (scripts) or a core library?
  bool mIsUserCode;
  Zilch::LibraryRef mZilchLibrary;
  ZilchShaderModuleRef mDependencies;

  // Store any zilch functions that were replaced by another function in this library.
  // This is used to replace the definition of more complicated zilch functions that could easily
  // be built using primitives that were already translated (such as LengthSq(a) => Dot(a, a))
  typedef HashMap<Zilch::Function*, ShaderFunction*> ImplementsMap;
  ImplementsMap mImplements;

  // Store any zilch functions that were extended onto another type by this library.
  typedef HashMap<Zilch::Function*, ShaderFunction*> ExtensionsMap;
  ExtensionsMap mExtensions;

  // A map of all zilch types in this library to their corresponding ShaderTypes.
  // All types that can be used in a shader should end up in here but there are various types that won't show up, such as String.
  typedef HashMap<Zilch::Type*, ShaderType*> ZilchToShaderTypeMap;
  ZilchToShaderTypeMap mZilchToShaderTypes;

  // A multi-map of all types to their dependents for this library (flattened to include all module dependency data)
  typedef HashMap<ShaderType*, HashSet<ShaderType*> > TypeDependentMultiMap;
  TypeDependentMultiMap mTypeDependents;
};

}//namespace Zero
