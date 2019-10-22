// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace DownloadUi
{
const cstr cLocation = "EditorUi/BackgroundTasks/Downloads";
Tweakable(Vec4, IconColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, ProgressPrimaryColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, ProgressBackgroundColor, Vec4(1, 1, 1, 1), cLocation);
} // namespace DownloadUi

String GetFileNameFromUrl(StringParam url)
{
  StringRange lastSlash = url.FindLastOf('/');
  return url.SubString(lastSlash.End(), url.End());
}

BackgroundTask* DownloadTaskJob::DownloadToBuffer(StringParam url)
{
  String fileName = GetFileNameFromUrl(url);
  return DownloadToBuffer(url, fileName);
}

BackgroundTask* DownloadTaskJob::DownloadToBuffer(StringParam url, StringParam fileName)
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

DownloadTaskJob::DownloadTaskJob(StringParam url, u64 forceCacheSeconds)
{
  mRequest = AsyncWebRequest::Create();
  AsyncWebRequest* request = mRequest;
  request->mForceCacheSeconds = forceCacheSeconds;

  // For platforms that support threading, we want to continue the web
  // response on a thread so it acts the same as a continued job.
  request->mSendEventsOnRequestThread = true;

  request->mUrl = url;
}

void DownloadTaskJob::Execute()
{
  // We connect on execute (not on construction) to ensure that anyone else who connects
  // will receive the event before we do, because we mark the background task as success or failure.
  AsyncWebRequest* request = mRequest;
  ConnectThisTo(request, Events::WebResponsePartialData, OnWebResponsePartialData);
  ConnectThisTo(request, Events::WebResponseComplete, OnWebResponseComplete);
  request->Run();
}

int DownloadTaskJob::Cancel()
{
  BackgroundTaskJob::Cancel();
  mRequest->Cancel();
  return 0;
}

float DownloadTaskJob::GetPercentageComplete()
{
  return mRequest->mProgress;
}

String DownloadTaskJob::GetData()
{
  return mRequest->GetData();
}

void DownloadTaskJob::OnWebResponsePartialData(WebResponseEvent* e)
{
  UpdateDownloadProgress();
}

void DownloadTaskJob::OnWebResponseComplete(WebResponseEvent* e)
{
  // The download failed if we didn't get the OK response
  if (e->mResponseCode != WebResponseCode::OK)
  {
    Failed();
    UpdateDownloadProgress();
    return;
  }

  // The download is completed
  mState = BackgroundTaskState::Completed;

  // Let them know we completed
  UpdateDownloadProgress();
}

void DownloadTaskJob::UpdateDownloadProgress()
{
  float percentComplete = GetPercentageComplete();
  String downloaded = HumanReadableFileSize(mRequest->mTotalDownloaded);
  String total = HumanReadableFileSize(mRequest->mTotalExpected);
  String progressText = String::Format("%0.0f%% (%s/%s)", percentComplete * 100.0f, downloaded.c_str(), total.c_str());
  UpdateProgress(mName, percentComplete, progressText);
}

} // namespace Zero
