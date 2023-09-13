// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
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

} // namespace Zero
