///////////////////////////////////////////////////////////////////////////////
///
/// \file ZilchPluginContent.cpp
/// 
/// Authors: Trevor Sundberg
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "ZilchPluginContent.hpp"
#include "Math/Random.hpp"
#include "Platform/FileSystem.hpp"
#include "Platform/FilePath.hpp"
#include "Platform/Utilities.hpp"
#include "Platform/PlatformSelector.hpp"
#include "Support/FileSupport.hpp"
#include "ContentSystem.hpp"
#include "ContentLibrary.hpp"
#include "Engine/Configuration.hpp"
#include "Engine/Engine.hpp"
#include "Engine/BuildVersion.hpp"

namespace Zero
{
  
//------------------------------------------------------------ ZilchPluginBuilder
ZilchDefineType(ZilchPluginBuilder, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::CallSetDefaults);
}

String GenerateGuid()
{
  // This isn't really 'cryptographically random' or anything safe, but it works
  Math::Random rand((uint)GenerateUniqueId64());
  char data[] = "{11111111-1111-1111-1111-111111111111}";
  for(size_t i = 0; i < sizeof(data); ++i)
  {
    if(data[i] == '1')
    {
      char letterIndex = (char)rand.IntRangeInEx(0, 16);
      if (letterIndex < 10)
        data[i] = '0' + letterIndex;
      else
        data[i] = 'A' + (letterIndex - 10);
    }
  }

  return String(data, sizeof(data) - 1);
}

String GetContentDirectory(ContentLibrary* library)
{
  return library->SourcePath;
}

String GetCodeDirectory(ContentLibrary* library, StringParam name)
{
  return FilePath::Combine(GetContentDirectory(library), name);
}

ZilchPluginBuilder::ZilchPluginBuilder()
{
  mSharedLibraryResourceId = 0;
}

void ZilchPluginBuilder::Rename(StringParam newName)
{
  DataBuilder::Rename(newName);
  DoNotifyWarning("Zilch Plugin" , "You must rename the plugin directory and all the contents (projects, solutions, make files, etc)");
}

String ZilchPluginBuilder::GetSharedLibraryPlatformName()
{
  StringBuilder builder;

  // This is not currently used because we ensured that Debug/Release builds
  // of the plugins are compatible with both Debug/Release builds of Zero
  // Note that this relates to linked libraries within the generated project files (plugin templates)
  // The template projects also generate dynamic libraries with extensions that could include Debug/Release
  //builder.Append(GetConfigurationString());
  //builder.Append('-');

  // Append the operating system name (or some grouped name for all OSes that support this shared library)
#if defined(PLATFORM_WINDOWS)
  builder.Append("Windows_NT");
#else
  builder.Append("Unknown");
#endif

  builder.Append('-');
  
  // Append the target machine architecture
#if defined(PLATFORM_WINDOWS)
  #if defined(PLATFORM_32)
    builder.Append("x86");
  #else
    builder.Append("x64");
  #endif
#endif
  String pluginFileName = builder.ToString();
  return pluginFileName;
}

String ZilchPluginBuilder::GetSharedLibraryPlatformBuildName()
{
  String platformName = GetSharedLibraryPlatformName();
  String platformBuildName = BuildString(platformName, "-", GetRevisionNumberString());
  return platformBuildName;
}

String ZilchPluginBuilder::GetSharedLibraryExtension(bool includeDot)
{
  String extension;
  if (includeDot)
    extension = BuildString(".", GetSharedLibraryPlatformBuildName(), "-zilchPlugin");
  else
    extension = BuildString(GetSharedLibraryPlatformBuildName(), "-zilchPlugin");
  return extension;
}

String ZilchPluginBuilder::GetSharedLibraryFileName()
{
  return BuildString(Name, GetSharedLibraryExtension(true));
}

void ZilchPluginBuilder::Serialize(Serializer& stream)
{
  DataBuilder::Serialize(stream);
  SerializeNameDefault(mSharedLibraryResourceId, ResourceId(0));
}

