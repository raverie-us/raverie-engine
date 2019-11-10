// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
typedef void (*GitCallback)(void* userData);

class Git : public ExplicitSingleton<Git>
{
public:
  Git();
  ~Git();

  // Clone a repository asynchronously.
  void Clone(StringParam url, StringParam directory, GitCallback completed, void* userData);
};

} // namespace Zero
