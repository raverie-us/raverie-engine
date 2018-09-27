///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"


void DisplayError(StringParam filePath, StringParam lineNumber, StringParam errMsg)
{
  // Display the error in visual studio format (so double-clicking the error line will work)
  ZPrint("%s(%s): \n\t%s\n", filePath.c_str(), lineNumber.c_str(), errMsg.c_str());
}

void LoadShaderDefinitions(Array<SerializedShaderDefinition>& shadersDefs, StringParam defFilePath)
{
  Status status;
  DataTreeLoader loader;
  loader.OpenFile(status, defFilePath.c_str());
  loader.SerializeField("Shaders", shadersDefs);
  loader.Close();
}

void LoadSettingsOrDefault(ZilchShaderSpirVSettings& settings, StringParam defFilePath)
{
  if(!FileExists(defFilePath))
  {
    settings.mErrorSettings = ZilchShaderErrorSettings();
    return;
  }

  SerializedErrorSettings serializedData;
  DataTreeLoader loader;
  Status status;
  loader.OpenFile(status, defFilePath.c_str());
  PolymorphicNode node;
  while(loader.GetPolymorphic(node))
  {
    if(node.TypeName == "ErrorSettings")
    {
      serializedData.Serialize(loader);
      settings.mErrorSettings.mFrontEndErrorOnNoMainFunction = serializedData.mErrorOnNoMain;
    }
  }
  loader.Close();
}

void LoadDirectorySettingsOrDefault(ZilchShaderSpirVSettings& settings, StringParam directoryPath)
{
  String settingsFilePath = FilePath::Combine(directoryPath, "Settings.txt");
  LoadSettingsOrDefault(settings, settingsFilePath);
}

void LoadFragmentCodeDirectory(SimpleZilchShaderIRGenerator& shaderGenerator, StringParam directory)
{
  // Load all of the fragments in the directory into the fragment project
  FileRange range(directory);
  for(; !range.Empty(); range.PopFront())
  {
    FileEntry entry = range.FrontEntry();
    String filePath = range.FrontEntry().GetFullPath();

    if(DirectoryExists(filePath))
      continue;

    if(FilePath::GetExtension(filePath).ToLower() == mFragmentExtension)
    {
      FileEntry entry = range.FrontEntry();
      String fragmentCode = ReadFileIntoString(entry.GetFullPath());
      shaderGenerator.AddFragmentCode(fragmentCode, entry.mFileName, nullptr);
    }
  }
}

//bool TestDiff(StringParam directory, StringParam testName, StringParam languageExt, StringParam generatedFileContents, ErrorReporter& reporter)
bool TestDiff(StringParam expectedFilePath, StringParam generatedFilePath, StringParam generatedFileContents, ErrorReporter& reporter)
{
  String expectedFileContents;
  if(FileExists(expectedFilePath))
  {
    expectedFileContents = ReadFileIntoString(expectedFilePath);
    // Deal with weirdness of line return characters (I don't care about that difference...)
    expectedFileContents = expectedFileContents.Replace("\r\n", "\n");
  }
  // there's no expected results yet, write out an empty file for the diff
  else
  {
    WriteStringRangeToFile(expectedFilePath, "");
  }

  if(generatedFileContents != expectedFileContents)
  {
    reporter.DisplayDiffs(expectedFilePath, generatedFilePath);
    return false;
  }
  return true;
}

bool TestDiff(StringParam expectedFilePath, StringParam generatedFilePath, ErrorReporter& reporter)
{
  // Load the generated file if it exists
  String generatedFileContents;
  if(FileExists(expectedFilePath))
  {
    generatedFileContents = ReadFileIntoString(generatedFilePath);
    // Deal with weirdness of line return characters (I don't care about that difference...)
    generatedFileContents = generatedFileContents.Replace("\r\n", "\n");
  }
  // Otherwise write out the generated file so we diff against an empty file
  else
    WriteStringRangeToFile(generatedFilePath, "");

  return TestDiff(expectedFilePath, generatedFilePath, generatedFileContents, reporter);
}

void FileToStream(File& file, TextStreamBuffer& stream)
{
  Status status;
  for(;;)
  {
    const size_t BufferSize = 4096;

    // We add 1 for an extra null terminator at the end
    // (technically the null terminator could go anywhere since we place it at the end of the amount we read)
    byte buffer[BufferSize + 1];

    size_t amountRead = file.Read(status, buffer, BufferSize);

    // Terminate this buffer and write it out
    buffer[amountRead] = '\0';
    stream.Write((cstr)buffer);

    if(status.Failed())
      break;
  }
}

void RunProcess(StringParam applicationPath, StringParam args, String* stdOutResult, String* stdErrResult)
{
  Status status;

  ProcessStartInfo startInfo;
  startInfo.mApplicationName = applicationPath;
  startInfo.mArguments = args;
  startInfo.mRedirectStandardOutput = true;
  startInfo.mRedirectStandardError = true;

  File standardOut;
  File standardError;
  TextStreamBuffer standardOutStream;
  TextStreamBuffer standardErrStream;

  Process process;
  process.Start(status, startInfo);
  if(stdOutResult != nullptr)
    process.OpenStandardOut(standardOut);
  if(stdErrResult != nullptr)
    process.OpenStandardError(standardError);

  while(process.IsRunning())
  {
    process.WaitForClose(100);
    if(stdOutResult != nullptr)
      FileToStream(standardOut, standardOutStream);
    if(stdErrResult != nullptr)
      FileToStream(standardError, standardErrStream);
  }
  if(stdOutResult != nullptr)
    FileToStream(standardOut, standardOutStream);
  if(stdErrResult != nullptr)
    FileToStream(standardError, standardErrStream);

  bool isRunning = process.IsRunning();


  String standardOutStr;
  if(stdOutResult != nullptr)
    *stdOutResult = standardOutStream.ToString();
  if(stdErrResult != nullptr)
    *stdErrResult = standardErrStream.ToString();
}
