// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#if !defined(WelderCompilerMsvc)
int vsprintf_s(char* buffer, size_t numberOfElements, const char* format, va_list args)
{
  return vsnprintf(buffer, numberOfElements, format, args);
}

int sprintf_s(char* buffer, size_t numberOfElements, const char* format, ...)
{
  va_list args;
  va_start(args, format);
  int result = vsnprintf(buffer, numberOfElements, format, args);
  va_end(args);
  return result;
}

int swprintf_s(wchar_t* buffer, size_t numberOfElements, const wchar_t* format, ...)
{
  va_list args;
  va_start(args, format);
  int result = vswprintf(buffer, numberOfElements, format, args);
  va_end(args);
  return result;
}

errno_t strcat_s(char* dest, rsize_t destsz, const char* src)
{
  if (strlen(src) >= destsz)
  {
    if (dest && destsz != 0)
      *dest = '\0';
    return 1;
  }

  strcat(dest, src);
  return 0;
}

errno_t wcscat_s(wchar_t* dest, rsize_t destsz, const wchar_t* src)
{
  if (wcslen(src) >= destsz)
  {
    if (dest && destsz != 0)
      *dest = '\0';
    return 1;
  }

  wcscat(dest, src);
  return 0;
}

errno_t strcpy_s(char* dest, rsize_t destsz, const char* src)
{
  if (strlen(src) >= destsz)
  {
    if (dest && destsz != 0)
      *dest = '\0';
    return 1;
  }

  strcpy(dest, src);
  return 0;
}

errno_t wcscpy_s(wchar_t* dest, rsize_t destsz, const wchar_t* src)
{
  if (wcslen(src) >= destsz)
  {
    if (dest && destsz != 0)
      *dest = '\0';
    return 1;
  }

  wcscpy(dest, src);
  return 0;
}

errno_t strncpy_s(char* dest, rsize_t destsz, const char* src, rsize_t count)
{
  if (count > destsz)
  {
    if (dest && destsz != 0)
      *dest = '\0';
    return 1;
  }

  strncpy(dest, src, count);
  return 0;
}
#endif
