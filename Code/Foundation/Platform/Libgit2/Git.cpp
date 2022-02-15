// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "../../../External/Libgit2/Repo/include/git2.h"

namespace Zero
{

Git::Git()
{
  git_libgit2_init();
}

Git::~Git()
{
  git_libgit2_shutdown();
}

void Git::Clone(StringParam url, StringParam directory, GitCallback completed, void* userData)
{
  git_repository* repository = nullptr;
  git_clone(&repository, url.c_str(), directory.c_str(), nullptr);
  if (completed)
    completed(userData);
}

} // namespace Zero
