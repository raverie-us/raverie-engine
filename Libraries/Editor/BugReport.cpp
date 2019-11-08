// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(BugReporterResponse);
} // namespace Events

ZilchDefineType(BugReporter, builder, type)
{
}

ZilchDefineType(BugReporterResponse, builder, type)
{
}

BugReporter::BugReporter(Composite* parent) : Composite(parent)
{
  mSent = false;
  this->SetLayout(CreateStackLayout());
  mMinSize = Vec2(10, 10);
  this->SetSize(Vec2(500, 600));

  new Label(this, cText, "ZeroHub Username:");
  mUsername = new TextBox(this);
  mUsername->SetEditable(true);

  mSelectorButton = new SelectorButton(this);
  mSelectorButton->CreateButton("Bug Report");
  mSelectorButton->CreateButton("Feature Request");
  mSelectorButton->SetSelectedItem(0, false);

  new Label(this, cText, "Title:");
  mTitle = new TextBox(this);
  mTitle->SetEditable(true);

  new Label(this, cText, "Description:");
  mDescription = new TextEditor(this);
  mDescription->SetMinSize(Vec2(10, 10));
  mDescription->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  mDescription->SetWordWrap(true);
  mDescription->DisableScrollBar(0);

  new Label(this, cText, "Reproduction Steps:");
  mRepro = new TextEditor(this);
  mRepro->SetMinSize(Vec2(10, 10));
  mRepro->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  mRepro->SetWordWrap(true);
  mRepro->DisableScrollBar(0);

  new Label(this, cText, "Include File:");

  auto fileRow = new Composite(this);
  fileRow->SetLayout(CreateStackLayout(LayoutDirection::RightToLeft, Vec2::cZero, Thickness::cZero));

  mBrowse = new TextButton(fileRow);
  mBrowse->SetText("Browse...");

  mIncludeFile = new TextBox(fileRow);
  mIncludeFile->SetSizing(SizeAxis::X, SizePolicy::Flex, 20);
  mIncludeFile->SetEditable(true);

  mIncludeScreenshot = new TextCheckBox(this);
  mIncludeScreenshot->SetText("Include Screenshot");

  mIncludeProject = new TextCheckBox(this);
  mIncludeProject->SetText("Include Project");

  mSend = new TextButton(this);
  mSend->SetText("Send");

  ConnectThisTo(mSend, Events::ButtonPressed, OnSend);
  ConnectThisTo(mBrowse, Events::ButtonPressed, OnBrowse);
}

BugReporter::~BugReporter()
{
}

void BugReporter::Reset()
{
  mSent = false;
  mUsername->SetText(String());
  mTitle->SetText(String());
  mDescription->SetAllText(String());
  mRepro->SetAllText(String());
  mIncludeFile->SetText(String());
  mIncludeScreenshot->SetChecked(false);
  mIncludeProject->SetChecked(false);

  mSelectorButton->SetSelectedItem(0, false);

  // Check for saved username from user's config.
  if (Z::gEditor != nullptr && Z::gEditor->mConfig != nullptr)
  {
    if (EditorConfig* editorConfig = Z::gEditor->mConfig->has(EditorConfig))
      mUsername->SetText(editorConfig->BugReportUsername);
  }

  // Set focus on username field if it's empty.
  if (mUsername->GetText().Empty())
    mUsername->TakeFocus();
  else
    mTitle->TakeFocus();
}

void BugReporter::OnBrowse(Event* event)
{
  // Set up the callback for when project file is selected
  const String CallBackEvent = "FolderCallback";
  if (!GetDispatcher()->IsConnected(CallBackEvent, this))
  {
    ConnectThisTo(this, CallBackEvent, OnBrowseSelected);
  }

  // Open the open file dialog
  FileDialogConfig* config = FileDialogConfig::Create();
  config->EventName = CallBackEvent;
  config->CallbackObject = this;
  config->Title = "Select attachment";
  config->AddFilter("All Files (*.*)", "*.*");
  Z::gEngine->has(OsShell)->OpenFile(config);
}

void BugReporter::OnBrowseSelected(OsFileSelection* event)
{
  if (event->Files.Size() > 0)
  {
    String path = event->Files[0];
    mIncludeFile->SetText(path);
  }
}

String GenerateTempFile(StringParam name, StringParam extension)
{
  String directory = GetTemporaryDirectory();
  String timeStamp = GetTimeAndDateStamp();
  String fileName = BuildString(name, timeStamp, extension);
  return FilePath::Combine(directory, fileName);
}

void BugReporter::OnSend(Event* event)
{
  // Verify that the user entered a title
  if (mTitle->GetText().Empty())
  {
    DoNotifyWarning("Bug Reporter", "You must give the bug a title");
    return;
  }

  // Prevent multiple clicks
  if (mSent)
    return;

  // Set the entered username on the config.
  if (Z::gEditor != nullptr && Z::gEditor->mConfig != nullptr)
  {
    if (EditorConfig* editorConfig = Z::gEditor->mConfig->has(EditorConfig))
      editorConfig->BugReportUsername = mUsername->GetText();
  }

  BugReportJob* job = new BugReportJob();

  job->mUsername = mUsername->GetText();
  job->mTitle = mTitle->GetText();
  job->mDescription = mDescription->GetAllText();
  job->mRepro = mRepro->GetAllText();
  job->mIncludedFile = mIncludeFile->GetText();

  job->mReportType = mSelectorButton->mButtons[mSelectorButton->GetSelectedItem()]->mButtonText->GetText();

  OsShell* shell = Z::gEngine->has(OsShell);

  if (mIncludeScreenshot->GetChecked())
  {
    shell->GetPrimaryMonitorImage(&job->mScreenshot);
  }

  if (mIncludeProject->GetChecked())
  {
    Cog* projectCog = Z::gEditor->mProject;
    if (projectCog)
    {
      ProjectSettings* project = projectCog->has(ProjectSettings);
      String projectFile = GenerateTempFile(project->ProjectName, ".zip");
      job->mFileName = projectFile;
      job->mProject = projectCog;
    }
    Z::gEditor->SaveAll(true);
  }

  // Start the blocking here (we end it in the web request).
  // There should never be any code path where we don't hit the web response /
  // end.
  SendBlockingTaskStart("Reporting");
  Z::gJobs->AddJob(job);

  mSent = true;
  CloseTabContaining(this);
}

