// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{
template <typename T>
RaverieDefineType(Range<T>, builder, type)
{
  type->CopyMode = ::Raverie::TypeCopyMode::ReferenceType;
  RaverieFullBindDestructor(builder, type, RaverieSelf);
  RaverieFullBindConstructor(builder, type, RaverieSelf, RaverieNoNames);
  RaverieFullBindGetterSetter(builder, type, &RaverieSelf::GetAll, RaverieNoOverload, RaverieNoSetter, RaverieNoOverload, "All");
  RaverieFullBindGetterSetter(
      builder, type, &RaverieSelf::GetCurrent, RaverieNoOverload, RaverieNoSetter, RaverieNoOverload, "Current");
  RaverieFullBindGetterSetter(
      builder, type, &RaverieSelf::IsEmpty, RaverieNoOverload, RaverieNoSetter, RaverieNoOverload, "IsEmpty");
  RaverieFullBindGetterSetter(
      builder, type, &RaverieSelf::IsNotEmpty, RaverieNoOverload, RaverieNoSetter, RaverieNoOverload, "IsNotEmpty");
  RaverieFullBindMethod(builder, type, &RaverieSelf::MoveNext, RaverieNoOverload, "MoveNext", RaverieNoNames);
  RaverieFullBindMethod(builder, type, &RaverieSelf::Reset, RaverieNoOverload, "Reset", RaverieNoNames);
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

#  define RaverieDeclareRange(RangeType) RaverieDeclareExternalType(RangeType)
#  define RaverieDefineRange(RangeType)                                                                                  \
    RaverieDefineExternalBaseType(RangeType, ::Raverie::TypeCopyMode::ReferenceType, builder, type)                             \
    {                                                                                                                  \
      RaverieBindDefaultCopyDestructor();                                                                                \
      RaverieFullBindGetterSetter(                                                                                       \
          builder, type, &::Raverie::RangeAdapter<RaverieSelf>::All, RaverieNoOverload, RaverieNoSetter, RaverieNoOverload, "All");   \
      RaverieFullBindGetterSetter(builder,                                                                               \
                                type,                                                                                  \
                                &::Raverie::RangeAdapter<RaverieSelf>::Current,                                                 \
                                RaverieNoOverload,                                                                       \
                                RaverieNoSetter,                                                                         \
                                RaverieNoOverload,                                                                       \
                                "Current");                                                                            \
      RaverieFullBindGetterSetter(builder,                                                                               \
                                type,                                                                                  \
                                &::Raverie::RangeAdapter<RaverieSelf>::IsNotEmpty,                                              \
                                RaverieNoOverload,                                                                       \
                                RaverieNoSetter,                                                                         \
                                RaverieNoOverload,                                                                       \
                                "IsNotEmpty");                                                                         \
      RaverieFullBindGetterSetter(builder,                                                                               \
                                type,                                                                                  \
                                &::Raverie::RangeAdapter<RaverieSelf>::IsEmpty,                                                 \
                                RaverieNoOverload,                                                                       \
                                RaverieNoSetter,                                                                         \
                                RaverieNoOverload,                                                                       \
                                "IsEmpty");                                                                            \
      RaverieFullBindMethod(                                                                                             \
          builder, type, &::Raverie::RangeAdapter<RaverieSelf>::MoveNext, RaverieNoOverload, "MoveNext", RaverieNoNames);           \
    }
#  define RaverieInitializeRange(RangeType) RaverieInitializeExternalType(RangeType)
#  define RaverieInitializeRangeAs(RangeType, Name) RaverieInitializeExternalTypeAs(RangeType, Name)
} // namespace Raverie
