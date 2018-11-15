/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_STUB_CODE_HPP
#define ZILCH_STUB_CODE_HPP

namespace Zilch
{
  // A helper for generating Zilch code (without implementation) for natively defined types
  // This code should be actually executable and can be used for 'Go To Definition' like features or for exporting
  // engine / application code to be used for auto-complete and more in an external editor
  // As we build stub code for native definitions, we need to keep track of all the code locations we've touched
  // so we can update the 'Code' portion of the location after we're finished generating stub code
  class ZeroShared StubCode
  {
  public:

    // Constructor
    StubCode();

    // A code builder that we store
    ZilchCodeBuilder Builder;
    
    // All the locations we've touched (including NameLocations)
    Array<CodeLocation*> NativeLocations;

    // If this is set we will set the Location and NameLocation portions of any member or type defined natively
    // This is used by library 'GenerateDefinitionStubCode' (be sure to set the 'GeneratedName' below)
    bool SetNativeLocations;

    // The name we the stub code (when we set the Origin on the CodeLocation for native
    String GeneratedOriginOrName;

    // Stringifies the builder and updates native locations if 'SetNativeLocations' is set
    String Finalize();
    
    // Generates a class/struct definition for this type based on all the members, properties, and functions
    // This will also output comments above the members (if a 'Description' is provided) as well as outputting
    // the proper attributes, get/sets, parameter names / types, etc
    // The stub version should compile (barring name conflicts) and should be usable as a placeholder for C++ bindings
    // It is also useful for when the user wants to view documentation / visualize a class that they don't have the code for (especially native)
    void Generate(BoundType* type);

    // Generates stub code for an array of functions
    void Generate(FunctionArray& functions);

    // Generates stub code for a function
    void Generate(Function* function);

    // Generates stub code for a sends statement
    void Generate(SendsEvent* sends);

    // Generates stub code for a property or field
    void Generate(Property* property);

    void GenerateHeader(Function* function);
    void GenerateHeader(Property* property);
    // Generates the header for a documented object including attributes, word-wrapped comments, etc
    void GenerateHeader(ReflectionObject* object);
    // Generates the header for a documented object including attributes, word-wrapped comments, etc...
    // Attributes are taken as a separate array as some 'attributes' are not on the reflection object
    // but are instead properties (IsStatic). The derived type should add any extra attributes if necessary.
    void GenerateHeader(ReflectionObject* object, Array<Attribute>& attributes);

    // Internally used to track native location starts
    void StartNativeLocation(CodeLocation& location);
    void EndNativeLocation(CodeLocation& location);
  };
}

#endif
