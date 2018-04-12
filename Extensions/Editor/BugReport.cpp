///////////////////////////////////////////////////////////////////////////////
/// Authors: Trevor Sundberg, Joshua Davis
/// Copyright 2010-2016, DigiPen Institute of Technology
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

BugReporter::BugReporter(Composite* parent) :
  Composite(parent)
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

  mIncludeClipboardImage = new TextCheckBox(this);
  mIncludeClipboardImage->SetText("Include Clipboard Image");

  mIncludeScreenshot = new TextCheckBox(this);
  mIncludeScreenshot->SetText("Include Screenshot");

  mIncludeProject = new TextCheckBox(this);
  mIncludeProject->SetText("Include Project");

  mSend = new TextButton(this);
  mSend->SetText("Send");

  ConnectThisTo(mSend, Events::ButtonPressed, OnSend);
  ConnectThisTo(mBrowse, Events::ButtonPressed, OnBrowse);
  ConnectThisTo(GetRootWidget(), Events::WidgetUpdate, OnUpdate);
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
  mIncludeClipboardImage->SetChecked(false);
  mIncludeScreenshot->SetChecked(false);
  mIncludeProject->SetChecked(false);

  mSelectorButton->SetSelectedItem(0, false);

  // Check for saved username from user's config.
  if (Z::gEditor != nullptr && Z::gEditor->mConfig != nullptr)
  {
    if (EditorConfig* editorConfig = Z::gEditor->mConfig->has(EditorConfig))
      mUsername->SetText(editorConfig->ZeroHubUsername);
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
  FileDialogConfig config;
  config.EventName = CallBackEvent;
  config.CallbackObject = this;
  config.Title = "Select attachment";
  config.AddFilter("All Files (*.*)", "*.*");
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

void BugReporter::OnUpdate(UpdateEvent* event)
{
  bool isClipboardChecked = mIncludeClipboardImage->GetChecked();

  if (isClipboardChecked)
  {
    OsShell* shell = Z::gEngine->has(OsShell);
    bool imageOnClipboard = shell->IsClipboardImageAvailable();

    if (!imageOnClipboard)
    {
      DoNotifyWarning("Bug Reporter", "There is no image on the clipboard, or the image type is not recognized (Bitmap, Dib, Tiff)");
      mIncludeClipboardImage->SetChecked(false);
    }
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
      editorConfig->ZeroHubUsername = mUsername->GetText();
  }

  BugReportJob* job = new BugReportJob();

  job->mUsername = mUsername->GetText();
  job->mTitle = mTitle->GetText();
  job->mDescription = mDescription->GetAllText();
  job->mRepro = mRepro->GetAllText();
  job->mIncludedFile = mIncludeFile->GetText();

  job->mReportType = mSelectorButton->mButtons[mSelectorButton->GetSelectedItem()]->mButtonText->GetText();

  OsShell* shell = Z::gEngine->has(OsShell);

  if (mIncludeClipboardImage->GetChecked())
  {
    shell->GetClipboardImage(&job->mClipboardImage);
  }

  if (mIncludeScreenshot->GetChecked())
  {
    shell->GetWindowImage(&job->mScreenshot);
  }

  if (mIncludeProject->GetChecked())
  {
    Cog* projectCog = Z::gEditor->mProject;
    if(projectCog)
    {
      ProjectSettings* project = projectCog->has(ProjectSettings);
      String projectFile = GenerateTempFile(project->ProjectName, ".zip");
      job->mFileName = projectFile;
      job->mProject = projectCog;
    }
    Z::gEditor->SaveAll(true);
  }

  //Z::gEngine->LoadingStart();
  Z::gJobs->AddJob(job);

  mSent = true;
  CloseTabContaining(this);
}

int BugReportJob::Execute()
{
  SendBlockingTaskStart("Reporting");

  size_t fileCount = 0;
  BlockingWebRequest request;
  String response;

  StringBuilder bugReportUrl;
  bugReportUrl.Append("https://bugs.zeroengine.io");

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
  if(project)
  {
    String projectDirectory = project->ProjectFolder;

    Status status;
    Archive projectArchive(ArchiveMode::Compressing);
    projectArchive.ArchiveDirectory(status, projectDirectory);
    projectArchive.WriteZipFile(mFileName);

    ++fileCount;
    request.AddFile(mFileName, BuildString("File", ToString(fileCount)));
  }

  if(!mIncludedFile.Empty())
  {
    ++fileCount;
    request.AddFile(mIncludedFile, BuildString("File", ToString(fileCount)));
  }

  if(mClipboardImage.Data != NULL)
  {
    String fileName = GenerateTempFile("ClipboardImage", ".png");
    Status status;
    SaveToPng(status, &mClipboardImage, fileName);
    if(status.Succeeded())
    {
      ++fileCount;
      request.AddFile(fileName, BuildString("File", ToString(fileCount)));
    }
  }

  if(mScreenshot.Data != NULL)
  {
    String fileName = GenerateTempFile("Screenshot", ".png");
    Status status;
    SaveToPng(status, &mScreenshot, fileName);
    if(status.Succeeded())
    {
      ++fileCount;
      request.AddFile(fileName, BuildString("File", ToString(fileCount)));
    }
  }

  // File the bug
  request.mUrl = bugReportUrl.ToString();
  response = request.Run();

  SendBlockingTaskFinish();
  return true;
}


}//namespace Zero
