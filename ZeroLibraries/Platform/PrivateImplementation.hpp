///////////////////////////////////////////////////////////////////////////////
///
/// \file File.hpp
/// Declaration of the helper macros.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Diagnostic/Diagnostic.hpp"

#define PlatformByteAlignmentSize sizeof(void*)

// Make sure the private data is of a type that is aligned for proper native alignment
// (most likely pointer reading alignment). Still take the size in bytes and round up to the nearest whole value.
#define ZeroDeclarePrivateDataBytes(SizeInBytes)                                                 \
  void* mPrivateData[(SizeInBytes + PlatformByteAlignmentSize - 1) / PlatformByteAlignmentSize];

#define ZeroDeclarePrivateData(Type, SizeInBytes) \
  ZeroDeclarePrivateDataBytes(SizeInBytes);       \
  Type(const Type& right);                        \
  Type& operator = (const Type& right);

// Gets the private object of a given pointer (doesn't assume self) and uses the provided variable name.
#define ZeroGetObjectPrivateData(Type, pointer, name) \
  Type* name = (Type*)(pointer)->mPrivateData;

#define ZeroGetPrivateData(Type) \
  Type* self = (Type*)mPrivateData;

#define ZeroAssertPrivateDataSize(Type)                                           \
  StaticAssert(IncreaseSizeOfPrivateData_PrivateTypeTooBig,                       \
    sizeof(Type) <= sizeof(mPrivateData),                                         \
    "Increase the size of the private data because the private type is too big");

// For completely pod private data, it's easier just to clear it out (no destructor)
#define ZeroMemClearPrivateData(Type)             \
  memset(mPrivateData, 0, sizeof(mPrivateData));  \
  ZeroAssertPrivateDataSize(Type);                \
  ZeroGetPrivateData(Type);

#define ZeroConstructPrivateData(Type, ...) \
  Type* self = new (mPrivateData) Type();   \
  ZeroAssertPrivateDataSize(Type);

#define ZeroDestructPrivateData(Type, ...)  \
    ((Type*)mPrivateData)->~Type();
