///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
//---------------------------------------------------------------------- Functor
class Functor
{
public:
  virtual Functor* Execute() = 0;
};

//------------------------------------------------- Static Function 0 Parameters
template <typename ReturnType>
class FunctorStatic0 : public Functor
{
public:
  Functor* Execute() override
  {
    mFunctionPointer();
    return this;
  }

  ReturnType (*mFunctionPointer)();
};

template <typename ReturnType>
Functor* CreateFunctor(ReturnType(*functionPointer)())
{
  ErrorIf(instance == nullptr);

  auto functor = new FunctorStatic0<ReturnType>();
  functor->mFunctionPointer = functionPointer;
  return functor;
}

//----------------------------------------------- Instance Function 0 Parameters
template <typename ReturnType, typename InstanceType>
class FunctorInstance0 : public Functor
{
public:
  Functor* Execute() override
  {
    (mInstance->*mFunctionPointer)();
    return this;
  }

  ReturnType (InstanceType::*mFunctionPointer)();
  InstanceType* mInstance;
};

template <typename ReturnType, typename InstanceType>
Functor* CreateFunctor(ReturnType (InstanceType::*functionPointer)(), InstanceType* instance)
{
  ErrorIf(instance == nullptr);

  auto functor = new FunctorInstance0<ReturnType, InstanceType>();
  functor->mFunctionPointer = functionPointer;
  functor->mInstance = instance;
  return functor;
}

//----------------------------------------- Instance Const Function 0 Parameters
template <typename ReturnType, typename InstanceType>
class FunctorInstanceConst0 : public Functor
{
public:
  Functor* Execute() override
  {
    (mInstance->*mFunctionPointer)();
    return this;
  }

  ReturnType (InstanceType::*mFunctionPointer)() const;
  InstanceType* mInstance;
};

template <typename ReturnType, typename InstanceType>
Functor* CreateFunctor(ReturnType (InstanceType::*functionPointer)() const, InstanceType* instance)
{
  ErrorIf(instance == nullptr);

  auto functor = new FunctorInstanceConst0<ReturnType, InstanceType>();
  functor->mFunctionPointer = functionPointer;
  functor->mInstance = instance;
  return functor;
}

//-------------------------------------------------- Static Function 1 Parameter
template <typename ReturnType, typename P0>
class FunctorStatic1 : public Functor
{
public:
  Functor* Execute() override
  {
    mFunctionPointer(mP0);
    return this;
  }

  ReturnType (*mFunctionPointer)(P0);
  P0 mP0;
};

template <typename ReturnType, typename P0>
Functor* CreateFunctor(ReturnType (*functionPointer)(P0), P0 p0)
{
  ErrorIf(instance == nullptr);

  auto functor = new FunctorStatic1<ReturnType, P0>();
  functor->mFunctionPointer = functionPointer;
  functor->mP0 = p0;
  return functor;
}

//------------------------------------------------ Instance Function 1 Parameter
template <typename ReturnType, typename InstanceType, typename P0>
class FunctorInstance1 : public Functor
{
public:
  Functor* Execute() override
  {
    (mInstance->*mFunctionPointer)(mP0);
    return this;
  }

  ReturnType (InstanceType::*mFunctionPointer)(P0);
  InstanceType* mInstance;
  P0 mP0;
};

template <typename ReturnType, typename InstanceType, typename P0>
Functor* CreateFunctor(ReturnType (InstanceType::*functionPointer)(P0), InstanceType* instance, P0 p0)
{
  ErrorIf(instance == nullptr);

  auto functor = new FunctorInstance1<ReturnType, InstanceType, P0>();
  functor->mFunctionPointer = functionPointer;
  functor->mInstance = instance;
  functor->mP0 = p0;
  return functor;
}

//------------------------------------------ Instance Const Function 1 Parameter
template <typename ReturnType, typename InstanceType, typename P0>
class FunctorInstanceConst1 : public Functor
{
public:
  Functor* Execute() override
  {
    (mInstance->*mFunctionPointer)(mP0);
    return this;
  }

  ReturnType (InstanceType::*mFunctionPointer)(P0) const;
  InstanceType* mInstance;
  P0 mP0;
};

