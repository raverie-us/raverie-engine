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
  void Execute() override;
  int Cancel() override;

  float GetPercentageComplete();
  String GetData();

  String mName;
  String mFileName;

protected:
  void OnWebResponsePartialData(WebResponseEvent* e);
  void OnWebResponseComplete(WebResponseEvent* e);
  void UpdateDownloadProgress();

  String mDownloadedLocation;
  HandleOf<AsyncWebRequest> mRequest;
};

}//namespace Zero
