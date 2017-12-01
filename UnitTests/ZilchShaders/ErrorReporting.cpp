///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

//-------------------------------------------------------------------LogMessage
LogMessage::LogMessage()
{

}

LogMessage::LogMessage(StringParam filePath, StringParam lineNumber, StringParam message)
{
  mFilePath = filePath;
  mLineNumber = lineNumber;
  mMessage = message;
}

//-------------------------------------------------------------------ErrorReporter
ErrorReporter::ErrorReporter()
{
  mAssert = true;
  mRunExternalDiffTool = true;
}

ErrorReporter::~ErrorReporter()
{
  ClearReports();
}

String ErrorReporter::FindErrorReport(StringParam codeLocation)
{
  // Find the error string builder if it exists (key = file name)
  FilePathInfo pathInfo = FilePath::GetPathInfo(codeLocation);
  StringBuilder* errorBuilder = mErrors.FindValue(pathInfo.FileName, nullptr);
  if(errorBuilder != nullptr)
    return errorBuilder->ToString();
  return String();
}

void ErrorReporter::ClearReports()
{
  // De-allocate the error message builders
  AutoDeclare(range, mErrors.All());
  for(; !range.Empty(); range.PopFront())
  {
    delete range.Front().second;
  }
  mErrors.Clear();
}

void ErrorReporter::OnTranslationError(TranslationErrorEvent* errorEvent)
{
  String errorMsg = BuildString(errorEvent->mShortMessage, ":\n", errorEvent->mFullMessage, "\n");
  errorMsg = errorEvent->mLocation.GetFormattedStringWithMessage(Zilch::MessageFormat::MsvcCpp, errorMsg);
  OnError(errorEvent->mLocation.Origin, errorMsg);
}

void ErrorReporter::OnCompilationError(Zilch::ErrorEvent* e)
{
  String errorMsg = BuildString(e->GetFormattedMessage(Zilch::MessageFormat::MsvcCpp), "\n");
  OnError(e->Location.Origin, errorMsg);
}

void ErrorReporter::OnValidationError(ValidationErrorEvent* e)
{
  String errorMsg = BuildString(e->GetFormattedMessage(Zilch::MessageFormat::MsvcCpp), "\n");
  OnError(e->mLocation.Origin, errorMsg);
}

void ErrorReporter::OnError(StringParam codeLocation, StringParam errorMsg)
{
  FilePathInfo pathInfo = FilePath::GetPathInfo(codeLocation);
  String outFilePath = FilePath::CombineWithExtension(pathInfo.Folder, pathInfo.FileName, ".generated.errors");

  // Create the error message builder if necessary
  if(!mErrors.ContainsKey(pathInfo.FileName))
    mErrors[pathInfo.FileName] = new StringBuilder();

  // Append the current error message
  StringBuilder* errorBuilder = mErrors[pathInfo.FileName];
  errorBuilder->Append(errorMsg);

  // Also report the message as normal so asserts will happen and we'll print to the console
  Report(errorMsg);
}

void ErrorReporter::Report(StringParam message)
{
  ZPrint("%s", message.c_str());

  if(mAssert)
    ZERO_DEBUG_BREAK;
}

void ErrorReporter::Report(StringParam filePath, StringParam lineNumber, StringParam header, StringParam message)
{
  if(!header.Empty())
    ZPrint("%s", header.c_str());

  // Display the error in visual studio format (so double-clicking the error line will work)
  ZPrint("%s(%s): \n\t%s\n", filePath.c_str(), lineNumber.c_str(), message.c_str());

  if(mAssert)
    ZERO_DEBUG_BREAK;
}

void ErrorReporter::Report(StringParam filePath, StringParam lineNumber, StringParam message)
{
  Report(filePath, lineNumber, String(), message);
}

void ErrorReporter::ReportCompilationWarning(StringParam filePath, StringParam lineNumber, StringParam message)
{
  bool assert = mAssert;
  mAssert = false;

  String header = "\n------------Compilation Warnings------------\n\n";
  Report(filePath, lineNumber, header, message);

  mAssert = assert;
}

void ErrorReporter::ReportCompilationWarning(const Array<LogMessage>& messages)
{
  String header = "\n------------Compilation Warnings------------\n\n";
  ZPrint("%s", header.c_str());

  // Display the error in visual studio format (so double-clicking the error line will work)
  for(size_t i = 0; i < messages.Size(); ++i)
  {
    const LogMessage& message = messages[i];
    ZPrint("%s(%s): \n\t%s\n", message.mFilePath.c_str(), message.mLineNumber.c_str(), message.mMessage.c_str());
  }
}

void ErrorReporter::ReportCompilationError(StringParam filePath, StringParam lineNumber, StringParam message)
{
  String header = "\n------------Compilation Errors------------\n\n";
  Report(filePath, lineNumber, header, message);
}

void ErrorReporter::ReportCompilationError(const Array<LogMessage>& messages)
{
  String header = "\n------------Compilation Errors------------\n\n";
  ZPrint("%s", header.c_str());

  // Display the error in visual studio format (so double-clicking the error line will work)
  for(size_t i = 0; i < messages.Size(); ++i)
  {
    const LogMessage& message = messages[i];
    ZPrint("%s(%s): \n\t%s\n", message.mFilePath.c_str(), message.mLineNumber.c_str(), message.mMessage.c_str());
  }

  if(mAssert)
    ZERO_DEBUG_BREAK;
}

void ErrorReporter::ReportLinkerError(StringParam filePath, StringParam lineNumber, StringParam message)
{
  String header = "\n------------Linker Errors------------\n\n";
  Report(filePath, lineNumber, header, message);
}

void ErrorReporter::ReportPostProcessError(StringParam testName, Vec4Param expected, Vec4Param result, int renderTargetIndex)
{
  String expectedVectorStr = String::Format("(%g, %g, %g, %g)", expected.x, expected.y, expected.z, expected.w);
  String resultVectorStr = String::Format("(%g, %g, %g, %g)", result.x, result.y, result.z, result.w);

  String message = String::Format("Post process %s failed target %d. Expected %s but got %s", testName.c_str(), renderTargetIndex, expectedVectorStr.c_str(), resultVectorStr.c_str());
  Report(message);
}

void ErrorReporter::DisplayDiffs(StringParam expectedFile, StringParam resultFile)
{
  if(mRunExternalDiffTool)
  {
    String parameters = String::Format("\"%s\" \"%s\"", resultFile.c_str(), expectedFile.c_str());
    String arguments = String::Format("tortoisemerge.exe %s", parameters.c_str());

    SimpleProcess process;
    process.ExecProcess("tortoisemerge", arguments.c_str(), nullptr, true);
    process.WaitForClose();
  }
}
