///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// UniquePointer takes sole ownership over a pointer and 
/// deletes it when is goes out of scope
template<typename Type>
class UniquePointer
{
public:
  UniquePointer(Type* type) { mPointer = type; }
  UniquePointer() { mPointer = nullptr; }
  ~UniquePointer() { Reset(); }

  UniquePointer(MoveReference<UniquePointer> other)
  {
    // Steal Pointer from rvalue
    mPointer = other->mPointer;
    other->mPointer = nullptr;
  }
  
  void operator=(MoveReference<UniquePointer> other)
  {
    if(this == &other)
      return;
    
    Reset();

    // Steal Pointer from rvalue
    mPointer = other.mPointer;
    other.mPointer = nullptr;
  }
  
#if defined(SupportsMoveSemantics) && false
  UniquePointer(UniquePointer&& other)
  {
    // Steal Pointer from rvalue
    mPointer = other.mPointer;
    other.mPointer = nullptr;
  }
  
  void operator=(UniquePointer&& other)
  {
    if(this == &other)
      return;
  
    Reset();
    // Steal Pointer from rvalue
    mPointer = other.mPointer;
    other.mPointer = nullptr;
  }
#else
  UniquePointer(const UniquePointer& otherConst)
  {
    UniquePointer& other = (UniquePointer&)otherConst;
    mPointer = other.mPointer;
    other.mPointer = nullptr;
  }
  
  void operator=(const UniquePointer& otherConst)
  {
    if(this == &otherConst)
      return;
  
    UniquePointer& other = (UniquePointer&)otherConst;

    Reset();
    // Steal Pointer from rvalue
    mPointer = other.mPointer;
    other.mPointer = nullptr;
  }
#endif

  Type* operator->()             { return mPointer;  }
  const Type* operator->() const { return mPointer;  }
  Type& operator*()              { return *mPointer; }
  const Type& operator*() const  { return *mPointer; }
  operator Type*()               { return mPointer;  }
  operator const Type*() const   { return mPointer;  }

  void operator=(Type* pointer)
  {
    Reset();
    mPointer = pointer;
  }

  // Reset pointer back to null
  void Reset()
  {
    if(mPointer)
    {
      delete mPointer;
      mPointer = nullptr;
    }
  }

  // Take ownership of pointer
  Type* Release()
  {
    Type* pointer = mPointer;
    mPointer = nullptr;
    return pointer;
  }

#if defined(SupportsMoveSemantics) && false
  // Can not copy UniquePointer
  UniquePointer(const UniquePointer& other);
  void operator=(const UniquePointer& other);
#endif

  Type* mPointer;
};

//------------------------------------------------------- Move Operator (String)
template<typename T>
struct MoveWithoutDestructionOperator<UniquePointer<T> >
{
  static inline void MoveWithoutDestruction(UniquePointer<T>* dest, UniquePointer<T>* source)
  {
    memcpy(dest, source, sizeof(UniquePointer<T>));
  }
};

template<typename type>
UniquePointer<type> AutoHandle(type* event)
{
  UniquePointer<type> handle(event);
  return handle;
}

template<typename type>
UniquePointer<type> MakeUnique()
{
  return UniquePointer<type>(new type());
}

template<typename type, typename param0>
UniquePointer<type> MakeUnique(const param0& p0)
{
  return UniquePointer<type>(new type(p0));
}

template<typename type, typename param0, typename param1>
UniquePointer<type> MakeUnique(const param0& p0, const param0& p1)
{
  return UniquePointer<type>(new type(p0, p1));
}

}
