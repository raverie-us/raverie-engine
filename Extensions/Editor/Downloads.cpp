///////////////////////////////////////////////////////////////////////////////
///
/// \file Downloads.cpp
/// 
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace DownloadUi
{
const cstr cLocation = "EditorUi/BackgroundTasks/Downloads";
Tweakable(Vec4, IconColor, Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ProgressPrimaryColor, Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ProgressBackgroundColor, Vec4(1,1,1,1), cLocation);
}

//******************************************************************************
String GetFileNameFromUrl(StringParam url)
{
  StringRange lastSlash = url.FindLastOf('/');
  return url.SubString(lastSlash.End(), url.End());
}

//---------------------------------------------------------------- Download Task
//******************************************************************************
BackgroundTask* DownloadTaskJob::DownloadToBuffer(StringParam url)
{
  String fileName = GetFileNameFromUrl(url);
  return DownloadToBuffer(url, fileName);
}

//******************************************************************************
BackgroundTask* DownloadTaskJob::DownloadToBuffer(StringParam url,
                                               StringParam fileName)
{
  const String cDownloadIcon = "TaskDownload";

  DownloadTaskJob* downloadTaskJob = new DownloadTaskJob(url);
  downloadTaskJob->mName = fileName;

  BackgroundTask* task = Z::gBackgroundTasks->CreateTask(downloadTaskJob);
  task->mName = fileName;
  task->mIconName = cDownloadIcon;
  task->mIconColor = DownloadUi::IconColor;
  task->mProgressPrimaryColor = DownloadUi::ProgressPrimaryColor;
  task->mProgressBackgroundColor = DownloadUi::ProgressBackgroundColor;
  task->Execute();


  return task;
}

//******************************************************************************
DownloadTaskJob::DownloadTaskJob(StringParam url)
{
  mRequest.mUrl = url;
  mBytesDownloaded = 0;
  mTotalBytes = 0;
}

//******************************************************************************
int DownloadTaskJob::Execute()
{
  ConnectThisTo(&mRequest, Events::PartialWebResponse, OnPartialWebResponse);
  ConnectThisTo(&mRequest, Events::WebResponse, OnWebResponse);
  mRequest.Run();
  return 1;
}

//******************************************************************************
int DownloadTaskJob::Cancel()
{
  BackgroundTaskJob::Cancel();
  mRequest.Cancel();
  return 0;
}

//******************************************************************************
float DownloadTaskJob::GetPercentageComplete()
{
  return float(mBytesDownloaded) / float(mTotalBytes);
}

//******************************************************************************
void DownloadTaskJob::OnPartialWebResponse(WebResponseEvent* e)
{
  mData = e->Data;

  // Add to the amount of data download
  mBytesDownloaded += mData.SizeInBytes();

  // Find the total bytes being sent from the headers
  forRange(String header, e->ResponseHeaders.All())
  {
    StringTokenRange r(header, ':');
    if(r.Front() == "Content-Length")
    {
      r.PopFront();
      StringRange rest = r.Front();
      u32 tempVal;
      ToValue(rest, tempVal);
      mTotalBytes = (u64)tempVal;
      break;
    }
  }

  UpdateDownloadProgress();
}

//******************************************************************************
void DownloadTaskJob::OnWebResponse(WebResponseEvent* e)
{
  // The download failed if we didn't get the OK response
  if(e->ResponseCode != WebResponseCode::OK)
  {
    Failed();
    UpdateDownloadProgress();
    return;
  }

  // The download is completed
  mState = BackgroundTaskState::Completed;

  // Let them know we completed
  UpdateDownloadProgress();

  mData = e->Data;
}

//******************************************************************************
void DownloadTaskJob::UpdateDownloadProgress()
{
  float percentComplete = GetPercentageComplete();
  String downloaded = HumanReadableFileSize(mBytesDownloaded);
  String total = HumanReadableFileSize(mTotalBytes);
  String progressText = String::Format("%0.0f%% (%s/%s)", percentComplete * 100.0f,
                                       downloaded.c_str(), total.c_str());
  UpdateProgress(mName, percentComplete, progressText);
}

}//namespace Zero
