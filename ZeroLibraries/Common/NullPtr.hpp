///////////////////////////////////////////////////////////////////////////////
///
/// \file NullPtr.hpp
///
/// Authors: Joshua Davis
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#if defined(ZeroSupportsNullptr)

namespace Zero
{
  typedef std::nullptr_t NullPointerType;
}

#else // Older versions of GCC / Clang

namespace Zero
{
  class __NullPointerType
  {
  };
  
  typedef const __NullPointerType* NullPointerType;
}

#define nullptr NULL

#endif
