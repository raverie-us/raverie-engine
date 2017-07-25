/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_WRAPPER_HPP
#define ZILCH_WRAPPER_HPP

namespace Zilch
{
  // Generates a wrapper around a BoundType and allows us to change who that class 'inherits' from as
  // well as allowing us to generate pre/post callbacks for every Property Get/Set and Function call
  // This class also acts as a component that is attached to the wrapper's BoundType (you should inherit from it)
  // There should only ever be one wrapper per wrapped type
  class ZeroShared Wrapper : public IZilchObject
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Generates a new type that wraps the passed in type (with an optional base that that our new type will inherit from)
    // The newly generated type will always be a class
    // All Fields will be turned into GetterSetters, and all GetterSetters will be directly wrapped with our own GetterSetters
    // Functions will also be wrapped and the return value passed through
    // Currently only types with default constructors will work (the generated wrapper also will have a default constructor)
    // In the future the generated type will receive all the same constructors (and will forward the parameters to the wrapped type upon construction)
    // Note: The base class MUST have a default constructor, as it is the only constructor that will be called
    static BoundType* Generate(LibraryBuilder& builder, Wrapper* wrapper, BoundType* innerType, BoundType* outerBaseType = nullptr);

    // Note that in PreFunction, the value will not have been returned yet (not invoked for constructors or destructors)
    virtual void PreFunction(Call& call);
    virtual void PostFunction(Call& call);

  private:

    static void ConstructorCall(Call& call, ExceptionReport& report);
    static void FunctionCall(Call& call, ExceptionReport& report);

  public:

    BoundType* InnerType;
    BoundType* OuterType;
  };
}

#endif
