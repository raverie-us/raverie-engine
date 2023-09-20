// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "RaverieShaderIRReflection.hpp"

namespace Raverie
{

ShaderResourceReflectionData::ShaderResourceReflectionData()
{
  mBinding = -1;
  mLocation = -1;
  mSizeInBytes = 0;
  mDescriptorSet = 0;
  mStride = 0;
  mOffsetInBytes = 0;
}

} // namespace Raverie