template <typename ReturnType, typename InstanceType, typename P0>
Functor* CreateFunctor(ReturnType (InstanceType::*functionPointer)(P0) const, InstanceType* instance, P0 p0)
{
  ErrorIf(instance == nullptr);

  auto functor = new FunctorInstanceConst1<ReturnType, InstanceType, P0>();
  functor->mFunctionPointer = functionPointer;
  functor->mInstance = instance;
  functor->mP0 = p0;
  return functor;
}


//-------------------------------------------------- Static Function 2 Parameter
template <typename ReturnType, typename P0, typename P1>
class FunctorStatic2 : public Functor
{
public:
  Functor* Execute() override
  {
    mFunctionPointer(mP0, mP1);
    return this;
  }

  ReturnType (*mFunctionPointer)(P0, P1);
  P0 mP0;
  P1 mP1;
};

template <typename ReturnType, typename P0, typename P1>
Functor* CreateFunctor(ReturnType (*functionPointer)(P0, P1), P0 p0, P1 p1)
{
  ErrorIf(instance == nullptr);

  auto functor = new FunctorStatic2<ReturnType, P0, P1>();
  functor->mFunctionPointer = functionPointer;
  functor->mP0 = p0;
  functor->mP1 = p1;
  return functor;
}

//------------------------------------------------ Instance Function 2 Parameter
template <typename ReturnType, typename InstanceType, typename P0, typename P1>
class FunctorInstance2 : public Functor
{
public:
  Functor* Execute() override
  {
    (mInstance->*mFunctionPointer)(mP0, mP1);
    return this;
  }

  ReturnType (InstanceType::*mFunctionPointer)(P0, P1);
  InstanceType* mInstance;
  P0 mP0;
  P1 mP1;
};

template <typename ReturnType, typename InstanceType, typename P0, typename P1>
Functor* CreateFunctor(ReturnType (InstanceType::*functionPointer)(P0, P1), InstanceType* instance, P0 p0, P1 p1)
{
  ErrorIf(instance == nullptr);

  auto functor = new FunctorInstance2<ReturnType, InstanceType, P0, P1>();
  functor->mFunctionPointer = functionPointer;
  functor->mInstance = instance;
  functor->mP0 = p0;
  functor->mP1 = p1;
  return functor;
}

//------------------------------------------ Instance Const Function 2 Parameter
template <typename ReturnType, typename InstanceType, typename P0, typename P1>
class FunctorInstanceConst2 : public Functor
{
public:
  Functor* Execute() override
  {
    (mInstance->*mFunctionPointer)(mP0, mP1);
    return this;
  }

  ReturnType (InstanceType::*mFunctionPointer)(P0, P1) const;
  InstanceType* mInstance;
  P0 mP0;
  P1 mP1;
};

template <typename ReturnType, typename InstanceType, typename P0, typename P1>
Functor* CreateFunctor(ReturnType (InstanceType::*functionPointer)(P0, P1) const, InstanceType* instance, P0 p0, P1 p1)
{
  ErrorIf(instance == nullptr);

  auto functor = new FunctorInstanceConst2<ReturnType, InstanceType, P0, P1>();
  functor->mFunctionPointer = functionPointer;
  functor->mInstance = instance;
  functor->mP0 = p0;
  functor->mP1 = p1;
  return functor;
}


//-------------------------------------------------- Static Function 3 Parameter
template <typename ReturnType, typename P0, typename P1, typename P2>
class FunctorStatic3 : public Functor
{
public:
  Functor* Execute() override
  {
    mFunctionPointer(mP0, mP1, mP2);
    return this;
  }

  ReturnType(*mFunctionPointer)(P0, P1, P2);
  P0 mP0;
  P1 mP1;
  P2 mP2;
};

template <typename ReturnType, typename P0, typename P1, typename P2>
Functor* CreateFunctor(ReturnType(*functionPointer)(P0, P1, P2), P0 p0, P1 p1, P2 p2)
{
  ErrorIf(instance == nullptr);

  auto functor = new FunctorStatic3<ReturnType, P0, P1, P2>();
  functor->mFunctionPointer = functionPointer;
  functor->mP0 = p0;
  functor->mP1 = p1;
  functor->mP2 = p2;
  return functor;
}

