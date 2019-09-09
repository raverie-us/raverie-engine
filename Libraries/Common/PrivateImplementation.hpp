// MIT Licensed (see LICENSE.md).
#pragma once

// Make sure the private data is of a type that is aligned for proper native
// alignment (most likely pointer reading alignment). Still take the size in
// bytes and round up to the nearest whole value.
#define ZeroDeclarePrivateDataBytes(SizeInBytes) MaxAlignmentType mPrivateData[ZeroAlignCount(SizeInBytes)];

#define ZeroDeclarePrivateData(Type, SizeInBytes)                                                                      \
  ZeroDeclarePrivateDataBytes(SizeInBytes);                                                                            \
  Type(const Type& right)                                                                                              \
  {                                                                                                                    \
  }                                                                                                                    \
  Type& operator=(const Type& right)                                                                                   \
  {                                                                                                                    \
    return *this;                                                                                                      \
  }

// Gets the private object of a given pointer (doesn't assume self) and uses the
// provided variable name.
#define ZeroGetObjectPrivateData(Type, pointer, name) Type* name = (Type*)(pointer)->mPrivateData;

#define ZeroGetPrivateData(Type) Type* self = (Type*)mPrivateData;

#define ZeroAssertPrivateDataSize(Type)                                                                                \
  static_assert(sizeof(Type) <= sizeof(mPrivateData),                                                                  \
                "Increase the size of the private data because the private "                                           \
                "type is too big");

// For completely pod private data, it's easier just to clear it out (no
// destructor)
#define ZeroMemClearPrivateData(Type)                                                                                  \
  memset(mPrivateData, 0, sizeof(mPrivateData));                                                                       \
  ZeroAssertPrivateDataSize(Type);                                                                                     \
  ZeroGetPrivateData(Type);

#define ZeroConstructPrivateData(Type, ...)                                                                            \
  Type* self = new (mPrivateData) Type();                                                                              \
  ZeroAssertPrivateDataSize(Type);

#define ZeroDestructPrivateData(Type, ...) ((Type*)mPrivateData)->~Type();
