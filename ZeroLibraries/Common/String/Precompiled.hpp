#pragma once
//Visual studio requires the precompiled header to be included the same for
//all files in the project EVEN when they are located in a different
//relative position. For G++ it needs to be a true path so this
//file redirects to the actual precompiled.
//For G++
#include "..\Precompiled.hpp"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
