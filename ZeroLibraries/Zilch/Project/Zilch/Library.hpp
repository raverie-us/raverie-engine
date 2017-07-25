/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_LIBRARY_HPP
#define ZILCH_LIBRARY_HPP

namespace Zilch
{
  // Type-defines
  typedef BoundType* (*InstantiateTemplateCallback)
  (
    LibraryBuilder& builder,
    StringParam baseName,
    StringParam fullyQualifiedName,
    const Array<Constant>& templateParameters,
    const void* userData
  );

  // An type parsed delegate is a structure that stores the callback and user-data
  class ZeroShared InstantiateTemplateDelegate
  {
  public:
    // Constructor
    InstantiateTemplateDelegate();

    // The associated callback / method
    InstantiateTemplateCallback Callback;

    // The 'this' pointer to the object
    const void* UserData;
  };

  // The type and name of a template parameter
  class ZeroShared TemplateParameter
  {
  public:
    // The name of the template argument
    String Name;

    // The type that the template's constant must be
    ConstantType::Enum Type;
  };

  // All the information we need to instantiate a new template
  class ZeroShared InstantiateTemplateInfo
  {
  public:
    // Get the fully qualified name of the template
    String GetFullName(const Array<Constant>& templateArguments);

    // The callback we use to instantiate the template (user provided)
    InstantiateTemplateDelegate Delegate;

    // The names of the template arguments
    Array<TemplateParameter> TemplateParameters;

    // The actual base name of the template
    String TemplateBaseName;
  };

  // A library stores all the types and 
  class ZeroShared Library
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Friends
    friend class LibraryBuilder;
    friend class Syntaxer;
    
    // Destructor
    ~Library();

    // Clears all components from all types within this library
    // This is implicit called by the destructor, however it may be useful to call this method
    // to clear any components that aren't from types defined in our library
    void ClearComponents();

    // For any native type wihin this library, we will generate stub code for that native type
    // We don't do this by default (to save memory) but we will generate and store the stub code on demand
    void GenerateDefinitionStubCode();

    // Either returns the special custom namespace name for plugins, or
    // if that name is empty it will return the library name
    String GetPluginNamespace();

    // The name of the library, used for debug purposes
    String Name;

    // If this library is ever used to generate a C++ header, then
    // this is the namespace that all generated code will go inside
    String NamespaceForPlugins;

    // The source code this library was built from (generally the result of a Project)
    // If this library is native / build via generation, this array will be empty
    // If a type is compiled from script, it should point back at the script it resulted from
    // Code locations also reference the index into this array
    Array<CodeEntry> Entries;

    // Store all the types compiled in this library
    BoundTypeMap BoundTypes;

    // Any types that we own that are non named types (delegate, qualifieds, etc)
    Array<Type*> OwnedTypes;

    // Store the array of bound functions
    FunctionArray OwnedFunctions;

    // Store any variable created by the functions
    Array<Variable*> OwnedVariables;

    // Store any properties created by the types (includes members)
    Array<Property*> OwnedProperties;

    // All the plugins that were loaded when this library was created
    Array<UniquePointer<Plugin> > OwnedPlugins;

    // Types sorted with base classes coming before derived classes
    // Useful for code generation with languages that require
    // base classes to be defined prior to derived classes (e.g. C++)
    // We especially use this in plugin stub code generation
    Array<BoundType*> TypesInDependencyOrder;

    // This map controls how we extend types with functions without using inheritance
    // We map a type guid to the functions we extend it with
    FunctionExtensionMap StaticExtensionFunctions;
    FunctionExtensionMap InstanceExtensionFunctions;

    // This map controls how we extend types with properties without using inheritance
    // We map a type guid to the properties we extend it with
    GetterSetterExtensionMap StaticExtensionGetterSetters;
    GetterSetterExtensionMap InstanceExtensionGetterSetters;

    // All the string literals we contain (effectively the string pool)
    StringArray StringLiterals;

    // Callbacks that we try and call to handle template
    HashMap<String, InstantiateTemplateInfo> TemplateHandlers;

    // Whether or not we generated stub code (typically for 'go to definition' features)
    bool GeneratedDefinitionStubCode;

