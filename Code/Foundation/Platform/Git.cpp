// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

Git::Git()
{
}

Git::~Git()
{
}

void Git::Clone(StringParam url, StringParam directory, GitCallback completed, void* userData)
{
  if (completed)
    completed(userData);
}

} // namespace Zero