//------------------------------------------------ Instance Function 3 Parameter
template <typename ReturnType, typename InstanceType, typename P0, typename P1, typename P2>
class FunctorInstance3 : public Functor
{
public:
  Functor* Execute() override
  {
    (mInstance->*mFunctionPointer)(mP0, mP1, mP2);
    return this;
  }

  ReturnType(InstanceType::*mFunctionPointer)(P0, P1, P2);
  InstanceType* mInstance;
  P0 mP0;
  P1 mP1;
  P2 mP2;
};

template <typename ReturnType, typename InstanceType, typename P0, typename P1, typename P2>
Functor* CreateFunctor(ReturnType(InstanceType::*functionPointer)(P0, P1, P2), InstanceType* instance, P0 p0, P1 p1, P2 p2)
{
  ErrorIf(instance == nullptr);

  auto functor = new FunctorInstance3<ReturnType, InstanceType, P0, P1, P2>();
  functor->mFunctionPointer = functionPointer;
  functor->mInstance = instance;
  functor->mP0 = p0;
  functor->mP1 = p1;
  functor->mP2 = p2;
  return functor;
}

//------------------------------------------ Instance Const Function 3 Parameter
template <typename ReturnType, typename InstanceType, typename P0, typename P1, typename P2>
class FunctorInstanceConst3 : public Functor
{
public:
  Functor* Execute() override
  {
    (mInstance->*mFunctionPointer)(mP0, mP1, mP2);
    return this;
  }

  ReturnType(InstanceType::*mFunctionPointer)(P0, P1, P2) const;
  InstanceType* mInstance;
  P0 mP0;
  P1 mP1;
  P2 mP2;
};

template <typename ReturnType, typename InstanceType, typename P0, typename P1, typename P2>
Functor* CreateFunctor(ReturnType(InstanceType::*functionPointer)(P0, P1, P2) const, InstanceType* instance, P0 p0, P1 p1, P2 p2)
{
  ErrorIf(instance == nullptr);

  auto functor = new FunctorInstanceConst3<ReturnType, InstanceType, P0, P1, P2>();
  functor->mFunctionPointer = functionPointer;
  functor->mInstance = instance;
  functor->mP0 = p0;
  functor->mP1 = p1;
  functor->mP2 = p2;
  return functor;
}


//----------------------------------------------- Set Instance Pointer To Member
template <typename InstanceType, typename MemberType>
class FunctorInstanceMember : public Functor
{
public:
  Functor* Execute() override
  {
    (mInstance->*mMemberPointer) = mValue;
    return this;
  }

  MemberType InstanceType::*mMemberPointer;
  InstanceType* mInstance;
  MemberType mValue;
};

template <typename InstanceType, typename MemberType>
Functor* CreateFunctor(MemberType InstanceType::*memberPointer, InstanceType* instance, MemberType value)
{
  ErrorIf(instance == nullptr);

  auto functor = new FunctorInstanceMember<InstanceType, MemberType>();
  functor->mMemberPointer = memberPointer;
  functor->mInstance = instance;
  functor->mValue = value;
  return functor;
}

//------------------------------------------------------------------ Set Pointer
template <typename PointerType, typename ValueType>
class FunctorSetPointer : public Functor
{
public:
  Functor* Execute() override
  {
    *mPointer = mValue;
    return this;
  }

  PointerType* mPointer;
  ValueType mValue;
};

template <typename PointerType, typename ValueType>
Functor* CreateFunctor(PointerType* pointer, ValueType value)
{
  ErrorIf(instance == nullptr);

  auto functor = new FunctorSetPointer<PointerType, ValueType>();
  functor->mPointer = pointer;
  functor->mValue = value;
  return functor;
}

//---------------------------------------------------------------- Set Reference
template <typename ReferenceType, typename ValueType>
Functor* CreateFunctor(ReferenceType& reference, ValueType value)
{
  ErrorIf(instance == nullptr);

  auto functor = new FunctorSetPointer<ReferenceType, ValueType>();
  functor->mPointer = &reference;
  functor->mValue = value;
  return functor;
}
} // namespace Zero