    // A pointer to any data the user wants to attach
    mutable const void* UserData;

    // Any user data that cant simply be represented by a pointer
    // Data can be written to the buffer and will be properly destructed
    // when this object is destroyed (must be read in the order it's written)
    mutable DestructibleBuffer ComplexUserData;

    // Whether this library was built in tolerant mode or not
    bool TolerantMode;

    // Whether this library came from a plugin
    bool CreatedByPlugin;

  private:

    // Constructor
    Library();

    // Computes types in base to derived order (done typically in CreateLibrary or in the Syntaxer)
    void ComputeTypesInDependencyOrderOnce();

    // Not copyable
    ZilchNoCopy(Library);

    // An intrusive reference count for memory handling
    ZilchRefLink(Library);
  };

  // Compute all the types from a list of libraries in dependency order
  void ComputeTypesInDependencyOrder
  (
    const Array<LibraryRef>& libraries,
    HashSet<LibraryRef>& outLibrarySet,
    Array<BoundType*>& outOrderedTypes
  );

  // For some reason we encounter linker errors when exporting a dll
  // unless we use explicit instantiation for these types (related to the .def file)
  template class Ref<Library>;

  // A hashing policy for the delegate type set
  class ZeroShared DelegateTypePolicy
  {
  public:
    // Hashing operator
    size_t operator() (DelegateType* type) const;

    // Comparison operator
    bool Equal(DelegateType* a, DelegateType* b) const;
  };

  // Make a hash set using the above policy
  typedef HashSet<DelegateType*, DelegateTypePolicy>  DelegateTypeSet;
  typedef DelegateTypeSet::range                      DelegateTypeRange;

  // Specifies how we want to check / verify token names in the library builder
  namespace TokenCheck
  {
    enum Enum
    {
      None = 0,
      IsUpper = 1,
      IsLower = 2,
      Asserts = 4,
      RemoveOuterBrackets = 8,
      NoMultipleInvalidCharacters = 16,
      IgnoreScopeResolution = 32
    };

    typedef unsigned Flags;
  }

  // An error code for how we instantiate templates
  namespace TemplateResult
  {
    enum Enum
    {
      // We succeeded at instantiating the template
      Success,

      // A template instantiator by the same name could not be found
      FailedNameNotFound,

      // The template instantiator provided by the user failed to instantiate a template
      FailedInstantiatorDidNotReturnType,

      // We failed to instantiate the template because we provided the wrong number of arguments
      FailedInvalidArgumentCount,

      // We failed to instantiate the template because we provided an invalid argument
      FailedArgumentTypeMismatch
    };
  }

  // What we return from instantiating a template
  class ZeroShared InstantiatedTemplate
  {
  public:
    // Constructor
    InstantiatedTemplate();

    // The type of the template (or null if it failed)
    BoundType* Type;

    // The number of arguments expected for this template
    size_t ExpectedArguments;

    // The result (or an error code for why we couldn't instantiate the template)
    TemplateResult::Enum Result;
  };

  // Create an array of types
  #define ZilchTypes(...) Array<Type*>(ZeroInit, __VA_ARGS__)

  // Create an array of constants
  #define ZilchConstants(...) Array<Constant>(ZeroInit, __VA_ARGS__)

  // Create an array of libraries
  #define ZilchLibraries(...) Array<LibraryRef>(ZeroInit, __VA_ARGS__)

  // Helper functions for generating parameter arrays (used in library building)
  ZeroShared ParameterArray OneParameter(Type* type);
  ZeroShared ParameterArray OneParameter(Type* type, StringParam name);
  ZeroShared ParameterArray TwoParameters(Type* type);
  ZeroShared ParameterArray TwoParameters(Type* type1, Type* type2);
  ZeroShared ParameterArray TwoParameters(Type* type, StringParam name1, StringParam name2);
  ZeroShared ParameterArray TwoParameters(Type* type1, StringParam name1, Type* type2, StringParam name2);
  ZeroShared ParameterArray ThreeParameters(Type* type);
  ZeroShared ParameterArray ThreeParameters(Type* type1, Type* type2, Type* type3);
  ZeroShared ParameterArray ThreeParameters(Type* type, StringParam name1, StringParam name2, StringParam name3);
  ZeroShared ParameterArray ThreeParameters(Type* type1, StringParam name1, Type* type2, StringParam name2, Type* type3, StringParam name3);
  ZeroShared ParameterArray FourParameters(Type* type);
  ZeroShared ParameterArray FourParameters(Type* type, StringParam name1, StringParam name2, StringParam name3, StringParam name4);
  ZeroShared ParameterArray FiveParameters(Type* type);
  ZeroShared ParameterArray FiveParameters(Type* type, StringParam name1, StringParam name2, StringParam name3, StringParam name4, StringParam name5);

  class ZeroShared LibraryBuilder
  {
  public:
    // Friends
    friend class Core;
    friend class CodeGenerator;
    friend class Syntaxer;
    friend class Module;

    // Constructor that takes the name of the library we want to build
    LibraryBuilder(StringParam name, StringParam namespaceForPlugins = String());

    // Get the name fo the library we are building
    String GetName();

    // Set the original source code entries that this library is being built from
    void SetEntries(const Array<CodeEntry>& entries);

    // Returns a pointer to the plugin if it is able to be loaded, or null if it failed to load
    // The plugin's lifetime will be associated with the library we are building (deleted when it dies)
    // All plugins loaded should have the '.zilchPlugin' extension (a shared object or dynamic linked library)
    // It may fail to load if the path specified isn't accessible, isn't a valid shared library,
    // or doesn't export the CreateZilchPlugin function
    // Loading a plugin will attempt to make a local/temporary copy so that dyanmic reloading can be done (on certain platforms)
    // This ideally prevents our program from locking the plugin file
    Plugin* LoadPlugin(Status& status, StringParam pluginFile, void* userData = nullptr);

    // Add a function to the library and bound type (pass nullptr for thisType if the function is static)
    Function* AddBoundFunction
    (
      BoundType* owner,
      StringParam name,
      BoundFn function,
      const ParameterArray& parameters,
      Type* returnType,
      FunctionOptions::Flags options,
      NativeVirtualInfo nativeVirtual = NativeVirtualInfo()
    );

    // Add a function to the library as an extension of another type (pass nullptr for thisType if the function is static)
    Function* AddExtensionFunction
    (
      BoundType* forType,
      StringParam name,
      BoundFn function,
      const ParameterArray& parameters,
      Type* returnType,
      FunctionOptions::Flags options
    );

    // Add an already created function to the library as an extension of another type
    void AddRawExtensionFunction(Function* function);
    
    // Add a pre-constructor to the library and bound type
    Function* AddBoundPreConstructor(BoundType* owner, BoundFn function);
    
    // Add a constructor to the library and bound type
    Function* AddBoundConstructor(BoundType* owner, BoundFn function, const ParameterArray& parameters);
    
    // Add a default constructor to the library and bound type
    Function* AddBoundDefaultConstructor(BoundType* owner, BoundFn function);
    
    // Add a destructor to the library and bound type
    Function* AddBoundDestructor(BoundType* owner, BoundFn function);

    // Add a member to the library and bound type
    Field* AddBoundField(BoundType* owner, StringParam name, Type* type, size_t offset, MemberOptions::Flags options);

    // Add a property to the library and bound type
    GetterSetter* AddBoundGetterSetter(BoundType* owner, StringParam name, Type* type, BoundFn set, BoundFn get, MemberOptions::Flags options);

    // Adds an enumeration value to the bound type
    GetterSetter* AddEnumValue(BoundType* owner, StringParam name, Integer value);

    // Add a property to the library as an extension of another type
    GetterSetter* AddExtensionGetterSetter(BoundType* forType, StringParam name, Type* type, BoundFn set, BoundFn get, MemberOptions::Flags options);

    // Add a sends event declaration to a type
    SendsEvent* AddSendsEvent(BoundType* forType, StringParam name, BoundType* sentType, StringParam description = String());

    // Attempts to instantiate a template (or returns a failure error code)
    InstantiatedTemplate InstantiateTemplate(StringParam baseName, const Array<Constant>& arguments, const LibraryArray& fromLibraries);

    // Add a function to the library (pass nullptr for thisType if the function is static)
    Function* CreateRawFunction
    (
      BoundType* owner,
      String name,
      BoundFn function,
      const ParameterArray& parameters,
      Type* returnType,
      FunctionOptions::Flags options,
      NativeVirtualInfo nativeVirtual = NativeVirtualInfo()
    );
    
    // Add a pre-constructor to the library
    Function* CreateRawPreConstructor(BoundType* owner, BoundFn function);
    
    // Add a constructor to the library
    Function* CreateRawConstructor(BoundType* owner, BoundFn function, const ParameterArray& parameters);
    
    // Add a default constructor to the library
    Function* CreateRawDefaultConstructor(BoundType* owner, BoundFn function);

    // Add a destructor to the library
    Function* CreateRawDestructor(BoundType* owner, BoundFn function);

    // Add a property to the library
    GetterSetter* CreateRawGetterSetter(BoundType* owner, String name, Type* type, BoundFn set, BoundFn get, MemberOptions::Flags options);

    // Add a field to the library
    Field* CreateRawField(BoundType* owner, String name, Type* type, size_t offset, MemberOptions::Flags options);

    // Add a variable to the library
    Variable* CreateRawVariable(Function* function, String name);


    // Add a string constant to the library (typically only done by the compiler)
    const String& AddStringLiteral(cstr text);
    
    // Add a string constant to the library (typically only done by the compiler)
    const String& AddStringLiteral(StringParam text);
    
    // Add a string constant to the library (typically only done by the compiler)
    const String& AddStringLiteral(StringRange text);

    
    // Get the type that reprsents one more level of indirection to the given type
    Type* ToHandleType(BoundType* type);

    // Get the type that reprsents one more level of indirection to the given type
    IndirectionType* ReferenceOf(BoundType* type);

    // Dereference the given type and get the resulting type from that
    BoundType* Dereference(IndirectionType* type);

    // Given a created delegate type, add or merge it into the delegate set (and return the proper type that we should be using...)
    DelegateType* GetDelegateType(const ParameterArray& parameters, Type* returnType);
    

    // Adds a callback to the library that will be called any time a user attempts to instantiate a template type with this name
    void AddTemplateInstantiator(StringParam baseName, InstantiateTemplateCallback callback, const StringArray& templateTypeParameters, void* userData);

    // Adds a callback to the library that will be called any time a user attempts to instantiate a template type with this name
    void AddTemplateInstantiator(StringParam baseName, InstantiateTemplateCallback callback, const Array<TemplateParameter>& templateParameters, void* userData);

    // Adds a bound type to the library builder
    BoundType* AddBoundType(StringParam name, TypeCopyMode::Enum copyMode, size_t size, size_t nativeVirtualCount = 0);

    // This is ONLY used by automatic C++ binding because the BoundType must already exist to solve dependency issues,
    // so the library builder cannot 'new' the BoundType itself
    void AddNativeBoundType(BoundType* type);
    void AddNativeBoundType(BoundType* type, BoundType* base, TypeCopyMode::Enum mode);

    // Find a named type by name, or return null
    BoundType* FindBoundType(StringParam name);

    // Create a library from the builder
    LibraryRef CreateLibrary();

    // After computing the sizes of all types, we can go back through any created delegates
    // and udpate their information (such as required stack space, parameter positions, etc)
    void ComputeDelegateAndFunctionSizesOnce();

    // Checks to see if an upper-identifier is valid
    static bool CheckUpperIdentifier(StringParam identifier);

    // Checks to see if a lower-identifier is valid
    static bool CheckLowerIdentifier(StringParam identifier);

    // Checks to see if a identifier is valid
    static bool CheckIdentifier(StringParam identifier, TokenCheck::Flags flags);

    // Attempts to fix an identifier to make it valid (this may make it collide with others)
    // Removed leading invalid characters until we reach a letter
    // The following invalid characters will be replaced with 'invalidCharacter' (default underscore)
    // Use '\0' for the invalid character to have it removed from the identifier
    // Identifiers will be forced to have the first letter uppercased or lowercased depending on the flags
    // If 'None' is specified, the identifier returned will not be modified to be upper/lower
    // If the string is empty, this will return either "empty" or "Empty"
    static String FixIdentifier(StringParam identifier, TokenCheck::Flags flags, char invalidCharacter = '_');

    // A pointer to any data the user wants to attach
    mutable const void* UserData;

    // Any user data that cant simply be represented by a pointer
    // Data can be written to the buffer and will be properly destructed
    // when this object is destroyed (must be read in the order it's written)
    mutable DestructibleBuffer ComplexUserData;

  public:
    // When types are added to the library, we set CreatableInScript to this value
    bool CreatableInScriptDefault;

    // The library builder always creates a single library
    // This should generally not be touched until the builder is finished building
    LibraryRef BuiltLibrary;

    // Store all the types compiled in this library
    BoundTypeMap BoundTypes;

  private:

    // A constant we can pass in as a get/set that signifies to not actually generate the get/set Function object
    static const BoundFn DoNotGenerate;

    // A constant that will generate a get/set Function object (can later be replaced)
    static const BoundFn NoOperation;

    // Generates the property getter/setter for fields
    // Must be called AFTER all sizes have been computed
    // Can be called multiple times (only checks fields with no get/set)
    void GenerateGetSetFields();

  private:

    // Store a set of all the delegate types (note that delegate types are shared, eg the same definition results in the same type)
    Array<DelegateType*> DelegateTypes;

    // Whether we computed the size of all the delegates and the function stack offsets
    // This only needs to be done once, generally after all types, members, and functions have been collected (after all syntaxing)
    bool ComputedDelegateAndFunctionSizes;

    // Not copyable
    ZilchNoCopy(LibraryBuilder);
  };

  // A module Contains many libraries
  class ZeroShared Module : public LibraryArray
  {
  public:

    // Constructor
    Module();

    // Finds a type in any of the libraries by name
    BoundType* FindType(StringParam name);

    // Link libraries together to create a single executable application
    ExecutableState* Link() const;

    // Builds a documentation object that Contains all the libraries, types, functions, etc
    DocumentationModule* BuildDocumentation();
    
    // Create an html/javascript file that defines our documentation
    String BuildDocumentationHtml();
    
    // Output documentation for the library into rst format
    void BuildDocumentationRst(StringParam directory);

  private:

    // Build documentation for a given type
    void BuildTypeDocumentation(BoundType* type, DocumentationType* docType);

    // Build documentation for a set of functions
    void BuildFunctionDocumentation(Array<DocumentationFunction*>& addTo, const FunctionMultiMap& functions);
    void BuildFunctionDocumentation(Array<DocumentationFunction*>& addTo, const FunctionArray& functions);

    // Build documentation for properties and members
    void BuildPropertyDocumentation(Array<DocumentationProperty*>& addTo, const FieldMap& fields);
    void BuildPropertyDocumentation(Array<DocumentationProperty*>& addTo, const GetterSetterMap& properties);
    void BuildPropertyDocumentation(Array<DocumentationProperty*>& addTo, const Property* property);

    // Builds a Json array of documented constructors
    void BuildJsonConstructors(JsonBuilder& json, const Array<DocumentationFunction*>& constructors, StringParam name);

    // Builds a Json array of documented functions
    void BuildJsonMethods(JsonBuilder& json, const Array<DocumentationFunction*>& functions, StringParam name);

    // Builds a Json array of documented properties
    void BuildJsonProperties(JsonBuilder& json, const Array<DocumentationProperty*>& properties, StringParam name);
  };

  namespace ForEachFunctorResult
  {
    enum Enum
    {
      Continue,
      Stop
    };
  }

  // The functor should return a ForEachFunctorResult::Enum and takes (Property*)
  template <typename Functor>
  ZeroSharedTemplate void ForEachExtensionGetterSetter(bool isStatic, const LibraryArray& libraries, BoundType* type, Functor functor)
  {
    // Loop through all the libraries
    for (size_t i = 0; i < libraries.Size(); ++i)
    {
      // We need to look up the entire heirarchy (the property could be on any base classes)
      BoundType* baseIterator = type;
      while (baseIterator != nullptr)
      {
        // Grab the current library
        const LibraryRef& library = libraries[i];

        // Get the guid of the type (this should be legal here since we've collected all members)
        GuidType guid = baseIterator->Hash();

        // Get the array of properties (may be empty)
        GetterSetterMap* properties = nullptr;
        
        // If we're resolving a static member
        if (isStatic)
          properties = library->StaticExtensionGetterSetters.FindPointer(guid);
        else
          properties = library->InstanceExtensionGetterSetters.FindPointer(guid);
        
        // If we got a valid array of properties...
        if (properties != nullptr)
        {
          ZilchForEach(GetterSetter* property, properties->Values())
          {
            if (functor(property) == ForEachFunctorResult::Stop)
              return;
          }
        }
          
        // Iterate to the next type
        baseIterator = baseIterator->BaseType;
      }
    }
  }

  // The functor should return a ForEachFunctorResult::Enum and takes (Function*)
  template <typename Functor>
  ZeroSharedTemplate void ForEachExtensionFunction(bool isStatic, const LibraryArray& libraries, BoundType* type, Functor functor)
  {
    // Loop through all the libraries
    for (size_t i = 0; i < libraries.Size(); ++i)
    {
      // We need to look up the entire heirarchy (the property could be on any base classes)
      BoundType* baseIterator = type;
      while (baseIterator != nullptr)
      {
        // Grab the current library
        const LibraryRef& library = libraries[i];

        // Get the guid of the type (this should be legal here since we've collected all members)
        GuidType guid = baseIterator->Hash();

        // Get the map of all functions
        FunctionMultiMap* functionMultiMap = nullptr;
        
        // If we're resolving a static function
        if (isStatic)
          functionMultiMap = library->StaticExtensionFunctions.FindPointer(guid);
        else
          functionMultiMap = library->InstanceExtensionFunctions.FindPointer(guid);
        
        // If we got a valid map from the type to the function overloads...
        if (functionMultiMap != nullptr)
        {
          // Walk through all overloads and functions...
          ZilchForEach(FunctionArray& overloads, functionMultiMap->Values())
          {
            ZilchForEach(Function* function, overloads.All())
            {
              if (function->OwningProperty != nullptr)
                continue;

              if (functor(function) == ForEachFunctorResult::Stop)
                return;
            }
          }
        }
          
        // Iterate to the next type
        baseIterator = baseIterator->BaseType;
      }
    }
  }

  // The functor should return a bool (true means stop, false means keep going) and takes (Property*)
  template <typename Functor>
  ZeroSharedTemplate void ForEachGetterSetter(bool isStatic, const LibraryArray& libraries, BoundType* type, Functor functor)
  {
    // Walk up the base class heirarchy
    BoundType* baseType = type;
    while (baseType != nullptr)
    {
      // Walk through all the properties on the bound type
      for (size_t i = 0; i < baseType->AllProperties.Size(); ++i)
      {
        // Grab the current property and check if its static or not
        Property* property = baseType->AllProperties[i];
        if (property->IsStatic == isStatic)
        {
          if (functor(property) == ForEachFunctorResult::Stop)
            return;
        }
      }

      // Iterate to the base class (could be null)
      baseType = baseType->BaseType;
    }
    
    // Now walk through extension properties
    ForEachExtensionGetterSetter(isStatic, libraries, type, functor);
  }

  // The functor should return a bool (true means stop, false means keep going) and takes (Function*)
  template <typename Functor>
  ZeroSharedTemplate void ForEachFunction(bool isStatic, const LibraryArray& libraries, BoundType* type, Functor functor)
  {
    // Walk up the base class heirarchy
    BoundType* baseType = type;
    while (baseType != nullptr)
    {
      // Walk through all the functions on the bound type
      for (size_t i = 0; i < baseType->AllFunctions.Size(); ++i)
      {
        // Grab the current function and check if its static or not
        Function* function = baseType->AllFunctions[i];
        if (function->OwningProperty == nullptr && function->IsStatic == isStatic)
        {
          if (functor(function) == ForEachFunctorResult::Stop)
            return;
        }
      }

      // Iterate to the base class (could be null)
      baseType = baseType->BaseType;
    }

    // Now walk through extension functions
    ForEachExtensionFunction(isStatic, libraries, type, functor);
  }
}

namespace Zero
{
  template class Array<Zilch::LibraryRef>;
  template class AllocationContainer<DefaultAllocator>;
}

#endif
