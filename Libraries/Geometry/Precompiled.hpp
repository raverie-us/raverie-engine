// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

#include "GeometryStandard.hpp"

namespace Geometry
{
/// Determines whether all intersection and geometry functions will utilize
/// extra checks to prevent floating point errors.
extern const bool cGeometrySafeChecks;
} // namespace Geometry
