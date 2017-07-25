/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

template <typename Class>
static void BoundConstructorVirtual(Call& call, ExceptionReport& report)
{
  Handle& handle = call.GetHandle(Call::This);
  byte* data = handle.Dereference();
  new (data) Class();
  call.GetState()->UpdateCppVirtualTable(data, ZilchTypeId(Class), handle.Type);
}
template <typename Class>
static Function* FromConstructorVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange spaceDelimitedNames)
{
  BoundFn boundFunction = BoundConstructorVirtual<Class>;
  ParameterArray parameters;
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundConstructor
  (
    classBoundType,
    boundFunction,
    parameters
  );
}
template <typename Class>
static void BoundConstructor(Call& call, ExceptionReport& report)
{
  Handle& handle = call.GetHandle(Call::This);
  byte* data = handle.Dereference();
  new (data) Class();
}
template <typename Class>
static Function* FromConstructor(LibraryBuilder& builder, BoundType* classBoundType, StringRange spaceDelimitedNames)
{
  BoundFn boundFunction = BoundConstructor<Class>;
  ParameterArray parameters;
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundConstructor
  (
    classBoundType,
    boundFunction,
    parameters
  );
}
template <typename Class, typename Arg0>
static void BoundConstructorVirtual(Call& call, ExceptionReport& report)
{
  Arg0 arg0 = call.Get<Arg0>(0);
  Handle& handle = call.GetHandle(Call::This);
  byte* data = handle.Dereference();
  new (data) Class(arg0);
  call.GetState()->UpdateCppVirtualTable(data, ZilchTypeId(Class), handle.Type);
}
template <typename Class, typename Arg0>
static Function* FromConstructorVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange spaceDelimitedNames)
{
  BoundFn boundFunction = BoundConstructorVirtual<Class, Arg0>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundConstructor
  (
    classBoundType,
    boundFunction,
    parameters
  );
}
template <typename Class, typename Arg0>
static void BoundConstructor(Call& call, ExceptionReport& report)
{
  Arg0 arg0 = call.Get<Arg0>(0);
  Handle& handle = call.GetHandle(Call::This);
  byte* data = handle.Dereference();
  new (data) Class(arg0);
}
template <typename Class, typename Arg0>
static Function* FromConstructor(LibraryBuilder& builder, BoundType* classBoundType, StringRange spaceDelimitedNames)
{
  BoundFn boundFunction = BoundConstructor<Class, Arg0>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundConstructor
  (
    classBoundType,
    boundFunction,
    parameters
  );
}
template <typename Class, typename Arg0, typename Arg1>
static void BoundConstructorVirtual(Call& call, ExceptionReport& report)
{
  Arg0 arg0 = call.Get<Arg0>(0);
  Arg1 arg1 = call.Get<Arg1>(1);
  Handle& handle = call.GetHandle(Call::This);
  byte* data = handle.Dereference();
  new (data) Class(arg0, arg1);
  call.GetState()->UpdateCppVirtualTable(data, ZilchTypeId(Class), handle.Type);
}
template <typename Class, typename Arg0, typename Arg1>
static Function* FromConstructorVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange spaceDelimitedNames)
{
  BoundFn boundFunction = BoundConstructorVirtual<Class, Arg0, Arg1>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = ZilchTypeId(Arg1);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundConstructor
  (
    classBoundType,
    boundFunction,
    parameters
  );
}
template <typename Class, typename Arg0, typename Arg1>
static void BoundConstructor(Call& call, ExceptionReport& report)
{
  Arg0 arg0 = call.Get<Arg0>(0);
  Arg1 arg1 = call.Get<Arg1>(1);
  Handle& handle = call.GetHandle(Call::This);
  byte* data = handle.Dereference();
  new (data) Class(arg0, arg1);
}
template <typename Class, typename Arg0, typename Arg1>
static Function* FromConstructor(LibraryBuilder& builder, BoundType* classBoundType, StringRange spaceDelimitedNames)
{
  BoundFn boundFunction = BoundConstructor<Class, Arg0, Arg1>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = ZilchTypeId(Arg1);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundConstructor
  (
    classBoundType,
    boundFunction,
    parameters
  );
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2>
static void BoundConstructorVirtual(Call& call, ExceptionReport& report)
{
  Arg0 arg0 = call.Get<Arg0>(0);
  Arg1 arg1 = call.Get<Arg1>(1);
  Arg2 arg2 = call.Get<Arg2>(2);
  Handle& handle = call.GetHandle(Call::This);
  byte* data = handle.Dereference();
  new (data) Class(arg0, arg1, arg2);
  call.GetState()->UpdateCppVirtualTable(data, ZilchTypeId(Class), handle.Type);
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2>
static Function* FromConstructorVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange spaceDelimitedNames)
{
  BoundFn boundFunction = BoundConstructorVirtual<Class, Arg0, Arg1, Arg2>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = ZilchTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = ZilchTypeId(Arg2);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundConstructor
  (
    classBoundType,
    boundFunction,
    parameters
  );
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2>
static void BoundConstructor(Call& call, ExceptionReport& report)
{
  Arg0 arg0 = call.Get<Arg0>(0);
  Arg1 arg1 = call.Get<Arg1>(1);
  Arg2 arg2 = call.Get<Arg2>(2);
  Handle& handle = call.GetHandle(Call::This);
  byte* data = handle.Dereference();
  new (data) Class(arg0, arg1, arg2);
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2>
static Function* FromConstructor(LibraryBuilder& builder, BoundType* classBoundType, StringRange spaceDelimitedNames)
{
  BoundFn boundFunction = BoundConstructor<Class, Arg0, Arg1, Arg2>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = ZilchTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = ZilchTypeId(Arg2);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundConstructor
  (
    classBoundType,
    boundFunction,
    parameters
  );
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
static void BoundConstructorVirtual(Call& call, ExceptionReport& report)
{
  Arg0 arg0 = call.Get<Arg0>(0);
  Arg1 arg1 = call.Get<Arg1>(1);
  Arg2 arg2 = call.Get<Arg2>(2);
  Arg3 arg3 = call.Get<Arg3>(3);
  Handle& handle = call.GetHandle(Call::This);
  byte* data = handle.Dereference();
  new (data) Class(arg0, arg1, arg2, arg3);
  call.GetState()->UpdateCppVirtualTable(data, ZilchTypeId(Class), handle.Type);
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
static Function* FromConstructorVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange spaceDelimitedNames)
{
  BoundFn boundFunction = BoundConstructorVirtual<Class, Arg0, Arg1, Arg2, Arg3>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = ZilchTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = ZilchTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = ZilchTypeId(Arg3);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundConstructor
  (
    classBoundType,
    boundFunction,
    parameters
  );
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
static void BoundConstructor(Call& call, ExceptionReport& report)
{
  Arg0 arg0 = call.Get<Arg0>(0);
  Arg1 arg1 = call.Get<Arg1>(1);
  Arg2 arg2 = call.Get<Arg2>(2);
  Arg3 arg3 = call.Get<Arg3>(3);
  Handle& handle = call.GetHandle(Call::This);
  byte* data = handle.Dereference();
  new (data) Class(arg0, arg1, arg2, arg3);
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
static Function* FromConstructor(LibraryBuilder& builder, BoundType* classBoundType, StringRange spaceDelimitedNames)
{
  BoundFn boundFunction = BoundConstructor<Class, Arg0, Arg1, Arg2, Arg3>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = ZilchTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = ZilchTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = ZilchTypeId(Arg3);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundConstructor
  (
    classBoundType,
    boundFunction,
    parameters
  );
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static void BoundConstructorVirtual(Call& call, ExceptionReport& report)
{
  Arg0 arg0 = call.Get<Arg0>(0);
  Arg1 arg1 = call.Get<Arg1>(1);
  Arg2 arg2 = call.Get<Arg2>(2);
  Arg3 arg3 = call.Get<Arg3>(3);
  Arg4 arg4 = call.Get<Arg4>(4);
  Handle& handle = call.GetHandle(Call::This);
  byte* data = handle.Dereference();
  new (data) Class(arg0, arg1, arg2, arg3, arg4);
  call.GetState()->UpdateCppVirtualTable(data, ZilchTypeId(Class), handle.Type);
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static Function* FromConstructorVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange spaceDelimitedNames)
{
  BoundFn boundFunction = BoundConstructorVirtual<Class, Arg0, Arg1, Arg2, Arg3, Arg4>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = ZilchTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = ZilchTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = ZilchTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = ZilchTypeId(Arg4);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundConstructor
  (
    classBoundType,
    boundFunction,
    parameters
  );
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static void BoundConstructor(Call& call, ExceptionReport& report)
{
  Arg0 arg0 = call.Get<Arg0>(0);
  Arg1 arg1 = call.Get<Arg1>(1);
  Arg2 arg2 = call.Get<Arg2>(2);
  Arg3 arg3 = call.Get<Arg3>(3);
  Arg4 arg4 = call.Get<Arg4>(4);
  Handle& handle = call.GetHandle(Call::This);
  byte* data = handle.Dereference();
  new (data) Class(arg0, arg1, arg2, arg3, arg4);
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static Function* FromConstructor(LibraryBuilder& builder, BoundType* classBoundType, StringRange spaceDelimitedNames)
{
  BoundFn boundFunction = BoundConstructor<Class, Arg0, Arg1, Arg2, Arg3, Arg4>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = ZilchTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = ZilchTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = ZilchTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = ZilchTypeId(Arg4);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundConstructor
  (
    classBoundType,
    boundFunction,
    parameters
  );
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static void BoundConstructorVirtual(Call& call, ExceptionReport& report)
{
  Arg0 arg0 = call.Get<Arg0>(0);
  Arg1 arg1 = call.Get<Arg1>(1);
  Arg2 arg2 = call.Get<Arg2>(2);
  Arg3 arg3 = call.Get<Arg3>(3);
  Arg4 arg4 = call.Get<Arg4>(4);
  Arg5 arg5 = call.Get<Arg5>(5);
  Handle& handle = call.GetHandle(Call::This);
  byte* data = handle.Dereference();
  new (data) Class(arg0, arg1, arg2, arg3, arg4, arg5);
  call.GetState()->UpdateCppVirtualTable(data, ZilchTypeId(Class), handle.Type);
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static Function* FromConstructorVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange spaceDelimitedNames)
{
  BoundFn boundFunction = BoundConstructorVirtual<Class, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = ZilchTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = ZilchTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = ZilchTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = ZilchTypeId(Arg4);
  DelegateParameter& p5 = parameters.PushBack();
  p5.ParameterType = ZilchTypeId(Arg5);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundConstructor
  (
    classBoundType,
    boundFunction,
    parameters
  );
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static void BoundConstructor(Call& call, ExceptionReport& report)
{
  Arg0 arg0 = call.Get<Arg0>(0);
  Arg1 arg1 = call.Get<Arg1>(1);
  Arg2 arg2 = call.Get<Arg2>(2);
  Arg3 arg3 = call.Get<Arg3>(3);
  Arg4 arg4 = call.Get<Arg4>(4);
  Arg5 arg5 = call.Get<Arg5>(5);
  Handle& handle = call.GetHandle(Call::This);
  byte* data = handle.Dereference();
  new (data) Class(arg0, arg1, arg2, arg3, arg4, arg5);
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static Function* FromConstructor(LibraryBuilder& builder, BoundType* classBoundType, StringRange spaceDelimitedNames)
{
  BoundFn boundFunction = BoundConstructor<Class, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = ZilchTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = ZilchTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = ZilchTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = ZilchTypeId(Arg4);
  DelegateParameter& p5 = parameters.PushBack();
  p5.ParameterType = ZilchTypeId(Arg5);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundConstructor
  (
    classBoundType,
    boundFunction,
    parameters
  );
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static void BoundConstructorVirtual(Call& call, ExceptionReport& report)
{
  Arg0 arg0 = call.Get<Arg0>(0);
  Arg1 arg1 = call.Get<Arg1>(1);
  Arg2 arg2 = call.Get<Arg2>(2);
  Arg3 arg3 = call.Get<Arg3>(3);
  Arg4 arg4 = call.Get<Arg4>(4);
  Arg5 arg5 = call.Get<Arg5>(5);
  Arg6 arg6 = call.Get<Arg6>(6);
  Handle& handle = call.GetHandle(Call::This);
  byte* data = handle.Dereference();
  new (data) Class(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
  call.GetState()->UpdateCppVirtualTable(data, ZilchTypeId(Class), handle.Type);
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static Function* FromConstructorVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange spaceDelimitedNames)
{
  BoundFn boundFunction = BoundConstructorVirtual<Class, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = ZilchTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = ZilchTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = ZilchTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = ZilchTypeId(Arg4);
  DelegateParameter& p5 = parameters.PushBack();
  p5.ParameterType = ZilchTypeId(Arg5);
  DelegateParameter& p6 = parameters.PushBack();
  p6.ParameterType = ZilchTypeId(Arg6);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundConstructor
  (
    classBoundType,
    boundFunction,
    parameters
  );
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static void BoundConstructor(Call& call, ExceptionReport& report)
{
  Arg0 arg0 = call.Get<Arg0>(0);
  Arg1 arg1 = call.Get<Arg1>(1);
  Arg2 arg2 = call.Get<Arg2>(2);
  Arg3 arg3 = call.Get<Arg3>(3);
  Arg4 arg4 = call.Get<Arg4>(4);
  Arg5 arg5 = call.Get<Arg5>(5);
  Arg6 arg6 = call.Get<Arg6>(6);
  Handle& handle = call.GetHandle(Call::This);
  byte* data = handle.Dereference();
  new (data) Class(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
}
template <typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static Function* FromConstructor(LibraryBuilder& builder, BoundType* classBoundType, StringRange spaceDelimitedNames)
{
  BoundFn boundFunction = BoundConstructor<Class, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>;
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = ZilchTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = ZilchTypeId(Arg2);
  DelegateParameter& p3 = parameters.PushBack();
  p3.ParameterType = ZilchTypeId(Arg3);
  DelegateParameter& p4 = parameters.PushBack();
  p4.ParameterType = ZilchTypeId(Arg4);
  DelegateParameter& p5 = parameters.PushBack();
  p5.ParameterType = ZilchTypeId(Arg5);
  DelegateParameter& p6 = parameters.PushBack();
  p6.ParameterType = ZilchTypeId(Arg6);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  return builder.AddBoundConstructor
  (
    classBoundType,
    boundFunction,
    parameters
  );
}
