// MIT Licensed (see LICENSE.md).
#pragma once

// The forRange macros use two for loops, so we need to do some extra work
// to make the 'break' keyword properly break out of both loops. The
// '__continueLoop' variable is used to both only loop once on the
// second for loop, and to detect breaks.
// This function is used to Assign values to '__continueLoop' inside an
// if statement while avoiding the warning of assignment in a conditional
// statement
template <typename T>
bool AssignValue(T& toBeAssigned, T value)
{
  toBeAssigned = value;
  return true;
}

template <typename RangeType>
bool PopFront(RangeType& range)
{
  range.PopFront();
  return true;
}

void VoidReturn();

// With auto this is much easier to define
#define ZeroForRangeHelper(value, rangeName, rangeExpr, rangePostPop, rangePrePop)                                     \
  if (bool __continueLoop = true)                                                                                      \
    for (AutoDeclare(rangeName, rangeExpr); __continueLoop == true && !rangeName.Empty();                              \
         __continueLoop ? rangePostPop : VoidReturn())                                                                 \
      if (::AssignValue(__continueLoop, false))                                                                        \
        for (value = rangeName.Front(); !__continueLoop; __continueLoop = true)                                        \
  rangePrePop

// With auto this is much easier to define
#define ZeroForRangeReferenceHelper(value, rangeName, rangeExpr, rangePostPop, rangePrePop)                            \
  if (bool __continueLoop = true)                                                                                      \
    for (AutoDeclareReference(rangeName, rangeExpr); __continueLoop == true && !rangeName.Empty();                     \
         __continueLoop ? rangePostPop : VoidReturn())                                                                 \
      if (::AssignValue(__continueLoop, false))                                                                        \
        for (value = rangeName.Front(); !__continueLoop; __continueLoop = true)                                        \
  rangePrePop

// This is the classic version that we use which will pop after the entire
// iteration of the loop is complete Because some ranges may return references
// to values they actually store, we must use this order of popping
#define forRange(value, rangeExpr) ZeroForRangeHelper(value, __rangeT, (rangeExpr).All(), __rangeT.PopFront(), )
#define forRangeRef(value, rangeExpr)                                                                                  \
  ZeroForRangeReferenceHelper(value, __rangeT, (rangeExpr).All(), __rangeT.PopFront(), )

// A newer version of range iteration which allows us to name our range variable
// (so we can query it) This version will call 'front' and then immediately
// after grabbing the value it will call 'PopFront' This means if the range does
// something irregular such as storing the value and modifying the stored value
// in PopFront, it will break This form of iteration is generally safer however
// for iterating through intrusive lists and unlinking them as you go
#define ZeroForRangeVar(value, rangeName, rangeExpr)                                                                   \
  ZeroForRangeHelper(value, rangeName, (rangeExpr).All(), VoidReturn(), if (PopFront(rangeName)))
#define ZeroForRangeRefVar(value, rangeName, rangeExpr)                                                                \
  ZeroForRangeReferenceHelper(value, rangeName, (rangeExpr).All(), VoidReturn(), if (PopFront(rangeName)))
