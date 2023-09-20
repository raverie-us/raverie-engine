// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Fills out the given hash set with all resources used by properties
// on the given object. Ignores hidden properties.
void GetResourcesFromProperties(HandleParam object, HashSet<Resource*>& resources);

} // namespace Raverie
