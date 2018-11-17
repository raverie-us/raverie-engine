///////////////////////////////////////////////////////////////////////////////
///
/// \file SourceControl.cpp
///  Source Control Interface
/// 
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(Revision, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
  ZilchBindFieldProperty(ChangeSet);
  ZilchBindFieldProperty(User);
  ZilchBindFieldProperty(Date);
  ZilchBindFieldProperty(Summary);
}

class FileSourceControl : public SourceControl
{
  void Add(Status& status, StringParam filePath) override
  {
    //nothing to do
  }

  void Remove(Status& status, StringParam filePath) override
  {
    DeleteFile(filePath);
  }

  void Rename(Status& status, StringParam sourcePath, StringParam destPath) override
  {
    MoveFile(destPath, sourcePath);
  }

  void GetRevisions(Status& status, StringParam path, Array<Revision>& revisions) override
  {
    status.SetFailed("Not supported");
  }
};

SourceControl* GetMercurial();
SourceControl* GetSvn();

SourceControl* GetSourceControl(StringParam sourceControlType)
{
  static FileSourceControl fileSourceControl;

  if(sourceControlType == "Mercurial")
    return GetMercurial();
  else if(sourceControlType == "Svn")
    return GetSvn();
  else
    return &fileSourceControl;
}

int RunSimpleCommandLine(Status& status, StringParam commandLine)
{
  TextStreamBuffer buffer;
  SimpleProcess process;
  process.ExecProcess("CommandLine", commandLine.c_str(), &buffer);
  int returnCode = process.WaitForClose();
  if(returnCode!=0)
    status.SetFailed(buffer.ToString());
  return returnCode;
}

}