void BugReportJob::Execute()
{
  BugReporterResponse* eventToSend = new BugReporterResponse();
  eventToSend->mRequest = AsyncWebRequest::Create();
  size_t fileCount = 0;
  AsyncWebRequest& request = *eventToSend->mRequest.Dereference();
  String response;

  // TODO: Needs to be fixed for the GitHub API.
  StringBuilder bugReportUrl;
  bugReportUrl.Append(Urls::cApiReportIssue);

  request.AddField("Key", "kcy43UsUp4Rz/X0OFnCHDmgZECqB9NZbUTdx7chShJA=");
  request.AddField("UserName", mUsername);
  request.AddField("Title", mTitle);
  request.AddField("ReportType", mReportType);
  if (!mDescription.Empty())
    request.AddField("Description", mDescription);
  if (!mRepro.Empty())
    request.AddField("Repro", mRepro);
  request.AddField("Revision", GetRevisionNumberString());
  request.AddField("ChangeSet", GetChangeSetString());
  request.AddField("Platform", GetPlatformString());
  request.AddField("BuildVersionName", GetBuildVersionName());

  ProjectSettings* project = mProject.has(ProjectSettings);
  if (project)
  {
    String projectDirectory = project->ProjectFolder;

    Status status;
    Archive projectArchive(ArchiveMode::Compressing);
    projectArchive.ArchiveDirectory(status, projectDirectory);
    projectArchive.WriteZipFile(mFileName);

    ++fileCount;
    request.AddFile(BuildString("File", ToString(fileCount)), mFileName);
  }

  if (!mIncludedFile.Empty())
  {
    ++fileCount;
    request.AddFile(BuildString("File", ToString(fileCount)), mIncludedFile);
  }

  if (mClipboardImage.Data != NULL)
  {
    String fileName = GenerateTempFile("ClipboardImage", ".png");
    Status status;
    SaveImage(status, fileName, &mClipboardImage, ImageSaveFormat::Png);
    if (status.Succeeded())
    {
      ++fileCount;
      request.AddFile(BuildString("File", ToString(fileCount)), fileName);
    }
  }

  if (mScreenshot.Data != NULL)
  {
    String fileName = GenerateTempFile("Screenshot", ".png");
    Status status;
    SaveImage(status, fileName, &mScreenshot, ImageSaveFormat::Png);
    if (status.Succeeded())
    {
      ++fileCount;
      request.AddFile(BuildString("File", ToString(fileCount)), fileName);
    }
  }

  request.mUrl = bugReportUrl.ToString();

  ConnectThisTo(&request, Events::WebResponseComplete, OnWebResponseComplete);
  request.Run();

  // Keep the BugReportJob alive until we get the web response.
  AddReference();
}

void BugReportJob::OnWebResponseComplete(WebResponseEvent* event)
{
  SendBlockingTaskFinish();

  // Check if http response indicates fail or success.
  // Waypoint returns the following on success of both filing the task and
  // uploading associated files: "Success: T%taskId% | %Title% successfully
  // added to phabricator" Waypoint returns the following on failure of either
  // filing the task or uploading associated files: "HTTP %ErrorCode% Upload
  // Failed: %Error Message%"
  String response = event->mData;
  if (response.StartsWith("Success:"))
  {
    // Extract the task ID
    Regex taskIdRegex("T\\d+");
    Matches taskIdMatches;
    taskIdRegex.Search(response, taskIdMatches);

    // If there are no task Id's in the response then direct to user to the
    // latest bug reports
    if (taskIdMatches.Empty())
    {
      String message = String::Format("ZeroHub returned success, but did not include a TaskID. Please "
                                      "visit %s to find your task, or contact a ZeroHub administrator.",
                                      Urls::cUserLatestIssues);
      DoNotifyWarning("Bug Reporter", message);
      return;
    }

    // Build the notify message
    String taskId = taskIdMatches.Front();
    StringBuilder notifyBuilder;
    notifyBuilder.Append(response);
    notifyBuilder.Append("Bug URL: ");
    notifyBuilder.Append(Urls::cUserSpecificIssue);
    notifyBuilder.Append(taskId);

    // Notify the user that their bug was submitted successfully
    DoNotify("Bug Reporter", notifyBuilder.ToString(), "Disk");
  }
  // If the response does not start with "Success:" then it failed, in which
  // case the server response is returned to the user.
  else
  {
    DoNotifyWarning("Bug Reporter", response);

    // Open the browser to the bug report form if the bug reporter failed to
    // file the bug from the editor
    Os::OpenUrl(Urls::cUserReportIssue);
  }

  // We kept ourselves alive until the request was done by adding a reference
  // count. Free that count here now since the request is done. (See the end of
  // Execute).
  Release();
}

} // namespace Zero
