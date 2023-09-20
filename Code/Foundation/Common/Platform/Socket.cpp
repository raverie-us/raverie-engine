// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

void Socket::Close()
{
  // Ignore any errors that are returned
  Status status;
  Close(status);
}

} // namespace Raverie
