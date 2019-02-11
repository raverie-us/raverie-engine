// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

ModalBackgroundTaskProgessBar::ModalBackgroundTaskProgessBar(
    Composite* parent, StringParam title, BackgroundTask* progressListener) :
    ModalStrip(parent)
{
  mCloseOnComplete = true;
  SetStripHeight(ModalSizeMode::Fixed, Pixels(84));

  mStripArea->SetLayout(CreateRowLayout());

  // Spacer
  new Spacer(mStripArea);

  Composite* center = new Composite(mStripArea);
  center->SetLayout(CreateStackLayout());
  center->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
  {
    mTitle = new Text(center, "ModalConfirmTitle");
    mTitle->SetText(title);
    mTitle->SizeToContents();
    mTitle->mAlign = TextAlign::Center;
    ProxyAndAnimateIn(mTitle, Pixels(-400, 0, 0), 0.22f, 0.1f, 0);

    mProgressBar = new ProgressBar(center);
    mProgressBar->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
    ProxyAndAnimateIn(mProgressBar, Pixels(-400, 0, 0), 0.22f, 0.1f, 0);
  }

  // Spacer
  new Spacer(mStripArea);
  ConnectThisTo(
      progressListener, Events::BackgroundTaskUpdated, OnProgressUpdated);
  ConnectThisTo(progressListener, Events::BackgroundTaskCompleted, OnTaskEnded);
  ConnectThisTo(progressListener, Events::BackgroundTaskFailed, OnTaskEnded);
}

void ModalBackgroundTaskProgessBar::OnProgressUpdated(BackgroundTaskEvent* e)
{
  UpdateProgress(e->PercentComplete);
}

void ModalBackgroundTaskProgessBar::OnTaskEnded(BackgroundTaskEvent* e)
{
  if (mCloseOnComplete)
    Close();
}

void ModalBackgroundTaskProgessBar::UpdateProgress(float percentComplete)
{
  mProgressBar->SetPercentage(percentComplete);
  if (percentComplete >= 1.0f && mCloseOnComplete)
    Close();
}

} // namespace Zero
