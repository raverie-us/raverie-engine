///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Utility/Status.hpp"
#include "SourceControl.hpp"
#include "String/StringBuilder.hpp"

namespace Zero
{

class Svn : public SourceControl
{
public:

  void Add(Status& status, StringParam filePath) override
  {
    // svn add, parents to add parent directories
    String filePathSafe = BuildString(" \"", filePath, "\"");
    String commandLine = BuildString("svn add ", filePathSafe, " --parents");
    RunSimpleCommandLine(status, commandLine);
  }

  void Remove(Status& status, StringParam filePath) override
  {
    // svn remove the file, force because the file may have local changes
    String filePathSafe = BuildString(" \"", filePath, "\"");
    String commandLine = BuildString("svn remove ", filePathSafe, " --force");
    RunSimpleCommandLine(status, commandLine);
  }

  void Rename(Status& status, StringParam sourcePath, StringParam destPath) override
  {
    // svn move
    String source = BuildString(" \"", sourcePath, "\"");
    String dest = BuildString(" \"", destPath, "\"");
    String commandLine = BuildString("svn move ", source, dest);
    RunSimpleCommandLine(status, commandLine);
  }

  void GetRevisions(Status& status, StringParam path, Array<Revision>& revisions) override
  {
  }
};

SourceControl* GetSvn()
{
  static Svn svn;
  return &svn;
}

}
