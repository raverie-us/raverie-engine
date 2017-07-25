/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_STATIC_LIBRARY_HPP
#define ZILCH_STATIC_LIBRARY_HPP

namespace Zilch
{
  namespace BuildState
  {
    enum Enum
    {
      NotBuilt,
      Building,
      Built
    };
  }

  // This is the interface we expect to see when creating any static
  // library / using the binding system. The only other function that
  // is expected is a static 'GetLibrary()' function which returns a
  // reference to your own type
  class ZeroShared StaticLibrary
  {
  public:
    // Friends
    friend class StaticLibraries;

    // Check if this library was built, is in the process of being built, or hasn't started building yet
    BuildState::Enum GetBuildState();

    // Whether we can currently build types or not (used in ZilchTypeId and Binding)
    virtual bool CanBuildTypes();

    // Get the library that we represent
    LibraryRef GetLibrary();

    // Get a reference to the library builder
    LibraryBuilder* GetBuilder();

    // Builds the library once and builds all dependencies
    virtual void BuildLibrary();

    // Clears the library, freeing all the types
    void ClearLibrary();

  public:

    // The name of the library
    String Name;

  protected:

    // Constructor
    // The namespace is optional and only used for plugins
    StaticLibrary(StringParam name, StringParam namespaceForPlugins = String());

    // Declare a virtual destructor
    virtual ~StaticLibrary();

    // Setup any custom binding for the library
    virtual void SetupBinding(LibraryBuilder& builder);

    // Any dependencies these libraries have upon other libraries
    // For example, many libraries depend upon the 'Core' library to be built before
    // themselves, so that primitive types such as 'Integer' and 'Real' exist
    Array<StaticLibrary*> Dependencies;

  private:

    // Tells us whether or not we built this library
    BuildState::Enum Build;

    // The library builder we use to create the static library
    LibraryBuilder* Builder;

    // The built library (only built after all the static classes get added to it)
    LibraryRef Library;
  };

#define ZilchDeclareStaticLibraryInternals(Name, Namespace, ...)    \
    /* Needed for binding macros work with this library */            \
    typedef Name ZilchLibrary;                                        \
    Name() :                                                          \
    ZZ::StaticLibrary(#Name, Namespace)                               \
    {                                                                 \
      ZilchDependency(ZZ::Core)                                       \
      __VA_ARGS__                                                     \
    }                                                                 \
    static Name* Instance;                                            \
    static void InitializeInstance()                                  \
    {                                                                 \
      ReturnIf(Instance != nullptr, ,                                 \
        "Can't initialize a static library more than once");          \
      Instance = new Name();                                          \
    }                                                                 \
    static void Destroy()                                             \
    {                                                                 \
      delete Instance;                                                \
      Instance = nullptr;                                             \
    }                                                                 \
    static Name& GetInstance()                                        \
    {                                                                 \
      ErrorIf(Instance == nullptr,                                    \
        "Attempted to get an uninitialized singleton static library");\
      return *Instance;                                               \
    }                                                                 \
    static ZZ::LibraryRef GetLibrary()                                \
    {                                                                 \
      return GetInstance().StaticLibrary::GetLibrary();               \
    }                                                                 \
    static ZZ::LibraryBuilder* GetBuilder()                           \
    {                                                                 \
      return GetInstance().StaticLibrary::GetBuilder();               \
    }                                                                 \
    static void BuildStaticLibrary()                                  \
    {                                                                 \
      InitializeInstance();                                           \
      return GetInstance().StaticLibrary::BuildLibrary();             \
    }                                                                 \
    void SetupBinding(ZZ::LibraryBuilder& builder) override;

  // Create a static library that we can use in C++ binding
  // Use this in a header (preferrably in a namespace)
  // After specifying the name, you can pass in a variable number of
  // ZilchDependency(Namespace::OtherLibrary) to mark dependencies
  // upon other static libraries
  // All libraries declared with this macro implicitly add a dependency on Core
  #define ZilchDeclareStaticLibrary(Name, Namespace, Linkage, ...)              \
    class Linkage Name : public ZZ::StaticLibrary                               \
    {                                                                           \
    public:                                                                     \
      ZilchDeclareStaticLibraryInternals(Name, Namespace, __VA_ARGS__)          \
    };

  // When we don't care about a namespace for a static library
  #define ZilchNoNamespace ZZ::String()
  
  // This allows us to define an initialize all the types that belong within our library
  // This should be put in a translational unit (cpp) that can see all of the types that need to be registered
  // We opted for this instead of automatic or pre-main initialization because there were issues on some compilers
  // that would optimize out pre-main code if it was never referenced anywhere (but would otherwise be available to script)
  #define ZilchDefineStaticLibrary(Name)                                        \
    Name* Name::Instance = nullptr;                                             \
    void Name::SetupBinding(ZZ::LibraryBuilder& builder)

  // Used in declaring dependencies upon other static libraries
  #define ZilchDependency(Library) this->Dependencies.PushBack(&Library::GetInstance());
}

#endif
