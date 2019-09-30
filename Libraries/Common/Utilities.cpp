// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
namespace Os
{
void SystemOpenFile(cstr file, uint verb, cstr parameters, cstr workingDirectory)
{
  Status status;
  SystemOpenFile(status, file, verb, parameters, workingDirectory);
}
} // namespace Os
} // namespace Zero
