///////////////////////////////////////////////////////////////////////////////
///
/// \file ForEachRange.hpp
/// Range based for each macro.
///
/// Authors: Chris Peters
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// The forRange macros use two for loops, so we need to do some extra work
// to make the 'break' keyword properly break out of both loops. The
// '__continueLoop' variable is used to both only loop once on the
// second for loop, and to detect breaks.
// This function is used to Assign values to '__continueLoop' inside an
// if statement while avoiding the warning of assignment in a conditional statement
template <typename T>
bool AssignValue(T& toBeAssigned, T value)
{
  toBeAssigned = value;
  return true;
}

#if !ZeroHasAuto && !ZeroHasTypeOf 

//namespace to contain helpers functions
namespace fr
{

//System for getting the type with out evaluating the expression.

//Type holder
template<typename type> struct typeOf{ typedef type encodedType; };

//Convert an expression of given type to an
//expression of type typeOf<type>
template<typename type>
typeOf<type> EncodeType(const type& t)
{
  return typeOf<type>();
}

//Convertible to typeOf<type> for any type
struct AnyTypeConvert
{
  template<typename type>
  operator typeOf<type> () const
  {
    return typeOf<type>();
  }
};

//Convert an expression to the type typeOf<type> without
//evaluating the expression by using the properties of the
//ternary operator.
#define ENCODED_TYPEOF(container) \
  (true ? fr::AnyTypeConvert() : fr::EncodeType(container))

struct RangeBase
{
  operator bool() const { return false; }
};

template<typename type>
struct AnyRange : RangeBase
{
  AnyRange(const type& r) : item(r) {}
  mutable type item;
};

//Return a stack version of the Range
//life time will be extended by reference
template<typename type>
AnyRange<type> BuildRange(const type& begin)
{
  return AnyRange<type>(begin);
}

template<typename type>
type& RangeCast(RangeBase& any)
{
  return static_cast<AnyRange<type>&>(any).item;
}

//Range based functions
template<typename type>
bool IsEmpty(RangeBase& r, typeOf<type>)
{
  return RangeCast<type>(r).Empty();
}

template<typename type>
void PopFront(RangeBase& r, typeOf<type>)
{
  RangeCast<type>(r).PopFront();
}

template<typename type>
typename type::value_type& 
  Front(RangeBase& r, typeOf<type>)
{
  return RangeCast<type>(r).Front();
}

}//namespace fr

#define forRange(value, rangeExpr)                                                 \
  if(bool __continueLoop = true)                                                   \
  for(fr::RangeBase& __rangeT = fr::BuildRange(rangeExpr);                         \
  __continueLoop == true && !fr::IsEmpty(__rangeT, ENCODED_TYPEOF(rangeExpr));     \
   fr::PopFront(__rangeT, ENCODED_TYPEOF(rangeExpr)))                              \
  if(::AssignValue(__continueLoop, false))                                         \
  for(value = fr::Front(__rangeT, ENCODED_TYPEOF(rangeExpr));                      \
  !__continueLoop; __continueLoop = true)

#else

template <typename RangeType>
bool PopFront(RangeType& range)
{
  range.PopFront();
  return true;
}

// With auto this is much easier to define
#define ZeroForRangeHelper(value, rangeName, rangeExpr, rangePostPop, rangePrePop)                                        \
  if(bool __continueLoop = true)                                                                                          \
  for(AutoDeclare(rangeName, rangeExpr); __continueLoop == true && !rangeName.Empty(); __continueLoop ? rangePostPop : 0) \
  if(::AssignValue(__continueLoop, false))                                                                                \
  for(value = rangeName.Front(); !__continueLoop; __continueLoop = true)                                                  \
  rangePrePop

// With auto this is much easier to define
#define ZeroForRangeReferenceHelper(value, rangeName, rangeExpr, rangePostPop, rangePrePop)                                        \
  if(bool __continueLoop = true)                                                                                                   \
  for(AutoDeclareReference(rangeName, rangeExpr); __continueLoop == true && !rangeName.Empty(); __continueLoop ? rangePostPop : 0) \
  if(::AssignValue(__continueLoop, false))                                                                                         \
  for(value = rangeName.Front(); !__continueLoop; __continueLoop = true)                                                           \
  rangePrePop

// This is the classic version that we use which will pop after the entire iteration of the loop is complete
// Because some ranges may return references to values they actually store, we must use this order of popping
#define forRange(value, rangeExpr) ZeroForRangeHelper(value, __rangeT, rangeExpr, __rangeT.PopFront(), )
#define forRangeRef(value, rangeExpr) ZeroForRangeReferenceHelper(value, __rangeT, rangeExpr, __rangeT.PopFront(), )

// A newer version of range iteration which allows us to name our range variable (so we can query it)
// This version will call 'front' and then immediately after grabbing the value it will call 'PopFront'
// This means if the range does something irregular such as storing the value and modifying the stored value in PopFront, it will break
// This form of iteration is generally safer however for iterating through intrusive lists and unlinking them as you go
#define ZeroForRangeVar(value, rangeName, rangeExpr) ZeroForRangeHelper(value, rangeName, rangeExpr, 0, if(PopFront(rangeName)))
#define ZeroForRangeRefVar(value, rangeName, rangeExpr) ZeroForRangeReferenceHelper(value, rangeName, rangeExpr, 0, if(PopFront(rangeName)))

#endif