void ZilchPluginBuilder::Generate(ContentInitializer& initializer)
{
  DataBuilder::Generate(initializer);

  if(mSharedLibraryResourceId == 0)
    mSharedLibraryResourceId = GenerateUniqueId64();
}

void ZilchPluginBuilder::BuildContent(BuildOptions& buildOptions)
{
  DataBuilder::BuildContent(buildOptions);

  String sharedLibraryFileName = GetSharedLibraryFileName();
  String destFile = FilePath::Combine(buildOptions.OutputPath, sharedLibraryFileName);
  String sourceFile = FilePath::Combine(buildOptions.SourcePath, sharedLibraryFileName);

  // We output a dummy empty shared library so that content won't get mad at us
  if(!FileExists(sourceFile))
  {
    byte value = 0;
    WriteToFile(sourceFile.c_str(), &value, 0);
  }

  CopyFile(destFile, sourceFile);
}

void ZilchPluginBuilder::BuildListing(ResourceListing& listing)
{
  DataBuilder::BuildListing(listing);
  
  String destFile = GetSharedLibraryFileName();
  static const String LibraryLoaderType("ZilchPluginLibrary");
  uint order = Z::gContentSystem->LoadOrderMap.FindValue(LibraryLoaderType, 10);
  listing.PushBack(ResourceEntry(order, LibraryLoaderType, Name, destFile, 
    mSharedLibraryResourceId, this->mOwner, this));
}

ContentItem* MakeZilchPluginContent(ContentInitializer& initializer)
{
  DataContent* content = new DataContent();
  content->mIgnoreMultipleResourcesWarning = true;
  content->Filename = initializer.Filename;
  ZilchPluginBuilder* builder = new ZilchPluginBuilder();
  builder->Generate(initializer);
  builder->LoaderType = "ZilchPluginSource";
  content->AddComponent(builder);

  String pluginName = initializer.Name;
  String codeDir = GetCodeDirectory(initializer.Library, pluginName);
  
  // If the directory already exists
  if(DirectoryExists(codeDir))
  {
    DoNotifyWarning("Zilch Plugin",
      String::Format("The directory for the Zilch plugin '%s' already exists, "
      "therefore we will not create the template project:\n  '%s'\n",
      pluginName.c_str(), codeDir.c_str()));
    return content;
  }

  Cog* configCog = Z::gEngine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);
  String templateDir = FilePath::Combine(mainConfig->DataDirectory, "ZilchCustomPluginTemplate");

  // Some templates require generated guids which we will create and replace
  static const String ReplaceGuid("{11111111-1111-1111-1111-111111111111}");
  static const String ReplaceName("%NAME%");
  static const String ReplaceIncludeGuard("%UPPERCASENAME%");

  String includeGuard = pluginName.ToUpper();
  String guid = GenerateGuid();
  
  CreateDirectoryAndParents(codeDir);
  for(FileRange files(templateDir); !files.Empty(); files.PopFront())
  {
    String templateFileName = files.Front();
    String templateFile = FilePath::Combine(templateDir, templateFileName);
    String outputFileName = templateFileName.Replace(ReplaceName, pluginName);
    String outputFile = FilePath::Combine(codeDir, outputFileName);
    
    // Everything we're writing in this template is string data that required replacements
    String data = ReadFileIntoString(templateFile.c_str());

    // Replace any guids and names inside the template text files
    // This could probably be done more efficiently
    data = data.Replace(ReplaceName, pluginName);
    data = data.Replace(ReplaceGuid, guid);
    data = data.Replace(ReplaceIncludeGuard, includeGuard);

    WriteToFile(outputFile.Data(), (byte*)data.Data(), data.SizeInBytes());
  }

  return content;
}

void CreateZilchPluginContent(ContentSystem* system)
{
  AddContentComponent<ZilchPluginBuilder>(system);
}

}
