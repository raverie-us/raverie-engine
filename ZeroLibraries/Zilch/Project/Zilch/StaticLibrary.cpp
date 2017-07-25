/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  StaticLibrary::StaticLibrary(StringParam name, StringParam namespaceForPlugins) :
    Name(name),
    Build(BuildState::NotBuilt)
  {
    // Create a library builder
    this->Builder = new LibraryBuilder(name, namespaceForPlugins);
  }

  //***************************************************************************
  StaticLibrary::~StaticLibrary()
  {
    delete this->Builder;
  }

  //***************************************************************************
  BuildState::Enum StaticLibrary::GetBuildState()
  {
    return this->Build;
  }
  
  //***************************************************************************
  bool StaticLibrary::CanBuildTypes()
  {
    return this->Build == BuildState::Building;
  }

  //***************************************************************************
  LibraryRef StaticLibrary::GetLibrary()
  {
    ZilchErrorIfNotStarted(StaticLibrary);

    // Attempt to build the library (it may already be building...)
    this->BuildLibrary();

    // If the library has not been built yet...
    ErrorIf
    (
      this->Library == nullptr,
      "The static library '%s' is currently in the process of being built and cannot be grabbed!",
      this->Name.c_str()
    );

    return this->Library;
  }

  //***************************************************************************
  LibraryBuilder* StaticLibrary::GetBuilder()
  {
    ZilchErrorIfNotStarted(StaticLibrary);

    // If the library was already built (the builder is destroyed...)
    if (this->Builder == nullptr)
    {
      // Throw an error
      Error("The static library '%s' was already built, and therefore its builder cannot be accessed.",
        this->Library->Name.c_str());
      return nullptr;
    }

    return this->Builder;
  }

  //***************************************************************************
  void StaticLibrary::SetupBinding(LibraryBuilder& builder)
  {
    // The base class does nothing, this is just to allow users to do their own setup
  }

  //***************************************************************************
  void StaticLibrary::BuildLibrary()
  {
    // Don't bother doing anything if we're already building or built
    if (this->Build != BuildState::NotBuilt)
      return;

    // Let the library know it's in the process of building
    // This is generally used to provide clear error messages in ZilchTypeId
    this->Build = BuildState::Building;

    // Build all dependent libraries
    ZilchForEach(StaticLibrary* dependency, this->Dependencies)
    {
      // Its ok to call BuildLibrary more than once
      dependency->BuildLibrary();
    }

    // Let the binding know we're currently building this library
    NativeBindingList::SetBuildingLibraryForThisThread(this->Builder->BuiltLibrary);

    // Before we run initializers, let the user setup anything they want
    this->SetupBinding(*this->Builder);

    // Create the library and store it for everyone to be able to access
    this->Library = this->Builder->CreateLibrary();

    // Destroy the library builder since it's no longer needed
    delete this->Builder;
    this->Builder = nullptr;

    // Let the binding know we're no longer building a library
    NativeBindingList::SetBuildingLibraryForThisThread(nullptr);

    // Let the library know it's done being built
    this->Build = BuildState::Built;
  }

  //***************************************************************************
  void StaticLibrary::ClearLibrary()
  {
    ErrorIf(this->Build != BuildState::Built,
      "It is not valid to shutdown a library that has not been built");

    this->Library = nullptr;
  }
}
