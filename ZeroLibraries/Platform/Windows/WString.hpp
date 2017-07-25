///////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "String/String.hpp"

namespace Zero
{

/// This class should only ever be used to convert from narrow strings to
/// wide strings for interaction with Windows API specific interactions
class WString
{
public:
  explicit WString(const wchar_t* str);
  explicit WString(StringParam str);
  WString();
  WString(const WString& rhs);
  ~WString();

  WString& operator=(const WString& rhs);

  const wchar_t* c_str() const;
  size_t Size() const;
  size_t SizeInBytes() const;

  byte* Data();
  
  bool IsEmpty() const;
private:
  void InternalDeepCopy(const WString& rhs);
  size_t mSize;
  mutable ByteBufferBlock mData;
  static wchar_t mEmptyReturn;
};

String  Narrow(const wchar_t* wstr);
String  Narrow(const WString& wstr);
WString Widen (const char* str);
WString Widen (const String& str);
int Utf16ToUtf8(int utf16);

} //namespace Zero