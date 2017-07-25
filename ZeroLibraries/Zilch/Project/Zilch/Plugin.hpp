/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_PLUGIN_HPP
#define ZILCH_PLUGIN_HPP

namespace Zilch
{
  namespace Events
  {
    // We send this even on a BuildPlugin prior to any code being built
    // During this phase we can populate the event with any dependent libraries (such as our own StaticLibrary)
    ZilchDeclareEvent(PreBuild, BuildEvent);
  }

  // The signature of the CreateZilchPlugin function that we look for in the shared library
  typedef Plugin* (*CreateZilchPluginFn)();

  namespace BuildReason
  {
    enum Enum
    {
      FullCompilation,
      AutoComplete,
      DefinitionQuery
    };
  }

  // An event sent to plugins when we are building a project
  // This event allows plugins to populate their own built libraries as dependencies
  class ZeroShared BuildEvent : public EventData
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);
    BuildEvent();

    // Finds a library from the dependencies by name
    LibraryRef FindLibrary(StringParam name);

    // Called when we want to add a library generated from a plugin
    // Note: This MUST be called because it affects how destruction of plugin generated types works
    void AddPluginDependency(LibraryRef library);

    Project* BuildingProject;
    Module* Dependencies;
    LibraryBuilder* Builder;
    BuildReason::Enum Reason;
  };
  
  // An event that plugins may broadcast to each other
  // This is meant to facilitate communication between plugins in a generic way
  // Without necessarily sharing code. An alternate solution
  // would be to mirror event structures on both sides of the plugins
  // Plugin events can be sent on any event handler, however the PluginEvent
  // also provides a global/static event handler to communicate globally
  class ZeroShared PluginEvent : public EventData
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);
    PluginEvent();

    // The following values may be used for any purpose
    // They are all initialized to null / zero / empty
    String        mString;
    const char*   mCString;
    Array<String> mStrings;
    long long     mInteger;
    float         mFloat;
    double        mDouble;
    bool          mBool;
    void*         mPointer;
    Handle        mHandle;
    Delegate      mDelegate;
    Any           mAny;

    // Useful for when you need to know if another plugin handled the event
    bool mHandled;

    // Any time plugins wish to send events globally, they may listen
    // on this object and send events via this object to all other plugins
    // Note: This should only be used by the main thread
    static EventHandler GlobalEvents;
  };

  // This class must be implemented to create a custom plugin (also implement the below function)
  // This class will be created upon building a library, and will be destroyed whenever the library itself dies
  class ZeroShared Plugin : public EventHandler
  {
  public:
    friend class Project;
    friend class Library;
    friend class LibraryBuilder;

    Plugin();
    virtual ~Plugin();

    // Invoked when we fully load an initialize the plugin and are building a project
    // At this time, the plugin may add dependencies to the project that is
    // currently building. The plugin may also build libraries at this time
    virtual void PreBuild(BuildEvent* event);

    // An opportunity to run any one time initialization logic for a plugin
    // This will only run upon a full compilation of Zilch (not when AutoComplete or Definition info is queried)
    virtual void Initialize(BuildEvent* event);

    // An opportunity to run any one time uninitialization logic for a plugin
    // This will only run upon a full compilation of Zilch (not when AutoComplete or Definition info is queried)
    virtual void Uninitialize();

  public:

    // User data passed by the call to LoadPlugin (or specified inside of a Project when adding a plugin)
    void* UserData;

  private:
    // Whether or not we ran 'Initialize' (not when AutoComplete or Definition info is queried)
    bool FullCompilationInitialized;
  };

  // Construct a handle for plugin purposes
  // This will use the currently AllocatingType on ExecutableState, as well as a PointerManager
  ZeroShared void CreateAllocatingHandle(Handle& handle, void* pointer);

  // Mangles function names so they become unique and do not collide with overloads
  class ZeroShared NameMangler
  {
  public:
    // Mangle all functions from all types within a library
    void MangleLibrary(LibraryRef library);

    // Get a unique id from a delegate type
    GuidType GetDelegateTypeId(DelegateType* type);

    // Find a function by mangled name and asserts if it cannot find it (function name is intentionally a cstr)
    Function* FindFunction(GuidType functionHash, const char* functionName, StringParam typeName);

  private:
    HashMap<DelegateType*, GuidType, DelegateTypePolicy> DelegateTypeToId;
    HashMap<GuidType, Function*> HashToFunction;
  };

  // Finds a bound type in the provided library with the specified name and asserts if it cannot find it (type name is intentionally a cstr)
  ZeroShared BoundType* FindLibraryType(LibraryRef library, const char* name);

  // A helper used to patch the specified type (used in stub code generation, also asserts)
  // Replaces the static bound type with the bound type found in the provided library with the specified name
  // Returns the patched bound type, else nullptr
  template <typename T>
  ZeroSharedTemplate BoundType* PatchLibraryType(LibraryRef library, const char* name)
  {
    BoundType*& outputType = ZilchTypeId(T);

    BoundType* foundType = FindLibraryType(library, name);
    if(foundType != nullptr)
    {
      // Our static bound type was never initialized? (We created this bound type?)
      if(outputType->IsInitialized() == false)
      {
        // We are responsible for releasing this bound type before replacing it
        delete outputType;
      }

      outputType = foundType;
    }

    return foundType;
  }

  class ZeroShared NativeName
  {
  public:
    NativeName();
    NativeName(StringParam className, StringParam parameterName, StringParam returnName);
    String Class;
    String Parameter;
    String Return;
    String Base;
  };

  // Generates C++ code that can be used within plugins which will allow users to
  // conveniently make calls to types bound to Zilch (or even Zilch scripts themselves)
  class ZeroShared NativeStubCode
  {
  public:
    NativeStubCode();

    // Given a library this will generate a C++ stub header and cpp file
    // The user must call HookUpX(BuildEvent) where X is the name of your library
    // The build event will come from the plugin PreBuild event
    // All libraries given here must be within the same namespace
    // Returns the namespace name that was used by all libraries
    String Generate(const LibraryArray& libraries);
    String Generate(LibraryParam library);

  public:

    // If this is set, then we assume we're going to include a precompiled
    // header of this name (that also includes our header)
    // The precompiled header must also defined 'ZeroImportDll'
    // Note: If this is not set to a define, then you MUST use quotes
    String PrecompiledHeader;

    // Text appended to the hpp file (at the top, middle after the include guards, and bottom)
    String HppHeader;
    String HppMiddle;
    String HppFooter;
    
    // Text appended to the cpp file (at the top, middle after the include guards, and bottom)
    String CppHeader;
    String CppMiddle;
    String CppFooter;

    // If you want to have a custom define emitted within the hpp's public section of a class
    // This is useful if you utilize the 'HppHeader' section above to create a custom define that adds members
    HashMap<BoundType*, String> CustomClassHeaderDefines;

    // These will be filled out with C++ code when we call 'Generate'
    String Hpp;
    String Cpp;

  private:

    NameMangler Mangler;
    HashMap<Type*, NativeName> TypeToCppName;
    HashSet<LibraryRef> LibrarySet;
    Array<BoundType*> TypesInDependencyOrder;
    String Namespace;
    const LibraryArray* Libraries;

    void WriteParameters(ZilchCodeBuilder& builder, DelegateType* delegateType);
    void WriteDescription(ZilchCodeBuilder& builder, ReflectionObject* object);
    
    NativeName GetCppTypeName(Type* type);
    String GenerateHpp();
    String GenerateCpp();
  };

  // Use this in a single translational unit (cpp) that can see the declaration of the above plugin
  #define ZilchDefinePluginInterface(PluginClass)                                                   \
    ZeroExportC long GetZilchPluginVersion() { return 0; }                                          \
    ZeroExportC Zilch::Plugin* CreateZilchPlugin() { return new PluginClass(); }
  
  // This is a common macro for implementing a single static library and plugin in one
  #define ZilchDeclareStaticLibraryAndPlugin(LibraryName, PluginName, ...)                          \
    ZilchDeclareStaticLibrary(LibraryName, ZilchNoNamespace, ZeroNoImportExport, __VA_ARGS__);      \
    class PluginName : public ::Zilch::Plugin                                                       \
    {                                                                                               \
    public:                                                                                         \
      void PreBuild(::Zilch::BuildEvent* event) override;                                           \
      void Initialize(::Zilch::BuildEvent* event) override;                                         \
      void Uninitialize() override;                                                                 \
    };

  // This is a common macro for implementing a single static library and plugin in one
  #define ZilchDefineStaticLibraryAndPlugin(LibraryName, PluginName, ...)                           \
    ZilchDefinePluginInterface(PluginName);                                                         \
    void PluginName::PreBuild(::Zilch::BuildEvent* event)                                           \
    {                                                                                               \
      __VA_ARGS__;                                                                                  \
      ::Zilch::LibraryRef library = LibraryName::GetInstance().GetLibrary();                        \
      library->UserData = this->UserData;                                                           \
      event->AddPluginDependency(library);                                                          \
    }                                                                                               \
    ZilchDefineStaticLibrary(LibraryName)

  #define ZilchDependencyStub(Name) HookUp##Name(event);

  // A base class for all reference native stubs that cannot be constructed,
  // destructed, or copied in any way (basically never derferenced)
  // WARNING: Do NOT add a virtual function to this class as it will destroy the layout of plugin stub classes
  class ZeroShared ReferenceType
  {
  public:
    ReferenceType(NoType) {}
    ZilchNoCopy(ReferenceType);
  };

  // A base class for all value types
  // WARNING: Do NOT add a virtual function to this class as it will destroy the layout of plugin stub classes
  class ZeroShared ValueType
  {
  public:
  };

  // A helper that returns the default constructed value of
  // any type (including POD C++ types, like int or int*)
  template <typename T>
  ZeroSharedTemplate T Default()
  {
    return T();
  }

  // All stub generated C++ code uses external binding with this library
  // This library pretends to be the static library that the type belongs to, but it will be redirected
  // when we hook-up types through the dll/so-boundary
  class ZeroShared PluginStubLibrary : public StaticLibrary
  {
  public:
    // Grab the singleton instance of the plugin stub
    static PluginStubLibrary& GetInstance();

    // This is a special case where we always allow plugins to "build" so that we can do type hookups
    bool CanBuildTypes() override;

  private:
    PluginStubLibrary();
    ~PluginStubLibrary();
    ZilchNoCopy(PluginStubLibrary);
  };

}

#endif
