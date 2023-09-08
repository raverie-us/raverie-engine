// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Platform/FpControl.hpp"

namespace Zero
{
uint FpuControlSystem::DefaultMask = 0;
bool FpuControlSystem::Active = false;

ScopeFpuExceptionsEnabler::ScopeFpuExceptionsEnabler()
{
}

ScopeFpuExceptionsEnabler::~ScopeFpuExceptionsEnabler()
{
}

ScopeFpuExceptionsDisabler::ScopeFpuExceptionsDisabler()
{
}

ScopeFpuExceptionsDisabler::~ScopeFpuExceptionsDisabler()
{
}

} // namespace Zero
