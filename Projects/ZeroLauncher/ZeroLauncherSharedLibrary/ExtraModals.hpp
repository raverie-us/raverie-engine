///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Josh Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------------ ModalProgess
/// A Modal with a progress bar. Currently only works with background tasks.
class ModalBackgroundTaskProgessBar : public ModalStrip
{
public:

  typedef ModalBackgroundTaskProgessBar ZilchSelf;
  /// Constructor.
  ModalBackgroundTaskProgessBar(Composite* parent, StringParam title, BackgroundTask* progressListener = nullptr);

  void OnProgressUpdated(BackgroundTaskEvent* e);
  void UpdateProgress(float percentComplete);

  Text* mTitle;
  ProgressBar* mProgressBar;
  bool mCloseOnComplete;
};

}//namespace Zero
