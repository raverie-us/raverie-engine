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
  mMinSize = Vec2(500, 600);

  new Label(this, cText, "Name:");
  mUserName = new TextBox(this);
  mUserName->SetEditable(true);

  new Label(this, cText, "Email:");
  mUserEmail = new TextBox(this);
  mUserEmail->SetEditable(true);

  new Label(this, cText, "Title:");
  mTitle = new TextBox(this);
  mTitle->SetEditable(true);

  new Label(this, cText, "Reproduction steps:");
  mRepro = new TextEditor(this);
  mRepro->SetMinSize(Vec2(100, 60));
  mRepro->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  mRepro->Append("\n\n");

  new Label(this, cText, "What's expected:");
  mExpected = new TextEditor(this);
  mExpected->SetMinSize(Vec2(100, 60));
  mExpected->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  mExpected->Append("\n\n");

  new Label(this, cText, "What happened:");
  mHappened = new TextEditor(this);
  mHappened->SetMinSize(Vec2(100, 60));
  mHappened->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  mHappened->Append("\n\n");
  
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
  mUserName->SetText(String());
  mUserEmail->SetText(String());
  mTitle->SetText(String());
  mExpected->SetAllText(String());
  mHappened->SetAllText(String());
  mRepro->SetAllText(String());
  mIncludeFile->SetText(String());
  mIncludeClipboardImage->SetChecked(false);
  mIncludeScreenshot->SetChecked(false);
  mIncludeProject->SetChecked(false);
  mUserName->TakeFocus();
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

  BugReportJob* job = new BugReportJob();

  job->mUserName = mUserName->GetText();
  job->mEmail = mUserEmail->GetText();
  job->mTitle = mTitle->GetText();
  job->mRepro = mRepro->GetAllText();
  job->mExpected = mExpected->GetAllText();
  job->mHappened = mHappened->GetAllText();
  job->mIncludedFile = mIncludeFile->GetText();

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
  request.AddField("UserName", mUserName);
  request.AddField("UserEmail", mEmail);
  request.AddField("Title", mTitle);
  request.AddField("Repro", mRepro);
  request.AddField("Expected", mExpected);
  request.AddField("Happened", mHappened);
  request.AddField("Revision", GetRevisionNumberString());
  request.AddField("ChangeSet", GetChangeSetString());
  request.AddField("Platform", GetPlatformString());
  request.AddField("BuildVersionName", GetBuildVersionName());
  request.AddField("Computer", BuildString(Os::ComputerName(), " - ", Os::UserName()));

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
