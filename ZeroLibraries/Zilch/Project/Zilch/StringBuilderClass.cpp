/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  ZilchDefineType(StringBuilderExtended, builder, type)
  {
    ZilchFullBindDestructor(builder, type, StringBuilderExtended);
    ZilchFullBindConstructor(builder, type, StringBuilderExtended, nullptr);

    ZilchFullBindMethod(builder, type, &StringBuilderExtended::ToString, ZilchNoOverload, "ToString", nullptr);

    ZilchFullBindMethod(builder, type, &StringBuilderExtended::Clear, ZilchNoOverload, "Clear", nullptr);
    
    ZilchFullBindMethod(builder, type, &StringBuilderExtended::Write, (void (StringBuilderExtended::*)(AnyParam)), "Write", nullptr);
    
    ZilchFullBindMethod(builder, type, &StringBuilderExtended::WriteLine, (void (StringBuilderExtended::*)()), "WriteLine", nullptr);
    ZilchFullBindMethod(builder, type, &StringBuilderExtended::WriteLine, (void (StringBuilderExtended::*)(AnyParam)), "WriteLine", nullptr);
  }
  
  //***************************************************************************
  void StringBuilderExtended::Write(AnyParam value)
  {
    this->Append(value.ToString());
  }
  
  //***************************************************************************
  void StringBuilderExtended::Write(StringParam value)
  {
    this->Append(value);
  }

  //***************************************************************************
  void StringBuilderExtended::Write(StringRange value)
  {
    this->Append(value);
  }

  //***************************************************************************
  void StringBuilderExtended::Write(cstr value)
  {
    this->Append(value);
  }

  //***************************************************************************
  void StringBuilderExtended::Write(char value)
  {
    this->Append(value);
  }

  //***************************************************************************
  void StringBuilderExtended::Write(Zero::Rune value)
  {
    this->Append(value);
  }

  //***************************************************************************
  void StringBuilderExtended::Write(Rune value)
  {
    this->Append(value.mValue);
  }

  //***************************************************************************
  void StringBuilderExtended::Write(Integer value)
  {
    this->Append(IntegerToString(value));
  }

  //***************************************************************************
  void StringBuilderExtended::Write(Integer2Param value)
  {
    this->Append(Integer2ToString(value));
  }

  //***************************************************************************
  void StringBuilderExtended::Write(Integer3Param value)
  {
    this->Append(Integer3ToString(value));
  }

  //***************************************************************************
  void StringBuilderExtended::Write(Integer4Param value)
  {
    this->Append(Integer4ToString(value));
  }

  //***************************************************************************
  void StringBuilderExtended::Write(unsigned long long value)
  {
    this->Append(String::Format("%llu", value));
  }

  //***************************************************************************
  void StringBuilderExtended::Write(DoubleInteger value)
  {
    this->Append(DoubleIntegerToString(value));
  }
  
  //***************************************************************************
  void StringBuilderExtended::Write(Boolean value)
  {
    this->Append(BooleanToString(value));
  }

  //***************************************************************************
  void StringBuilderExtended::Write(Boolean2Param value)
  {
    this->Append(Boolean2ToString(value));
  }

  //***************************************************************************
  void StringBuilderExtended::Write(Boolean3Param value)
  {
    this->Append(Boolean3ToString(value));
  }

  //***************************************************************************
  void StringBuilderExtended::Write(Boolean4Param value)
  {
    this->Append(Boolean4ToString(value));
  }
  
  //***************************************************************************
  void StringBuilderExtended::Write(Real value)
  {
    this->Append(RealToString(value));
  }

  //***************************************************************************
  void StringBuilderExtended::Write(DoubleReal value)
  {
    this->Append(DoubleRealToString(value));
  }
  
  //***************************************************************************
  void StringBuilderExtended::Write(Real2Param value)
  {
    this->Append(Real2ToString(value));
  }
  
  //***************************************************************************
  void StringBuilderExtended::Write(Real3Param value)
  {
    this->Append(Real3ToString(value));
  }
  
  //***************************************************************************
  void StringBuilderExtended::Write(Real4Param value)
  {
    this->Append(Real4ToString(value));
  }
  
  //***************************************************************************
  void StringBuilderExtended::Write(QuaternionParam value)
  {
    this->Append(QuaternionToString(value));
  }
  
  //***************************************************************************
  void StringBuilderExtended::WriteLine()
  {
    this->Append("\n");
  }
  
  //***************************************************************************
  void StringBuilderExtended::WriteLine(AnyParam value)
  {
    this->Write(value);
    this->WriteLine();
  }
  
  //***************************************************************************
  void StringBuilderExtended::WriteLine(StringParam value)
  {
    this->Write(value);
    this->WriteLine();
  }

  //***************************************************************************
  void StringBuilderExtended::WriteLine(StringRange value)
  {
    this->Write(value);
    this->WriteLine();
  }

  //***************************************************************************
  void StringBuilderExtended::WriteLine(cstr value)
  {
    this->Write(value);
    this->WriteLine();
  }

  //***************************************************************************
  void StringBuilderExtended::WriteLine(char value)
  {
    this->Write(value);
    this->WriteLine();
  }

  //***************************************************************************
  void StringBuilderExtended::WriteLine(Zero::Rune value)
  {
    this->Write(value);
    this->WriteLine();
  }

  //***************************************************************************
  void StringBuilderExtended::WriteLine(Rune value)
  {
    this->Write(value);
    this->WriteLine();
  }
  
  //***************************************************************************
  void StringBuilderExtended::WriteLine(Integer value)
  {
    this->Write(value);
    this->WriteLine();
  }

  //***************************************************************************
  void StringBuilderExtended::WriteLine(Integer2Param value)
  {
    this->Write(value);
    this->WriteLine();
  }

  //***************************************************************************
  void StringBuilderExtended::WriteLine(Integer3Param value)
  {
    this->Write(value);
    this->WriteLine();
  }

  //***************************************************************************
  void StringBuilderExtended::WriteLine(Integer4Param value)
  {
    this->Write(value);
    this->WriteLine();
  }
  
  //***************************************************************************
  void StringBuilderExtended::WriteLine(unsigned long long value)
  {
    this->Write(value);
    this->WriteLine();
  }
  
  //***************************************************************************
  void StringBuilderExtended::WriteLine(DoubleInteger value)
  {
    this->Write(value);
    this->WriteLine();
  }
  
  //***************************************************************************
  void StringBuilderExtended::WriteLine(Boolean value)
  {
    this->Write(value);
    this->WriteLine();
  }

  //***************************************************************************
  void StringBuilderExtended::WriteLine(Boolean2Param value)
  {
    this->Write(value);
    this->WriteLine();
  }

  //***************************************************************************
  void StringBuilderExtended::WriteLine(Boolean3Param value)
  {
    this->Write(value);
    this->WriteLine();
  }

  //***************************************************************************
  void StringBuilderExtended::WriteLine(Boolean4Param value)
  {
    this->Write(value);
    this->WriteLine();
  }
  
  //***************************************************************************
  void StringBuilderExtended::WriteLine(Real value)
  {
    this->Write(value);
    this->WriteLine();
  }
  
  //***************************************************************************
  void StringBuilderExtended::WriteLine(DoubleReal value)
  {
    this->Write(value);
    this->WriteLine();
  }
  
  //***************************************************************************
  void StringBuilderExtended::WriteLine(Real2Param value)
  {
    this->Write(value);
    this->WriteLine();
  }
  
  //***************************************************************************
  void StringBuilderExtended::WriteLine(Real3Param value)
  {
    this->Write(value);
    this->WriteLine();
  }
  
  //***************************************************************************
  void StringBuilderExtended::WriteLine(Real4Param value)
  {
    this->Write(value);
    this->WriteLine();
  }
  
  //***************************************************************************
  void StringBuilderExtended::WriteLine(QuaternionParam value)
  {
    this->Write(value);
    this->WriteLine();
  }
  
  //***************************************************************************
  void StringBuilderExtended::Clear()
  {
    this->Deallocate();
  }

  //***************************************************************************
  String StringBuilderExtended::ToString() const
  {
    return StringBuilder::ToString();
  }

  //***************************************************************************
  ZilchDefineType(Rune, builder, type)
  {
    type->ToStringFunction = &Rune::ToString;

    ZilchFullBindConstructor(builder, type, Rune, nullptr);
    ZilchFullBindConstructor(builder, type, Rune, "value", int);
    ZilchFullBindDestructor(builder, type, Rune);
    ZilchBindGetterSetter(Value);
  }
  // Zero::Runes are converted to and treated as Zilch::Runes
  ZilchDeclareDefineImplicitRedirectType(Zero::Rune, Zilch::Rune, ZeroShared);
  //***************************************************************************
  Rune::Rune()
  {
    mValue.value = 0;
  }

  //***************************************************************************
  Rune::Rune(int value)
  {
    mValue = Zero::Rune(value);
  }
  
  Rune::Rune(const Zero::Rune zeroRune)
    : mValue(zeroRune)
  {

  }

  //***************************************************************************
  String Rune::ToString(const BoundType* type, const byte* data)
  {
    Rune* rune = (Rune*)data;
    return String(rune->mValue);
  }

  int Rune::GetValue()
  {
    return mValue.value;
  }

  void Rune::SetValue(int value)
  {
    mValue.value = value;
  }
  
  Rune::operator Zero::Rune() const
  {
    return mValue;
  }

  //***************************************************************************
  ZilchDefineType(RuneIterator, builder, type)
  {
    ZilchFullBindDestructor(builder, type, RuneIterator);
    ZilchFullBindConstructor(builder, type, RuneIterator, nullptr);

    BoundType* boolType = ZilchTypeId(Boolean);
    BoundType* integerType = ZilchTypeId(int);
    BoundType* voidType = ZilchTypeId(void);
    BoundType* runeType = ZilchTypeId(Rune);
    BoundType* stringType = ZilchTypeId(String);
    type->ToStringFunction = &RuneIterator::ToString;

    // Range interface
    builder.AddBoundGetterSetter(type, "All", type, nullptr, &RuneIterator::All, MemberOptions::None);
    builder.AddBoundFunction(type, "MoveNext", &RuneIterator::Increment, ParameterArray(), voidType, MemberOptions::None);
    builder.AddBoundGetterSetter(type, "Current", runeType, nullptr, &RuneIterator::Current, MemberOptions::None);
    builder.AddBoundGetterSetter(type, "IsNotEmpty", boolType, nullptr, &RuneIterator::IsNotEmpty, MemberOptions::None);

    // Iterator interface
    builder.AddBoundFunction(type, "Increment", &RuneIterator::Increment, ParameterArray(), voidType, MemberOptions::None);
    builder.AddBoundFunction(type, "Decrement", &RuneIterator::Decrement, ParameterArray(), voidType, MemberOptions::None);
    builder.AddBoundFunction(type, "Equals", &RuneIterator::Equals, OneParameter(type), boolType, MemberOptions::None);

    builder.AddBoundGetterSetter(type, "ByteIndex", integerType, nullptr, &RuneIterator::GetByteIndex, MemberOptions::None);
    builder.AddBoundGetterSetter(type, "OriginalString", stringType, nullptr, &StringRangeExtended::GetOriginalString, FunctionOptions::None);
  }

  //***************************************************************************
  String RuneIterator::ToString(const BoundType* type, const byte* data)
  {
    RuneIterator* iterator = (RuneIterator*)data;

    if (!iterator->mRange.Empty())
      return String(iterator->mRange.Front());

    return String();
  }

  //***************************************************************************
  void RuneIterator::All(Call& call, ExceptionReport& report)
  {
    call.SetHandle(Call::Return, call.GetHandle(Call::This));
  }

  //***************************************************************************
  void RuneIterator::Current(Call& call, ExceptionReport& report)
  {
    RuneIterator* self = (RuneIterator*)call.GetHandle(Call::This).Dereference();
    if(self->mRange.Empty())
    {
      // Throw an exception since the range was empty and we called Current
      call.GetState()->ThrowException(report, "The iterator reached the end and an attempt was made to get the current value");
      return;
    }

    Rune result(self->mRange.Front());
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  void RuneIterator::IsNotEmpty(Call& call, ExceptionReport& report)
  {
    RuneIterator* self = (RuneIterator*)call.GetHandle(Call::This).Dereference();

    call.Set(Call::Return, !self->mRange.Empty());
  }

  //***************************************************************************
  void RuneIterator::Increment(Call& call, ExceptionReport& report)
  {
    RuneIterator* self = (RuneIterator*)call.GetHandle(Call::This).Dereference();

    if(self->mRange.Empty())
    {
      call.GetState()->ThrowException(report, "Cannot increment an iterator past the end of the original string");
      return;
    }

    self->mRange.PopFront();
  }
  
  //***************************************************************************
  void RuneIterator::Decrement(Call& call, ExceptionReport& report)
  {
    RuneIterator* self = (RuneIterator*)call.GetHandle(Call::This).Dereference();
    self->mRange.DecrementPointerByRune(self->mRange.mBegin);
    if(self->mRange.Begin() < self->mRange.mOriginalString.Begin())
    {
      call.GetState()->ThrowException(report, "Cannot decrement an iterator before the begin of the original string");
      return;
    }
  }

  //***************************************************************************
  void RuneIterator::Equals(Call& call, ExceptionReport& report)
  {
    RuneIterator* self = (RuneIterator*)call.GetHandle(Call::This).Dereference();
    RuneIterator* param0 = call.Get<RuneIterator*>(0);

    bool result = self->mRange.Begin() == param0->mRange.Begin();
    call.Set(Call::Return, result);
  }
  
  //***************************************************************************
  void RuneIterator::GetByteIndex(Call& call, ExceptionReport& report)
  {
    RuneIterator* self = (RuneIterator*)call.GetHandle(Call::This).Dereference();
    int byteIndex = (int)(self->mRange.Begin() - self->mRange.mOriginalString.Begin());
    call.Set(Call::Return, byteIndex);
  }

  //***************************************************************************
  void RuneIterator::GetOriginalString(Call& call, ExceptionReport& report)
  {
    RuneIterator* self = (RuneIterator*)call.GetHandle(Call::This).Dereference();

    call.SetHandle(Call::Return, &self->mRange.mOriginalString);
  }

  //***************************************************************************
  void RuneIterator::FindRuneIndexFromIterator(Call& call, ExceptionReport& report)
  {
    //for now, just assume the byte index is the same as the rune index
    GetByteIndex(call, report);
  }

  //***************************************************************************
  ZilchDefineType(StringRangeExtended, builder, type)
  {
    ZilchFullBindDestructor(builder, type, StringRangeExtended);
    ZilchFullBindConstructor(builder, type, StringRangeExtended, nullptr);

    BoundType* booleanType = ZilchTypeId(bool);
    BoundType* integerType = ZilchTypeId(Integer);
    BoundType* iteratorType = ZilchTypeId(RuneIterator);
    BoundType* stringType = ZilchTypeId(String);
    BoundType* splitRangeType = ZilchTypeId(StringSplitRangeExtended);
    type->ToStringFunction = &StringRangeExtended::ToString;

    // Range interface
    builder.AddBoundGetterSetter(type, "All", type, nullptr, &StringRangeExtended::All, MemberOptions::None);
    ZilchFullBindMethod(builder, type, &StringRangeExtended::MoveNext, ZilchNoOverload, "MoveNext", nullptr);
    builder.AddBoundGetterSetter(type, "Current", ZilchTypeId(Rune), nullptr, &StringRangeExtended::Current, MemberOptions::None); 
    builder.AddBoundGetterSetter(type, "Empty", booleanType, nullptr, &StringRangeExtended::Empty, MemberOptions::None);
    builder.AddBoundGetterSetter(type, "IsNotEmpty", booleanType, nullptr, &StringRangeExtended::IsNotEmpty, MemberOptions::None);

    builder.AddBoundGetterSetter(type, "Begin", iteratorType, nullptr, &StringRangeExtended::Begin, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns the RuneIterator at the start of this range.");
    ZilchFullBindMethod(builder, type, &StringRangeExtended::CompareTo, ZilchNoOverload, "CompareTo", nullptr)
      ->Description = ZilchDocumentString("Returns if this string range is equal to the given range.");
    ZilchFullBindMethod(builder, type, &StringRangeExtended::Contains, ZilchNoOverload, "Contains", nullptr)
      ->Description = ZilchDocumentString("Returns if the string Contains the specified substring.");
    builder.AddBoundGetterSetter(type, "End", iteratorType, nullptr, &StringRangeExtended::End, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns the RuneIterator at the end (one past the last Rune) of this range.");
    ZilchFullBindMethod(builder, type, &StringRangeExtended::EndsWith, ZilchNoOverload, "EndsWith", nullptr)
      ->Description = ZilchDocumentString("Returns if the string ends with the specified substring.");
    builder.AddBoundFunction(type, "FindFirstOf", &StringRangeExtended::FindFirstOf, OneParameter(type), type, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns a StringRange that Contains the first occurrence of given StringRange.");
    builder.AddBoundFunction(type, "FindLastOf", &StringRangeExtended::FindLastOf, OneParameter(type), type, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns a StringRange that Contains the last occurrence of given StringRange.");
    builder.AddBoundFunction(type, "FindRangeInclusive", &StringRangeExtended::FindRangeInclusive, TwoParameters(type, "startRange", "endRange"), type, FunctionOptions::None)
      ->Description = ZilchDocumentString("Finds the first StringRange that starts with 'startRange' and ends with 'endRange'. This substring includes 'startRange' and 'endRange'.");
    builder.AddBoundFunction(type, "FindRangeExclusive", &StringRangeExtended::FindRangeExclusive, TwoParameters(type, "startRange", "endRange"), type, FunctionOptions::None)
      ->Description = ZilchDocumentString("Finds the first StringRange that starts with 'startRange' and ends with 'endRange'. This substring excludes 'startRange' and 'endRange'.");
    ZilchFullBindMethod(builder, type, &StringRangeExtended::Replace, ZilchNoOverload, "Replace", "oldValue newValue")
      ->Description = ZilchDocumentString("Returns a new string with all occurances of a substrings replaced with another substring.");
    builder.AddBoundFunction(type, "Split", StringRangeExtended::Split, OneParameter(type, "separator"), splitRangeType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Splits the string, according to the separator string, into a range of substrings.");
    builder.AddBoundFunction(type, "SubString", &StringRangeExtended::SubString, TwoParameters(iteratorType, "begin", "end"), type, FunctionOptions::None)
      ->Description = ZilchDocumentString("Constructs a StringRange from the given begin and end iterators.");
    builder.AddBoundFunction(type, "SubStringBytes", &StringRangeExtended::SubStringBytes, TwoParameters(integerType, "startByteIndex", "lengthInBytes"), type, FunctionOptions::None)
      ->Description = ZilchDocumentString("Constructs a substring based upon a number of bytes. WARNING: strings are UTF8 so indexing by bytes could produce unexpected results on non-ascii strings.");
    ZilchFullBindMethod(builder, type, &StringRangeExtended::StartsWith, ZilchNoOverload, "StartsWith", nullptr)
      ->Description = ZilchDocumentString("Returns if the string ends with the specified substring.");
    builder.AddBoundFunction(type, "Trim", &StringRangeExtended::Trim, ParameterArray(), type, FunctionOptions::None)
      ->Description = ZilchDocumentString("Trims all leading and trailing whitespace.");
    builder.AddBoundFunction(type, "TrimEnd", &StringRangeExtended::TrimEnd, ParameterArray(), type, FunctionOptions::None)
      ->Description = ZilchDocumentString("Trims all trailing whitespace.");
    builder.AddBoundFunction(type, "TrimStart", &StringRangeExtended::TrimStart, ParameterArray(), type, FunctionOptions::None)
      ->Description = ZilchDocumentString("Trims all leading whitespace.");
    ZilchFullBindMethod(builder, type, &StringRangeExtended::ToLower, ZilchNoOverload, "ToLower", nullptr)
      ->Description = ZilchDocumentString("Returns a copy of the string that has been converted to lowercase.");
    builder.AddBoundFunction(type, "ToString", &StringRangeExtended::ConvertToString, ParameterArray(), stringType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns a new string of the current range.");
    ZilchFullBindMethod(builder, type, &StringRangeExtended::ToUpper, ZilchNoOverload, "ToUpper", nullptr)
      ->Description = ZilchDocumentString("Returns a copy of the string that has been converted to uppercase.");

    builder.AddBoundGetterSetter(type, "OriginalString", stringType, nullptr, &StringRangeExtended::GetOriginalString, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns the entire string that this range was constructed from.");
    builder.AddBoundFunction(type, "RuneIteratorFromByteIndex", &StringRangeExtended::RuneIteratorFromByteIndex, OneParameter(integerType, "byteIndex"), iteratorType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Finds the iterator from a byte index. WARNING: Strings are UTF8 and constructing an iterator from bytes indices can make an iterator in the middle of a rune.");
    builder.AddBoundFunction(type, "RuneIteratorFromRuneIndex", &StringRangeExtended::RuneIteratorFromRuneIndex, OneParameter(integerType, "runeIndex"), iteratorType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Finds the iterator from a rune index (the 'character' index). WARNING: this may be slow as finding an iterator from rune index requires a linear search.");
  }

  //***************************************************************************
  String StringRangeExtended::ToString(const BoundType* type, const byte* data)
  {
    StringRangeExtended* range = (StringRangeExtended*)data;

    return range->mRange;
  }

  //***************************************************************************
  void StringRangeExtended::All(Call& call, ExceptionReport& report)
  {
    call.SetHandle(Call::Return, call.GetHandle(Call::This));
  }

  //***************************************************************************
  void StringRangeExtended::MoveNext()
  {
    mRange.PopFront();
  }

  //***************************************************************************
  void StringRangeExtended::Current(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();

    Rune rune;
    if(!self->mRange.Empty())
      rune = Rune(self->mRange.Front());
    call.Set(Call::Return, rune);
  }

  //***************************************************************************
  void StringRangeExtended::Empty(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();

    call.Set(Call::Return, self->mRange.Empty());
  }

  //***************************************************************************
  void StringRangeExtended::IsNotEmpty(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();

    call.Set(Call::Return, !self->mRange.Empty());
  }

  //***************************************************************************
  void StringRangeExtended::Begin(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();

    StringRange result = StringRange(self->mRange.Begin(), self->mOriginalStringReference.End());
    SetResultIterator(call, report, result);
  }

  //***************************************************************************
  void StringRangeExtended::ConvertToString(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();

    if(ValidateRange(self->mOriginalStringReference, self->mRange) == false)
      return;

    String result = self->mRange;
    call.Set(Call::Return, &result);
  }

  //***************************************************************************
  int StringRangeExtended::CompareTo(StringRangeExtended range)
  {
    return mRange.CompareTo(range.mRange);
  }

  //***************************************************************************
  bool StringRangeExtended::Contains(StringRangeExtended string)
  {
    return mRange.Contains(string.mRange);
  }

  //***************************************************************************
  void StringRangeExtended::End(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();

    StringRange result = StringRange(self->mRange.End(), self->mOriginalStringReference.End());
    SetResultIterator(call, report, result);
  }

  //***************************************************************************
  bool StringRangeExtended::EndsWith(StringRangeExtended subString)
  {
    return mRange.EndsWith(subString.mRange);
  }

  //***************************************************************************
  void StringRangeExtended::FindFirstOf(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();
    StringRangeExtended* searchRange = call.Get<StringRangeExtended*>(0);

    if(ValidateRange(self->mOriginalStringReference, self->mRange) == false)
      return;

    StringRange result = self->mRange.FindFirstOf(searchRange->mRange);
    SetResultStringRange(call, report, self->mOriginalStringReference, result);
  }

  //***************************************************************************
  void StringRangeExtended::FindLastOf(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();
    StringRangeExtended* searchRange = call.Get<StringRangeExtended*>(0);

    if(ValidateRange(self->mOriginalStringReference, self->mRange) == false)
      return;

    StringRange result = self->mRange.FindLastOf(searchRange->mRange);
    SetResultStringRange(call, report, self->mOriginalStringReference, result);
  }

  //***************************************************************************
  void StringRangeExtended::FindRangeExclusive(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();
    StringRangeExtended* beginRange = call.Get<StringRangeExtended*>(0);
    StringRangeExtended* endRange = call.Get<StringRangeExtended*>(1);

    if(ValidateRange(self->mOriginalStringReference, self->mRange) == false)
      return;

    StringRange result = self->mRange.FindRangeExclusive(beginRange->mRange, endRange->mRange);
    SetResultStringRange(call, report, self->mOriginalStringReference, result);
  }

  //***************************************************************************
  void StringRangeExtended::FindRangeInclusive(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();
    StringRangeExtended* beginRange = call.Get<StringRangeExtended*>(0);
    StringRangeExtended* endRange = call.Get<StringRangeExtended*>(1);

    if(ValidateRange(self->mOriginalStringReference, self->mRange) == false)
      return;

    StringRange result = self->mRange.FindRangeInclusive(beginRange->mRange, endRange->mRange);
    SetResultStringRange(call, report, self->mOriginalStringReference, result);
  }

  //***************************************************************************
  String StringRangeExtended::Replace(StringRangeExtended oldValue, StringRangeExtended newValue)
  {
    return mRange.Replace(oldValue.mRange, newValue.mRange);
  }

  //***************************************************************************
  void StringRangeExtended::RuneIteratorFromByteIndex(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();
    int byteIndex = call.Get<int>(0);

    RuneIteratorFromByteIndexInternal(call, report, self->mOriginalStringReference, self->mRange, byteIndex);
  }

  //***************************************************************************
  void StringRangeExtended::RuneIteratorFromByteIndexInternal(Call& call, ExceptionReport& report, StringParam strRef, StringRange range, int byteIndex)
  {
    int sizeInBytes = (int)range.SizeInBytes();

    if(byteIndex < 0)
    {
      call.GetState()->ThrowException(report, "A negative byte index is not supported");
      return;
    }

    if(byteIndex >= sizeInBytes)
    {
      call.GetState()->ThrowException(report, "Byte index is greater than the size of the range");
      return;
    }
    cstr byteStart = range.Data() + byteIndex;
    cstr byteEnd   = byteStart + (sizeInBytes - byteIndex);
    StringRange result(range.mOriginalString, byteStart, byteEnd);
    SetResultIterator(call, report, result);
  }

  //***************************************************************************
  void StringRangeExtended::RuneIteratorFromRuneIndex(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();
    int byteIndex = call.Get<int>(0);

    RuneIteratorFromRuneIndexInternal(call, report, self->mOriginalStringReference, self->mRange, byteIndex);
  }

  //***************************************************************************
  void StringRangeExtended::RuneIteratorFromRuneIndexInternal(Call& call, ExceptionReport& report, StringParam strRef, StringRange range, int runeIndex)
  {
    if (runeIndex < 0)
    {
      call.GetState()->ThrowException(report, "A negative rune index is not supported");
      return;
    }

    StringIterator it = range.Begin() + runeIndex;
    StringRange result(it, range.End());
    SetResultIterator(call, report, result);
  }

  //***************************************************************************
  void StringRangeExtended::Split(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();
    StringRangeExtended* separatorStr = call.Get<StringRangeExtended*>(0);

    if(ValidateRange(self->mOriginalStringReference, self->mRange) == false)
      return;

    SetResultStringSplitRange(call, report, self->mOriginalStringReference, self->mRange.Split(separatorStr->mRange));
  }

  //***************************************************************************
  bool StringRangeExtended::StartsWith(StringRangeExtended subString)
  {
    return mRange.StartsWith(subString.mRange);
  }

  //***************************************************************************
  void StringRangeExtended::SubString(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();
    RuneIterator* beginIterator = call.Get<RuneIterator*>(0);
    RuneIterator* endIterator = call.Get<RuneIterator*>(1);
    //make sure that the begin and end are within the valid range of the original string
    cstr begin = Math::Clamp(beginIterator->mRange.mBegin, self->mOriginalStringReference.Data(), self->mOriginalStringReference.EndData());
    cstr end = Math::Clamp(endIterator->mRange.mBegin, self->mOriginalStringReference.Data(), self->mOriginalStringReference.EndData());
    //make sure the end is equal to or after begin
    if(end < begin)
      end = begin;

    SetResultStringRange(call, report, self->mOriginalStringReference, StringRange(self->mOriginalStringReference, begin, end));
  }

  //***************************************************************************
  void StringRangeExtended::SubStringBytes(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();
    int bytesStart = call.Get<int>(0);
    int bytesLength = call.Get<int>(1);

    cstr dataStart  = self->mRange.mOriginalString.Data();
    cstr rangeStart = dataStart + bytesStart;
    cstr rangeEnd   = rangeStart + bytesLength;
    StringRange result(self->mRange.mOriginalString, rangeStart, rangeEnd);
    SetResultStringRange(call, report, self->mOriginalStringReference, result);
  }

  //***************************************************************************
  void StringRangeExtended::Trim(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();

    if(ValidateRange(self->mOriginalStringReference, self->mRange) == false)
      return;

    StringRange result = self->mRange.Trim();
    SetResultStringRange(call, report, self->mOriginalStringReference, result);
  }

  //***************************************************************************
  void StringRangeExtended::TrimEnd(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();

    if(ValidateRange(self->mOriginalStringReference, self->mRange) == false)
      return;

    StringRange result = self->mRange.TrimEnd();
    SetResultStringRange(call, report, self->mOriginalStringReference, result);
  }

  //***************************************************************************
  void StringRangeExtended::TrimStart(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();

    if(ValidateRange(self->mOriginalStringReference, self->mRange) == false)
      return;

    StringRange result = self->mRange.TrimStart();
    ValidateRange(self->mOriginalStringReference, result);
    SetResultStringRange(call, report, self->mOriginalStringReference, result);
  }

  //***************************************************************************
  String StringRangeExtended::ToLower()
  {
    return mRange.ToLower();
  }

  //***************************************************************************
  String StringRangeExtended::ToUpper()
  {
    return mRange.ToUpper();
  }

  //***************************************************************************
  void StringRangeExtended::SetResultStringRange(Call& call, ExceptionReport& report, const String& strRef, StringRange result)
  {
    Handle rangeHandle = call.GetState()->AllocateDefaultConstructedHeapObject(ZilchTypeId(StringRangeExtended), report, HeapFlags::ReferenceCounted);
    StringRangeExtended& stringRange = *(StringRangeExtended*)rangeHandle.Dereference();
    stringRange.mOriginalStringReference = strRef;
    stringRange.mRange = result;
    call.SetHandle(Call::Return, rangeHandle);
  }

  //***************************************************************************
  void StringRangeExtended::SetResultStringSplitRange(Call& call, ExceptionReport& report, StringParam strRef, const Zero::StringSplitRange& result)
  {
    Handle rangeHandle = call.GetState()->AllocateHeapObject(ZilchTypeId(StringSplitRangeExtended), report, HeapFlags::ReferenceCounted);
    StringSplitRangeExtended& stringRange = *(StringSplitRangeExtended*)rangeHandle.Dereference();
    stringRange.mOriginalStringReference = strRef;
    stringRange.mSplitRange = result;
    call.SetHandle(Call::Return, rangeHandle);
  }

  //***************************************************************************
  void StringRangeExtended::SetResultIterator(Call& call, ExceptionReport& report, StringRange result)
  {
    Handle rangeHandle = call.GetState()->AllocateDefaultConstructedHeapObject(ZilchTypeId(RuneIterator), report, HeapFlags::ReferenceCounted);
    RuneIterator& stringRange = *(RuneIterator*)rangeHandle.Dereference();
    stringRange.mRange = result;
    call.SetHandle(Call::Return, rangeHandle);
  }

  //***************************************************************************
  void StringRangeExtended::GetOriginalString(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* self = (StringRangeExtended*)call.GetHandle(Call::This).Dereference();

    call.SetHandle(Call::Return, &self->mOriginalStringReference);
  }

  bool StringRangeExtended::ValidateRange(StringParam strRef, const StringRange& range)
  {
    //the range is empty so it's always valid
    if(range.Begin() == range.End())
      return true;

    if(range.Begin() < strRef.Begin() || range.Begin() > strRef.End() ||
       range.End() < strRef.Begin() || range.End() > strRef.End() ||
       range.Begin() > range.End())
    {
      ExecutableState* state = ExecutableState::CallingState;
      ExceptionReport& report = state->GetCallingReport();
      state->ThrowException(report, "The range is invalid. Most likely the begin/end iterators were manually moved.");
      return false;
    }
    
    return true;
  }

  //***************************************************************************
  ZilchDefineType(StringSplitRangeExtended, builder, type)
  {
    ZilchFullBindDestructor(builder, type, StringRangeExtended);

    // Range interface
    builder.AddBoundGetterSetter(type, "All", type, nullptr, &StringSplitRangeExtended::All, MemberOptions::None);
    builder.AddBoundFunction(type, "MoveNext", &StringSplitRangeExtended::MoveNext, ParameterArray(), ZilchTypeId(void), MemberOptions::None);
    builder.AddBoundGetterSetter(type, "Current", ZilchTypeId(StringRangeExtended), nullptr, &StringSplitRangeExtended::Current, MemberOptions::None);
    builder.AddBoundGetterSetter(type, "IsNotEmpty", ZilchTypeId(Boolean), nullptr, &StringSplitRangeExtended::IsNotEmpty, MemberOptions::None);
  }

  //***************************************************************************
  void StringSplitRangeExtended::All(Call& call, ExceptionReport& report)
  {
    call.SetHandle(Call::Return, call.GetHandle(Call::This));
  }

  //***************************************************************************
  void StringSplitRangeExtended::MoveNext(Call& call, ExceptionReport& report)
  {
    StringSplitRangeExtended* self = (StringSplitRangeExtended*)call.GetHandle(Call::This).Dereference();
    self->mSplitRange.PopFront();
  }

  //***************************************************************************
  void StringSplitRangeExtended::Current(Call& call, ExceptionReport& report)
  {
    StringSplitRangeExtended* self = (StringSplitRangeExtended*)call.GetHandle(Call::This).Dereference();

    StringRangeExtended::SetResultStringRange(call, report, self->mOriginalStringReference, self->mSplitRange.Front());
  }

  //***************************************************************************
  void StringSplitRangeExtended::IsNotEmpty(Call& call, ExceptionReport& report)
  {
    StringSplitRangeExtended* self = (StringSplitRangeExtended*)call.GetHandle(Call::This).Dereference();
    call.Set(Call::Return, !self->mSplitRange.Empty());
  }
}
