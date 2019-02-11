// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Zilch
{
Composition::Composition()
{
}

Composition::~Composition()
{
}

Composition* Composition::GetBaseComposition()
{
  return nullptr;
}

void Composition::ClearComponents()
{
  this->Components.Clear();
}
} // namespace Zilch
