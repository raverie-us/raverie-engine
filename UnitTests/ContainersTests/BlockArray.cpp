///////////////////////////////////////////////////////////////////////////////
///
///  \file BlockArray.cpp
///  Unit tests for the block array.
///  
///  Authors: Joshua Davis
///  Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Utility/Typedefs.hpp"
#include "Math/Vector3.hpp"

#include "CppUnitLite2/CppUnitLite2.h"
#include "BlockArraySuite.hpp"
#include "String/String.hpp"


CreateBlockArrayTests(uint,6,PolicyUint);
CreateBlockArrayTests(uint,3,PolicyUint);
CreateBlockArrayTests(Vec3,3,PolicyVec3);
CreateBlockArrayTests(Vec3,8,PolicyVec3);
