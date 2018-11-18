///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------Hash Policy for enum types
template<>
struct ZeroShared HashPolicy<spv::Op> : public HashPolicy<int>
{
};

template<>
struct ZeroShared HashPolicy<spv::Capability> : public HashPolicy<int>
{
};

}//namespace Zero
