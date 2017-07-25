///////////////////////////////////////////////////////////////////////////////
///
/// \file Precompiled.hpp
/// Precompiled header for posix library.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Utility/Standard.hpp"
#include "String/FixedString.hpp"
#include "Memory/Memory.hpp"

#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#include <new>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cctype>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;
