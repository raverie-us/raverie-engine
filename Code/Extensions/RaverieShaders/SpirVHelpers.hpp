// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// for enum types
template <>
struct HashPolicy<spv::Op> : public HashPolicy<int>
{
};

template <>
struct HashPolicy<spv::Capability> : public HashPolicy<int>
{
};

} // namespace Raverie
