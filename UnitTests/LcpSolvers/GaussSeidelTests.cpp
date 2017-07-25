///////////////////////////////////////////////////////////////////////////////
///
///  \file GausSeidelTests.cpp
///  Unit tests for Gauss-Seidel Solver
///  
///  Authors: Joshua Davis
///  Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"

#include "LcpStandard.hpp"
#include "Math/GaussSeidelSolver.hpp"

#include "MatrixGenerators.hpp"
#include "Matrix3Suite.hpp"
#include "BlockMatrixSuite.hpp"
#include "FormatErrorCallbacks.hpp"

TestNormalMatrices(Math::GaussSeidelSolver,Math::IndexDim3Policy,GS,50);
TestBlockMatrices(Math::GaussSeidelSolver,Math::BlockCgPolicy,GS,3000);
RandomTestNormalMatrices(Math::GaussSeidelSolver,Math::IndexDim3Policy,GS,1000,1000);
RandomTestBlockMatrices(Math::GaussSeidelSolver,Math::BlockCgPolicy,GS,1000,12,1000);
