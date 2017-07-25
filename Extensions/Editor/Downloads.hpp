///////////////////////////////////////////////////////////////////////////////
///
/// \file Downloads.hpp
/// 
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//---------------------------------------------------------------- Download Task
class DownloadTaskJob : public BackgroundTaskJob
{
public:
  static BackgroundTask* DownloadToBuffer(StringParam url);
  static BackgroundTask* DownloadToBuffer(StringParam url, StringParam fileName);

  typedef DownloadTaskJob ZilchSelf;
  DownloadTaskJob(StringParam url);

  /// Job Interface.
  int Execute() override;
  int Cancel() override;

  float GetPercentageComplete();

  String mName;
  String mData;
  String mFileName;

protected:
  void OnPartialWebResponse(WebResponseEvent* e);
  void OnWebResponse(WebResponseEvent* e);
  void UpdateDownloadProgress();

  String mDownloadedLocation;
  u64 mBytesDownloaded, mTotalBytes;
  BlockingWebRequest mRequest;
};

}//namespace Zero
