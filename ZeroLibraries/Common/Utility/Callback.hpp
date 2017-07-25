///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Reese Jones
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//---------------------------------------------------------------------- CallBack0
template <typename ReturnType>
class Callback0
{
public:
  virtual ReturnType operator()() = 0;
};

//------------------------------------------------- Static Function
template <typename ReturnType>
class CallbackStatic0 : public Callback0< ReturnType >
{
public:
  virtual ReturnType operator()() override
  {
    return (*mFunctionPointer)();
  }

  ReturnType(*mFunctionPointer)();
};

template <typename ReturnType>
Callback0<ReturnType>* CreateCallback(ReturnType(*functionPointer)())
{
  typename CallbackStatic0<ReturnType>* callback = new CallbackStatic0<ReturnType>();
  callback->mFunctionPointer = functionPointer;
  return callback;
}

//------------------------------------------------- Member Function
template <typename ReturnType, typename InstanceType>
class CallbackInstance0 : public Callback0< ReturnType >
{
public:
  virtual ReturnType operator()() override
  {
    return (mInstance->*mFunctionPointer)();
  }

  ReturnType (InstanceType::*mFunctionPointer)();
  InstanceType* mInstance;
};

template <typename ReturnType, typename InstanceType>
Callback0<ReturnType>* CreateCallback(ReturnType(InstanceType::*functionPointer)(), InstanceType* instance)
{
  typename CallbackInstance0<ReturnType, InstanceType>* callback = new CallbackInstance0<ReturnType, InstanceType>();
  callback->mFunctionPointer = functionPointer;
  callback->mInstance = instance;
  return callback;
}

//---------------------------------------------------------------------- CallBack1
template <typename ReturnType, typename P0>
class Callback1
{
public:
  virtual ReturnType operator()(P0 p0) = 0;
};

//------------------------------------------------- Static Function
template <typename ReturnType, typename P0>
class CallbackStatic1 : public Callback1< ReturnType, P0 >
{
public:
  virtual ReturnType operator()(P0 p0) override
  {
    return (*mFunctionPointer)(p0);
  }

  ReturnType(*mFunctionPointer)(P0 p0);
};

template <typename ReturnType, typename P0>
Callback1<ReturnType, P0>* CreateCallback(ReturnType(*functionPointer)(P0))
{
  typename CallbackStatic1<ReturnType, P0>* callback = new CallbackStatic1<ReturnType, P0>();
  callback->mFunctionPointer = functionPointer;
  return callback;
}

//------------------------------------------------- Member Function
template <typename ReturnType, typename InstanceType, typename P0>
class CallbackInstance1 : public Callback1< ReturnType, P0 >
{
public:
  virtual ReturnType operator()(P0 p0) override
  {
    return (mInstance->*mFunctionPointer)(p0);
  }

  ReturnType(InstanceType::*mFunctionPointer)(P0 p0);
  InstanceType* mInstance;
};

template <typename ReturnType, typename InstanceType, typename P0>
Callback1<ReturnType, P0>* CreateCallback(ReturnType(InstanceType::*functionPointer)(P0), InstanceType* instance)
{
  typename CallbackInstance1<ReturnType, InstanceType, P0>* callback = new CallbackInstance1<ReturnType, InstanceType, P0>();
  callback->mFunctionPointer = functionPointer;
  callback->mInstance = instance;
  return callback;
}

//---------------------------------------------------------------------- CallBack2
template <typename ReturnType, typename P0, typename P1>
class Callback2
{
public:
  virtual ReturnType operator()(P0 p0, P1 p1) = 0;
};

//------------------------------------------------- Static Function
template <typename ReturnType, typename P0, typename P1>
class CallbackStatic2 : public Callback2< ReturnType, P0, P1 >
{
  public:
  virtual ReturnType operator()(P0 p0, P1 p1) override
  {
    return (*mFunctionPointer)(p0, p1);
  }

  ReturnType(*mFunctionPointer)(P0 p0, P1 p1);
};

template <typename ReturnType, typename P0, typename P1>
Callback2<ReturnType, P0, P1>* CreateCallback(ReturnType(*functionPointer)(P0, P1))
{
  typename CallbackStatic2<ReturnType, P0, P1>* callback = new CallbackStatic2<ReturnType, P0, P1>();
  callback->mFunctionPointer = functionPointer;
  return callback;
}

//------------------------------------------------- Member Function
template <typename ReturnType, typename InstanceType, typename P0, typename P1>
class CallbackInstance2 : public Callback2< ReturnType, P0, P1 >
{
public:
  virtual ReturnType operator()(P0 p0, P1 p1) override
  {
    return (mInstance->*mFunctionPointer)(p0, p1);
  }

  ReturnType(InstanceType::*mFunctionPointer)(P0 p0, P1 p1);
  InstanceType* mInstance;
};

template <typename ReturnType, typename InstanceType, typename P0, typename P1>
Callback2<ReturnType, P0, P1>* CreateCallback(ReturnType(InstanceType::*functionPointer)(P0, P1), InstanceType* instance)
{
  typename CallbackInstance2<ReturnType, InstanceType, P0, P1>* callback = new CallbackInstance2<ReturnType, InstanceType, P0, P1>();
  callback->mFunctionPointer = functionPointer;
  callback->mInstance = instance;
  return callback;
}

//---------------------------------------------------------------------- CallBack3
template <typename ReturnType, typename P0, typename P1, typename P2>
class Callback3
{
public:
  virtual ReturnType operator()(P0 p0, P1 p1, P2 p2) = 0;
};

//------------------------------------------------- Static Function
template <typename ReturnType, typename P0, typename P1, typename P2>
class CallbackStatic3 : public Callback3< ReturnType, P0, P1, P2 >
{
public:
  virtual ReturnType operator()(P0 p0, P1 p1, P2 p2) override
  {
    return (*mFunctionPointer)(p0, p1, p2);
  }

  ReturnType(*mFunctionPointer)(P0 p0, P1 p1, P2 p2);
};

template <typename ReturnType, typename P0, typename P1, typename P2>
Callback3<ReturnType, P0, P1, P2>* CreateCallback(ReturnType(*functionPointer)(P0, P1, P2))
{
  typename CallbackStatic3<ReturnType, P0, P1, P2>* callback = new CallbackStatic3<ReturnType, P0, P1, P2>();
  callback->mFunctionPointer = functionPointer;
  return callback;
}

//------------------------------------------------- Member Function
template <typename ReturnType, typename InstanceType, typename P0, typename P1, typename P2>
class CallbackInstance3 : public Callback3< ReturnType, P0, P1, P2 >
{
public:
  virtual ReturnType operator()(P0 p0, P1 p1, P2 p2) override
  {
    return (mInstance->*mFunctionPointer)(p0, p1, p2);
  }

  ReturnType(InstanceType::*mFunctionPointer)(P0 p0, P1 p1, P2 p2);
  InstanceType* mInstance;
};

template <typename ReturnType, typename InstanceType, typename P0, typename P1, typename P2>
Callback3<ReturnType, P0, P1, P2>* CreateCallback(ReturnType(InstanceType::*functionPointer)(P0, P1, P2), InstanceType* instance)
{
  typename CallbackInstance3<ReturnType, InstanceType, P0, P1, P2>* callback = new CallbackInstance3<ReturnType, InstanceType, P0, P1, P2>();
  callback->mFunctionPointer = functionPointer;
  callback->mInstance = instance;
  return callback;
}

} // namespace Zero
