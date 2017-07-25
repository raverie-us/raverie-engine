///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------ WebBrowserWidget
ZilchDefineType(MarketWidget, builder, type)
{
}

const char* cMarketURL = "https://market.zeroengine.io/?q=products";

MarketWidget::MarketWidget(Composite* composite) :
  WebBrowserWidget(composite, WebBrowserSetup(cMarketURL, cWebBrowserDefaultSize, false, Vec4(0.2f, 0.2f, 0.2f, 1.0f)))
{
  ConnectThisTo(mBrowser, Events::WebBrowserDownloadStarted, OnDownloadStarted);
}

void OnPackageDownloadCallback(BackgroundTask* task, Job* job);

void OnGenericDownloadCallback(BackgroundTask* task, Job* job)
{
  DownloadTaskJob* downloadJob = (DownloadTaskJob*)job;
  String location = FilePath::Combine(GetTemporaryDirectory(), downloadJob->mName);
  WriteStringRangeToFile(location, downloadJob->mData);
  Os::SystemOpenFile(location.c_str());
}

void MarketWidget::OnDownloadStarted(WebBrowserDownloadEvent* event)
{
  BackgroundTask* task = DownloadTaskJob::DownloadToBuffer(event->mUrl, event->mSuggestedFileName);

  String extension = FilePath::GetExtension(event->mSuggestedFileName);

  if (extension == "zeropack")
  {
    task->mCallback = &OnPackageDownloadCallback;
  }
  else
  {
    task->mCallback = &OnGenericDownloadCallback;
  }
}

} // namespace Zero
