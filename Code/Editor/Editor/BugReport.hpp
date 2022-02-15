// MIT Licensed (see LICENSE.md).
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
} // namespace Events

/// Event carrying the AsyncWebRequest back to the main thread
/// so we can run it and listen for the completed response.
class BugReporterResponse : public Event
{
public:
  ZilchDeclareType(BugReporterResponse, TypeCopyMode::ReferenceType);
  HandleOf<AsyncWebRequest> mRequest;
};

class BugReporter : public Composite
{
public:
  ZilchDeclareType(BugReporter, TypeCopyMode::ReferenceType);

  BugReporter(Composite* parent);
  ~BugReporter();

  void Reset();

  // Internal
  void OnSend(Event* event);
  void OnBrowse(Event* event);
  void OnBrowseSelected(OsFileSelection* event);

  TextBox* mUsername;
  TextBox* mTitle;
  TextEditor* mDescription;
  TextEditor* mRepro;
  TextButton* mSend;
  SelectorButton* mSelectorButton;

  TextBox* mIncludeFile;
  TextButton* mBrowse;
  TextCheckBox* mIncludeScreenshot;
  TextCheckBox* mIncludeProject;
  bool mSent;
};

class BugReportJob : public Job
{
public:
  typedef BugReportJob ZilchSelf;
  void Execute();
  void OnWebResponseComplete(WebResponseEvent* event);

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

} // namespace Zero
