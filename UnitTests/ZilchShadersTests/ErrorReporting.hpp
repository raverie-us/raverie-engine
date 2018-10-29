///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

//-------------------------------------------------------------------LogMessage
// A cached message from a compilation or linking log
class LogMessage
{
public:
  LogMessage();
  LogMessage(StringParam filePath, StringParam lineNumber, StringParam message);

  String mFilePath;
  String mLineNumber;
  String mMessage;
};

//-------------------------------------------------------------------ErrorReporter
// Basic class to wrap reporting errors
class ErrorReporter : public Zilch::EventHandler
{
public:
  ErrorReporter();
  ~ErrorReporter();

  String FindErrorReport(StringParam codeLocation);
  void ClearReports();
  void OnTranslationError(TranslationErrorEvent* errorEvent);
  void OnCompilationError(Zilch::ErrorEvent* e);
  void OnValidationError(ValidationErrorEvent* e);
  void OnError(StringParam codeLocation, StringParam errorMsg);

  void Report(StringParam message);
  void Report(StringParam filePath, StringParam lineNumber, StringParam message);
  void Report(StringParam filePath, StringParam lineNumber, StringParam header, StringParam message);
  void ReportCompilationWarning(StringParam filePath, StringParam lineNumber, StringParam message);
  void ReportCompilationWarning(const Array<LogMessage>& messages);
  void ReportCompilationError(StringParam filePath, StringParam lineNumber, StringParam message);
  void ReportCompilationError(const Array<LogMessage>& messages);
  void ReportLinkerError(StringParam filePath, StringParam lineNumber, StringParam message);
  void ReportPostProcessError(StringParam testName, Vec4Param expected, Vec4Param result, int renderTargetIndex);

  void DisplayDiffs(StringParam expectedFile, StringParam resultFile);

  bool mAssert;
  bool mRunExternalDiffTool;

private:
  HashMap<String, StringBuilder*> mErrors;
};
