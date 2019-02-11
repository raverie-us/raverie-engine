// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

void Socket::Close()
{
  // Ignore any errors that are returned
  Status status;
  Close(status);
}

} // namespace Zero
