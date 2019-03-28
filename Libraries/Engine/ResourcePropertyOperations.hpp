// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

// Fills out the given hash set with all resources used by properties
// on the given object. Ignores hidden properties.
void GetResourcesFromProperties(HandleParam object, HashSet<Resource*>& resources);

} // namespace Zero
