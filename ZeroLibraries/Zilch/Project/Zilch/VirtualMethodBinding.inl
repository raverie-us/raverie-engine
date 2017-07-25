/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

template <typename FunctionType, FunctionType function, typename Class>
void VirtualThunk()
{
  byte* virtualTable = *(byte**)this;
  byte* typePointer = virtualTable - sizeof(BoundType*) - sizeof(ExecutableState*);
  byte* executableStatePointer = virtualTable - sizeof(ExecutableState*);
  BoundType* type = *(BoundType**)typePointer;
  ExecutableState* state = *(ExecutableState**)(executableStatePointer);
  GuidType virtualId = type->Hash() ^ TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionToCall = state->ThunksToFunctions.FindValue(virtualId, nullptr);
  ErrorIf(functionToCall == nullptr,
    "There was no function found by the guid, what happened?");
  HandleManagers& managers = HandleManagers::GetInstance();
  HandleManager* pointerManager = managers.GetSharedManager(ZilchManagerId(PointerManager));
  Handle thisHandle;
  thisHandle.Manager = pointerManager;
  thisHandle.Type = type;
  pointerManager->ObjectToHandle((byte*)this, thisHandle);
  Call call(functionToCall, state);
  call.SetHandle(Call::This, thisHandle);
  ExceptionReport report;
  call.Invoke(report);
  return;
}
template <typename FunctionType, FunctionType function, typename Class>
static Function* FromVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)())
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class>;
  ZilchAutoVal(thunk, (&VirtualThunk<FunctionType, function, Class, void>));
  ParameterArray parameters;
  ParseParameterArrays(parameters, spaceDelimitedNames);
  NativeVirtualInfo nativeVirtual;
  nativeVirtual.Index = TypeBinding::GetVirtualMethodIndex(function);
  nativeVirtual.Thunk = (TypeBinding::VirtualTableFn)thunk;
  nativeVirtual.Guid = TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionRef = builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    ZilchTypeId(void),
    FunctionOptions::Virtual,
    nativeVirtual
  );
  ++classBoundType->BoundNativeVirtualCount;
  ErrorIf(classBoundType->BoundNativeVirtualCount > classBoundType->RawNativeVirtualCount,
    "The number of bound virtual functions must never exceed the actual v-table count");
  return functionRef;
}
template <typename FunctionType, FunctionType function, typename Class, typename Arg0>
void VirtualThunk(Arg0 arg0)
{
  byte* virtualTable = *(byte**)this;
  byte* typePointer = virtualTable - sizeof(BoundType*) - sizeof(ExecutableState*);
  byte* executableStatePointer = virtualTable - sizeof(ExecutableState*);
  BoundType* type = *(BoundType**)typePointer;
  ExecutableState* state = *(ExecutableState**)(executableStatePointer);
  GuidType virtualId = type->Hash() ^ TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionToCall = state->ThunksToFunctions.FindValue(virtualId, nullptr);
  ErrorIf(functionToCall == nullptr,
    "There was no function found by the guid, what happened?");
  HandleManagers& managers = HandleManagers::GetInstance();
  HandleManager* pointerManager = managers.GetSharedManager(ZilchManagerId(PointerManager));
  Handle thisHandle;
  thisHandle.Manager = pointerManager;
  thisHandle.Type = type;
  pointerManager->ObjectToHandle((byte*)this, thisHandle);
  Call call(functionToCall, state);
  call.SetHandle(Call::This, thisHandle);
  call.Set<Arg0>(0, arg0);
  ExceptionReport report;
  call.Invoke(report);
  return;
}
template <typename FunctionType, FunctionType function, typename Class, typename Arg0>
static Function* FromVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0))
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0>;
  ZilchAutoVal(thunk, (&VirtualThunk<FunctionType, function, Class, void, Arg0>));
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  NativeVirtualInfo nativeVirtual;
  nativeVirtual.Index = TypeBinding::GetVirtualMethodIndex(function);
  nativeVirtual.Thunk = (TypeBinding::VirtualTableFn)thunk;
  nativeVirtual.Guid = TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionRef = builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    ZilchTypeId(void),
    FunctionOptions::Virtual,
    nativeVirtual
  );
  ++classBoundType->BoundNativeVirtualCount;
  ErrorIf(classBoundType->BoundNativeVirtualCount > classBoundType->RawNativeVirtualCount,
    "The number of bound virtual functions must never exceed the actual v-table count");
  return functionRef;
}
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1>
void VirtualThunk(Arg0 arg0, Arg1 arg1)
{
  byte* virtualTable = *(byte**)this;
  byte* typePointer = virtualTable - sizeof(BoundType*) - sizeof(ExecutableState*);
  byte* executableStatePointer = virtualTable - sizeof(ExecutableState*);
  BoundType* type = *(BoundType**)typePointer;
  ExecutableState* state = *(ExecutableState**)(executableStatePointer);
  GuidType virtualId = type->Hash() ^ TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionToCall = state->ThunksToFunctions.FindValue(virtualId, nullptr);
  ErrorIf(functionToCall == nullptr,
    "There was no function found by the guid, what happened?");
  HandleManagers& managers = HandleManagers::GetInstance();
  HandleManager* pointerManager = managers.GetSharedManager(ZilchManagerId(PointerManager));
  Handle thisHandle;
  thisHandle.Manager = pointerManager;
  thisHandle.Type = type;
  pointerManager->ObjectToHandle((byte*)this, thisHandle);
  Call call(functionToCall, state);
  call.SetHandle(Call::This, thisHandle);
  call.Set<Arg0>(0, arg0);
  call.Set<Arg1>(1, arg1);
  ExceptionReport report;
  call.Invoke(report);
  return;
}
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1>
static Function* FromVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1))
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1>;
  ZilchAutoVal(thunk, (&VirtualThunk<FunctionType, function, Class, void, Arg0, Arg1>));
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = ZilchTypeId(Arg1);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  NativeVirtualInfo nativeVirtual;
  nativeVirtual.Index = TypeBinding::GetVirtualMethodIndex(function);
  nativeVirtual.Thunk = (TypeBinding::VirtualTableFn)thunk;
  nativeVirtual.Guid = TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionRef = builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    ZilchTypeId(void),
    FunctionOptions::Virtual,
    nativeVirtual
  );
  ++classBoundType->BoundNativeVirtualCount;
  ErrorIf(classBoundType->BoundNativeVirtualCount > classBoundType->RawNativeVirtualCount,
    "The number of bound virtual functions must never exceed the actual v-table count");
  return functionRef;
}
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2>
void VirtualThunk(Arg0 arg0, Arg1 arg1, Arg2 arg2)
{
  byte* virtualTable = *(byte**)this;
  byte* typePointer = virtualTable - sizeof(BoundType*) - sizeof(ExecutableState*);
  byte* executableStatePointer = virtualTable - sizeof(ExecutableState*);
  BoundType* type = *(BoundType**)typePointer;
  ExecutableState* state = *(ExecutableState**)(executableStatePointer);
  GuidType virtualId = type->Hash() ^ TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionToCall = state->ThunksToFunctions.FindValue(virtualId, nullptr);
  ErrorIf(functionToCall == nullptr,
    "There was no function found by the guid, what happened?");
  HandleManagers& managers = HandleManagers::GetInstance();
  HandleManager* pointerManager = managers.GetSharedManager(ZilchManagerId(PointerManager));
  Handle thisHandle;
  thisHandle.Manager = pointerManager;
  thisHandle.Type = type;
  pointerManager->ObjectToHandle((byte*)this, thisHandle);
  Call call(functionToCall, state);
  call.SetHandle(Call::This, thisHandle);
  call.Set<Arg0>(0, arg0);
  call.Set<Arg1>(1, arg1);
  call.Set<Arg2>(2, arg2);
  ExceptionReport report;
  call.Invoke(report);
  return;
}
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2>
static Function* FromVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1, Arg2))
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1, Arg2>;
  ZilchAutoVal(thunk, (&VirtualThunk<FunctionType, function, Class, void, Arg0, Arg1, Arg2>));
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = ZilchTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = ZilchTypeId(Arg2);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  NativeVirtualInfo nativeVirtual;
  nativeVirtual.Index = TypeBinding::GetVirtualMethodIndex(function);
  nativeVirtual.Thunk = (TypeBinding::VirtualTableFn)thunk;
  nativeVirtual.Guid = TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionRef = builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    ZilchTypeId(void),
    FunctionOptions::Virtual,
    nativeVirtual
  );
  ++classBoundType->BoundNativeVirtualCount;
  ErrorIf(classBoundType->BoundNativeVirtualCount > classBoundType->RawNativeVirtualCount,
    "The number of bound virtual functions must never exceed the actual v-table count");
  return functionRef;
}
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
void VirtualThunk(Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3)
{
  byte* virtualTable = *(byte**)this;
  byte* typePointer = virtualTable - sizeof(BoundType*) - sizeof(ExecutableState*);
  byte* executableStatePointer = virtualTable - sizeof(ExecutableState*);
  BoundType* type = *(BoundType**)typePointer;
  ExecutableState* state = *(ExecutableState**)(executableStatePointer);
  GuidType virtualId = type->Hash() ^ TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionToCall = state->ThunksToFunctions.FindValue(virtualId, nullptr);
  ErrorIf(functionToCall == nullptr,
    "There was no function found by the guid, what happened?");
  HandleManagers& managers = HandleManagers::GetInstance();
  HandleManager* pointerManager = managers.GetSharedManager(ZilchManagerId(PointerManager));
  Handle thisHandle;
  thisHandle.Manager = pointerManager;
  thisHandle.Type = type;
  pointerManager->ObjectToHandle((byte*)this, thisHandle);
  Call call(functionToCall, state);
  call.SetHandle(Call::This, thisHandle);
  call.Set<Arg0>(0, arg0);
  call.Set<Arg1>(1, arg1);
  call.Set<Arg2>(2, arg2);
  call.Set<Arg3>(3, arg3);
  ExceptionReport report;
  call.Invoke(report);
  return;
}
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
static Function* FromVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1, Arg2, Arg3))
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1, Arg2, Arg3>;
  ZilchAutoVal(thunk, (&VirtualThunk<FunctionType, function, Class, void, Arg0, Arg1, Arg2, Arg3>));
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
  NativeVirtualInfo nativeVirtual;
  nativeVirtual.Index = TypeBinding::GetVirtualMethodIndex(function);
  nativeVirtual.Thunk = (TypeBinding::VirtualTableFn)thunk;
  nativeVirtual.Guid = TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionRef = builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    ZilchTypeId(void),
    FunctionOptions::Virtual,
    nativeVirtual
  );
  ++classBoundType->BoundNativeVirtualCount;
  ErrorIf(classBoundType->BoundNativeVirtualCount > classBoundType->RawNativeVirtualCount,
    "The number of bound virtual functions must never exceed the actual v-table count");
  return functionRef;
}
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
void VirtualThunk(Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
{
  byte* virtualTable = *(byte**)this;
  byte* typePointer = virtualTable - sizeof(BoundType*) - sizeof(ExecutableState*);
  byte* executableStatePointer = virtualTable - sizeof(ExecutableState*);
  BoundType* type = *(BoundType**)typePointer;
  ExecutableState* state = *(ExecutableState**)(executableStatePointer);
  GuidType virtualId = type->Hash() ^ TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionToCall = state->ThunksToFunctions.FindValue(virtualId, nullptr);
  ErrorIf(functionToCall == nullptr,
    "There was no function found by the guid, what happened?");
  HandleManagers& managers = HandleManagers::GetInstance();
  HandleManager* pointerManager = managers.GetSharedManager(ZilchManagerId(PointerManager));
  Handle thisHandle;
  thisHandle.Manager = pointerManager;
  thisHandle.Type = type;
  pointerManager->ObjectToHandle((byte*)this, thisHandle);
  Call call(functionToCall, state);
  call.SetHandle(Call::This, thisHandle);
  call.Set<Arg0>(0, arg0);
  call.Set<Arg1>(1, arg1);
  call.Set<Arg2>(2, arg2);
  call.Set<Arg3>(3, arg3);
  call.Set<Arg4>(4, arg4);
  ExceptionReport report;
  call.Invoke(report);
  return;
}
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static Function* FromVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4))
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1, Arg2, Arg3, Arg4>;
  ZilchAutoVal(thunk, (&VirtualThunk<FunctionType, function, Class, void, Arg0, Arg1, Arg2, Arg3, Arg4>));
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
  NativeVirtualInfo nativeVirtual;
  nativeVirtual.Index = TypeBinding::GetVirtualMethodIndex(function);
  nativeVirtual.Thunk = (TypeBinding::VirtualTableFn)thunk;
  nativeVirtual.Guid = TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionRef = builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    ZilchTypeId(void),
    FunctionOptions::Virtual,
    nativeVirtual
  );
  ++classBoundType->BoundNativeVirtualCount;
  ErrorIf(classBoundType->BoundNativeVirtualCount > classBoundType->RawNativeVirtualCount,
    "The number of bound virtual functions must never exceed the actual v-table count");
  return functionRef;
}
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
void VirtualThunk(Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
{
  byte* virtualTable = *(byte**)this;
  byte* typePointer = virtualTable - sizeof(BoundType*) - sizeof(ExecutableState*);
  byte* executableStatePointer = virtualTable - sizeof(ExecutableState*);
  BoundType* type = *(BoundType**)typePointer;
  ExecutableState* state = *(ExecutableState**)(executableStatePointer);
  GuidType virtualId = type->Hash() ^ TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionToCall = state->ThunksToFunctions.FindValue(virtualId, nullptr);
  ErrorIf(functionToCall == nullptr,
    "There was no function found by the guid, what happened?");
  HandleManagers& managers = HandleManagers::GetInstance();
  HandleManager* pointerManager = managers.GetSharedManager(ZilchManagerId(PointerManager));
  Handle thisHandle;
  thisHandle.Manager = pointerManager;
  thisHandle.Type = type;
  pointerManager->ObjectToHandle((byte*)this, thisHandle);
  Call call(functionToCall, state);
  call.SetHandle(Call::This, thisHandle);
  call.Set<Arg0>(0, arg0);
  call.Set<Arg1>(1, arg1);
  call.Set<Arg2>(2, arg2);
  call.Set<Arg3>(3, arg3);
  call.Set<Arg4>(4, arg4);
  call.Set<Arg5>(5, arg5);
  ExceptionReport report;
  call.Invoke(report);
  return;
}
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static Function* FromVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5))
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>;
  ZilchAutoVal(thunk, (&VirtualThunk<FunctionType, function, Class, void, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>));
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
  NativeVirtualInfo nativeVirtual;
  nativeVirtual.Index = TypeBinding::GetVirtualMethodIndex(function);
  nativeVirtual.Thunk = (TypeBinding::VirtualTableFn)thunk;
  nativeVirtual.Guid = TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionRef = builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    ZilchTypeId(void),
    FunctionOptions::Virtual,
    nativeVirtual
  );
  ++classBoundType->BoundNativeVirtualCount;
  ErrorIf(classBoundType->BoundNativeVirtualCount > classBoundType->RawNativeVirtualCount,
    "The number of bound virtual functions must never exceed the actual v-table count");
  return functionRef;
}
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
void VirtualThunk(Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
{
  byte* virtualTable = *(byte**)this;
  byte* typePointer = virtualTable - sizeof(BoundType*) - sizeof(ExecutableState*);
  byte* executableStatePointer = virtualTable - sizeof(ExecutableState*);
  BoundType* type = *(BoundType**)typePointer;
  ExecutableState* state = *(ExecutableState**)(executableStatePointer);
  GuidType virtualId = type->Hash() ^ TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionToCall = state->ThunksToFunctions.FindValue(virtualId, nullptr);
  ErrorIf(functionToCall == nullptr,
    "There was no function found by the guid, what happened?");
  HandleManagers& managers = HandleManagers::GetInstance();
  HandleManager* pointerManager = managers.GetSharedManager(ZilchManagerId(PointerManager));
  Handle thisHandle;
  thisHandle.Manager = pointerManager;
  thisHandle.Type = type;
  pointerManager->ObjectToHandle((byte*)this, thisHandle);
  Call call(functionToCall, state);
  call.SetHandle(Call::This, thisHandle);
  call.Set<Arg0>(0, arg0);
  call.Set<Arg1>(1, arg1);
  call.Set<Arg2>(2, arg2);
  call.Set<Arg3>(3, arg3);
  call.Set<Arg4>(4, arg4);
  call.Set<Arg5>(5, arg5);
  call.Set<Arg6>(6, arg6);
  ExceptionReport report;
  call.Invoke(report);
  return;
}
template <typename FunctionType, FunctionType function, typename Class, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static Function* FromVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, void (Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6))
{
  BoundFn boundFunction = BoundInstance<FunctionType, function, Class, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>;
  ZilchAutoVal(thunk, (&VirtualThunk<FunctionType, function, Class, void, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>));
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
  NativeVirtualInfo nativeVirtual;
  nativeVirtual.Index = TypeBinding::GetVirtualMethodIndex(function);
  nativeVirtual.Thunk = (TypeBinding::VirtualTableFn)thunk;
  nativeVirtual.Guid = TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionRef = builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    ZilchTypeId(void),
    FunctionOptions::Virtual,
    nativeVirtual
  );
  ++classBoundType->BoundNativeVirtualCount;
  ErrorIf(classBoundType->BoundNativeVirtualCount > classBoundType->RawNativeVirtualCount,
    "The number of bound virtual functions must never exceed the actual v-table count");
  return functionRef;
}
template <typename FunctionType, FunctionType function, typename Class, typename Return>
Return VirtualThunkReturn()
{
  byte* virtualTable = *(byte**)this;
  byte* typePointer = virtualTable - sizeof(BoundType*) - sizeof(ExecutableState*);
  byte* executableStatePointer = virtualTable - sizeof(ExecutableState*);
  BoundType* type = *(BoundType**)typePointer;
  ExecutableState* state = *(ExecutableState**)(executableStatePointer);
  GuidType virtualId = type->Hash() ^ TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionToCall = state->ThunksToFunctions.FindValue(virtualId, nullptr);
  ErrorIf(functionToCall == nullptr,
    "There was no function found by the guid, what happened?");
  HandleManagers& managers = HandleManagers::GetInstance();
  HandleManager* pointerManager = managers.GetSharedManager(ZilchManagerId(PointerManager));
  Handle thisHandle;
  thisHandle.Manager = pointerManager;
  thisHandle.Type = type;
  pointerManager->ObjectToHandle((byte*)this, thisHandle);
  Call call(functionToCall, state);
  call.SetHandle(Call::This, thisHandle);
  ExceptionReport report;
  call.Invoke(report);
  return call.Get<Return>(Call::Return);
}
template <typename FunctionType, FunctionType function, typename Class, typename Return>
static Function* FromVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return(Class::*)())
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return>;
  ZilchAutoVal(thunk, (&VirtualThunkReturn<FunctionType, function, Class, Return>));
  ParameterArray parameters;
  ParseParameterArrays(parameters, spaceDelimitedNames);
  NativeVirtualInfo nativeVirtual;
  nativeVirtual.Index = TypeBinding::GetVirtualMethodIndex(function);
  nativeVirtual.Thunk = (TypeBinding::VirtualTableFn)thunk;
  nativeVirtual.Guid = TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionRef = builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    ZilchTypeId(Return),
    FunctionOptions::Virtual,
    nativeVirtual
  );
  ++classBoundType->BoundNativeVirtualCount;
  ErrorIf(classBoundType->BoundNativeVirtualCount > classBoundType->RawNativeVirtualCount,
    "The number of bound virtual functions must never exceed the actual v-table count");
  return functionRef;
}
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0>
Return VirtualThunkReturn(Arg0 arg0)
{
  byte* virtualTable = *(byte**)this;
  byte* typePointer = virtualTable - sizeof(BoundType*) - sizeof(ExecutableState*);
  byte* executableStatePointer = virtualTable - sizeof(ExecutableState*);
  BoundType* type = *(BoundType**)typePointer;
  ExecutableState* state = *(ExecutableState**)(executableStatePointer);
  GuidType virtualId = type->Hash() ^ TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionToCall = state->ThunksToFunctions.FindValue(virtualId, nullptr);
  ErrorIf(functionToCall == nullptr,
    "There was no function found by the guid, what happened?");
  HandleManagers& managers = HandleManagers::GetInstance();
  HandleManager* pointerManager = managers.GetSharedManager(ZilchManagerId(PointerManager));
  Handle thisHandle;
  thisHandle.Manager = pointerManager;
  thisHandle.Type = type;
  pointerManager->ObjectToHandle((byte*)this, thisHandle);
  Call call(functionToCall, state);
  call.SetHandle(Call::This, thisHandle);
  call.Set<Arg0>(0, arg0);
  ExceptionReport report;
  call.Invoke(report);
  return call.Get<Return>(Call::Return);
}
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0>
static Function* FromVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return(Class::*)(Arg0))
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0>;
  ZilchAutoVal(thunk, (&VirtualThunkReturn<FunctionType, function, Class, Return, Arg0>));
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  NativeVirtualInfo nativeVirtual;
  nativeVirtual.Index = TypeBinding::GetVirtualMethodIndex(function);
  nativeVirtual.Thunk = (TypeBinding::VirtualTableFn)thunk;
  nativeVirtual.Guid = TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionRef = builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    ZilchTypeId(Return),
    FunctionOptions::Virtual,
    nativeVirtual
  );
  ++classBoundType->BoundNativeVirtualCount;
  ErrorIf(classBoundType->BoundNativeVirtualCount > classBoundType->RawNativeVirtualCount,
    "The number of bound virtual functions must never exceed the actual v-table count");
  return functionRef;
}
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1>
Return VirtualThunkReturn(Arg0 arg0, Arg1 arg1)
{
  byte* virtualTable = *(byte**)this;
  byte* typePointer = virtualTable - sizeof(BoundType*) - sizeof(ExecutableState*);
  byte* executableStatePointer = virtualTable - sizeof(ExecutableState*);
  BoundType* type = *(BoundType**)typePointer;
  ExecutableState* state = *(ExecutableState**)(executableStatePointer);
  GuidType virtualId = type->Hash() ^ TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionToCall = state->ThunksToFunctions.FindValue(virtualId, nullptr);
  ErrorIf(functionToCall == nullptr,
    "There was no function found by the guid, what happened?");
  HandleManagers& managers = HandleManagers::GetInstance();
  HandleManager* pointerManager = managers.GetSharedManager(ZilchManagerId(PointerManager));
  Handle thisHandle;
  thisHandle.Manager = pointerManager;
  thisHandle.Type = type;
  pointerManager->ObjectToHandle((byte*)this, thisHandle);
  Call call(functionToCall, state);
  call.SetHandle(Call::This, thisHandle);
  call.Set<Arg0>(0, arg0);
  call.Set<Arg1>(1, arg1);
  ExceptionReport report;
  call.Invoke(report);
  return call.Get<Return>(Call::Return);
}
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1>
static Function* FromVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return(Class::*)(Arg0, Arg1))
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1>;
  ZilchAutoVal(thunk, (&VirtualThunkReturn<FunctionType, function, Class, Return, Arg0, Arg1>));
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = ZilchTypeId(Arg1);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  NativeVirtualInfo nativeVirtual;
  nativeVirtual.Index = TypeBinding::GetVirtualMethodIndex(function);
  nativeVirtual.Thunk = (TypeBinding::VirtualTableFn)thunk;
  nativeVirtual.Guid = TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionRef = builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    ZilchTypeId(Return),
    FunctionOptions::Virtual,
    nativeVirtual
  );
  ++classBoundType->BoundNativeVirtualCount;
  ErrorIf(classBoundType->BoundNativeVirtualCount > classBoundType->RawNativeVirtualCount,
    "The number of bound virtual functions must never exceed the actual v-table count");
  return functionRef;
}
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2>
Return VirtualThunkReturn(Arg0 arg0, Arg1 arg1, Arg2 arg2)
{
  byte* virtualTable = *(byte**)this;
  byte* typePointer = virtualTable - sizeof(BoundType*) - sizeof(ExecutableState*);
  byte* executableStatePointer = virtualTable - sizeof(ExecutableState*);
  BoundType* type = *(BoundType**)typePointer;
  ExecutableState* state = *(ExecutableState**)(executableStatePointer);
  GuidType virtualId = type->Hash() ^ TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionToCall = state->ThunksToFunctions.FindValue(virtualId, nullptr);
  ErrorIf(functionToCall == nullptr,
    "There was no function found by the guid, what happened?");
  HandleManagers& managers = HandleManagers::GetInstance();
  HandleManager* pointerManager = managers.GetSharedManager(ZilchManagerId(PointerManager));
  Handle thisHandle;
  thisHandle.Manager = pointerManager;
  thisHandle.Type = type;
  pointerManager->ObjectToHandle((byte*)this, thisHandle);
  Call call(functionToCall, state);
  call.SetHandle(Call::This, thisHandle);
  call.Set<Arg0>(0, arg0);
  call.Set<Arg1>(1, arg1);
  call.Set<Arg2>(2, arg2);
  ExceptionReport report;
  call.Invoke(report);
  return call.Get<Return>(Call::Return);
}
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2>
static Function* FromVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return(Class::*)(Arg0, Arg1, Arg2))
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2>;
  ZilchAutoVal(thunk, (&VirtualThunkReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2>));
  ParameterArray parameters;
  DelegateParameter& p0 = parameters.PushBack();
  p0.ParameterType = ZilchTypeId(Arg0);
  DelegateParameter& p1 = parameters.PushBack();
  p1.ParameterType = ZilchTypeId(Arg1);
  DelegateParameter& p2 = parameters.PushBack();
  p2.ParameterType = ZilchTypeId(Arg2);
  ParseParameterArrays(parameters, spaceDelimitedNames);
  NativeVirtualInfo nativeVirtual;
  nativeVirtual.Index = TypeBinding::GetVirtualMethodIndex(function);
  nativeVirtual.Thunk = (TypeBinding::VirtualTableFn)thunk;
  nativeVirtual.Guid = TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionRef = builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    ZilchTypeId(Return),
    FunctionOptions::Virtual,
    nativeVirtual
  );
  ++classBoundType->BoundNativeVirtualCount;
  ErrorIf(classBoundType->BoundNativeVirtualCount > classBoundType->RawNativeVirtualCount,
    "The number of bound virtual functions must never exceed the actual v-table count");
  return functionRef;
}
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
Return VirtualThunkReturn(Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3)
{
  byte* virtualTable = *(byte**)this;
  byte* typePointer = virtualTable - sizeof(BoundType*) - sizeof(ExecutableState*);
  byte* executableStatePointer = virtualTable - sizeof(ExecutableState*);
  BoundType* type = *(BoundType**)typePointer;
  ExecutableState* state = *(ExecutableState**)(executableStatePointer);
  GuidType virtualId = type->Hash() ^ TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionToCall = state->ThunksToFunctions.FindValue(virtualId, nullptr);
  ErrorIf(functionToCall == nullptr,
    "There was no function found by the guid, what happened?");
  HandleManagers& managers = HandleManagers::GetInstance();
  HandleManager* pointerManager = managers.GetSharedManager(ZilchManagerId(PointerManager));
  Handle thisHandle;
  thisHandle.Manager = pointerManager;
  thisHandle.Type = type;
  pointerManager->ObjectToHandle((byte*)this, thisHandle);
  Call call(functionToCall, state);
  call.SetHandle(Call::This, thisHandle);
  call.Set<Arg0>(0, arg0);
  call.Set<Arg1>(1, arg1);
  call.Set<Arg2>(2, arg2);
  call.Set<Arg3>(3, arg3);
  ExceptionReport report;
  call.Invoke(report);
  return call.Get<Return>(Call::Return);
}
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
static Function* FromVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return(Class::*)(Arg0, Arg1, Arg2, Arg3))
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2, Arg3>;
  ZilchAutoVal(thunk, (&VirtualThunkReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2, Arg3>));
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
  NativeVirtualInfo nativeVirtual;
  nativeVirtual.Index = TypeBinding::GetVirtualMethodIndex(function);
  nativeVirtual.Thunk = (TypeBinding::VirtualTableFn)thunk;
  nativeVirtual.Guid = TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionRef = builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    ZilchTypeId(Return),
    FunctionOptions::Virtual,
    nativeVirtual
  );
  ++classBoundType->BoundNativeVirtualCount;
  ErrorIf(classBoundType->BoundNativeVirtualCount > classBoundType->RawNativeVirtualCount,
    "The number of bound virtual functions must never exceed the actual v-table count");
  return functionRef;
}
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
Return VirtualThunkReturn(Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
{
  byte* virtualTable = *(byte**)this;
  byte* typePointer = virtualTable - sizeof(BoundType*) - sizeof(ExecutableState*);
  byte* executableStatePointer = virtualTable - sizeof(ExecutableState*);
  BoundType* type = *(BoundType**)typePointer;
  ExecutableState* state = *(ExecutableState**)(executableStatePointer);
  GuidType virtualId = type->Hash() ^ TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionToCall = state->ThunksToFunctions.FindValue(virtualId, nullptr);
  ErrorIf(functionToCall == nullptr,
    "There was no function found by the guid, what happened?");
  HandleManagers& managers = HandleManagers::GetInstance();
  HandleManager* pointerManager = managers.GetSharedManager(ZilchManagerId(PointerManager));
  Handle thisHandle;
  thisHandle.Manager = pointerManager;
  thisHandle.Type = type;
  pointerManager->ObjectToHandle((byte*)this, thisHandle);
  Call call(functionToCall, state);
  call.SetHandle(Call::This, thisHandle);
  call.Set<Arg0>(0, arg0);
  call.Set<Arg1>(1, arg1);
  call.Set<Arg2>(2, arg2);
  call.Set<Arg3>(3, arg3);
  call.Set<Arg4>(4, arg4);
  ExceptionReport report;
  call.Invoke(report);
  return call.Get<Return>(Call::Return);
}
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
static Function* FromVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return(Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4))
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2, Arg3, Arg4>;
  ZilchAutoVal(thunk, (&VirtualThunkReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2, Arg3, Arg4>));
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
  NativeVirtualInfo nativeVirtual;
  nativeVirtual.Index = TypeBinding::GetVirtualMethodIndex(function);
  nativeVirtual.Thunk = (TypeBinding::VirtualTableFn)thunk;
  nativeVirtual.Guid = TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionRef = builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    ZilchTypeId(Return),
    FunctionOptions::Virtual,
    nativeVirtual
  );
  ++classBoundType->BoundNativeVirtualCount;
  ErrorIf(classBoundType->BoundNativeVirtualCount > classBoundType->RawNativeVirtualCount,
    "The number of bound virtual functions must never exceed the actual v-table count");
  return functionRef;
}
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
Return VirtualThunkReturn(Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
{
  byte* virtualTable = *(byte**)this;
  byte* typePointer = virtualTable - sizeof(BoundType*) - sizeof(ExecutableState*);
  byte* executableStatePointer = virtualTable - sizeof(ExecutableState*);
  BoundType* type = *(BoundType**)typePointer;
  ExecutableState* state = *(ExecutableState**)(executableStatePointer);
  GuidType virtualId = type->Hash() ^ TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionToCall = state->ThunksToFunctions.FindValue(virtualId, nullptr);
  ErrorIf(functionToCall == nullptr,
    "There was no function found by the guid, what happened?");
  HandleManagers& managers = HandleManagers::GetInstance();
  HandleManager* pointerManager = managers.GetSharedManager(ZilchManagerId(PointerManager));
  Handle thisHandle;
  thisHandle.Manager = pointerManager;
  thisHandle.Type = type;
  pointerManager->ObjectToHandle((byte*)this, thisHandle);
  Call call(functionToCall, state);
  call.SetHandle(Call::This, thisHandle);
  call.Set<Arg0>(0, arg0);
  call.Set<Arg1>(1, arg1);
  call.Set<Arg2>(2, arg2);
  call.Set<Arg3>(3, arg3);
  call.Set<Arg4>(4, arg4);
  call.Set<Arg5>(5, arg5);
  ExceptionReport report;
  call.Invoke(report);
  return call.Get<Return>(Call::Return);
}
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
static Function* FromVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return(Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5))
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>;
  ZilchAutoVal(thunk, (&VirtualThunkReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5>));
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
  NativeVirtualInfo nativeVirtual;
  nativeVirtual.Index = TypeBinding::GetVirtualMethodIndex(function);
  nativeVirtual.Thunk = (TypeBinding::VirtualTableFn)thunk;
  nativeVirtual.Guid = TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionRef = builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    ZilchTypeId(Return),
    FunctionOptions::Virtual,
    nativeVirtual
  );
  ++classBoundType->BoundNativeVirtualCount;
  ErrorIf(classBoundType->BoundNativeVirtualCount > classBoundType->RawNativeVirtualCount,
    "The number of bound virtual functions must never exceed the actual v-table count");
  return functionRef;
}
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
Return VirtualThunkReturn(Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
{
  byte* virtualTable = *(byte**)this;
  byte* typePointer = virtualTable - sizeof(BoundType*) - sizeof(ExecutableState*);
  byte* executableStatePointer = virtualTable - sizeof(ExecutableState*);
  BoundType* type = *(BoundType**)typePointer;
  ExecutableState* state = *(ExecutableState**)(executableStatePointer);
  GuidType virtualId = type->Hash() ^ TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionToCall = state->ThunksToFunctions.FindValue(virtualId, nullptr);
  ErrorIf(functionToCall == nullptr,
    "There was no function found by the guid, what happened?");
  HandleManagers& managers = HandleManagers::GetInstance();
  HandleManager* pointerManager = managers.GetSharedManager(ZilchManagerId(PointerManager));
  Handle thisHandle;
  thisHandle.Manager = pointerManager;
  thisHandle.Type = type;
  pointerManager->ObjectToHandle((byte*)this, thisHandle);
  Call call(functionToCall, state);
  call.SetHandle(Call::This, thisHandle);
  call.Set<Arg0>(0, arg0);
  call.Set<Arg1>(1, arg1);
  call.Set<Arg2>(2, arg2);
  call.Set<Arg3>(3, arg3);
  call.Set<Arg4>(4, arg4);
  call.Set<Arg5>(5, arg5);
  call.Set<Arg6>(6, arg6);
  ExceptionReport report;
  call.Invoke(report);
  return call.Get<Return>(Call::Return);
}
template <typename FunctionType, FunctionType function, typename Class, typename Return, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
static Function* FromVirtual(LibraryBuilder& builder, BoundType* classBoundType, StringRange name, StringRange spaceDelimitedNames, Return(Class::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6))
{
  BoundFn boundFunction = BoundInstanceReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>;
  ZilchAutoVal(thunk, (&VirtualThunkReturn<FunctionType, function, Class, Return, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6>));
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
  NativeVirtualInfo nativeVirtual;
  nativeVirtual.Index = TypeBinding::GetVirtualMethodIndex(function);
  nativeVirtual.Thunk = (TypeBinding::VirtualTableFn)thunk;
  nativeVirtual.Guid = TypeBinding::GetFunctionUniqueId<FunctionType, function>();
  Function* functionRef = builder.AddBoundFunction
  (
    classBoundType,
    name,
    boundFunction,
    parameters,
    ZilchTypeId(Return),
    FunctionOptions::Virtual,
    nativeVirtual
  );
  ++classBoundType->BoundNativeVirtualCount;
  ErrorIf(classBoundType->BoundNativeVirtualCount > classBoundType->RawNativeVirtualCount,
    "The number of bound virtual functions must never exceed the actual v-table count");
  return functionRef;
}
