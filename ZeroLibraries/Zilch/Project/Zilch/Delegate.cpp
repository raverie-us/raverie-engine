/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  Delegate::Delegate() :
    BoundFunction(nullptr)
  {
    ZilchErrorIfNotStarted(Delegate);
  }

  //***************************************************************************
  bool Delegate::operator==(const Delegate& rhs) const
  {
    // First start by making sure the function indices are the same
    if (this->BoundFunction != rhs.BoundFunction)
      return false;
    
    // We also need to compare the handles
    return this->ThisHandle == rhs.ThisHandle;
  }

  //***************************************************************************
  bool Delegate::operator==(Zero::NullPointerType) const
  {
    return this->IsNull();
  }
  
  //***************************************************************************
  bool Delegate::operator!=(const Delegate& rhs) const
  {
    return !(*this == rhs);
  }

  //***************************************************************************
  bool Delegate::operator!=(Zero::NullPointerType) const
  {
    return !this->IsNull();
  }
  
  //***************************************************************************
  size_t Delegate::Hash() const
  {
    ErrorIf(this->BoundFunction->Hash == 0, "The function's hash was most likely not computed");

    // Hash the function first
    int hash = (int)(this->BoundFunction->Hash * 735977940532462813);

    // Combine that hash with the handle hash
    return hash ^ this->ThisHandle.Hash();
  }

  //***************************************************************************
  bool Delegate::IsNull() const
  {
    // If we have no function, this delegate is null
    if (this->BoundFunction == nullptr)
      return true;

    // We have a valid function, if it is static then this cannot possibly be null
    if (this->BoundFunction->IsStatic)
      return false;

    // This must be an instance delegate, so finally return that it is null if the 'this' handle is null
    return this->ThisHandle.IsNull();
  }

  //***************************************************************************
  bool Delegate::IsNotNull() const
  {
    return !IsNull();
  }

  //***************************************************************************
  Any Delegate::Invoke(const Array<Any>& arguments)
  {
    ReturnIf(this->BoundFunction == nullptr, Any(),
      "Attempted to invoke a null delegate");

    // This is unsafe, but we use a static assert (after the ArrayClass, offsetof with 0) to guarantee it will work
    ArrayClass<Any>* zilchArray = reinterpret_cast<ArrayClass<Any>*>(const_cast<Array<Any>*>(&arguments));
    return this->BoundFunction->Invoke(this->ThisHandle, zilchArray);
  }
}
