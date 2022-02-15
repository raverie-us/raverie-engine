// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Hashing.hpp"
#include "Guid.hpp"

namespace Zero
{

size_t Guid::Hash() const
{
  return HashPolicy<u64>()(mValue);
}

} // namespace Zero
