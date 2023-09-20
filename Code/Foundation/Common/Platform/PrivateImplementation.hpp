// MIT Licensed (see LICENSE.md).
#pragma once

// Make sure the private data is of a type that is aligned for proper native
// alignment (most likely pointer reading alignment). Still take the size in
// bytes and round up to the nearest whole value.
#define RaverieDeclarePrivateDataBytes(SizeInBytes) MaxAlignmentType mPrivateData[RaverieAlignCount(SizeInBytes)];

#define RaverieDeclarePrivateData(Type, SizeInBytes)                                                                                                                                                   \
  RaverieDeclarePrivateDataBytes(SizeInBytes);                                                                                                                                                         \
  Type(const Type& right)                                                                                                                                                                              \
  {                                                                                                                                                                                                    \
  }                                                                                                                                                                                                    \
  Type& operator=(const Type& right)                                                                                                                                                                   \
  {                                                                                                                                                                                                    \
    return *this;                                                                                                                                                                                      \
  }

// Gets the private object of a given pointer (doesn't assume self) and uses the
// provided variable name.
#define RaverieGetObjectPrivateData(Type, pointer, name) Type* name = (Type*)(pointer)->mPrivateData;

#define RaverieGetPrivateData(Type) Type* self = (Type*)mPrivateData;

#define RaverieAssertPrivateDataSize(Type)                                                                                                                                                             \
  static_assert(sizeof(Type) <= sizeof(mPrivateData),                                                                                                                                                  \
                "Increase the size of the private data because the private "                                                                                                                           \
                "type is too big");

#define RaverieConstructPrivateData(Type, ...)                                                                                                                                                         \
  Type* self = new (mPrivateData) Type();                                                                                                                                                              \
  RaverieAssertPrivateDataSize(Type);

#define RaverieDestructPrivateData(Type, ...) ((Type*)mPrivateData)->~Type();
