///////////////////////////////////////////////////////////////////////////////
/// Authors: Trevor Sundberg
/// Copyright 2010-2011, DigiPen Institute of Technology
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
class Editor;
class TextEditor;
class WebResponseEvent;
class UpdateEvent;
class TextButton;

class BugReporter : public Composite
{
public:
  typedef BugReporter ZilchSelf;

  BugReporter(Composite* parent);
  ~BugReporter();

  void Reset();

  // Internal
  void OnSend(Event* event);
  void OnBrowse(Event* event);
  void OnBrowseSelected(OsFileSelection* event);
  void OnUpdate(UpdateEvent* event);
  
  TextBox* mUserName;
  TextBox* mUserEmail;
  TextBox* mTitle;
  TextButton* mSend;
  TextEditor* mExpected;
  TextEditor* mHappened;
  TextEditor* mRepro;
  
  TextBox* mIncludeFile;
  TextButton* mBrowse;
  TextCheckBox* mIncludeClipboardImage;
  TextCheckBox* mIncludeScreenshot;
  TextCheckBox* mIncludeProject;
  bool mSent;
};


class BugReportJob : public Job
{
public:
  int Execute();

  String mFileName;
  CogId mProject;
  String mUserName;
  String mEmail;
  String mTitle;
  String mRepro;
  String mExpected;
  String mHappened;
  String mIncludedFile;
  Image mScreenshot;
  Image mClipboardImage;
};


}//namespace Zero
