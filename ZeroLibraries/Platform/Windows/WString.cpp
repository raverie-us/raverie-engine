///////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "WString.hpp"

namespace Zero
{

wchar_t WString::mEmptyReturn(L'\0');

WString::WString(const wchar_t* str)
  : mSize(0)
{
  if(!str)
    return;
  //wcslen does not include the null terminating character in its count
  mSize = wcslen(str) + 1;
  wchar_t* wstr = new wchar_t[mSize];
  wcscpy(wstr, str);
  mData.SetData((byte*)wstr, mSize * sizeof(wchar_t), true);
}

WString::WString(StringParam str)
  : mSize(0)
{
  if(str.Empty())
    return;
  //first read the number of wide characters we need to allocate for our
  //wide character buffer going from UTF8 encoded chars -> UTF16 (windows wchar)
  //this step is necessary when using MultiByteToWideChar
  mSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
  wchar_t* wstr = new wchar_t[mSize];
  //using the acquired information needed allocate a destination buffer and covert
  //the utf8 encoded character string to a wide string
  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, mSize);
  mData.SetData((byte*)wstr, mSize * sizeof(wchar_t), true);
}

WString::WString(const WString& rhs)
{
  InternalDeepCopy(rhs);
}

WString::WString()
  : mSize(0)
{

}

WString::~WString()
{

}

WString& WString::operator=(const WString& rhs)
{
  // Deal with self assignment
  if (&rhs == this)
    return *this;

  if (mSize > 0)
    mData.Deallocate();

  InternalDeepCopy(rhs);

  return *this;
}

const wchar_t* WString::c_str() const
{
  if(mData.Size())
    return (wchar_t*)mData.GetBegin();
  return &mEmptyReturn;
}

size_t WString::Size() const
{
  return mSize;
}

size_t WString::SizeInBytes() const
{
  return mSize * sizeof(wchar_t);
}

byte* WString::Data()
{
  return mData.GetBegin();
}

bool WString::IsEmpty() const
{
  if(mSize)
    return false;
  return true;
}

void WString::InternalDeepCopy(const WString& rhs)
{
  mSize = rhs.mSize;

  Status status;
  size_t sizeInBytes = rhs.SizeInBytes();
  byte* data = new byte[sizeInBytes];
  rhs.mData.Read(status, data, sizeInBytes);

  mData.SetData(data, sizeInBytes, true);
}

String Narrow(const wchar_t *wstr)
{
  //first read the number of wide characters we need to allocate for our
  //character buffer going from UTF16 (windows wchar) -> UTF8 encoded chars
  //this step is necessary when using WideCharToMultiByte
  size_t size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
  //wcslen does not include the null terminating character in its count
  size_t wstrSize = wcslen(wstr) + 1;
  char* str = new char[size];
  //using the acquired information needed allocate a destination buffer and covert
  //the wide string to a utf8 encoded character string
  WideCharToMultiByte(CP_UTF8, 0, wstr, wstrSize, str, size, NULL, NULL);
  String ret(str);
  delete str;
  return ret;
}

String Narrow(const WString& wstr)
{
  if (wstr.IsEmpty())
    return String();
  //first read the number of wide characters we need to allocate for our
  //character buffer going from UTF16 (windows wchar) -> UTF8 encoded chars
  //this step is necessary when using WideCharToMultiByte
  size_t size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
  char* str = new char[size];
  //using the acquired information needed allocate a destination buffer and covert
  //the wide string to a utf8 encoded character string
  WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.Size(), str, size, NULL, NULL);
  String ret(str);
  delete str;
  return ret;
}

WString Widen(const char *str)
{
  if (!str)
    return WString();
  //first read the number of wide characters we need to allocate for our
  //wide character buffer going from UTF8 encoded chars -> UTF16 (windows wchar)
  //this step is necessary when using MultiByteToWideChar
  size_t size = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
  wchar_t* wstr = new wchar_t[size];
  //using the acquired information needed allocate a destination buffer and covert
  //the utf8 encoded character string to a wide string
  MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, size);
  WString ret(wstr);
  delete wstr;
  return ret;
}

WString Widen(const String &str)
{
  return WString(str);
}

int Utf16ToUtf8(int utf16)
{
  //standard ascii character, no need to convert, just return it.
  if (utf16 < 128)
    return utf16;
 
  //take the straight code point value and put it in a wchar
  wchar_t inputUTF16 = (wchar_t)utf16;

  size_t size = WideCharToMultiByte(CP_UTF8, 0, &inputUTF16, 1, NULL, 0, NULL, NULL);

  //output buffer is exact required size, no null terminator processed
  char outputUTF8[4] = {0};
  WideCharToMultiByte(CP_UTF8, 0, &inputUTF16, 1, outputUTF8, size, NULL, NULL);
  
  //after converting to UTF8 pack the code points back into an int
  int key = 0;
  for (size_t i = 0; i < size; ++i)
  {
    key <<= 8;
    key += (uchar)outputUTF8[i];
  }

  return key;
}

} //namespace Zero