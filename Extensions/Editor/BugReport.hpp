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

namespace Events
{
	DeclareEvent(BugReporterResponse);
}//namespace Events

/// Event carrying the string http response from the waypoint server between the BugReportJob and the BugReporter instance
class BugReporterResponse : public Event
{
public:
	ZilchDeclareType(TypeCopyMode::ReferenceType);
	BugReporterResponse();
	BugReporterResponse(String response);
	String mResponse;
};

class BugReporter : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef BugReporter ZilchSelf;

  BugReporter(Composite* parent);
  ~BugReporter();

  void Reset();

  // Internal
  void OnSend(Event* event);
  void OnBrowse(Event* event);
  void OnBrowseSelected(OsFileSelection* event);
  void OnUpdate(UpdateEvent* event);
  void OnBugReporterResponse(BugReporterResponse* event);
  
  TextBox* mUsername;
  TextBox* mTitle;
  TextEditor* mDescription;
  TextEditor* mRepro;
  TextButton* mSend;
  SelectorButton* mSelectorButton;
  
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
  String mUsername;
  String mReportType;
  String mTitle;
  String mDescription;
  String mRepro;
  String mIncludedFile;
  Image mScreenshot;
  Image mClipboardImage;
};


}//namespace Zero
