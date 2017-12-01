///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "ZilchShaderIRReflection.hpp"

namespace Zero
{

//-------------------------------------------------------------------MemberReflection
ShaderResourceReflectionData::ShaderResourceReflectionData()
{
  mBinding = -1;
  mLocation = -1;
  mSizeInBytes = 0;
  mDescriptorSet = 0;
  mStride = 0;
  mOffsetInBytes = 0;
}

}//namespace Zero
