// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

//for enum types
template <>
struct ZeroShared HashPolicy<spv::Op> : public HashPolicy<int>
{
};

template <>
struct ZeroShared HashPolicy<spv::Capability> : public HashPolicy<int>
{
};

} // namespace Zero
