///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

String mFragmentExtension = "zilchfrag";

String RenderResults::mZilchKey = "Zilch";

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
  if(mAssert)
  {
    String parameters = String::Format("\"%s\" \"%s\"", resultFile.c_str(), expectedFile.c_str());
    String arguments = String::Format("tortoisemerge.exe %s", parameters.c_str());

    SimpleProcess process;
    process.ExecProcess("tortoisemerge", arguments.c_str(), nullptr, true);
    process.WaitForClose();
  }
}

//-------------------------------------------------------------------FragmentInfo
FragmentInfo::FragmentInfo()
{

}

FragmentInfo::FragmentInfo(StringParam filePath)
{
  mFilePath = filePath;
  mFragmentCode = ReadFileIntoString(filePath);
}

FragmentInfo::FragmentInfo(StringParam filePath, StringParam fragmentCode)
{
  mFilePath = filePath;
  mFragmentCode = fragmentCode;
}

//-------------------------------------------------------------------BaseRenderer
BaseRenderer::BaseRenderer()
{
  // buffer for fullscreen triangle
  mFullScreenTriangleVerts[0] = {Vec3(-1, 3, 0), Vec2(0, -1), Vec4()};
  mFullScreenTriangleVerts[1] = {Vec3(-1, -1, 0), Vec2(0, 1), Vec4()};
  mFullScreenTriangleVerts[2] = {Vec3(3, -1, 0), Vec2(2, 1), Vec4()};
}
