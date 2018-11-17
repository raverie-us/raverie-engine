///////////////////////////////////////////////////////////////////////////////
///
/// \file BuildOptions.hpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------ Content Build Options
typedef Array<ContentItem*> ContentItemArray;

DeclareEnum3(ProcessingLevel, Full, Production, Fast);

DeclareEnum2(Packaging, Packaged, Directory);

DeclareEnum2(BuildMode, Rebuild, Incremental);

DeclareEnum5(BuildStatus, Starting, Completed, Running, Canceled, Failed);

//Standard error codes for building.
DeclareEnum5(BuildErrors, Success, FileNotFound, InvalidFormat, InvalidSettings, OutOfMemory);

//Options used to control content building
class BuildOptions
{
public:
  //Full Production is full optimization (slow)
  //Production Is fast to process but still good for runtime
  //Fast Is just for fast build times.
  ProcessingLevel::Enum ProcessingLevel;

  //Verbosity
  //Minimal is minimal (one line per process)
  //Detailed is details for debugging
  Verbosity::Enum Verbosity;

  //How assets are packaged.
  Packaging::Enum Packaging;

  ///Full rebuild or incremental
  BuildMode::Enum BuildMode;

  //Current build status, used to cancel builds
  BuildStatus::Enum BuildStatus;

  //Should send progress messages
  bool SendProgress;

  //Any Content Item Failed?
  bool Failure;

  String OutputPath;
  String SourcePath;
  String ToolPath;
  //The error message if the build fails.
  String Message;

  //File that need editor processing.
  ContentItemArray EditorProcessing;

  TextStream* BuildTextStream;
};

}
