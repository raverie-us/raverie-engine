///////////////////////////////////////////////////////////////////////////////
///
/// \file ContentUtility.cpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "ContentUtility.hpp"
#include "ContentItem.hpp"
#include "Platform/FileSystem.hpp"
#include "Platform/FilePath.hpp"

namespace Zero
{

bool NeedToBuild(StringParam source, StringParam destination)
{
  return !(CheckFileTime(destination, source) >= 0);
}

bool NeedToBuild(BuildOptions& options, StringParam source, StringParam destination)
{
  if(options.BuildMode == BuildMode::Incremental)
  {
    int fileCmp = CheckFileTime(destination, source);

    //destination is older
    if(fileCmp == -1)
    {
      if(options.Verbosity == Verbosity::Detailed)
      {
        String msg = String::Format("Needs to build because '%s' is newer than '%s'\n", destination.c_str(), source.c_str());
        options.BuildTextStream->Write(msg.c_str());
      }
      return true;
    }
    else//fileCmp = 0 or 1
    {
      if(options.Verbosity == Verbosity::Detailed)
      {
        String msg = String::Format("File '%s' is up to date relative to '%s'\n", destination.c_str(), source.c_str());
        options.BuildTextStream->Write(msg.c_str());
      }
      return false;
    }
  }
  else
  {
    //Always build in non incremental
    return true;
  }
}

bool CheckFileAndMeta(BuildOptions& options, StringParam sourceFile, StringParam destFile)
{
  String metaFile = BuildString(sourceFile, ".meta");
  bool fileOutOfDate =  NeedToBuild(options, sourceFile, destFile);
  bool metaFileOutOfDate =  NeedToBuild(options, metaFile, destFile);
  return fileOutOfDate || metaFileOutOfDate;
}

bool CheckToolFile(BuildOptions& options, StringParam outputFile, StringParam toolFile)
{
  String toolFileFull = FilePath::Combine(options.ToolPath, toolFile);
  String destFileFull = FilePath::Combine(options.OutputPath, outputFile);
  bool needToBuild = NeedToBuild(options, toolFileFull, destFileFull);
  return needToBuild;
}

bool FileSizeDiffers(cstr destination, cstr source)
{
  u32 sizeA = GetFileSize(destination);
  u32 sizeB = GetFileSize(source);
  return sizeA != sizeB;
}

bool CheckFileMetaAndSize(BuildOptions& options, StringParam sourceFile, StringParam destFile)
{
  if(CheckFileAndMeta(options, sourceFile, destFile))
    return true;

  if(FileSizeDiffers(sourceFile.c_str(), destFile.c_str()))
    return true;

  return false;
}

}
