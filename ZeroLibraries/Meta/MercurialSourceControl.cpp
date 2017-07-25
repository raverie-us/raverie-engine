///////////////////////////////////////////////////////////////////////////////
///
/// \file MercurialSourceControl.cpp
///  Implementation of Mercurial Source Control
/// 
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "SourceControl.hpp"
#include "Platform/Process.hpp"
#include "String/StringBuilder.hpp"
#include "Platform/FileSystem.hpp"

namespace Zero
{

class Mercurial : public SourceControl
{
public:

  void Add(Status& status, StringParam filePath) override
  {
    String filePathSafe = BuildString("\"", filePath, "\"");
    String commandLine = BuildString("hg add ", filePathSafe);
    RunSimpleCommandLine(status, commandLine);

  }

  void Remove(Status& status, StringParam filePath) override
  {
    String filePathSafe = BuildString("\"", filePath, "\"");
    String commandLine = BuildString("hg remove ", filePathSafe);

    TextStreamBuffer buffer;
    SimpleProcess process;
    process.ExecProcess("Log", commandLine.c_str(), &buffer);
    int returnCode = process.WaitForClose();
    if(returnCode!=0)
    {
      // May have failed because file was only marked for add
      // so try and hg forget the file
      buffer.Write("hg remove failed trying hg forget...\n");

      String commandLineForget = BuildString("hg forget ", filePathSafe);
      SimpleProcess processForget;
      processForget.ExecProcess("Log", commandLineForget.c_str(), &buffer);
      int returnCodeForget = processForget.WaitForClose();
      if(returnCodeForget !=0)
      {
        status.SetFailed(buffer.ToString());
      }
      else
      {
        DeleteFile(filePath);
      }
    }
  }

  void Rename(Status& status, StringParam sourcePath, StringParam destPath) override
  {
    // Build command line
    String source = BuildString("\"", sourcePath, "\"");
    String dest = BuildString("\"", destPath, "\"");
    String commandLine = BuildString("hg rename ", source, " ", dest);

    RunSimpleCommandLine(status, commandLine);
  }

  void GetRevisions(Status& status, StringParam sourcePath, Array<Revision>& revisions) override
  {
  }
};

SourceControl* GetMercurial()
{
  static Mercurial mercurial;
  return &mercurial;
}

}
