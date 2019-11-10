// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
void GitCloneJob::OnCloneComplete(void* userData)
{
  GitCloneJob* self = (GitCloneJob*)userData;
  self->ExecuteAsyncEnd();
}

void GitCloneJob::ExecuteAsyncBegin()
{
  Git::GetInstance()->Clone(mUrl, mDirectory, &OnCloneComplete, this);
}

}
