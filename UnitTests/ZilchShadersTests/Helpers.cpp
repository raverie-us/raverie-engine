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

void LoadCompositeDirectory(SimpleZilchShaderGenerator& shaderGenerator, StringParam directory, StringParam defFileName)
{
  // Load all of the fragments in the directory into the fragment project
  FileRange range(directory);
  for(; !range.Empty(); range.PopFront())
  {
    String filePath = range.FrontEntry().GetFullPath();
    if(FilePath::GetExtension(filePath).ToLower() == mFragmentExtension)
    {
      FileEntry entry = range.FrontEntry();
      String fragmentCode = ReadFileIntoString(entry.GetFullPath());
      shaderGenerator.AddFragmentCode(fragmentCode, entry.mFileName, nullptr);
    }
  }

  // Compile all fragments
  shaderGenerator.CompileAndTranslateFragments();

  // Load the shader def file
  Array<SerializedShaderDefinition> shaders;
  String defPath = FilePath::CombineWithExtension(directory, defFileName, ".txt");
  LoadShaderDefinitions(shaders, defPath);

  // Create composites for all of the shader defs
  for(size_t i = 0; i < shaders.Size(); ++i)
  {
    ZilchShaderDefinition shaderDef;
    shaderDef.mShaderName = shaders[i].mShaderName;
    // Iterate over all of the fragment names and add the
    for(size_t j = 0; j < shaders[i].mFragmentNames.Size(); ++j)
    {
      String fragmentName = shaders[i].mFragmentNames[j];
      ShaderType* fragmentType = shaderGenerator.mFragmentLibraryRef->FindType(fragmentName);
      if(fragmentType == nullptr)
      {
        Error("Fragment %s doesn't exist in library", fragmentName.c_str());
        continue;
      }

      // If the translator doesn't support the current type then skip it. 
      // This happens when using the Gsls130 translator on geometry fragments.
      if(!shaderGenerator.mTranslator->SupportsFragmentType(fragmentType))
        continue;

      // Append this type to the current shader
      shaderDef.mFragmentTypes.PushBack(fragmentType);
    }

    // Compose the Shader
    shaderGenerator.ComposeShader(shaderDef);
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

void TestCompilation(SimpleZilchShaderGenerator& shaderGenerator, BaseRenderer& renderer, ShaderType* type, StringParam directory, ErrorReporter& reporter)
{
  // Only output fragment types (and their dependencies)
  if(type->mFragmentType == FragmentType::None)
    return;

  ShaderTypeTranslation shaderResult;
  shaderGenerator.mTranslator->BuildFinalShader(type, shaderResult);
  String generatedResults = shaderResult.mTranslation;

  String languageExt = shaderGenerator.mTranslator->GetFullLanguageString();
  String extension = languageExt.ToLower();
  String testName = FilePath::GetFileNameWithoutExtension(type->mZilchName);

  String expectedFilePath = FilePath::CombineWithExtension(directory, testName, BuildString(".expected.", extension));
  String generatedFilePath = FilePath::CombineWithExtension(directory, testName, BuildString(".generated.", extension));
  WriteStringRangeToFile(generatedFilePath, generatedResults.All());
  
  bool diffedCleanly = TestDiff(expectedFilePath, generatedFilePath, generatedResults, reporter);
  //if(diffedCleanly)
  //  renderer.CompileShader(generatedFilePath, generatedResults, type->mFragmentType, reporter);
}

void TestShaderFileCompilation(SimpleZilchShaderGenerator& shaderGenerator, BaseRenderer& renderer, StringParam filePath, StringParam directory, ErrorReporter& reporter)
{
  // Clear the previous results
  shaderGenerator.ClearAll();

  // Load the fragment code and compile
  String fragmentCode = ReadFileIntoString(filePath);
  shaderGenerator.AddFragmentCode(fragmentCode, filePath, nullptr);

  ZilchShaderModuleRef fragmentDependencies = new ZilchShaderModule();
  shaderGenerator.mFragmentProject.CompileAndTranslate(fragmentDependencies, shaderGenerator.mTranslator, shaderGenerator.mSettings, true);

  //shaderGenerator.CompileAndTranslateFragments();

  // Test all fragment types (not helpers) that resulted from this.
  // We have to do this as a file can contain multiple fragments of different names than the file.
  AutoDeclare(typeRange, shaderGenerator.mFragmentLibraryRef->mTypes.Values());
  for(; !typeRange.Empty(); typeRange.PopFront())
    TestCompilation(shaderGenerator, renderer, typeRange.Front(), directory, reporter);
}

void TestShaderCompilationOfDirectory(SimpleZilchShaderGenerator& shaderGenerator, BaseRenderer& renderer, StringParam directory, ErrorReporter& reporter)
{
  FileRange range(directory);
  for(; !range.Empty(); range.PopFront())
  {
    if(range.FrontEntry().mFileName != "Simple.zilchFrag")
      continue;
    String filePath = range.FrontEntry().GetFullPath();
    if(FilePath::GetExtension(filePath).ToLower() == mFragmentExtension)
      TestShaderFileCompilation(shaderGenerator, renderer, filePath, directory, reporter);
  }
}

void WriteAndDiffComposites(SimpleZilchShaderGenerator& shaderGenerator, StringParam directory, ErrorReporter& reporter)
{
  String languageExt = shaderGenerator.mTranslator->GetFullLanguageString();

  // Iterate over all composited shaders in the current generator
  SimpleZilchShaderGenerator::ShaderDefinitionMap::range shaderDefRange = shaderGenerator.mShaderDefinitionMap.All();
  for(; !shaderDefRange.Empty(); shaderDefRange.PopFront())
  {
    String shaderName = shaderDefRange.Front().first;
    ZilchShaderDefinition& shaderDef = shaderDefRange.Front().second;

    // Merge all of the zilch composites together into one string
    StringBuilder builder;
    for(size_t i = 0; i < FragmentType::Size; ++i)
    {
      builder.Append(shaderDef.mShaderData[i].mZilchCode);
    }

    // Write out the generated file and then diff it to the expected one
    String expectedFilePath = FilePath::CombineWithExtension(directory, shaderDef.mShaderName, BuildString(".expected.", languageExt, "composite"));
    String generatedFilePath = FilePath::CombineWithExtension(directory, shaderDef.mShaderName, BuildString(".generated.", languageExt, "composite"));
    WriteStringRangeToFile(generatedFilePath, builder.ToString());
    TestDiff(expectedFilePath, generatedFilePath, reporter);
  }
}

void TestCompilationAndLinkingOfCompositesInDirectory(SimpleZilchShaderGenerator& shaderGenerator, BaseRenderer& renderer, StringParam directory, ErrorReporter& reporter)
{
  shaderGenerator.ClearFragmentsProjectAndLibrary();
  shaderGenerator.ClearShadersProjectAndLibrary();
  LoadCompositeDirectory(shaderGenerator, directory, "ShaderDefinitions");
  WriteAndDiffComposites(shaderGenerator, directory, reporter);

  shaderGenerator.CompileAndTranslateShaders();

  AutoDeclare(shaderDefRange, shaderGenerator.mShaderDefinitionMap.Values());
  for(; !shaderDefRange.Empty(); shaderDefRange.PopFront())
  {
    ZilchShaderDefinition& shaderDef = shaderDefRange.Front();

    Array<FragmentInfo> fragments;
    fragments.Resize(FragmentType::Size);
    for(size_t i = 0; i < FragmentType::Size; ++i)
      fragments[i] = BuildShaderAndDiff(shaderGenerator, shaderDef, renderer, (FragmentType::Enum)i, directory, reporter);
    
    renderer.CompileAndLinkShader(fragments, reporter);
  }
}

void TestRuntime(SimpleZilchShaderGenerator& shaderGenerator, BaseRenderer& renderer, StringParam directory, ErrorReporter& reporter)
{
  shaderGenerator.ClearFragmentsProjectAndLibrary();
  shaderGenerator.ClearShadersProjectAndLibrary();
  LoadCompositeDirectory(shaderGenerator, directory, "ShaderDefinitions");
  WriteAndDiffComposites(shaderGenerator, directory, reporter);

  // Compile and translate the shader library
  shaderGenerator.CompileAndTranslateShaders();

  // Iterate over all of the resultant shaders and run them
  AutoDeclare(shaderDefRange, shaderGenerator.mShaderDefinitionMap.Values());
  for(; !shaderDefRange.Empty(); shaderDefRange.PopFront())
  {
    ZilchShaderDefinition& shaderDef = shaderDefRange.Front();

    String shaderName = shaderDef.mShaderName;
    ShaderType* pixelShaderType = shaderGenerator.mShaderLibraryRef->FindType(shaderDef.mShaderData[FragmentType::Pixel].mZilchClassName);

    Array<FragmentInfo> fragments;
    fragments.Resize(FragmentType::Size);
    for(size_t i = 0; i < fragments.Size(); ++i)
      fragments[i] = BuildShaderAndDiff(shaderGenerator, shaderDef, renderer, (FragmentType::Enum)i, directory, reporter);

    RenderResults results;
    String targetLanguage = shaderGenerator.mTranslator->GetFullLanguageString();
    RenderResult& shaderResult = results.mLanguageResult[targetLanguage];
    RenderResult& zilchResult = results.mLanguageResult[RenderResults::mZilchKey];

    // Collect the results from zilch
    ComputeZilchRenderResults(shaderGenerator, pixelShaderType->mZilchName, results);
    // Then collect them from the renderer
    renderer.RunPostProcess(fragments, shaderResult, reporter);

    for(size_t i = 0; i < mMaxRenderTargets; ++i)
    {
      //if(results.mTargets[i] == false)
      //  continue;
      Vec4 zilchData = zilchResult.mData[i];
      Vec4 shaderData = shaderResult.mData[i];
      if(Math::LengthSq(zilchData - shaderData) > 0.0001f)
        reporter.ReportPostProcessError(shaderName, zilchData, shaderData, i);
    }
  }
}

FragmentInfo BuildShaderAndDiff(SimpleZilchShaderGenerator& shaderGenerator, ZilchShaderDefinition& shaderDef, BaseRenderer& renderer, FragmentType::Enum fragmentType, StringParam directory, ErrorReporter& reporter)
{
  FragmentInfo result;
  result.mFragmentType = fragmentType;

  String shaderName = shaderDef.mShaderName;
  // If this shader type wasn't generated then don't check
  // (for instance, there might not have been a geometry shader)
  if(shaderDef.mShaderData[fragmentType].mZilchCode.Empty())
    return result;

  // Get the shader type for given shader's fragment
  ShaderType* shaderType = shaderGenerator.mShaderLibraryRef->FindType(shaderDef.mShaderData[fragmentType].mZilchClassName);
  // Build the final shader string
  ShaderTypeTranslation shaderResult;
  shaderGenerator.mTranslator->BuildFinalShader(shaderType, shaderResult);

  // Write out the fragment to a file
  String languageExt = shaderGenerator.mTranslator->GetFullLanguageString();
  String extension = languageExt.ToLower();
  String expectedFilePath = FilePath::CombineWithExtension(directory, shaderType->mZilchName, BuildString(".expected.", extension));
  String generatedFilePath = FilePath::CombineWithExtension(directory, shaderType->mZilchName, BuildString(".generated.", extension));
  result.mFilePath = generatedFilePath;
  result.mFragmentCode = shaderResult.mTranslation.All();
  WriteStringRangeToFile(result.mFilePath, result.mFragmentCode);

  // Diff the vertex and pixel fragment
  bool diffedCleanly = TestDiff(expectedFilePath, generatedFilePath, result.mFragmentCode, reporter);
  return result;
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
