///////////////////////////////////////////////////////////////////////////////
///
/// \file Precompiled.hpp
/// Precompiled header for the intersection library.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "GeometryStandard.hpp"

namespace Geometry
{
  ///Determines whether all intersection and geometry functions will utilize extra 
  ///checks to prevent floating point errors.
  extern const bool cGeometrySafeChecks;
}

