// MIT Licensed (see LICENSE.md).

#pragma once
#ifndef ZILCH_RANGE_BINDING_HPP
#  define ZILCH_RANGE_BINDING_HPP

namespace Zilch
{
template <typename T>
ZilchDefineType(Range<T>, builder, type)
{
  type->CopyMode = ZZ::TypeCopyMode::ReferenceType;
  ZilchFullBindDestructor(builder, type, ZilchSelf);
  ZilchFullBindConstructor(builder, type, ZilchSelf, ZilchNoNames);
  ZilchFullBindGetterSetter(builder, type, &ZilchSelf::GetAll, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "All");
  ZilchFullBindGetterSetter(
      builder, type, &ZilchSelf::GetCurrent, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "Current");
  ZilchFullBindGetterSetter(
      builder, type, &ZilchSelf::IsEmpty, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "IsEmpty");
  ZilchFullBindGetterSetter(
      builder, type, &ZilchSelf::IsNotEmpty, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "IsNotEmpty");
  ZilchFullBindMethod(builder, type, &ZilchSelf::MoveNext, ZilchNoOverload, "MoveNext", ZilchNoNames);
  ZilchFullBindMethod(builder, type, &ZilchSelf::Reset, ZilchNoOverload, "Reset", ZilchNoNames);
}

// For when we are binding a standard range on a container
template <typename RangeAdapterBaseType>
class RangeAdapter : public RangeAdapterBaseType
{
  typedef typename RangeAdapterBaseType::FrontResult FrontResult;

public:
  RangeAdapterBaseType* All()
  {
    return this;
  }

  typename RangeAdapterBaseType::FrontResult Current()
  {
    if (this->IsEmpty())
    {
      // Throw an exception since the range was empty and we called Current
      if (ExecutableState::CallingState)
        ExecutableState::CallingState->ThrowException("The range reached the end and an attempt was made to get the "
                                                      "current value");

      return GetInvalid<FrontResult>();
    }

    return this->Front();
  }

  bool IsNotEmpty()
  {
    return !this->Empty();
  }

  bool IsEmpty()
  {
    return this->Empty();
  }

  void MoveNext()
  {
    if (this->IsEmpty())
    {
      // Throw an exception since the range was empty and we called MoveNext
      if (ExecutableState::CallingState)
        ExecutableState::CallingState->ThrowException("The range reached the end, but then an attempt was made to make "
                                                      "it iterate forward more");
      return;
    }

    this->PopFront();
  }
};

#  define ZilchDeclareRange(RangeType) ZilchDeclareExternalType(RangeType)
#  define ZilchDefineRange(RangeType)                                                                                  \
    ZilchDefineExternalBaseType(RangeType, ZZ::TypeCopyMode::ReferenceType, builder, type)                             \
    {                                                                                                                  \
      ZilchBindDefaultCopyDestructor();                                                                                \
      ZilchFullBindGetterSetter(                                                                                       \
          builder, type, &ZZ::RangeAdapter<ZilchSelf>::All, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "All");   \
      ZilchFullBindGetterSetter(builder,                                                                               \
                                type,                                                                                  \
                                &ZZ::RangeAdapter<ZilchSelf>::Current,                                                 \
                                ZilchNoOverload,                                                                       \
                                ZilchNoSetter,                                                                         \
                                ZilchNoOverload,                                                                       \
                                "Current");                                                                            \
      ZilchFullBindGetterSetter(builder,                                                                               \
                                type,                                                                                  \
                                &ZZ::RangeAdapter<ZilchSelf>::IsNotEmpty,                                              \
                                ZilchNoOverload,                                                                       \
                                ZilchNoSetter,                                                                         \
                                ZilchNoOverload,                                                                       \
                                "IsNotEmpty");                                                                         \
      ZilchFullBindGetterSetter(builder,                                                                               \
                                type,                                                                                  \
                                &ZZ::RangeAdapter<ZilchSelf>::IsEmpty,                                                 \
                                ZilchNoOverload,                                                                       \
                                ZilchNoSetter,                                                                         \
                                ZilchNoOverload,                                                                       \
                                "IsEmpty");                                                                            \
      ZilchFullBindMethod(                                                                                             \
          builder, type, &ZZ::RangeAdapter<ZilchSelf>::MoveNext, ZilchNoOverload, "MoveNext", ZilchNoNames);           \
    }
#  define ZilchInitializeRange(RangeType) ZilchInitializeExternalType(RangeType)
#  define ZilchInitializeRangeAs(RangeType, Name) ZilchInitializeExternalTypeAs(RangeType, Name)
} // namespace Zilch

#endif
